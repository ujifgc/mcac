#pragma once

class DeviceManager {
public:
	String device_names[MAX_CONCURRENT_DEVICES] = { };
	CLSID device_clsids[MAX_CONCURRENT_DEVICES] = { 0 };
	Device* devices[MAX_CONCURRENT_DEVICES] = { 0 };
	String device_errors[MAX_CONCURRENT_DEVICES] = { };

public:
	DeviceManager();
	~DeviceManager();

public:
	Device* create_device(String name);
	void cleanup_except(int active_device_index);
	void cleanup(String name);
	const String* get_device_names();
	void read_device_names();
	AsioDevice* get_instance(int index);
	int devices_count = 0;
};
