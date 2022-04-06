#include "core.h"
#include "asio_api.h"
#include "device.h"
#include "device_manager.h"
#include "asio_callbacks_wrapper.h"
#include "writer.h"
#include "MainComponent.h"

extern HWND message_window;
extern DeviceManager* device_manager;

void AsioDevice::refuse(long error, const char *message = "") {
	if (error) {
		if (is_open) close();
		if (message[0] == 0 && driver != nullptr) {
			char driver_error_message[256];
			driver->getErrorMessage(driver_error_message);
			throw driver_error_message;
		}
		else {
			throw message;
		}
	}
}

AsioDevice::AsioDevice(IASIO *_driver, int _index) {
	plan = dsNone;
	status = dsNone;
	output_samples_per_10ms = int(default_sample_rate / 100);
	driver = _driver;
	index = _index;

	driver_version = driver->getDriverVersion();
	driver->getDriverName(driver_name);
	name = String(driver_name);

	stop_signal = CreateEvent(nullptr, true, false, nullptr);
	receive_signal = CreateEvent(nullptr, false, false, nullptr);
	InitializeCriticalSection(&buffer_section);
	InitializeCriticalSection(&switch_section);
}

AsioDevice::~AsioDevice() {
	uninit();

	DeleteCriticalSection(&switch_section);
	DeleteCriticalSection(&buffer_section);
}

bool AsioDevice::switch_status() {
	if (status == plan) return true;

	EnterCriticalSection(&switch_section);

	try {
		switch (plan) {
		case dsNone:
			uninit();
			PostMessage(message_window, WM_USER_INPUT_STATUS, index, 0);
			break;
		case dsOpen:
			if (!is_initialized) init();
			if (!is_open) open();
			mlog(name, "MCAC: status set to open", llDebug);
			PostMessage(message_window, WM_USER_INPUT_STATUS, index, 0);
			break;
		}
	}
	catch (const char* error) {
		if (device_manager) {
			device_manager->device_errors[index] = error;
			PostMessage(message_window, WM_USER_INPUT_ERROR, index, 0);
			uninit();
			Sleep(1000); //FIXME
			PostMessage(message_window, WM_USER_RESTART, index, 0);
		}
	}

	LeaveCriticalSection(&switch_section);

	return (status == plan);
}

void AsioDevice::set_status(enum DeviceStatus new_status) {
	plan = new_status;
	switch_status();
}

void AsioDevice::init() {
	if (is_initialized) return;

	int success = 0, error = 0;

	success = driver->init(message_window);
	refuse(!success);

	driver_version = driver->getDriverVersion();
	driver->getDriverName(driver_name);
	name = String(driver_name);

	error = driver->getChannels(&input_channels_number, &output_channels_number);
	refuse(error);

	error = driver->getBufferSize(&min_buffer_size, &max_buffer_size, &preferred_buffer_size, &buffer_size_granularity);
	refuse(error);

	ASIOChannelInfo info = { 0 };
	info.isInput = 1;
	error = driver->getChannelInfo(&info);
	refuse(error);
	sample_type = info.type;
	refuse(sample_type != ASIOSTFloat32LSB && sample_type != ASIOSTInt32LSB, "MCAC: Unsupported sample type");

	initialize_sample_rates();
	initialize_channel_names();

	output_ready_available = !!driver->outputReady();

	is_initialized = true;
	status = dsInitialized;
}

void AsioDevice::uninit() {
	if (is_open) close();

	is_initialized = false;
	status = dsNone;
}

void AsioDevice::open() {
	int error = 0;

	if (is_open) return;

	refuse(!is_initialized, "MCAC: Driver is not initialized");

	ASIOBufferInfo* infop = buffer_infos;
	for (int i = 0; i < input_channels_number; i += 1, infop++) {
		infop->isInput = ASIOTrue;
		infop->channelNum = i;
	}
	for (int i = 0; i < output_channels_number; i += 1, infop++) {
		infop->isInput = ASIOFalse;
		infop->channelNum = i;
	}

	error = driver->createBuffers(buffer_infos, input_channels_number + output_channels_number, preferred_buffer_size, &callbacks[index]);
	refuse(error);

	for (int i = 0; i < input_channels_number; i += 1) {
		sample_type_buffers[i] = (float*)calloc(preferred_buffer_size, sizeof(float));
	}
	circlebuf_init(&input_buffer);
	circlebuf_init(&interleaved_buffer);
	driver_buffers_allocated = true;

	initialize_latencies();

	EnterCriticalSection(&writer_section);
	if (writer) {
		writer->uninit();
		delete writer;
		writer = nullptr;
	}

	writer = new Writer(this);
	writer->init(sample_rate);
	LeaveCriticalSection(&writer_section);

	capture_thread = CreateThread(nullptr, 0, CaptureThread, this, 0, nullptr);

	error = driver->start();
	refuse(error);

	is_open = true;
	status = dsOpen;
}

