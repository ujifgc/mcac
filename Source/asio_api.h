#pragma once

#define WIN32_MEAN_AND_LEAN
#include <windows.h>
#include "asio/iasiodrv.h"

#include <JuceHeader.h>
#include "obs-utils/WinHandle.hpp"

//==
// per https://en.wikipedia.org/wiki/Sampling_(signal_processing)#Sampling_rate
//
const double standard_sample_rates[] = {
8000.0, 11025.0, 16000.0, 22050.0, 32000.0, 37800.0, 44056.0, 44100.0,
47250.0, 48000.0, 50000.0, 50400.0, 64000.0, 88200.0, 96000.0, 176400.0,
192000.0, 352800.0, 2822400.0, 5644800.0, 11289600.0, 22579200.0
};
const double default_sample_rate = 44100.0;

class AsioDevice {
	IASIO *driver = nullptr;
	int index = -1;

	long driver_version = 0;
	char driver_name[64] = { 0 };
	
	long min_buffer_size = 0, max_buffer_size = 0, preferred_buffer_size = 0, buffer_size_granularity = 0;
	long input_latency = 0, output_latency = 0;
	Array<double> supported_sample_rates;
	bool output_ready_available = false;

	ASIOBufferInfo buffer_infos[MAX_INPUT_CHANNELS + MAX_OUTPUT_CHANNELS] = { 0 };

	int output_samples_per_10ms = 0;

	float* sample_type_buffers[MAX_INPUT_CHANNELS] = { 0 };

	struct circlebuf input_buffer = { 0 };
	struct circlebuf interleaved_buffer = { 0 };

	WinHandle stop_signal, receive_signal;
	WinHandle capture_thread;
	bool is_initialized = false, is_open = false;
	bool failed_on_close = false;
	int reconnect_count = 0;
	CRITICAL_SECTION buffer_section, switch_section;

	bool driver_buffers_allocated = false;

	static DWORD WINAPI CaptureThread(void* data);

public:
	long input_channels_number = 0, output_channels_number = 0;
	StringArray input_channel_names, output_channel_names;
	double sample_rate = 0.0;
	long sample_type = 0;

	static ASIOTime* buffer_switch_time_info(int device_index, ASIOTime* timeInfo, long index, ASIOBool processNow);
	static void buffer_switch(int device_index, long index, ASIOBool processNow);
	static void sample_rate_changed(int device_index, ASIOSampleRate sRate);
	static long message(int device_index, long selector, long value, void* message, double* opt);

private:
	void init();
	void uninit();
	void refuse(long, const char*);
	void initialize_sample_rates();
	void initialize_channel_names();
	void initialize_latencies();
	bool switch_status();

private:
	void push_received_buffers(int buffer_index);

public:
	String name;
	DeviceStatus status, plan;

	void set_status(enum DeviceStatus);

public:
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

	void set_sample_rate(double);
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

inline void try_release(IASIO* driver) {
	__try { driver->Release(); }
	__except (EXCEPTION_EXECUTE_HANDLER) {}
}

inline void mlog(String device, String message, LogLevel message_level = llInfo) {
	extern FileLogger* logger;
	extern LogLevel log_level;
	const char* level_labels[] = { "DEBUG", "INFO", "WARNING", "ERROR", "FATAL" };

	if (logger && message_level >= log_level)
		logger->logMessage(Time::getCurrentTime().formatted("%Y-%m-%d_%H-%M-%S") + " [" + device + "] " + level_labels[(int)message_level] + ": " + message);

	if (message_level == llFatal) throw message;
}

