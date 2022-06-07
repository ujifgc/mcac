#include "core.h"
#include "MainComponent.h"
#include "writer.h"

HMODULE coreaudio_library = NULL;
Writer* writer = nullptr;
CRITICAL_SECTION writer_section = { 0 };

Writer::Writer() {
	InitializeCriticalSection(&file_section);

	core_audio_ready = load_core_audio();

	circlebuf_init(&output_buffer);
	circlebuf_init(&zero_buffer);
}

Writer::~Writer() {
	uninit();

	circlebuf_free(&output_buffer);
	circlebuf_free(&zero_buffer);

	DeleteCriticalSection(&file_section);
}

void Writer::init(double _sample_rate, byte _channels) {
	stop = true;

	if (_sample_rate > 0) sample_rate = (int)_sample_rate;
	if (_channels > 0) channels = _channels;

	if (channels == 0) channels = 1;
	if (channels == 7) channels = 8; // coreaudio seems to can't write 7 channels

	AudioStreamBasicDescription in = {}, out = {};

	in.mSampleRate = sample_rate;
	in.mChannelsPerFrame = channels;
	in.mFormatID = kAudioFormatLinearPCM;
	in.mFormatFlags = kAudioFormatFlagIsPacked | kAudioFormatFlagIsFloat | kAudioFormatFlagIsNonInterleaved;
	in.mBytesPerFrame = BYTES_PER_SAMPLE;
	in.mFramesPerPacket = 1;
	in.mBytesPerPacket = in.mFramesPerPacket * in.mBytesPerFrame;
	in.mBitsPerChannel = BYTES_PER_SAMPLE * 8;

	bytes_per_input_packet = in.mBytesPerPacket;
	
	out.mSampleRate = sample_rate;
	out.mChannelsPerFrame = in.mChannelsPerFrame;
	out.mFormatID = kAudioFormatMPEG4AAC;

	OSStatus error = 0;

	if (!core_audio_ready) {
		message = "MCAC: AAC library not available";
		PostMessage(message_window, WM_USER_WRITER_ERROR, 0, 0);
		stop = true;
		return;
	}

	error = AudioConverterNew(&in, &out, &converter);
	if (error) {
		message = (error == 'fmt?') ? "MCAC: unsupported input format" : "MCAC: converter failed";
		PostMessage(message_window, WM_USER_WRITER_ERROR, 0, 0);
		converter = nullptr;
		return;
	}

	UInt32 rate_control = kAudioCodecBitRateControlMode_Variable;
	error = AudioConverterSetProperty(converter, kAudioCodecPropertyBitRateControlMode, sizeof(rate_control), &rate_control);

	UInt32 converter_quality = kAudioConverterQuality_Max;
	error = AudioConverterSetProperty(converter, kAudioConverterCodecQuality, sizeof(converter_quality), &converter_quality);

	const SInt32 layout_map[9] = {
		kAudioChannelLayoutTag_UseChannelBitmap,
		kAudioChannelLayoutTag_Mono,
		kAudioChannelLayoutTag_Stereo,
		kAudioChannelLayoutTag_AAC_3_0,
		kAudioChannelLayoutTag_AAC_4_0,
		kAudioChannelLayoutTag_AAC_5_0,
		kAudioChannelLayoutTag_AAC_6_0,
		kAudioChannelLayoutTag_AAC_7_0,
		kAudioChannelLayoutTag_AAC_Octagonal,
	};

	AudioChannelLayout acl = { 0 };
	acl.mChannelLayoutTag = layout_map[channels];
	UInt32 acl_size = sizeof(acl);

	error = AudioConverterSetProperty(converter, kAudioConverterInputChannelLayout, acl_size, &acl);
	error = AudioConverterSetProperty(converter, kAudioConverterOutputChannelLayout, acl_size, &acl);

	const SInt32 channel_map[9][8] = {
		{ 0, 0, 0, 0, 0, 0, 0, 0 },
		{ 0, 0, 0, 0, 0, 0, 0, 0 }, // L
		{ 0, 1, 0, 0, 0, 0, 0, 0 }, // L R
		{ 2, 0, 1, 0, 0, 0, 0, 0 }, // C L R
		{ 2, 0, 1, 3, 0, 0, 0, 0 }, // C L R C2
		{ 2, 0, 1, 3, 4, 0, 0, 0 }, // C L R L2 R2
		{ 2, 0, 1, 4, 5, 3, 0, 0 }, // C L R L2 R2 C2 L
		{ 2, 0, 1, 6, 7, 4, 5, 0 },
		{ 2, 0, 1, 6, 7, 4, 5, 3 }, // C L R L2 R2 L3 R3 C2
	};

	error = AudioConverterSetProperty(converter, kAudioConverterChannelMap, channels * sizeof(SInt32), channel_map[channels]);

	String encoder_quality = ReadSettingsString(ENCODER_QUALITY_INI_KEY);
	UInt32 quality = encoder_quality.isEmpty() ? DEFAULT_ENCODER_QUALITY : encoder_quality.getIntValue();
	if (quality > MAX_ENCODER_QUALITY) quality = MAX_ENCODER_QUALITY;
	WriteSettings(ENCODER_QUALITY_INI_KEY, String(quality));
	error = AudioConverterSetProperty(converter, kAudioCodecPropertySoundQualityForVBR, sizeof(quality), &quality);

	UInt32 output_buffer_size = 0;
	UInt32 size = sizeof(output_buffer_size);
	error = AudioConverterGetProperty(converter, kAudioConverterPropertyMaximumOutputPacketSize, &size, &output_buffer_size);
	if (error) output_buffer_size = DEFAULT_OUTPUT_BUFFER_SIZE;
	circlebuf_upsize(&output_buffer, output_buffer_size);

	message = "Ready";
	PostMessage(message_window, WM_USER_WRITER_STATUS, 0, 0);
}