void AsioDevice::close() {
	if (is_open) driver->stop();
	is_open = false;

	SetEvent(stop_signal);

	if (capture_thread.Valid()) {
		WaitForSingleObject(capture_thread, INFINITE);
		capture_thread = NULL;
	}

	EnterCriticalSection(&writer_section);
	if (writer) {
		writer->uninit();
		delete writer;
		writer = nullptr;
	}
	LeaveCriticalSection(&writer_section);

	if (driver_buffers_allocated) {
		driver->disposeBuffers();

		for (int i = 0; i < input_channels_number; i += 1) {
			if (sample_type_buffers[i]) {
				free(sample_type_buffers[i]);
				sample_type_buffers[i] = NULL;
			}
		}

		circlebuf_free(&input_buffer);
		circlebuf_free(&interleaved_buffer);

		driver_buffers_allocated = false;
	}

	status = dsInitialized;

	ResetEvent(stop_signal);
}

void AsioDevice::initialize_latencies() {
	int error = 0;
	
	error = driver->getLatencies(&input_latency, &output_latency);
	refuse(error);
}

void AsioDevice::initialize_channel_names() {
	int i = 0, error = 0;

	for (i = 0; i < input_channels_number; i += 1) {
		ASIOChannelInfo info = { 0 };
		info.channel = i;
		info.isInput = 1;
		error = driver->getChannelInfo(&info);
		refuse(error);
		input_channel_names.set(i, info.name);
	}
	input_channel_names.appendNumbersToDuplicates(false, true);

	for (i = 0; i < output_channels_number; i += 1) {
		ASIOChannelInfo info = { 0 };
		info.channel = i;
		info.isInput = 0;
		error = driver->getChannelInfo(&info);
		refuse(error);
		output_channel_names.set(i, info.name);
	}
	output_channel_names.appendNumbersToDuplicates(false, true);
}

void AsioDevice::initialize_sample_rates() {
	int error = 0;

	for (double testing_sample_rate : standard_sample_rates) {
		if (0 == driver->canSampleRate(testing_sample_rate)) {
			supported_sample_rates.add(testing_sample_rate);
		}
	}

	error = driver->getSampleRate(&sample_rate);
	refuse(error);

	if (sample_rate < supported_sample_rates.getFirst()) {
		set_sample_rate(default_sample_rate);
	}
	else {
		supported_sample_rates.addIfNotAlreadyThere(sample_rate);
	}

	supported_sample_rates.sort();
}

void AsioDevice::set_sample_rate(double new_sample_rate) {
	int error = 0;

	error = driver->setSampleRate(new_sample_rate);
	refuse(error);

	error = driver->getSampleRate(&sample_rate);
	refuse(error);

	refuse(sample_rate != new_sample_rate, "MCAC: Device failed to set sample rate");
}

DWORD WINAPI AsioDevice::CaptureThread(void* data) {
	AsioDevice* device = static_cast<AsioDevice*>(data);
	if (!device) throw "MCAC: Thread could not find parent device";

	HANDLE signals[2] = { device->receive_signal, device->stop_signal };

	while (true) {
		int wait_result = WaitForMultipleObjects(2, signals, false, INFINITE);
		if (wait_result == WAIT_OBJECT_0) {
			EnterCriticalSection(&device->buffer_section);
			if (writer) writer->write_packet(&device->input_buffer);
			LeaveCriticalSection(&device->buffer_section);
		}
		else if (wait_result == WAIT_OBJECT_0 + 1) {
			break;
		}
		else {
			throw "MCAC: Abnormal termination of capture thread";
		}
	}

	return 0;
}

long AsioDevice::message(int device_index, long selector, long value, void*, double*) {
	if (!device_manager) return 0;
	AsioDevice* device = device_manager->get_instance(device_index);
	if (!device) throw "MCAC: Callback could not find current device";

	switch (selector) {
	case kAsioSelectorSupported:
		if (value == kAsioEngineVersion
			|| value == kAsioResetRequest
			|| value == kAsioBufferSizeChange
			|| value == kAsioResyncRequest
			|| value == kAsioLatenciesChanged
			|| value == kAsioSupportsTimeInfo
			|| value == kAsioSupportsTimeCode
			|| value == kAsioSupportsInputMonitor
			|| value == kAsioOverload)
			return 1;
		return 0;
	case kAsioEngineVersion:
		return 2;
	case kAsioResetRequest:
		PostMessage(message_window, WM_USER_RESET_REQUEST, device_index, 0);
		return 1;
	case kAsioBufferSizeChange:
		device->reinit();
		return 1;
	case kAsioResyncRequest:
		device->reopen();
		return 1;
	case kAsioLatenciesChanged:
		device->reopen();
		return 1;
	case kAsioSupportsTimeInfo:
		return 1;
	case kAsioSupportsTimeCode:
		return 0;
	case kAsioOverload:
		mlog(device->name, "driver reported overload", llWarn);
		return 1;
	}
	return 0;
}

