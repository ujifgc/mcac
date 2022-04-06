#pragma once

class Device {
	IASIO* driver = nullptr;
	AsioDevice* instance = nullptr;
	class DeviceManager* manager = nullptr;
	volatile bool stop = false;

	static DWORD WINAPI OwnerThread(void* data);
	WinHandle request_signal, result_signal;
	WinHandle owner_thread;

	String request;
	String name;
	//String error;
	int index;
	CLSID clsid = { 0 };

public:
	~Device();
	Device(String name, DeviceManager *manager, int index);
	HANDLE open();
	void onSignal(String request);
	void init_instance();

public:
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
		return instance ? instance->status : dsNone;
	}
	AsioDevice* get_instance() {
		return instance;
	}
	String get_name() {
		return name;
	}
	int get_index() {
		return index;
	}
};
