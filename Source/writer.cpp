#include "MainComponent.h"
#include "writer.h"

HMODULE coreaudio_library = NULL;
Writer* writer = nullptr;
CRITICAL_SECTION writer_section = { 0 };

Writer::Writer(class AsioDevice *_device) {
	device = _device;

	InitializeCriticalSection(&file_section);

	load_core_audio();

	circlebuf_init(&encode_buffer);
	circlebuf_init(&output_buffer);
}

Writer::~Writer() {
	uninit();

	circlebuf_free(&encode_buffer);
	circlebuf_free(&output_buffer);

	DeleteCriticalSection(&file_section);
}

void Writer::init() {
	init(sample_rate);
}

void Writer::init(double _sample_rate) {
	stop = true;

	int max_input_channels = device->input_channels_number;
	active_channels_count = 0;
	memset(shifted_channel_indexes, -1, sizeof(shifted_channel_indexes));
	for (int i = 0; i < max_input_channels; i += 1) {
		if (active_channels[i]) {
			shifted_channel_indexes[active_channels_count] = i;
			active_channels_count += 1;
		}
	}

	channels = (byte)active_channels_count;
	if (channels == 0) channels = 1;
	if (channels == 7) channels = 8; // coreaudio seems to can't write 7 channels

	sample_rate = (int)_sample_rate;

	AudioStreamBasicDescription in = { 0 }, out = { 0 };

	in.mSampleRate = sample_rate;
	in.mChannelsPerFrame = channels;
	in.mFormatID = kAudioFormatLinearPCM;
	in.mFormatFlags = kAudioFormatFlagIsPacked | kAudioFormatFlagIsFloat;
	in.mBytesPerFrame = channels * BYTES_PER_SAMPLE;
	in.mFramesPerPacket = 1;
	in.mBytesPerPacket = in.mFramesPerPacket * in.mBytesPerFrame;
	in.mBitsPerChannel = BYTES_PER_SAMPLE * 8;

	bytes_per_input_packet = in.mBytesPerPacket;
	
	out.mSampleRate = sample_rate;
	out.mChannelsPerFrame = in.mChannelsPerFrame;
	out.mFormatID = kAudioFormatMPEG4AAC;

	OSStatus error;
	UInt32 size = sizeof(out);

	error = AudioFormatGetProperty(kAudioFormatProperty_FormatInfo, 0, NULL, &size, &out);

	error = AudioConverterNew(&in, &out, &converter);
	if (error) {
		message = (error == 'fmt?') ? "MCAC: unsupported input format" : "MCAC: converter failed";
		PostMessage(message_window, WM_USER_WRITER_ERROR, 0, 0);
		converter = nullptr;
		return;
	}

	String rate_control_name = ReadSettingsString("encoder_rate_control").toLowerCase();
	UInt32 rate_control = (rate_control_name == "cbr") ? kAudioCodecBitRateControlMode_Constant : kAudioCodecBitRateControlMode_Variable;
	error = AudioConverterSetProperty(converter, kAudioCodecPropertyBitRateControlMode, sizeof(rate_control), &rate_control);

	UInt32 converter_quality = kAudioConverterQuality_Max;
	error = AudioConverterSetProperty(converter, kAudioConverterCodecQuality, sizeof(converter_quality), &converter_quality);

	const SInt32 channel_map[9][8] = {
		{ 0, 0, 0, 0, 0, 0, 0, 0 },
		{ 0, 0, 0, 0, 0, 0, 0, 0 },
		{ 0, 1, 0, 0, 0, 0, 0, 0 }, // L R
		{ 2, 0, 1, 0, 0, 0, 0, 0 }, // C L R
		{ 2, 0, 1, 3, 0, 0, 0, 0 }, // C L R C2
		{ 2, 0, 1, 3, 4, 0, 0, 0 }, // C L R L2 R2
		{ 2, 0, 1, 4, 5, 3, 0, 0 }, // C L R L2 R2 C2 L
		{ 2, 0, 1, 6, 7, 4, 5, 0 }, // failing
		{ 2, 0, 1, 6, 7, 4, 5, 3 }, // C L R L2 R2 L3 R3 C2
	};

	error = AudioConverterSetProperty(converter, kAudioConverterChannelMap, channels * sizeof(SInt32), channel_map[channels]);

	error = AudioConverterGetPropertyInfo(converter, kAudioConverterApplicableEncodeBitRates, &size, NULL);
	AudioValueRange* bitrate_infos = (AudioValueRange*)calloc(size, 1);
	if (bitrate_infos) {
		AudioConverterGetProperty(converter, kAudioConverterApplicableEncodeBitRates, &size, bitrate_infos);
		for (int i = 0; i < size / sizeof(AudioValueRange); i += 1) {
			bitrates.add((int)bitrate_infos[i].mMinimum);
		}
		free(bitrate_infos);
	}

	switch (rate_control) {
		case kAudioCodecBitRateControlMode_Constant: {
			String encoder_bitrate = ReadSettingsString("encoder_bitrate");
			UInt32 bitrate = encoder_bitrate.isEmpty() ? DEFAULT_ENCODER_BITRATE : encoder_bitrate.getIntValue();
			if (bitrate < 1000) bitrate *= 1000;
			if (-1 == bitrates.indexOf(bitrate)) {
				int diff = INT_MAX, newdiff = INT_MAX, closest_bitrate = 0;
				for (int i = 0; i < bitrates.size(); i += 1) {
					newdiff = abs(int(bitrates[i] - bitrate));
					if (newdiff <= diff) {
						closest_bitrate = bitrates[i];
						diff = newdiff;
					}
				}
				bitrate = closest_bitrate;
			}
			WriteSettings("encoder_bitrate", String(bitrate));
			error = AudioConverterSetProperty(converter, kAudioCodecPropertyCurrentTargetBitRate, sizeof(bitrate), &bitrate);
			break;
		}
		case kAudioCodecBitRateControlMode_Variable: {
			String encoder_quality = ReadSettingsString("encoder_quality");
			UInt32 quality = encoder_quality.isEmpty() ? DEFAULT_ENCODER_QUALITY : encoder_quality.getIntValue();
			if (quality < 0) quality = 0;
			if (quality > 127) quality = 127;
			WriteSettings("encoder_quality", String(quality));
			error = AudioConverterSetProperty(converter, kAudioCodecPropertySoundQualityForVBR, sizeof(quality), &quality);
			break;
		}
	}

	UInt32 output_buffer_size;
	size = sizeof(output_buffer_size);
	error = AudioConverterGetProperty(converter, kAudioConverterPropertyMaximumOutputPacketSize, &size, &output_buffer_size);
	if (error) output_buffer_size = 32768;
	circlebuf_upsize(&output_buffer, output_buffer_size);

	message = "Ready";
	PostMessage(message_window, WM_USER_WRITER_STATUS, 0, 0);
}

