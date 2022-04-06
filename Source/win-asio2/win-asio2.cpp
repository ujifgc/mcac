#define WIN32_MEAN_AND_LEAN
#include <windows.h>
#include <mmdeviceapi.h>
#include <audioclient.h>
#include <propsys.h>

#include <obs-module.h>
#include <util/util.hpp>
#include <util/base.h>
#include <util/platform.h>
#include <util/windows/HRError.hpp>
#include <util/windows/ComPtr.hpp>
#include <util/windows/WinHandle.hpp>
#include <util/windows/CoTaskMemPtr.hpp>
#include <util/threading.h>

#include <vector>
#include <string>

#include "asio/iasiodrv.h"
#include "asio/asiolist.h"

struct AudioDeviceInfo {
	std::string name;
	std::string id;
};

using namespace std;

#define OPT_DEVICE_ID "device_id"
#define OPT_USE_DEVICE_TIMING "use_device_timing"
#define MAX_ASIO_INPUT_CHANNELS 32
#define MAX_ASIO_OUTPUT_CHANNELS 32

const double twoRaisedTo32 = 4294967296.0;
#define ASIO64toUint64(a)  ((uint64_t)((a).lo) + ((uint64_t)((a).hi) << 32))

class WinASIO2Source {
	ComPtr<IASIO>               device;
	
	obs_source_t                *obsSource;
	string                      device_id;
	string                      device_name;
	bool                        useDeviceTiming = false;

	bool                        reconnecting = false;
	bool                        previouslyFailed = false;
	WinHandle                   reconnectThread;
	WinHandle                   captureThread;

	bool                        isASIOActive = false;

	WinHandle                   stopSignal;
	WinHandle                   receiveSignal;

	float *alignmentBuffer = nullptr;
	uint32_t alignmentBufferSampleCount;
	uint32_t writeSampleIndex;
	uint32_t readSampleIndex;

	uint64_t timestampBuffer[480];
	uint32_t timestampBufferIndex;

	CRITICAL_SECTION bufferSection;

	uint32_t obsSampleRate = 0;
	speaker_layout obsSpeakers = SPEAKERS_MONO;
	uint32_t obsOutputSampleRate = 48000;
	uint32_t obsSamplesPer10ms = 480;

	ASIOSampleType asioSampleType;

	// ASIO Info
	//ASIOChannelInfo channelInfos[MAX_ASIO_INPUT_CHANNELS + MAX_ASIO_OUTPUT_CHANNELS];

	ASIOTimeStamp nanoSeconds;
	ASIOSamples samples;
	ASIOSamples tcSamples;

	// ASIOGetBufferSize()
	long           asioBufferSampleCount;

	// ASIOGetSampleRate()
	ASIOSampleRate sampleRate;

	// ASIOGetChannels()
	long           inputChannels;
	long           outputChannels;

	// ASIOGetLatencies ()
	long           inputLatency;
	long           outputLatency;

	// ASIOCreateBuffers()
	ASIOBufferInfo bufferInfos[MAX_ASIO_INPUT_CHANNELS + MAX_ASIO_OUTPUT_CHANNELS];

	bool           postOutput;

	ASIOCallbacks asioCallbacks;

	static ASIOTime * bufferSwitchTimeInfo(ASIOTime *timeInfo, long index, ASIOBool processNow);
	static void bufferSwitch(long index, ASIOBool processNow);
	static void sampleRateChanged(ASIOSampleRate sRate);
	static long asioMessages(long selector, long value, void* message, double* opt);

	static DWORD WINAPI ReconnectThread(void *data);
	static DWORD WINAPI CaptureThread(void *data);
	bool ReadCapturedData();
	uint64_t ApproximateTimestamp(int sampleIndex);

	void Reconnect();
	void Initialize();
	void Uninitialize();
	bool TryInitialize();
	void UpdateSettings(obs_data_t *settings);
public:
	WinASIO2Source(obs_data_t *settings, obs_source_t *source_);
	inline ~WinASIO2Source();
	inline void asioSourceStart();
	inline void asioSourceStop();
	void Update(obs_data_t *settings);
};

WinASIO2Source *theAsioSource = nullptr;

WinASIO2Source::WinASIO2Source(obs_data_t *settings, obs_source_t *source) {
	obsSource = source;
	::CoInitialize(nullptr);

	obsOutputSampleRate = audio_output_get_sample_rate(obs_get_audio());
	obsSamplesPer10ms = obsOutputSampleRate / 100;

	stopSignal = CreateEvent(nullptr, true, false, nullptr);
	receiveSignal = CreateEvent(nullptr, false, false, nullptr);
	UpdateSettings(settings);

	InitializeCriticalSection(&bufferSection);
}

