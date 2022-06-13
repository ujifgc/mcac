#pragma once

#define WIN32_MEAN_AND_LEAN
#include <windows.h>

#include "asio/iasiodrv.h"
#include "juce/JuceHeader.h"
#include "obs/WinHandle.h"

const ASIOSampleRate supported_sample_rates[] = {
	44100.0, 48000.0, 96000.0, 88200.0, 64000.0, 32000.0, 24000.0, 22050.0, 16000.0, 12000.0, 11025.0, 8000.0
};

class AsioDevice {
	IASIO *driver = nullptr;
	int index = -1;

	char driver_name[64] = {};

	long min_buffer_size = 0, max_buffer_size = 0, buffer_size = 0, buffer_size_granularity = 0;

	ASIOBufferInfo buffer_infos[MAX_INPUT_CHANNELS] = {};

	circlebuf input_buffers[MAX_INPUT_CHANNELS] = {};
	circlebuf *input_buffer_pointers[MAX_INPUT_CHANNELS] = {};

	WinHandle stop_signal, receive_signal;
	WinHandle capture_thread;
	CRITICAL_SECTION buffer_section, status_section;

	static DWORD WINAPI CaptureThread(void* data);

	byte active_channels_number = 0;

	DeviceStatus requested_status = DeviceStatus::None;

	void init();
	void uninit();
	void refuse(long, const char*);
	void initialize_sample_rates();
	void initialize_channels();
	void push_received_buffers(int buffer_index);

public:
	long input_channels_number = 0, output_channels_number = 0;
	StringArray input_channel_names;

	ASIOSampleRate sample_rate = 0.0;
	ASIOSampleType sample_type = -1;

	DeviceStatus status = DeviceStatus::None;
	bool switch_status(DeviceStatus _requested_status = DeviceStatus::Undefined);

	static ASIOTime* buffer_switch_time_info(int device_index, ASIOTime* timeInfo, long index, ASIOBool processNow);
	static void buffer_switch(int device_index, long index, ASIOBool processNow);
	static void sample_rate_changed(int device_index, ASIOSampleRate sRate);
	static long message(int device_index, long selector, long value, void* message, double* opt);

	AsioDevice(IASIO *driver, int index);
	~AsioDevice();

	void open();
	void close();

	void reinit() {
		uninit();
		switch_status();
	}

	void reopen() {
		close();
		switch_status();
	}

	byte update_active_channels();
};

static inline uint32_t littleEndianInt(const void* const bytes) noexcept {
	return *static_cast<const uint32_t*> (bytes);
}

static inline void convertInt32ToFloat(const char* src, float* dest, int numSamples) noexcept {
	const double g = 1.0 / 0x7fffffff;
	while (--numSamples >= 0) {
		*dest++ = (float)(g * (int)littleEndianInt(src));
		src += 4;
	}
}

inline void mlog(String device, String message, LogLevel message_level = LogLevel::Info) {
	extern FileLogger* logger;
	extern LogLevel log_level;

	if (logger && message_level >= log_level)
		logger->logMessage(Time::getCurrentTime().formatted("%Y-%m-%d_%H-%M-%S") + " [" + device + "] " + level_labels[(int)message_level] + ": " + message);

	if (message_level == LogLevel::Fatal) throw message;
}