void AsioDevice::sample_rate_changed(int device_index, ASIOSampleRate) {
	if (!device_manager) return;
	AsioDevice* device = device_manager->get_instance(device_index);
	if (!device) throw "MCAC: Callback could not find current device";

	device->reinit();
}

void AsioDevice::buffer_switch(int device_index, long index, ASIOBool processNow) {
	if (!device_manager) return;
	AsioDevice* device = device_manager->get_instance(device_index);
	if (!device) throw "MCAC: Callback could not find current device";

	int error = 0;
	ASIOTime timeInfo = { 0 };

	error = device->driver->getSamplePosition(&timeInfo.timeInfo.samplePosition, &timeInfo.timeInfo.systemTime);
	device->refuse(error);

	timeInfo.timeInfo.flags = kSystemTimeValid | kSamplePositionValid;
	buffer_switch_time_info(device_index, &timeInfo, index, processNow);
}

ASIOTime* AsioDevice::buffer_switch_time_info(int device_index, ASIOTime* timeInfo, long index, ASIOBool processNow) {
	if (!device_manager) return nullptr;
	AsioDevice* device = device_manager->get_instance(device_index);
	if (!device) throw "MCAC: Callback could not find current device";

	//if (processNow != ASIOTrue) return nullptr;

	EnterCriticalSection(&device->buffer_section);

	device->push_received_buffers(index);

	SetEvent(device->receive_signal);

	LeaveCriticalSection(&device->buffer_section);

	if (device->output_ready_available) device->driver->outputReady();

	return nullptr;

	UNREFERENCED_PARAMETER(timeInfo);
	UNREFERENCED_PARAMETER(processNow);
}

static inline void* fill_interleaved_buffer(size_t channels, size_t samples, float** fdata, struct circlebuf* interleaved_buffer) {
	circlebuf_upsize(interleaved_buffer, channels * samples * BYTES_PER_SAMPLE);
	float* buffer = (float*)circlebuf_data(interleaved_buffer, 0);
	extern float last_buffer_magnitude[MAX_INPUT_CHANNELS];

	for (size_t c = 0; c < channels; c++) {
		if (fdata[c]) {
			float sum_of_squares = 0.0f;
			for (size_t i = 0; i < samples; i++) {
				float sample = buffer[i * channels + c] = fdata[c][i];
				sum_of_squares += sample * sample;
			}
			last_buffer_magnitude[c] = sqrtf(sum_of_squares / samples);
		}
		else {
			for (size_t i = 0; i < samples; i++) {
				buffer[i * channels + c] = 0.0f;
			}
			last_buffer_magnitude[c] = 0.0f;
		}
	}
	for (size_t c = channels; c < MAX_INPUT_CHANNELS; c++) {
		last_buffer_magnitude[c] = 0.0f;
	}

	return buffer;
}

static ULONGLONG last_time = 0;

void AsioDevice::push_received_buffers(int buffer_index) {
	int skipped = 0;
	for (int channel = 0; channel < input_channels_number; channel += 1) {
		if (active_channels[channel]) {
			if (sample_type == ASIOSTInt32LSB) {
				convertInt32ToFloat((const char*)buffer_infos[channel].buffers[buffer_index], sample_type_buffers[channel - skipped], preferred_buffer_size);
			}
			else {
				memcpy(sample_type_buffers[channel - skipped], buffer_infos[channel].buffers[buffer_index], preferred_buffer_size * BYTES_PER_SAMPLE);
			}
		}
		else {
			skipped += 1;
		}
	}

	fill_interleaved_buffer(active_channels_count, preferred_buffer_size, sample_type_buffers, &interleaved_buffer);
	circlebuf_push_back(&input_buffer, circlebuf_data(&interleaved_buffer, 0), active_channels_count * preferred_buffer_size * BYTES_PER_SAMPLE);

	extern float last_buffer_magnitude[MAX_INPUT_CHANNELS];
	extern float buffer_magnitude[MAX_INPUT_CHANNELS];

	for (int channel = 0; channel < input_channels_number; channel += 1) {
		if (buffer_magnitude[channel] < last_buffer_magnitude[channel]) {
			buffer_magnitude[channel] = last_buffer_magnitude[channel];
		}
		else {
			buffer_magnitude[channel] *= 0.99f;
		}
	}

	ULONGLONG time = GetTickCount64();
	if (time > last_time + 3) {
		main_component->triggerAsyncUpdate();
		last_time = time;
	}
}