void Writer::uninit() {
	close(false);

	if (converter) AudioConverterDispose(converter);
}

bool Writer::open() {
	if (!converter) {
		message = "MCAC: converter is not ready";
		PostMessage(message_window, WM_USER_WRITER_ERROR, 0, 0);
		stop = true;
		return false;
	}

	String filename = output_folder_path + "\\output_" + Time::getCurrentTime().formatted("%Y-%m-%d_%H-%M-%S") + ".aac";

	EnterCriticalSection(&file_section);
	if (file) fclose(file);
	file = fopen(filename.toStdString().c_str(), "wb");
	LeaveCriticalSection(&file_section);

	if (file) {
		message = "Recording";
		PostMessage(message_window, WM_USER_WRITER_STATUS, 0, 0);
		stop = false;
		return true;
	}
	else {
		message = "MCAC: failed to create output file. ";
		message += strerror(errno);
		PostMessage(message_window, WM_USER_WRITER_ERROR, 0, 0);
		stop = true;
		return false;
	}
}

void Writer::close(bool report) {
	stop = true;

	EnterCriticalSection(&file_section);
	if (file) {
		fclose(file);
		file = nullptr;
	}
	LeaveCriticalSection(&file_section);

	if (report) {
		message = "Ready";
		PostMessage(message_window, WM_USER_WRITER_STATUS, 0, 0);
	}
}

OSStatus Writer::input_data_provider(AudioConverterPtr inAudioConverter, UInt32* ioNumberDataPackets, AudioBufferList* ioData, AudioStreamPacketDescription** outDataPacketDescription, void* inUserData) {
	Writer* active_writer = (Writer*)inUserData;

	UInt32 bytes_required = (*ioNumberDataPackets) * active_writer->bytes_per_input_packet;

	if (!active_writer->input_buffers[0] || active_writer->input_buffers[0]->size < bytes_required) {
		*ioNumberDataPackets = 0;
		return MORE_DATA_REQUIRED;
	}

	for (int channel = 0; channel < active_writer->channels; channel++) {
		ioData->mBuffers[channel].mNumberChannels = 1;
		ioData->mBuffers[channel].mDataByteSize = bytes_required;
		if (!active_writer->input_buffers[channel] || active_writer->input_buffers[channel]->size < bytes_required) {
			active_writer->input_buffers[channel] = &active_writer->zero_buffer;
			circlebuf_push_back_zero(active_writer->input_buffers[channel], bytes_required);
		}
		ioData->mBuffers[channel].mData = circlebuf_data(active_writer->input_buffers[channel], 0);
		circlebuf_pop_front(active_writer->input_buffers[channel], ioData->mBuffers[channel].mData, bytes_required);
	}

	return 0;

	(void)inAudioConverter;
	(void)outDataPacketDescription;
}

void Writer::write_packet(circlebuf **_input_buffers) {
	if (!core_audio_ready) {
		message = "MCAC: AAC library not available";
		PostMessage(message_window, WM_USER_WRITER_ERROR, 0, 0);
		stop = true;
	}

	if (stop) {
		for (int i = 0; i < MAX_INPUT_CHANNELS; i++) {
			if (_input_buffers[i] == nullptr) break;
			circlebuf_pop_front(_input_buffers[i], nullptr, _input_buffers[i]->size);
		}
		return;
	}

	input_buffers = _input_buffers;

	UInt32 packets_count = 1;
	AudioBufferList output_buffers = { 0 };
	output_buffers.mNumberBuffers = 1;
	output_buffers.mBuffers[0].mNumberChannels = channels;
	output_buffers.mBuffers[0].mData = circlebuf_data(&output_buffer, 0);
	output_buffers.mBuffers[0].mDataByteSize = (unsigned)output_buffer.size;

	AudioConverterFillComplexBuffer(converter, input_data_provider, this, &packets_count, &output_buffers, NULL);

	if (packets_count > 0) {
		EnterCriticalSection(&file_section);

		if (file) {
			size_t items_wrote = fwrite(
				adts_packet_header(output_buffers.mBuffers[0].mDataByteSize, sample_rate, channels),
				ADTS_PACKET_HEADER_LENGTH, 1,
				file);

			if (items_wrote < 1) {
				message = "MCAC: failed to write packet header. ";
				message += strerror(errno);
				stop = true;
			}
			else {
				items_wrote = fwrite(
					output_buffers.mBuffers[0].mData,
					output_buffers.mBuffers[0].mDataByteSize, 1,
					file);

				if (items_wrote < 1) {
					message = "MCAC: failed to write packet data. ";
					message += strerror(errno);
					stop = true;
				}
			}
		}

		LeaveCriticalSection(&file_section);

		if (stop) {
			PostMessage(message_window, WM_USER_WRITER_ERROR, 0, 0);
		}
	}
}
