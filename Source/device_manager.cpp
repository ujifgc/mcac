#include "core.h"
#include "asio_api.h"
#include "device.h"
#include "device_manager.h"

extern HWND message_window;

DeviceManager::DeviceManager() {
	read_device_names();
}

DeviceManager::~DeviceManager() {
	for (int index = 0; index < MAX_CONCURRENT_DEVICES; index += 1) {
		delete devices[index];
	}
}

Device* DeviceManager::create_device(String name) {
	Device* device = nullptr;
	int active_device_index = -1;

	for (int index = 0; index < MAX_CONCURRENT_DEVICES; index += 1) {
		if (device_names[index] == name) {
			if (devices[index] == nullptr) devices[index] = new Device(name, this, index);
			device = devices[index];
			active_device_index = index;
		}
	}

	if (active_device_index >= 0) {
		PostMessage(message_window, WM_USER_CLEANUP, active_device_index, 0);
	}

	return device;
}

void DeviceManager::cleanup(String name) {
	for (int index = 0; index < MAX_CONCURRENT_DEVICES; index += 1) {
		if (devices[index] != nullptr && device_names[index] == name) {
			auto copy = devices[index];
			devices[index] = nullptr;
			delete copy;
		}
	}
}

void DeviceManager::cleanup_except(int active_device_index) {
	for (int index = 0; index < MAX_CONCURRENT_DEVICES; index += 1) {
		if (devices[index] != nullptr && index != active_device_index) {
			auto copy = devices[index];
			devices[index] = nullptr;
			delete copy;
		}
	}
}

const String* DeviceManager::get_device_names() {
	return device_names;
}

void DeviceManager::read_device_names() {
	for (int index = 0; index < MAX_CONCURRENT_DEVICES; index += 1) {
		device_names[index].clear();
	}

	HKEY asio_root_key = 0;
	int index = 0;
	CLSID clsid;
	const WCHAR* software_asio = L"SOFTWARE\\ASIO";
	const String asio_root_path = String(L"HKEY_LOCAL_MACHINE\\") + software_asio + L"\\";
	const String classes_clsid_path = String(L"HKEY_CLASSES_ROOT\\CLSID\\");

	if (ERROR_SUCCESS == RegOpenKeyW(HKEY_LOCAL_MACHINE, software_asio, &asio_root_key)) {
		WCHAR device_key_name[256] = { 0 };
		while (ERROR_SUCCESS == RegEnumKeyW(asio_root_key, index, device_key_name, sizeof(device_key_name) / sizeof(WCHAR))) {
			String description = juce::WindowsRegistry::getValue(asio_root_path + String(device_key_name) + L"\\Description");
			String clsid_string = juce::WindowsRegistry::getValue(asio_root_path + String(device_key_name) + L"\\CLSID");
			String driver_path = juce::WindowsRegistry::getValue(classes_clsid_path + clsid_string + L"\\InprocServer32\\");
			if (driver_path.length() > 0) {
				HRESULT res = CLSIDFromString(clsid_string.toWideCharPointer(), &clsid);
				if (SUCCEEDED(res)) {
					device_names[index] = description;
					device_clsids[index] = clsid;
				}
			}
			index += 1;
		}

		RegCloseKey(asio_root_key);
	}

	devices_count = index;
}

AsioDevice* DeviceManager::get_instance(int index) {
	Device* device = devices[index];
	return device ? device->get_instance() : nullptr;
}
