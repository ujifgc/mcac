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
