#include "core.h"
#include "device.h"
#include "device_manager.h"
#include "asio_callbacks_wrapper.h"
#include "writer.h"

extern HWND message_window;
extern DeviceManager* device_manager;
extern class Writer* writer;
extern CRITICAL_SECTION writer_section;
extern bool active_channels[MAX_INPUT_CHANNELS];

void AsioDevice::refuse(long error, const char *message = "") {
	if (error) {
		if (status >= DeviceStatus::Open) close();
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
	driver = _driver;
	index = _index;

	requested_status = DeviceStatus::None;
	status = DeviceStatus::None;

	driver->getDriverVersion(); // Some drivers need this to be called before init

	stop_signal = CreateEvent(nullptr, true, false, nullptr);
	receive_signal = CreateEvent(nullptr, false, false, nullptr);

	InitializeCriticalSection(&buffer_section);
	InitializeCriticalSection(&status_section);
}

AsioDevice::~AsioDevice() {
	uninit();

	DeleteCriticalSection(&status_section);
	DeleteCriticalSection(&buffer_section);
}

bool AsioDevice::switch_status(DeviceStatus _requested_status) {
	if (_requested_status != DeviceStatus::Undefined) requested_status = _requested_status;
	if (status == requested_status) return true;

	EnterCriticalSection(&status_section);

	try {
		switch (requested_status) {
		case DeviceStatus::None:
			uninit();
			PostMessage(message_window, WM_USER_INPUT_STATUS, index, 0);
			break;
		case DeviceStatus::Open:
			init();
			open();
			mlog(driver_name, "MCAC: status set to open", LogLevel::Debug);
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

	LeaveCriticalSection(&status_section);

	return (status == requested_status);
}

void AsioDevice::init() {
	if (status >= DeviceStatus::Initialized) return;

	int error = 0;

	error = !driver->init(message_window);
	refuse(error);

	driver->getDriverName(driver_name);

	error = driver->getChannels(&input_channels_number, &output_channels_number);
	refuse(error);

	error = driver->getBufferSize(&min_buffer_size, &max_buffer_size, &buffer_size, &buffer_size_granularity);
	refuse(error);

	initialize_sample_rates();
	initialize_channels();

	status = DeviceStatus::Initialized;
}

void AsioDevice::uninit() {
	if (status >= DeviceStatus::Open) close();

	status = DeviceStatus::None;
}

void AsioDevice::open() {
	if (status >= DeviceStatus::Open) return;
	refuse(status < DeviceStatus::Initialized, "MCAC: Driver is not initialized");

	int error = 0;

	EnterCriticalSection(&buffer_section);
	for (int i = 0; i < input_channels_number; i += 1) {
		buffer_infos[i].isInput = ASIOTrue;
		buffer_infos[i].channelNum = i;
		circlebuf_init(&input_buffers[i]);
	}
	error = driver->createBuffers(buffer_infos, input_channels_number, buffer_size, &callbacks[index]);
	LeaveCriticalSection(&buffer_section);

	refuse(error);

	update_active_channels();

	EnterCriticalSection(&writer_section);
	if (writer) {
		delete writer;
		writer = nullptr;
	}
	writer = new Writer();
	writer->init(sample_rate, active_channels_number);
	LeaveCriticalSection(&writer_section);

	capture_thread = CreateThread(nullptr, 0, CaptureThread, this, 0, nullptr);

	error = driver->start();
	refuse(error);

	status = DeviceStatus::Open;
}

void AsioDevice::close() {
	if (status >= DeviceStatus::Open) driver->stop();

	SetEvent(stop_signal);

	if (capture_thread.Valid()) {
		WaitForSingleObject(capture_thread, INFINITE);
		capture_thread = NULL;
	}

	EnterCriticalSection(&writer_section);
	if (writer) {
		delete writer;
		writer = nullptr;
	}
	LeaveCriticalSection(&writer_section);

	EnterCriticalSection(&buffer_section);
	driver->disposeBuffers();
	memset(buffer_infos, 0, sizeof(buffer_infos));
	for (int i = 0; i < input_channels_number; i += 1) {
		circlebuf_free(&input_buffers[i]);
	}
	LeaveCriticalSection(&buffer_section);

	status = DeviceStatus::Initialized;

	ResetEvent(stop_signal);
}

void AsioDevice::initialize_channels() {
	int i = 0, error = 0;

	for (i = 0; i < input_channels_number; i += 1) {
		ASIOChannelInfo info = { 0 };
		info.channel = i;
		info.isInput = 1;
		error = driver->getChannelInfo(&info);
		refuse(error);

		if (sample_type == -1) {
			sample_type = info.type;
			refuse(sample_type != ASIOSTFloat32LSB && sample_type != ASIOSTInt32LSB, "MCAC: Unsupported sample type");
		}

		input_channel_names.set(i, info.name);
	}

	input_channel_names.appendNumbersToDuplicates(false, true);
}

void AsioDevice::initialize_sample_rates() {
	int error = 0;

	error = driver->getSampleRate(&sample_rate);
	refuse(error);

	for (double try_sample_rate : supported_sample_rates) {
		if (try_sample_rate == sample_rate) return;
	}

	for (double try_sample_rate : supported_sample_rates) {
		if (ASE_OK == driver->canSampleRate(try_sample_rate)) {
			error = driver->setSampleRate(try_sample_rate);
			refuse(error);

			error = driver->getSampleRate(&sample_rate);
			refuse(error);

			refuse(sample_rate != try_sample_rate, "MCAC: Unsupported sample rate");

			return;
		}
	}

	refuse(false, "MCAC: Unsupported sample rate");
}

DWORD WINAPI AsioDevice::CaptureThread(void* data) {
	AsioDevice* device = static_cast<AsioDevice*>(data);
	if (!device) throw "MCAC: Could not find current device";

	HANDLE signals[2] = { device->receive_signal, device->stop_signal };

	while (true) {
		switch (WaitForMultipleObjects(2, signals, false, INFINITE)) {
		case WAIT_OBJECT_0: // device->receive_signal
			EnterCriticalSection(&device->buffer_section);
			if (writer) writer->write_packet(device->input_buffer_pointers);
			LeaveCriticalSection(&device->buffer_section);
			continue;
		case WAIT_OBJECT_0 + 1: // device->stop_signal
			return 0;
		default:
			throw "MCAC: Abnormal termination of capture thread";
		}
	}
	return 0;
}

long AsioDevice::message(int device_index, long selector, long value, void*, double*) {
	if (!device_manager) return 0;
	AsioDevice* device = device_manager->get_instance(device_index);
	if (!device) throw "MCAC: Could not find current device";

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
	case kAsioLatenciesChanged:
		device->reopen();
		return 1;
	case kAsioSupportsTimeInfo:
		return 0;
	case kAsioSupportsTimeCode:
		return 0;
	case kAsioOverload:
		mlog(device->driver_name, "driver reported overload", LogLevel::Warn);
		return 1;
	}
	return 0;
}

void AsioDevice::sample_rate_changed(int device_index, ASIOSampleRate) {
	if (!device_manager) return;
	AsioDevice* device = device_manager->get_instance(device_index);
	if (!device) throw "MCAC: Could not find current device";

	device->reinit();
}

void AsioDevice::buffer_switch(int device_index, long index, ASIOBool processNow) {
	if (!device_manager) return;
	AsioDevice* device = device_manager->get_instance(device_index);
	if (!device) throw "MCAC: Could not find current device";

	EnterCriticalSection(&device->buffer_section);
	device->push_received_buffers(index);
	SetEvent(device->receive_signal);
	LeaveCriticalSection(&device->buffer_section);

	(void)processNow;
}

ASIOTime* AsioDevice::buffer_switch_time_info(int device_index, ASIOTime* timeInfo, long index, ASIOBool processNow) {
	buffer_switch(device_index, index, processNow);

	return nullptr;

	(void)timeInfo;
}

static ULONGLONG last_time = 0;

void AsioDevice::push_received_buffers(int buffer_index) {
	extern float last_buffer_magnitude[MAX_INPUT_CHANNELS];
	extern float buffer_magnitude[MAX_INPUT_CHANNELS];

	for (int channel = 0; channel < input_channels_number; channel += 1) {
		if (active_channels[channel]) {
			size_t data_size = input_buffers[channel].size;
			circlebuf_upsize(&input_buffers[channel], data_size + (size_t)buffer_size * BYTES_PER_SAMPLE);
			if (sample_type == ASIOSTInt32LSB) {
				convertInt32ToFloat((const char*)buffer_infos[channel].buffers[buffer_index], (float*)circlebuf_data(&input_buffers[channel], data_size), buffer_size);
			}
			else {
				memcpy(circlebuf_data(&input_buffers[channel], data_size), buffer_infos[channel].buffers[buffer_index], (size_t)buffer_size * BYTES_PER_SAMPLE);
			}
			last_buffer_magnitude[channel] = ((float*)circlebuf_data(&input_buffers[channel], 0))[0];
		}
		else {
			last_buffer_magnitude[channel] = 0.0f;
		}

		if (buffer_magnitude[channel] < last_buffer_magnitude[channel]) {
			buffer_magnitude[channel] = last_buffer_magnitude[channel];
		}
		else {
			buffer_magnitude[channel] *= 0.99f;
		}
	}

	ULONGLONG time = GetTickCount64();
	if (time > last_time + 3) {
		PostMessage(message_window, WM_USER_ASYNC_UPDATE, 0, 0);
		last_time = time;
	}
}

byte AsioDevice::update_active_channels() {
	active_channels_number = 0;
	memset(input_buffer_pointers, 0, sizeof(input_buffer_pointers));

	for (int i = 0; i < input_channels_number; i++) {
		if (active_channels[i]) {
			input_buffer_pointers[active_channels_number] = &input_buffers[i];
			active_channels_number += 1;
		}
	}

	return active_channels_number;
}
