#pragma once

#define bfree(pointer) free(pointer)
#define brealloc(pointer, size) realloc(pointer, size)
#include "obs/circlebuf.h"
#undef bfree
#undef bfealloc

#define WM_USER_INPUT_STATUS (WM_USER + 1)
#define WM_USER_RESET_REQUEST (WM_USER + 2)
#define WM_USER_UPDATE (WM_USER + 3)
#define WM_USER_INPUT_ERROR (WM_USER + 4)
#define WM_USER_RESTART (WM_USER + 5)
#define WM_USER_CLEANUP (WM_USER + 6)
#define WM_USER_WRITER_UPDATE (WM_USER + 7)
#define WM_USER_WRITER_ERROR (WM_USER + 8)
#define WM_USER_WRITER_STATUS (WM_USER + 9)
#define WM_USER_ASYNC_UPDATE (WM_USER + 10)

#define MAX_CONCURRENT_DEVICES 32

constexpr int MAX_INPUT_CHANNELS = 32;
constexpr int MAX_OUTPUT_CHANNELS = 32;
constexpr int RECONNECT_INTERVAL = 1000;
constexpr int REQUEST_INTERVAL = 1000;
constexpr int BYTES_PER_SAMPLE = 4;
constexpr int DEFAULT_ENCODER_QUALITY = 127;
constexpr int MAX_ENCODER_QUALITY = 127;
constexpr int DEFAULT_OUTPUT_BUFFER_SIZE = 32768;
constexpr int TEMPORARY_BUFFER_SIZE = 4096;
constexpr int MORE_DATA_REQUIRED = 1;

enum class DeviceStatus : int {
	None = 'none',
	Initialized = 'init',
	Open = 'open',
	Fail = 'fail',
};

static const char* level_labels[] = { "DEBUG", "INFO", "WARNING", "ERROR", "FATAL" };

enum class LogLevel : int {
	Debug = 0,
	Info = 1,
	Warn = 2,
	Error = 3,
	Fatal = 4,
};

enum class DeviceRequest : int {
	Open = 'open',
};