inline WinASIO2Source::~WinASIO2Source() {
	if (isASIOActive) asioSourceStop();
	DeleteCriticalSection(&bufferSection);
}

inline void WinASIO2Source::asioSourceStart() {
	if (isASIOActive) {
		blog(LOG_INFO, "[WinASIO2Source::Start] ASIO source '%s' is already active", device_id.c_str());
		return;
	}
	if (!TryInitialize()) {
		blog(LOG_INFO, "[WinASIO2Source::Start] Device '%s' not found.  Waiting for device", device_id.c_str());
		Reconnect();
	}
}

void WinASIO2Source::asioSourceStop() {
	if (!isASIOActive) {
		blog(LOG_INFO, "[WinASIO2Source::Start] ASIO source '%s' is already stopped", device_id.c_str());
		return;
	}
	Uninitialize();
}

void WinASIO2Source::Update(obs_data_t *settings) {
	string newDevice = obs_data_get_string(settings, OPT_DEVICE_ID);
	bool restart = newDevice.compare(device_id) != 0;

	if (restart)
		asioSourceStop();

	UpdateSettings(settings);

	if (restart)
		asioSourceStart();
}

void WinASIO2Source::UpdateSettings(obs_data_t *settings)
{
	device_id = obs_data_get_string(settings, OPT_DEVICE_ID);
	useDeviceTiming = obs_data_get_bool(settings, OPT_USE_DEVICE_TIMING);
}

void WinASIO2Source::Reconnect()
{
	reconnecting = true;
	reconnectThread = CreateThread(nullptr, 0, WinASIO2Source::ReconnectThread, this, 0, nullptr);

	if (!reconnectThread.Valid())
		blog(LOG_WARNING, "[WinASIO2Source::Reconnect] Failed to initialize reconnect thread: %lu", GetLastError());
}

#define RECONNECT_INTERVAL 3000

DWORD WINAPI WinASIO2Source::ReconnectThread(void *data)
{
	WinASIO2Source *source = static_cast<WinASIO2Source*>(data);

	os_set_thread_name("WinASIO2Source: reconnect thread");

	while (WAIT_TIMEOUT == WaitForSingleObject(source->stopSignal, RECONNECT_INTERVAL)) {
		if (source->TryInitialize()) break;
	}

	source->reconnectThread = 0;
	source->reconnecting = false;
	return 0;
}

bool WinASIO2Source::TryInitialize()
{
	try {
		Initialize();
	}
	catch (HRError error) {
		if (previouslyFailed)
			return isASIOActive;

		blog(LOG_WARNING, "[WinASIO2Source::TryInitialize]:[%s] %s: %lX",
			device_name.empty() ? device_id.c_str() : device_name.c_str(),
			error.str, error.hr);
	}
	catch (const char *error) {
		if (previouslyFailed)
			return isASIOActive;

		blog(LOG_WARNING, "[WinASIO2Source::TryInitialize]:[%s] %s",
			device_name.empty() ? device_id.c_str() : device_name.c_str(),
			error);
	}

	previouslyFailed = !isASIOActive;
	return isASIOActive;
}

inline uint32_t littleEndianInt(const void* const bytes) noexcept { return *static_cast<const uint32_t*> (bytes); }

static inline void convertInt32ToFloat(const char* src, float* dest, int numSamples) noexcept
{
	const double g = 1.0 / 0x7fffffff;

	while (--numSamples >= 0)
	{
		*dest++ = (float)(g * (int)littleEndianInt(src));
		src += 4;
	}
}

DWORD switchCounter = 0;

ASIOTime * WinASIO2Source::bufferSwitchTimeInfo(ASIOTime *timeInfo, long index, ASIOBool processNow)
{
	WinASIO2Source *asioSource = theAsioSource;
	
	if (asioSource == nullptr) return nullptr;
	if (processNow != ASIOTrue) return nullptr;

	uint64_t timestampAtCallback = asioSource->useDeviceTiming ? 0 : os_gettime_ns();

	EnterCriticalSection(&asioSource->bufferSection);

	asioSource->timestampBuffer[asioSource->timestampBufferIndex] = asioSource->useDeviceTiming ?
		ASIO64toUint64(timeInfo->timeInfo.systemTime) :
		(timestampAtCallback - ((uint64_t)asioSource->asioBufferSampleCount * 1000000000ULL / (uint64_t)asioSource->obsSampleRate));
	asioSource->timestampBufferIndex += 1;
	if (asioSource->timestampBufferIndex >= asioSource->obsSamplesPer10ms)
		asioSource->timestampBufferIndex = 0;

	if (asioSource->asioSampleType == ASIOSTInt32LSB) {
		convertInt32ToFloat((const char *)asioSource->bufferInfos[0].buffers[index], asioSource->alignmentBuffer + asioSource->writeSampleIndex, asioSource->asioBufferSampleCount);
	} else {
		memcpy(asioSource->alignmentBuffer + asioSource->writeSampleIndex, asioSource->bufferInfos[0].buffers[index], asioSource->asioBufferSampleCount * sizeof(float));
	}
	asioSource->writeSampleIndex += asioSource->asioBufferSampleCount;
	if (asioSource->writeSampleIndex >= asioSource->alignmentBufferSampleCount) asioSource->writeSampleIndex = 0;

	LeaveCriticalSection(&asioSource->bufferSection);

    SetEvent(asioSource->receiveSignal);

	if (asioSource->postOutput)
		ASIOOutputReady();

	return nullptr;
}

