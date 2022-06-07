#pragma once

#include "asio_api.h"

class Device {
	IASIO* driver = nullptr;
	class DeviceManager* manager = nullptr;
	volatile bool is_stopping = false;

	static DWORD WINAPI OwnerThread(void* data);
	WinHandle request_signal, result_signal;
	WinHandle owner_thread;

	DeviceRequest request;
	String name;
	int index = -1;
	CLSID clsid = {};

public:
	AsioDevice* instance = nullptr;

	~Device();
	Device(String name, DeviceManager *manager, int index);
	HANDLE open();
	void onSignal(DeviceRequest request);
	void init_instance();

public:
	byte update_active_channels() {
		return instance ? instance->update_active_channels() : 0;
	}

	StringArray get_input_channel_names() {
		return instance ? instance->input_channel_names : StringArray();
	}
	StringArray get_output_channel_names() {
		return instance ? instance->output_channel_names : StringArray();
	}
	double get_sample_rate() {
		return instance ? instance->sample_rate : 0;
	}
	int get_sample_type() {
		return instance ? instance->sample_type : 0;
	}
	DeviceStatus get_device_status() {
		return instance ? instance->status : DeviceStatus::None;
	}
};