void Writer::uninit() {
	close(false);

	AudioConverterDispose(converter);
}

void Writer::open() {
	if (!converter) {
		message = "MCAC: converter is not ready";
		PostMessage(message_window, WM_USER_WRITER_ERROR, 0, 0);
		stop = true;
	}

	String filename = output_folder_path + "\\output_" + Time::getCurrentTime().formatted("%Y-%m-%d_%H-%M-%S") + ".aac";

	EnterCriticalSection(&file_section);
	file = fopen(filename.toStdString().c_str(), "wb");
	LeaveCriticalSection(&file_section);

	if (!file) {
		message = "MCAC: failed to create output file. ";
		message += strerror(errno);
		PostMessage(message_window, WM_USER_WRITER_ERROR, 0, 0);
	}
	else {
		message = "Recording";
		PostMessage(message_window, WM_USER_WRITER_STATUS, 0, 0);
		stop = false;
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

#define MORE_DATA_REQUIRED 1

OSStatus Writer::input_data_provider(AudioConverterRef inAudioConverter, UInt32* ioNumberDataPackets, AudioBufferList* ioData, AudioStreamPacketDescription** outDataPacketDescription, void* inUserData) {
	Writer* active_writer = (Writer*)inUserData;

	UInt32 bytes_required = (*ioNumberDataPackets) * active_writer->bytes_per_input_packet;

	if (active_writer->input_buffer->size < bytes_required) {
		*ioNumberDataPackets = 0;
		return MORE_DATA_REQUIRED;
	}

	circlebuf_upsize(&active_writer->encode_buffer, bytes_required);

	ioData->mBuffers[0].mNumberChannels = active_writer->channels;
	ioData->mBuffers[0].mDataByteSize = bytes_required;
	ioData->mBuffers[0].mData = circlebuf_data(&active_writer->encode_buffer, 0);

	circlebuf_pop_front(active_writer->input_buffer, ioData->mBuffers[0].mData, bytes_required);

	return 0;

	UNUSED_PARAMETER(inAudioConverter);
	UNUSED_PARAMETER(outDataPacketDescription);
}

void Writer::write_packet(circlebuf* _input_buffer) {
	if (stop) {
		circlebuf_pop_front(_input_buffer, nullptr, _input_buffer->size);
		return;
	}

	input_buffer = _input_buffer;

	UInt32 packets_count = 1;
	AudioBufferList output_buffers = { 0 };
	output_buffers.mNumberBuffers = 1;
	output_buffers.mBuffers[0].mNumberChannels = channels;
	output_buffers.mBuffers[0].mData = circlebuf_data(&output_buffer, 0);
	output_buffers.mBuffers[0].mDataByteSize = (unsigned)output_buffer.size;

	AudioConverterFillComplexBuffer(converter, input_data_provider, this, &packets_count, &output_buffers, NULL);

	size_t items_wrote = 0;

	if (packets_count > 0) {
		EnterCriticalSection(&file_section);

		if (file) {
			items_wrote = fwrite(
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