uint64_t WinASIO2Source::ApproximateTimestamp(int sampleIndex) {
	double timestampApproximateIndex = (double)(sampleIndex) / asioBufferSampleCount;
	int floorIndex = (int)std::floor(timestampApproximateIndex);
	int ceilIndex = (int)std::ceil(timestampApproximateIndex);
	uint64_t leftTS = timestampBuffer[floorIndex];
	uint64_t rightTS = timestampBuffer[ceilIndex];
	return (uint64_t)(leftTS + (rightTS - leftTS) * (timestampApproximateIndex - floorIndex));
}

bool WinASIO2Source::ReadCapturedData() {
	obs_source_audio data = { 0 };
	data.frames = obsSamplesPer10ms;
	data.speakers = obsSpeakers;
	data.samples_per_sec = obsSampleRate;
	data.format = AUDIO_FORMAT_FLOAT;

	EnterCriticalSection(&bufferSection);

	while (readSampleIndex + obsSamplesPer10ms <= writeSampleIndex || (writeSampleIndex == 0 && readSampleIndex > 0)) {
		data.data[0] = (const uint8_t*)(alignmentBuffer + readSampleIndex);
		data.timestamp = ApproximateTimestamp(readSampleIndex);

		obs_source_output_audio(obsSource, &data);

		readSampleIndex += obsSamplesPer10ms;
		if (readSampleIndex >= alignmentBufferSampleCount) readSampleIndex = 0;
	}

	LeaveCriticalSection(&bufferSection);

	return true;
}

#define CAPTURE_INTERVAL INFINITE

DWORD WINAPI WinASIO2Source::CaptureThread(void *data)
{
	WinASIO2Source *source = static_cast<WinASIO2Source*>(data);

	os_set_thread_name("WinASIO2Source: audio capture thread");

	HANDLE signals[2] = { source->receiveSignal, source->stopSignal };

	while (true) {
		int waitResult = WaitForMultipleObjects(2, signals, false, CAPTURE_INTERVAL);
		if (waitResult == WAIT_OBJECT_0 || waitResult == WAIT_TIMEOUT) {
			source->ReadCapturedData();
		} else if (waitResult == WAIT_OBJECT_0 + 1) {
			break;
		} else {
			blog(LOG_ERROR, "[WinASIO2Source::CaptureThread] Abnormal termination of '%s' capture thread", source->device_name.c_str());
			break;
		}
	}

	return 0;
}

void WinASIO2Source::bufferSwitch(long index, ASIOBool processNow)
{
	ASIOTime timeInfo = { 0 };

	if (ASIOGetSamplePosition(&timeInfo.timeInfo.samplePosition, &timeInfo.timeInfo.systemTime) == ASE_OK)
		timeInfo.timeInfo.flags = kSystemTimeValid | kSamplePositionValid;

	bufferSwitchTimeInfo(&timeInfo, index, processNow);
}

void WinASIO2Source::sampleRateChanged(ASIOSampleRate sRate)
{
	blog(LOG_INFO, "WinASIO2Source: Sample rate changed to '%f' (not implemented)", sRate);
}

