#include "core.h"
#include "device.h"
#include "device_manager.h"

extern HWND message_window;

Device::Device(String _name, DeviceManager* _manager, int _index) {
	name = _name;
	manager = _manager;
	index = _index;
	request = DeviceRequest::None;
	request_signal = CreateEvent(nullptr, false, false, nullptr);
	owner_thread = CreateThread(nullptr, 0, OwnerThread, this, 0, nullptr);
	SetThreadPriority(owner_thread, THREAD_PRIORITY_HIGHEST);
}

Device::~Device() {
	request = DeviceRequest::Stop;
	SetEvent(request_signal);
	WaitForSingleObject(owner_thread, 10 * RECONNECT_INTERVAL);
	if (owner_thread.Valid()) {
		mlog(name, "MCAC: failed to gracefully exit owner thread", LogLevel::Error);
		//TerminateThread(owner_thread, 0);
	}
	mlog(name, __func__, LogLevel::Debug);
}

void Device::open() {
	request = DeviceRequest::Open;
	SetEvent(request_signal);
}

inline void try_release(IASIO* driver) {
	__try { driver->Release(); }
	__except (EXCEPTION_EXECUTE_HANDLER) {}
}

DWORD WINAPI Device::OwnerThread(void* data) {
	Device* device = (Device*)data;
	if (!device) throw "MCAC: Thread could not find device data";

	bool com_initialized = SUCCEEDED(CoInitializeEx(0, COINIT_APARTMENTTHREADED));
	
	String devname = device->name;

	while (device->request != DeviceRequest::Stop) {
		switch (WaitForSingleObject(device->request_signal, REQUEST_INTERVAL)) {
		case WAIT_TIMEOUT:
			break;
		case WAIT_OBJECT_0:
			device->onSignal(device->request);
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
		if (SUCCEEDED(CoCreateInstance(clsid, nullptr, CLSCTX_INPROC_SERVER, clsid, (void**)&driver)))
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
			if (instance) instance->switch_status(DeviceStatus::Open);
			break;
		case DeviceRequest::Stop:
		case DeviceRequest::None:
			break;
		default:
			mlog(name, "MCAC: unknown signalled", LogLevel::Debug);
			break;
	}
}
