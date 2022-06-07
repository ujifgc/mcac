#pragma once

#include <shlobj_core.h>

#include "coreaudio-writer.h"

extern HMODULE coreaudio_library;

#define GET_PROC(NAME) \
	NAME = (##NAME##_t)GetProcAddress(coreaudio_library, #NAME); \
	if (!NAME) return false;

inline bool load_core_audio() {
	if (coreaudio_library) return true;

	WCHAR toolbox_path[TEMPORARY_BUFFER_SIZE] = {};
	SHGetFolderPathW(NULL, CSIDL_PROGRAM_FILES_COMMON, NULL, 0, toolbox_path);
	wcscat(toolbox_path, L"\\Apple\\Apple Application Support");

	SetDllDirectoryW(toolbox_path);
	coreaudio_library = LoadLibrary("CoreAudioToolbox.dll");
	SetDllDirectoryW(NULL);

	if (coreaudio_library) {
		GET_PROC(AudioConverterNew);
		GET_PROC(AudioConverterDispose);
		GET_PROC(AudioConverterReset);
		GET_PROC(AudioConverterGetProperty);
		GET_PROC(AudioConverterGetPropertyInfo);
		GET_PROC(AudioConverterSetProperty);
		GET_PROC(AudioConverterFillComplexBuffer);
		GET_PROC(AudioFormatGetProperty);
		GET_PROC(AudioFormatGetPropertyInfo);
		return true;
	}

	return false;
}

class Writer {
	AudioConverterPtr converter = nullptr;
	unsigned bytes_per_input_packet = 0;
	unsigned sample_rate = 0;
	circlebuf output_buffer = {}, zero_buffer = {};
	circlebuf** input_buffers = nullptr;
	FILE* file = nullptr;
	bool stop = true;
	CRITICAL_SECTION file_section;
	class AsioDevice* device = nullptr;
	bool core_audio_ready = false;

public:
	String message;
	byte channels = 0;

	Writer();
	~Writer();
	void init(double _sample_rate = 0.0f, byte active_channels_count = 0);
	void uninit();
	bool open();
	void close(bool report = true);
	static OSStatus input_data_provider(AudioConverterPtr inAudioConverter, UInt32* ioNumberDataPackets, AudioBufferList* ioData, AudioStreamPacketDescription** outDataPacketDescription, void* inUserData);
	void write_packet(circlebuf **_input_buffers);
};