long WinASIO2Source::asioMessages(long selector, long value, void* message, double* opt)
{
	WinASIO2Source *asioSource = theAsioSource;
	long ret = 0;
	switch (selector)
	{
	case kAsioSelectorSupported:
		if (value == kAsioResetRequest
			|| value == kAsioEngineVersion
			|| value == kAsioResyncRequest
			|| value == kAsioLatenciesChanged
			// the following three were added for ASIO 2.0, you don't necessarily have to support them
			|| value == kAsioSupportsTimeInfo
			|| value == kAsioSupportsTimeCode
			|| value == kAsioSupportsInputMonitor)
			ret = 1L;
		break;
	case kAsioResetRequest:
		if (asioSource) {
			asioSource->asioSourceStop();
			asioSource->asioSourceStart();
		}
		ret = 1L;
		break;
	case kAsioResyncRequest:
		ret = 1L;
		break;
	case kAsioLatenciesChanged:
		ret = 1L;
		break;
	case kAsioEngineVersion:
		ret = 2L;
		break;
	case kAsioSupportsTimeInfo:
		ret = 1L;
		break;
	case kAsioSupportsTimeCode:
		ret = 0L;
		break;
	}
	return ret;
}

void WinASIO2Source::Initialize()
{
	HRESULT res;

	wstring wdevid = wstring(device_id.begin(), device_id.end());
	CLSID clsid = { 0 };
	res = CLSIDFromString(wdevid.c_str(), &clsid);

	::CoInitialize(0);
	res = ::CoCreateInstance(clsid, nullptr, CLSCTX_INPROC_SERVER, clsid, (void**)device.Assign());
	if (FAILED(res)) throw HRError("Failed to create asio", res);

	ASIOSetDriver(device);

	ASIODriverInfo driverInfo = { 0 };

	res = ASIOInit(&driverInfo);
	if (res != ASE_OK) throw HRError("Failed to init asio device", res);
	device_name = driverInfo.name;

	res = ASIOGetChannels(&inputChannels, &outputChannels);
	if (res != ASE_OK) throw HRError("Failed to get channels info", res);

	long minSize, maxSize, granularity;
	res = ASIOGetBufferSize(&minSize, &maxSize, &asioBufferSampleCount, &granularity);
	if (res != ASE_OK) throw HRError("Failed to get buffer size", res);

	res = ASIOGetSampleRate(&sampleRate);
	if (res != ASE_OK) throw HRError("Failed to get sample rate", res);
	obsSampleRate = (uint32_t)sampleRate;

	postOutput = (ASIOOutputReady() == ASE_OK);
	
	memset(bufferInfos, 0, sizeof(bufferInfos));
	ASIOBufferInfo *infop = bufferInfos;
	for (int i = 0; i < inputChannels; i++, infop++) {
		infop->isInput = ASIOTrue;
		infop->channelNum = i;
	}
	for (int i = 0; i < outputChannels; i++, infop++) {
		infop->isInput = ASIOFalse;
		infop->channelNum = i;
	}

	asioCallbacks.bufferSwitch = &bufferSwitch;
	asioCallbacks.sampleRateDidChange = &sampleRateChanged;
	asioCallbacks.asioMessage = &asioMessages;
	asioCallbacks.bufferSwitchTimeInfo = &bufferSwitchTimeInfo;

	res = ASIOCreateBuffers(bufferInfos, inputChannels + outputChannels, asioBufferSampleCount, &asioCallbacks);
	if (res != ASE_OK) throw HRError("Failed to create buffers", res);

	ASIOChannelInfo channelInfo = { 0 };
	channelInfo.channel = bufferInfos[0].channelNum;
	channelInfo.isInput = bufferInfos[0].isInput;
	res = ASIOGetChannelInfo(&channelInfo);
	if (res != ASE_OK)
		throw HRError("Failed to get channel infos", res);
	if (channelInfo.type != ASIOSTFloat32LSB && channelInfo.type != ASIOSTInt32LSB)
		throw HRError("Unsupported sample type", res);
	asioSampleType = channelInfo.type;

	res = ASIOGetLatencies(&inputLatency, &outputLatency);
	if (res != ASE_OK) throw HRError("Failed to get latencies", res);

	alignmentBufferSampleCount = asioBufferSampleCount * obsSamplesPer10ms;
	alignmentBuffer = (float*)brealloc(alignmentBuffer, alignmentBufferSampleCount * sizeof(float));
	if (!alignmentBuffer) throw HRError("Failed to allocate alignbent buffer", -1);
	readSampleIndex = writeSampleIndex = 0;
	
	memset(timestampBuffer, 0, sizeof(timestampBuffer));
	timestampBufferIndex = 0;
	
	res = ASIOStart();
	if (res != ASE_OK) throw HRError("Failed to start ASIO", res);
	
	isASIOActive = true;

	captureThread = CreateThread(nullptr, 0, WinASIO2Source::CaptureThread, this, 0, nullptr);

	blog(LOG_INFO, "[WinASIO2Source] Device '%s' initialized", device_name.c_str());
}

