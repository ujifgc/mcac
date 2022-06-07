#define WIN32_LEAN_AND_MEAN
#include <combaseapi.h>

#include "juce/JuceHeader.h"
#include "core.h"

extern String instance_name;
extern String settings_file;

void WriteSettings(String key, String value) {
    WritePrivateProfileStringW(instance_name.toWideCharPointer(), key.toWideCharPointer(), value.toWideCharPointer(), settings_file.toWideCharPointer());
}

String ReadSettingsString(String key, String default_value = "") {
    WCHAR buf[TEMPORARY_BUFFER_SIZE] = {};
    GetPrivateProfileStringW(instance_name.toWideCharPointer(), key.toWideCharPointer(), default_value.toWideCharPointer(), buf, TEMPORARY_BUFFER_SIZE, settings_file.toWideCharPointer());
    return String(buf);
}

String getMainTitle() {
    String title = L"MCAC — Multi Channel Audio Capture";

    char* main_module_filename = NULL;
    BOOL success = false;
    LPVOID product_version_data = NULL;
    UINT product_version_length = 0;
    char buf[TEMPORARY_BUFFER_SIZE] = {};

    _get_pgmptr(&main_module_filename);
    success = GetFileVersionInfo(main_module_filename, NULL, TEMPORARY_BUFFER_SIZE, buf);

    if (success) {
        success = VerQueryValue(buf, "\\StringFileInfo\\000904b0\\ProductVersion", &product_version_data, &product_version_length);
    }

    if (success) {
        title += " v. " + String((char*)product_version_data, product_version_length);
    }

    return title;
}
