#define WIN32_LEAN_AND_MEAN
#include "combaseapi.h"
#include <JuceHeader.h>

#include "core.h"

extern String instance_name;
extern String settings_file;

void WriteSettings(String key, String value) {
    WritePrivateProfileStringW(instance_name.toWideCharPointer(), key.toWideCharPointer(), value.toWideCharPointer(), settings_file.toWideCharPointer());
}

String ReadSettingsString(String key, String default_value = "") {
    WCHAR buf[MAX_PATH];
    GetPrivateProfileStringW(instance_name.toWideCharPointer(), key.toWideCharPointer(), default_value.toWideCharPointer(), buf, MAX_PATH, settings_file.toWideCharPointer());
    return String(buf);
}

String getTitle() {
    String title = L"MCAC — Multi Channel Audio Capture";

    char* main_module_filename = NULL;
    BOOL success = false;
    LPVOID product_version_data = NULL;
    UINT product_version_length = 0;
    const int VERSION_DATA_BUF_SIZE = 4096;
    char buf[VERSION_DATA_BUF_SIZE];

    _get_pgmptr(&main_module_filename);
    success = GetFileVersionInfo(main_module_filename, NULL, VERSION_DATA_BUF_SIZE, buf);

    if (success) {
        success = VerQueryValue(buf, "\\StringFileInfo\\000904b0\\ProductVersion", &product_version_data, &product_version_length);
    }

    if (success) {
        title += " v. " + String((char*)product_version_data, product_version_length);
    }

    return title;
}