void WinASIO2Source::Uninitialize() {
	SetEvent(stopSignal);

	if (captureThread.Valid()) {
		WaitForSingleObject(captureThread, INFINITE);
		captureThread = 0;
	}

	if (reconnectThread.Valid()) {
		WaitForSingleObject(reconnectThread, INFINITE);
		reconnectThread = 0;
	}

	isASIOActive = false;

	device.Release();

	bfree(alignmentBuffer);
	alignmentBuffer = nullptr;
	
	ResetEvent(stopSignal);

	blog(LOG_INFO, "[WinASIO2Source] Device '%s' Terminated", device_name.c_str());
}

static const char *GetWinASIO2InputName(void*)
{
	return obs_module_text("AudioInputASIO2");
}

static void *CreateWinASIO2Input(obs_data_t *settings, obs_source_t *source)
{
	if (theAsioSource) {
		blog(LOG_ERROR, "[CreateWinASIO2Input] Only one active ASIO source allowed");
		return nullptr;
	}

	try {
		theAsioSource = new WinASIO2Source(settings, source);
		return theAsioSource;
	}
	catch (const char *error) {
		blog(LOG_ERROR, "[CreateWinASIO2Input] %s", error);
	}

	return nullptr;
}

static void DestroyWinASIO2Source(void *obj)
{
	if (obj) {
		delete static_cast<WinASIO2Source*>(obj);
		theAsioSource = nullptr;
	}
}

static void UpdateWinASIO2Source(void *obj, obs_data_t *settings)
{
	if (obj) static_cast<WinASIO2Source*>(obj)->Update(settings);
}

#define ASIO_TAG "[ASIO2] "

void GetWinASIO2AudioDevices(vector<AudioDeviceInfo> &devices)
{
	devices.clear();

	try {
		AsioDriverList list;
		LONG asio_driver_number = list.asioGetNumDev();
		
		for (int i = 0; i < asio_driver_number; i++) {
			char driver_name[MAXDRVNAMELEN + sizeof(ASIO_TAG)] = ASIO_TAG;
			list.asioGetDriverName(i, driver_name + sizeof(ASIO_TAG) - 1, MAXDRVNAMELEN);

			CLSID clsid;
			list.asioGetDriverCLSID(i, &clsid);
			char guid_string[39]; // 32 hex chars + 4 hyphens + 2 brackets + null terminator
			snprintf(
				guid_string, sizeof(guid_string) / sizeof(guid_string[0]),
				"{%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x}",
				clsid.Data1, clsid.Data2, clsid.Data3,
				clsid.Data4[0], clsid.Data4[1], clsid.Data4[2],
				clsid.Data4[3], clsid.Data4[4], clsid.Data4[5],
				clsid.Data4[6], clsid.Data4[7]);

			AudioDeviceInfo info = { driver_name, guid_string };
			devices.push_back(info);
		}
	}
	catch (HRError error) {
		blog(LOG_WARNING, "[GetWinASIO2AudioDevices] %s: %lX",
			error.str, error.hr);
	}
}

static obs_properties_t *GetWinASIO2PropertiesInput(void *data)
{
	if (!data) return nullptr;

	obs_properties_t *props = obs_properties_create();
	vector<AudioDeviceInfo> devices;

	obs_property_t *device_prop = obs_properties_add_list(props,
		OPT_DEVICE_ID, obs_module_text("Device"),
		OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_STRING);

	GetWinASIO2AudioDevices(devices);

	for (size_t i = 0; i < devices.size(); i++) {
		AudioDeviceInfo &device = devices[i];
		obs_property_list_add_string(device_prop,
			device.name.c_str(), device.id.c_str());
	}

	//WinASIO2Source *source = static_cast<WinASIO2Source*>(data);
	obs_properties_add_bool(props, OPT_USE_DEVICE_TIMING, obs_module_text("UseDeviceTiming"));

	return props;
}

void HideSource(void *data) {
	if (data) static_cast<WinASIO2Source*>(data)->asioSourceStop();
}

void ShowSource(void *data) {
	if (data) static_cast<WinASIO2Source*>(data)->asioSourceStart();
}

void RegisterWinASIO2Input() {
	obs_source_info info = { 0 };

	info.id              = "winasio2_input_capture";
	info.type            = OBS_SOURCE_TYPE_INPUT;
	info.output_flags    = OBS_SOURCE_AUDIO | OBS_SOURCE_DO_NOT_DUPLICATE;
	info.get_name        = GetWinASIO2InputName;
	info.create          = CreateWinASIO2Input;
	info.destroy         = DestroyWinASIO2Source;
	info.update          = UpdateWinASIO2Source;
	info.get_properties  = GetWinASIO2PropertiesInput;
	info.hide            = HideSource;
	info.show            = ShowSource;

	obs_register_source(&info);
}
