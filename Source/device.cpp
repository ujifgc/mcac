#include "core.h"
#include "device.h"
#include "device_manager.h"

extern HWND message_window;

Device::Device(String _name, DeviceManager* _manager, int _index) {
	name = _name;
	manager = _manager;
	index = _index;
	is_stopping = false;
	request_signal = CreateEvent(nullptr, false, false, nullptr);
	result_signal = CreateEvent(nullptr, false, false, nullptr);
	owner_thread = CreateThread(nullptr, 0, OwnerThread, this, 0, nullptr);
	SetThreadPriority(owner_thread, THREAD_PRIORITY_HIGHEST);
}

Device::~Device() {
	is_stopping = true;
	SetEvent(request_signal);
	WaitForSingleObject(owner_thread, 10 * RECONNECT_INTERVAL);
	if (owner_thread.Valid()) {
		mlog(name, "MCAC: failed to gracefully exit owner thread", LogLevel::Error);
		//TerminateThread(owner_thread, 0);
	}
	mlog(name, __func__, LogLevel::Debug);
}

HANDLE Device::open() {
	ResetEvent(result_signal);
	request = DeviceRequest::Open;
	SetEvent(request_signal);
	return result_signal;
}

DWORD WINAPI Device::OwnerThread(void* data) {
	Device* device = (Device*)data;
	if (!device) throw "MCAC: Thread could not find device data";

	HRESULT hr = CoInitializeEx(0, COINIT_APARTMENTTHREADED);
	bool com_initialized = SUCCEEDED(hr);
	
	String devname = device->name;

	while (!device->is_stopping) {
		switch (WaitForSingleObject(device->request_signal, REQUEST_INTERVAL)) {
		case WAIT_TIMEOUT:
			break;
		case WAIT_OBJECT_0:
			if (!device->is_stopping) device->onSignal(device->request);
			break;
		default:
			throw "MCAC: Abnormal termination of owner thread";
		}
	}

	delete device->instance;

	if (device->driver != nullptr) {
		try_release(device->driver);
		device->driver = nullptr;
	}

	if (com_initialized) CoUninitialize();

	SetEvent(device->result_signal);
	device->owner_thread = NULL;

	return 0;
}

void Device::init_instance() {
	int device_index = -1;
	for (int i = 0; i < MAX_CONCURRENT_DEVICES; i += 1) {
		if (manager->device_names[i] == name) {
			device_index = i;
			break;
		}
	}
	if (device_index >= 0) {
		clsid = manager->device_clsids[device_index];
		HRESULT hres = CoCreateInstance(clsid, nullptr, CLSCTX_INPROC_SERVER, clsid, (void**)&driver);

		if (SUCCEEDED(hres))
			instance = new AsioDevice(driver, index);
		else
			mlog(name, "MCAC: Failed to interface with the driver", LogLevel::Debug);
	}
	else {
		mlog(name, "MCAC: driver name not found", LogLevel::Debug);
	}
}

void Device::onSignal(DeviceRequest _request) {
	switch (_request) {
		case DeviceRequest::Open:
			mlog(name, "MCAC: open signalled", LogLevel::Debug);
			if (!instance) init_instance();
			if (instance) instance->set_status(DeviceStatus::Open);
			break;
		default:
			mlog(name, "MCAC: unknown signalled", LogLevel::Debug);
			break;
	}
}
