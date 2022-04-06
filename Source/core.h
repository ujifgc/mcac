#pragma once
#pragma warning( disable : 26812 6258 33011 )

#define bfree(A) free(A)
#define brealloc(A, B) realloc(A, B)
#include "circlebuf.h"
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

#define MAX_INPUT_CHANNELS 32
#define MAX_OUTPUT_CHANNELS 32
#define RECONNECT_INTERVAL 1000
#define REQUEST_INTERVAL 1000
#define MAX_CONCURRENT_DEVICES 32
#define BYTES_PER_SAMPLE 4
#define DEFAULT_ENCODER_BITRATE 320000
#define DEFAULT_ENCODER_QUALITY 127

enum DeviceStatus {
	dsNone = 'none',
	dsInitialized = 'init',
	dsOpen = 'open',
	dsFail = 'fail'
};

enum LogLevel {
	llDebug = 0,
	llInfo = 1,
	llWarn = 2,
	llError = 3,
	llFatal = 4,
};
