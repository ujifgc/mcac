#pragma once

#if MAX_CONCURRENT_DEVICES != 32
#error add callback declarations to reflect MAX_CONCURRENT_DEVICES
#endif

#define DECLARE_CALLBACKS(INDEX)                                                                            \
	inline void buffer_switch_##INDEX(long index, ASIOBool processNow) {                                    \
      return AsioDevice::buffer_switch(INDEX, index, processNow); }                                         \
	inline void sample_rate_changed_##INDEX(ASIOSampleRate sRate) {                                         \
      return AsioDevice::sample_rate_changed(INDEX, sRate); }                                               \
	inline long message_##INDEX(long selector, long value, void* message, double* opt) {                    \
      return AsioDevice::message(INDEX, selector, value, message, opt); }                                   \
	inline ASIOTime* buffer_switch_time_info_##INDEX(ASIOTime* timeInfo, long index, ASIOBool processNow) { \
      return AsioDevice::buffer_switch_time_info(INDEX, timeInfo, index, processNow); }                     \

DECLARE_CALLBACKS(0)	DECLARE_CALLBACKS(1)	DECLARE_CALLBACKS(2)	DECLARE_CALLBACKS(3)
DECLARE_CALLBACKS(4)	DECLARE_CALLBACKS(5)	DECLARE_CALLBACKS(6)	DECLARE_CALLBACKS(7)
DECLARE_CALLBACKS(8)	DECLARE_CALLBACKS(9)	DECLARE_CALLBACKS(10)	DECLARE_CALLBACKS(11)
DECLARE_CALLBACKS(12)	DECLARE_CALLBACKS(13)	DECLARE_CALLBACKS(14)	DECLARE_CALLBACKS(15)
DECLARE_CALLBACKS(16)	DECLARE_CALLBACKS(17)	DECLARE_CALLBACKS(18)	DECLARE_CALLBACKS(19)
DECLARE_CALLBACKS(20)	DECLARE_CALLBACKS(21)	DECLARE_CALLBACKS(22)	DECLARE_CALLBACKS(23)
DECLARE_CALLBACKS(24)	DECLARE_CALLBACKS(25)	DECLARE_CALLBACKS(26)	DECLARE_CALLBACKS(27)
DECLARE_CALLBACKS(28)	DECLARE_CALLBACKS(29)	DECLARE_CALLBACKS(30)	DECLARE_CALLBACKS(31)

#undef DECLARE_CALLBACKS

#define ADDRESS_CALLBACKS(INDEX) \
	{ &buffer_switch_##INDEX, &sample_rate_changed_##INDEX, &message_##INDEX, &buffer_switch_time_info_##INDEX },

ASIOCallbacks callbacks[MAX_CONCURRENT_DEVICES] = {
	ADDRESS_CALLBACKS(0)	ADDRESS_CALLBACKS(1)	ADDRESS_CALLBACKS(2)	ADDRESS_CALLBACKS(3)
	ADDRESS_CALLBACKS(4)	ADDRESS_CALLBACKS(5)	ADDRESS_CALLBACKS(6)	ADDRESS_CALLBACKS(7)
	ADDRESS_CALLBACKS(8)	ADDRESS_CALLBACKS(9)	ADDRESS_CALLBACKS(10)	ADDRESS_CALLBACKS(11)
	ADDRESS_CALLBACKS(12)	ADDRESS_CALLBACKS(13)	ADDRESS_CALLBACKS(14)	ADDRESS_CALLBACKS(15)
	ADDRESS_CALLBACKS(16)	ADDRESS_CALLBACKS(17)	ADDRESS_CALLBACKS(18)	ADDRESS_CALLBACKS(19)
	ADDRESS_CALLBACKS(20)	ADDRESS_CALLBACKS(21)	ADDRESS_CALLBACKS(22)	ADDRESS_CALLBACKS(23)
	ADDRESS_CALLBACKS(24)	ADDRESS_CALLBACKS(25)	ADDRESS_CALLBACKS(26)	ADDRESS_CALLBACKS(27)
	ADDRESS_CALLBACKS(28)	ADDRESS_CALLBACKS(29)	ADDRESS_CALLBACKS(30)	ADDRESS_CALLBACKS(31) 
};

#undef ADDRESS_CALLBACKS
