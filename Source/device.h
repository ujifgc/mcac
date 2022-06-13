#pragma once

#include "asio_api.h"

class Device {
	IASIO* driver = nullptr;
	class DeviceManager* manager = nullptr;

	static DWORD WINAPI OwnerThread(void* data);
	WinHandle request_signal;
	WinHandle owner_thread;

	volatile DeviceRequest request;
	String name;
	int index = -1;
	CLSID clsid = {};

public:
	AsioDevice* instance = nullptr;

	~Device();
	Device(String name, DeviceManager *manager, int index);
	void open();
	void onSignal(DeviceRequest request);
	void init_instance();

public:
	byte update_active_channels() {
		return instance ? instance->update_active_channels() : 0;
	}
	DeviceStatus get_device_status() {
		return instance ? instance->status : DeviceStatus::None;
	}
};
