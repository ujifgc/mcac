#pragma once

#include "shlobj_core.h"
#include "coreaudio-writer.h"

extern HMODULE coreaudio_library;

#define GET_PROC(NAME) \
	NAME = (##NAME##_t)GetProcAddress(coreaudio_library, #NAME); \
	if (!NAME) return false;

inline bool load_core_audio() {
	if (coreaudio_library) return true;

	WCHAR common_folder[2 * MAX_PATH] = { 0 };
	SHGetFolderPathW(NULL, CSIDL_PROGRAM_FILES_COMMON, NULL, 0, common_folder);
	wcscat(common_folder, L"\\Apple\\Apple Application Support");

	SetDllDirectoryW(common_folder);
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
	AudioConverterRef converter = nullptr;
	unsigned bytes_per_input_packet = 0;
	unsigned sample_rate = 0;
	struct circlebuf output_buffer = { 0 };
	circlebuf** input_buffers = nullptr;
	FILE* file = nullptr;
	bool stop = true;
	CRITICAL_SECTION file_section;
	Array<int> bitrates;
	int qualities[6] = { 0, 16, 32, 64, 96, 127 };
	class AsioDevice* device = nullptr;
	bool core_audio_ready = false;

public:
	String message;
	byte channels = 0;

	Writer(class AsioDevice*);
	~Writer();
	void init();
	void init(double _sample_rate);
	void uninit();
	bool open();
	void close(bool report = true);
	static OSStatus input_data_provider(AudioConverterRef inAudioConverter, UInt32* ioNumberDataPackets, AudioBufferList* ioData, AudioStreamPacketDescription** outDataPacketDescription, void* inUserData);
	void write_packet(circlebuf **_input_buffers);
};
