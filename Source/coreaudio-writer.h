#pragma once

typedef unsigned long UInt32;
typedef signed long SInt32;
typedef signed long long SInt64;
typedef double Float64;

typedef SInt32 OSStatus;
typedef unsigned char Boolean;

typedef UInt32 AudioFormatPropertyID;

enum {
    kVariableLengthArray = 1,
};

struct OpaqueAudioConverter;
typedef struct OpaqueAudioConverter *AudioConverterRef;
typedef UInt32 AudioConverterPropertyID;

struct AudioValueRange {
	Float64 mMinimum;
	Float64 mMaximum;
};
typedef struct AudioValueRange AudioValueRange;

struct AudioBuffer {
	UInt32 mNumberChannels;
	UInt32 mDataByteSize;
	void *mData;
};
typedef struct AudioBuffer AudioBuffer;

struct AudioBufferList {
	UInt32 mNumberBuffers;
	AudioBuffer mBuffers[kVariableLengthArray];
};
typedef struct AudioBufferList AudioBufferList;

struct AudioStreamBasicDescription {
	Float64 mSampleRate;
	UInt32 mFormatID;
	UInt32 mFormatFlags;
	UInt32 mBytesPerPacket;
	UInt32 mFramesPerPacket;
	UInt32 mBytesPerFrame;
	UInt32 mChannelsPerFrame;
	UInt32 mBitsPerChannel;
	UInt32 mReserved;
};
typedef struct AudioStreamBasicDescription AudioStreamBasicDescription;

struct AudioStreamPacketDescription {
	SInt64 mStartOffset;
	UInt32 mVariableFramesInPacket;
	UInt32 mDataByteSize;
};
typedef struct AudioStreamPacketDescription AudioStreamPacketDescription;

typedef UInt32 AudioChannelLabel;
typedef UInt32 AudioChannelLayoutTag;

struct AudioChannelDescription {
	AudioChannelLabel mChannelLabel;
	UInt32 mChannelFlags;
	float mCoordinates[3];
};
typedef struct AudioChannelDescription AudioChannelDescription;

struct AudioChannelLayout {
	AudioChannelLayoutTag mChannelLayoutTag;
	UInt32 mChannelBitmap;
	UInt32 mNumberChannelDescriptions;
	AudioChannelDescription mChannelDescriptions[kVariableLengthArray];
};
typedef struct AudioChannelLayout AudioChannelLayout;

typedef OSStatus (*AudioConverterComplexInputDataProc)(
	AudioConverterRef inAudioConverter,
	UInt32 *ioNumberDataPackets,
	AudioBufferList *ioData,
	AudioStreamPacketDescription **outDataPacketDescription,
	void *inUserData);

enum {
	kAudioCodecPropertyNameCFString = 'lnam',
	kAudioCodecPropertyManufacturerCFString = 'lmak',
	kAudioCodecPropertyFormatCFString = 'lfor',
	//kAudioCodecPropertyHasVariablePacketByteSizes          = 'vpk?',
	kAudioCodecPropertySupportedInputFormats = 'ifm#',
	kAudioCodecPropertySupportedOutputFormats = 'ofm#',
	kAudioCodecPropertyAvailableInputSampleRates = 'aisr',
	kAudioCodecPropertyAvailableOutputSampleRates = 'aosr',
	kAudioCodecPropertyAvailableBitRateRange = 'abrt',
	kAudioCodecPropertyMinimumNumberInputPackets = 'mnip',
	kAudioCodecPropertyMinimumNumberOutputPackets = 'mnop',
	kAudioCodecPropertyAvailableNumberChannels = 'cmnc',
	kAudioCodecPropertyDoesSampleRateConversion = 'lmrc',
	kAudioCodecPropertyAvailableInputChannelLayoutTags = 'aicl',
	kAudioCodecPropertyAvailableOutputChannelLayoutTags = 'aocl',
	kAudioCodecPropertyInputFormatsForOutputFormat = 'if4o',
	kAudioCodecPropertyOutputFormatsForInputFormat = 'of4i',
	kAudioCodecPropertyFormatInfo = 'acfi',
};

enum {
	kAudioCodecPropertyInputBufferSize = 'tbuf',
	kAudioCodecPropertyPacketFrameSize = 'pakf',
	kAudioCodecPropertyMaximumPacketByteSize = 'pakb',
	kAudioCodecPropertyCurrentInputFormat = 'ifmt',
	kAudioCodecPropertyCurrentOutputFormat = 'ofmt',
	kAudioCodecPropertyMagicCookie = 'kuki',
	kAudioCodecPropertyUsedInputBufferSize = 'ubuf',
	kAudioCodecPropertyIsInitialized = 'init',
	kAudioCodecPropertyCurrentTargetBitRate = 'brat',
	kAudioCodecPropertyCurrentInputSampleRate = 'cisr',
	kAudioCodecPropertyCurrentOutputSampleRate = 'cosr',
	kAudioCodecPropertyQualitySetting = 'srcq',
	kAudioCodecPropertyApplicableBitRateRange = 'brta',
	kAudioCodecPropertyApplicableInputSampleRates = 'isra',
	kAudioCodecPropertyApplicableOutputSampleRates = 'osra',
	kAudioCodecPropertyPaddedZeros = 'pad0',
	kAudioCodecPropertyPrimeMethod = 'prmm',
	kAudioCodecPropertyPrimeInfo = 'prim',
	kAudioCodecPropertyCurrentInputChannelLayout = 'icl ',
	kAudioCodecPropertyCurrentOutputChannelLayout = 'ocl ',
	kAudioCodecPropertySettings = 'acs ',
	kAudioCodecPropertyFormatList = 'acfl',
	kAudioCodecPropertyBitRateControlMode = 'acbf',
	kAudioCodecPropertySoundQualityForVBR = 'vbrq',
	kAudioCodecPropertyMinimumDelayMode = 'mdel' 
};

enum {
	kAudioCodecBitRateControlMode_Constant = 0,
	kAudioCodecBitRateControlMode_LongTermAverage = 1,
	kAudioCodecBitRateControlMode_VariableConstrained = 2,
	kAudioCodecBitRateControlMode_Variable = 3,
};

enum {
	kAudioFormatLinearPCM = 'lpcm',
	kAudioFormatAC3 = 'ac-3',
	kAudioFormat60958AC3 = 'cac3',
	kAudioFormatAppleIMA4 = 'ima4',
	kAudioFormatMPEG4AAC = 'aac ',
	kAudioFormatMPEG4CELP = 'celp',
	kAudioFormatMPEG4HVXC = 'hvxc',
	kAudioFormatMPEG4TwinVQ = 'twvq',
	kAudioFormatMACE3 = 'MAC3',
	kAudioFormatMACE6 = 'MAC6',
	kAudioFormatULaw = 'ulaw',
	kAudioFormatALaw = 'alaw',
	kAudioFormatQDesign = 'QDMC',
	kAudioFormatQDesign2 = 'QDM2',
	kAudioFormatQUALCOMM = 'Qclp',
	kAudioFormatMPEGLayer1 = '.mp1',
	kAudioFormatMPEGLayer2 = '.mp2',
	kAudioFormatMPEGLayer3 = '.mp3',
	kAudioFormatTimeCode = 'time',
	kAudioFormatMIDIStream = 'midi',
	kAudioFormatParameterValueStream = 'apvs',
	kAudioFormatAppleLossless = 'alac',
	kAudioFormatMPEG4AAC_HE = 'aach',
	kAudioFormatMPEG4AAC_LD = 'aacl',
	kAudioFormatMPEG4AAC_ELD = 'aace',
	kAudioFormatMPEG4AAC_ELD_SBR = 'aacf',
	kAudioFormatMPEG4AAC_ELD_V2 = 'aacg',
	kAudioFormatMPEG4AAC_HE_V2 = 'aacp',
	kAudioFormatMPEG4AAC_Spatial = 'aacs',
	kAudioFormatAMR = 'samr',
	kAudioFormatAudible = 'AUDB',
	kAudioFormatiLBC = 'ilbc',
	kAudioFormatDVIIntelIMA = 0x6D730011,
	kAudioFormatMicrosoftGSM = 0x6D730031,
	kAudioFormatAES3 = 'aes3',
};

enum {
	kAudioFormatFlagIsFloat = (1L << 0),
	kAudioFormatFlagIsBigEndian = (1L << 1),
	kAudioFormatFlagIsSignedInteger = (1L << 2),
	kAudioFormatFlagIsPacked = (1L << 3),
	kAudioFormatFlagIsAlignedHigh = (1L << 4),
	kAudioFormatFlagIsNonInterleaved = (1L << 5),
	kAudioFormatFlagIsNonMixable = (1L << 6),
	kAudioFormatFlagsAreAllClear = (1L << 31),

	kLinearPCMFormatFlagIsFloat = kAudioFormatFlagIsFloat,
	kLinearPCMFormatFlagIsBigEndian = kAudioFormatFlagIsBigEndian,
	kLinearPCMFormatFlagIsSignedInteger = kAudioFormatFlagIsSignedInteger,
	kLinearPCMFormatFlagIsPacked = kAudioFormatFlagIsPacked,
	kLinearPCMFormatFlagIsAlignedHigh = kAudioFormatFlagIsAlignedHigh,
	kLinearPCMFormatFlagIsNonInterleaved = kAudioFormatFlagIsNonInterleaved,
	kLinearPCMFormatFlagIsNonMixable = kAudioFormatFlagIsNonMixable,
	kLinearPCMFormatFlagsAreAllClear = kAudioFormatFlagsAreAllClear,

	kAppleLosslessFormatFlag_16BitSourceData = 1,
	kAppleLosslessFormatFlag_20BitSourceData = 2,
	kAppleLosslessFormatFlag_24BitSourceData = 3,
	kAppleLosslessFormatFlag_32BitSourceData = 4,
};

enum {
	kAudioFormatFlagsNativeEndian = 0
};

enum {
	// AudioStreamBasicDescription structure properties
	kAudioFormatProperty_FormatInfo = 'fmti',
	kAudioFormatProperty_FormatName = 'fnam',
	kAudioFormatProperty_EncodeFormatIDs = 'acof',
	kAudioFormatProperty_DecodeFormatIDs = 'acif',
	kAudioFormatProperty_FormatList = 'flst',
	kAudioFormatProperty_ASBDFromESDS = 'essd',
	kAudioFormatProperty_ChannelLayoutFromESDS = 'escl',
	kAudioFormatProperty_OutputFormatList = 'ofls',
	kAudioFormatProperty_Encoders = 'aven',
	kAudioFormatProperty_Decoders = 'avde',
	kAudioFormatProperty_FormatIsVBR = 'fvbr',
	kAudioFormatProperty_FormatIsExternallyFramed = 'fexf',
	kAudioFormatProperty_AvailableEncodeBitRates = 'aebr',
	kAudioFormatProperty_AvailableEncodeSampleRates = 'aesr',
	kAudioFormatProperty_AvailableEncodeChannelLayoutTags = 'aecl',
	kAudioFormatProperty_AvailableEncodeNumberChannels = 'avnc',
	kAudioFormatProperty_ASBDFromMPEGPacket = 'admp',
	//
	// AudioChannelLayout structure properties
	kAudioFormatProperty_BitmapForLayoutTag = 'bmtg',
	kAudioFormatProperty_MatrixMixMap = 'mmap',
	kAudioFormatProperty_ChannelMap = 'chmp',
	kAudioFormatProperty_NumberOfChannelsForLayout = 'nchm',
	kAudioFormatProperty_ValidateChannelLayout = 'vacl',
	kAudioFormatProperty_ChannelLayoutForTag = 'cmpl',
	kAudioFormatProperty_TagForChannelLayout = 'cmpt',
	kAudioFormatProperty_ChannelLayoutName = 'lonm',
	kAudioFormatProperty_ChannelLayoutSimpleName = 'lsnm',
	kAudioFormatProperty_ChannelLayoutForBitmap = 'cmpb',
	kAudioFormatProperty_ChannelName = 'cnam',
	kAudioFormatProperty_ChannelShortName = 'csnm',
	kAudioFormatProperty_TagsForNumberOfChannels = 'tagc',
	kAudioFormatProperty_PanningMatrix = 'panm',
	kAudioFormatProperty_BalanceFade = 'balf',
	//
	// ID3 tag (MP3 metadata) properties
	kAudioFormatProperty_ID3TagSize = 'id3s',
	kAudioFormatProperty_ID3TagToDictionary = 'id3d',
};

enum {
	kAudioConverterPropertyMinimumInputBufferSize = 'mibs',
	kAudioConverterPropertyMinimumOutputBufferSize = 'mobs',
	kAudioConverterPropertyMaximumInputBufferSize = 'xibs',
	kAudioConverterPropertyMaximumInputPacketSize = 'xips',
	kAudioConverterPropertyMaximumOutputPacketSize = 'xops',
	kAudioConverterPropertyCalculateInputBufferSize = 'cibs',
	kAudioConverterPropertyCalculateOutputBufferSize = 'cobs',
	kAudioConverterPropertyInputCodecParameters = 'icdp',
	kAudioConverterPropertyOutputCodecParameters = 'ocdp',
	kAudioConverterSampleRateConverterAlgorithm = 'srci',
	kAudioConverterSampleRateConverterComplexity = 'srca',
	kAudioConverterSampleRateConverterQuality = 'srcq',
	kAudioConverterSampleRateConverterInitialPhase = 'srcp',
	kAudioConverterCodecQuality = 'cdqu',
	kAudioConverterPrimeMethod = 'prmm',
	kAudioConverterPrimeInfo = 'prim',
	kAudioConverterChannelMap = 'chmp',
	kAudioConverterDecompressionMagicCookie = 'dmgc',
	kAudioConverterCompressionMagicCookie = 'cmgc',
	kAudioConverterEncodeBitRate = 'brat',
	kAudioConverterEncodeAdjustableSampleRate = 'ajsr',
	kAudioConverterInputChannelLayout = 'icl ',
	kAudioConverterOutputChannelLayout = 'ocl ',
	kAudioConverterApplicableEncodeBitRates = 'aebr',
	kAudioConverterAvailableEncodeBitRates = 'vebr',
	kAudioConverterApplicableEncodeSampleRates = 'aesr',
	kAudioConverterAvailableEncodeSampleRates = 'vesr',
	kAudioConverterAvailableEncodeChannelLayoutTags = 'aecl',
	kAudioConverterCurrentOutputStreamDescription = 'acod',
	kAudioConverterCurrentInputStreamDescription = 'acid',
	kAudioConverterPropertySettings = 'acps',
	kAudioConverterPropertyBitDepthHint = 'acbd',
	kAudioConverterPropertyFormatList = 'flst',
};

enum {
	kAudioConverterQuality_Max = 0x7F,
	kAudioConverterQuality_High = 0x60,
	kAudioConverterQuality_Medium = 0x40,
	kAudioConverterQuality_Low = 0x20,
	kAudioConverterQuality_Min = 0,
};

enum {
	kAudio_UnimplementedError = -4,
	kAudio_FileNotFoundError = -43,
	kAudio_FilePermissionError = -54,
	kAudio_TooManyFilesOpenError = -42,
	kAudio_BadFilePathError = '!pth', // 0x21707468, 561017960
	kAudio_ParamError = -50,
	kAudio_MemFullError = -108,

	kAudioConverterErr_FormatNotSupported = 'fmt?',
	kAudioConverterErr_OperationNotSupported = 0x6F703F3F,
	// 'op??', integer used because of trigraph
	kAudioConverterErr_PropertyNotSupported = 'prop',
	kAudioConverterErr_InvalidInputSize = 'insz',
	kAudioConverterErr_InvalidOutputSize = 'otsz',
	// e.g. byte size is not a multiple of the frame size
	kAudioConverterErr_UnspecifiedError = 'what',
	kAudioConverterErr_BadPropertySizeError = '!siz',
	kAudioConverterErr_RequiresPacketDescriptionsError = '!pkd',
	kAudioConverterErr_InputSampleRateOutOfRange = '!isr',
	kAudioConverterErr_OutputSampleRateOutOfRange = '!osr',
};

typedef OSStatus (*AudioConverterNew_t)(
	const AudioStreamBasicDescription *inSourceFormat,
	const AudioStreamBasicDescription *inDestinationFormat,
	AudioConverterRef *outAudioConverter);

typedef OSStatus (*AudioConverterDispose_t)(
	AudioConverterRef inAudioConverter);

typedef OSStatus (*AudioConverterReset_t)(
	AudioConverterRef inAudioConverter);

typedef OSStatus (*AudioConverterGetProperty_t)(
	AudioConverterRef inAudioConverter,
	AudioConverterPropertyID inPropertyID,
	UInt32 *ioPropertyDataSize,
	void *outPropertyData);

typedef OSStatus (*AudioConverterGetPropertyInfo_t)(
	AudioConverterRef inAudioConverter,
	AudioConverterPropertyID inPropertyID,
	UInt32 *outSize,
	Boolean *outWritable);

typedef OSStatus (*AudioConverterSetProperty_t)(
	AudioConverterRef inAudioConverter,
	AudioConverterPropertyID inPropertyID,
	UInt32 inPropertyDataSize,
	const void *inPropertyData);

typedef OSStatus (*AudioConverterFillComplexBuffer_t)(
	AudioConverterRef inAudioConverter,
	AudioConverterComplexInputDataProc inInputDataProc,
	void *inInputDataProcUserData,
	UInt32 *ioOutputDataPacketSize,
	AudioBufferList *outOutputData,
	AudioStreamPacketDescription *outPacketDescription);

typedef OSStatus (*AudioFormatGetProperty_t)(
	AudioFormatPropertyID inPropertyID,
	UInt32 inSpecifierSize,
	const void *inSpecifier,
	UInt32 *ioPropertyDataSize,
	void *outPropertyData);

typedef OSStatus (*AudioFormatGetPropertyInfo_t)(
	AudioFormatPropertyID inPropertyID,
	UInt32 inSpecifierSize,
	const void *inSpecifier,
	UInt32 *outPropertyDataSize);

static AudioConverterNew_t AudioConverterNew = nullptr;
static AudioConverterDispose_t AudioConverterDispose = nullptr;
static AudioConverterReset_t AudioConverterReset = nullptr;
static AudioConverterGetProperty_t AudioConverterGetProperty = nullptr;
static AudioConverterGetPropertyInfo_t AudioConverterGetPropertyInfo = nullptr;
static AudioConverterSetProperty_t AudioConverterSetProperty = nullptr;
static AudioConverterFillComplexBuffer_t AudioConverterFillComplexBuffer = nullptr;
static AudioFormatGetProperty_t AudioFormatGetProperty = nullptr;
static AudioFormatGetPropertyInfo_t AudioFormatGetPropertyInfo = nullptr;

inline uint8_t getSampleRateTableIndex(uint32_t sampleRate) {
    if (sampleRate == 44100) { return 4; }
    if (sampleRate == 48000) { return 3; }

	static const uint32_t kSampleRateTable[] = {
		96000, 88200, 64000, 48000, 44100, 32000,
		24000, 22050, 16000, 12000, 11025, 8000
	};
	const uint8_t tableSize = sizeof(kSampleRateTable) / sizeof(kSampleRateTable[0]);
	for (uint8_t index = 0; index < tableSize; index++) {
		if (sampleRate == kSampleRateTable[index]) return index;
	}

	return 4;
}

#define ADTS_PACKET_HEADER_LENGTH 7
#define OMX_AUDIO_AACObjectLC 2
/*
* ADTS (Audio data transport stream) header structure.
* It consists of 7 or 9 bytes (with or without CRC):
* 12 bits of syncword 0xFFF, all bits must be 1
* 1 bit of field ID. 0 for MPEG-4, and 1 for MPEG-2
* 2 bits of MPEG layer. If in MPEG-TS, set to 0
* 1 bit of protection absense. Set to 1 if no CRC.
* 2 bits of profile code. Set to 1 (The MPEG-4 Audio
*   object type minus 1. We are using AAC-LC = 2)
* 4 bits of sampling frequency index code (15 is not allowed)
* 1 bit of private stream. Set to 0.
* 3 bits of channel configuration code. 0 resevered for inband PCM
* 1 bit of originality. Set to 0.
* 1 bit of home. Set to 0.
* 1 bit of copyrighted steam. Set to 0.
* 1 bit of copyright start. Set to 0.
* 13 bits of frame length. It included 7 or 9 bytes header length.
*   it is set to (protection absense ? 7 : 9) + size(AAC frame)
* 11 bits of buffer fullness. 0x7FF for VBR.
* 2 bits of frames count in one packet. Set to 0.
*/
inline uint8_t* adts_packet_header(uint32_t packetLength, uint32_t mSampleRate, uint8_t mChannelCount) {
	static uint8_t header[ADTS_PACKET_HEADER_LENGTH];

	uint8_t data = 0xFF;
	header[0] = data;

	uint8_t kFieldId = 0;
	uint8_t kMpegLayer = 0;
	uint8_t kProtectionAbsense = 1;
	data = 0xF0;
	data |= (kFieldId << 3);
	data |= (kMpegLayer << 1);
	data |= kProtectionAbsense;
	header[1] = data;

	uint8_t kProfileCode = OMX_AUDIO_AACObjectLC - 1;
	uint8_t kSampleFreqIndex = getSampleRateTableIndex(mSampleRate);
	uint8_t kPrivateStream = 0;
	uint8_t kChannelConfigCode = mChannelCount;
	data = (kProfileCode << 6);
	data |= (kSampleFreqIndex << 2);
	data |= (kPrivateStream << 1);
	data |= (kChannelConfigCode >> 2);
	header[2] = data;

	// 4 bits from originality to copyright start
	uint8_t kCopyright = 0;
	uint32_t kFrameLength = ADTS_PACKET_HEADER_LENGTH + packetLength;
	data = ((kChannelConfigCode & 3) << 6);
	data |= (kCopyright << 2);
	data |= ((kFrameLength & 0x1800) >> 11);
	header[3] = data;

	data = (uint8_t)((kFrameLength & 0x07F8) >> 3);
	header[4] = data;

	uint32_t kBufferFullness = 0x7FF;  // VBR
	data = ((kFrameLength & 0x07) << 5);
	data |= ((kBufferFullness & 0x07C0) >> 6);
	header[5] = data;

	uint8_t kFrameCount = 0;
	data = ((kBufferFullness & 0x03F) << 2);
	data |= kFrameCount;
	header[6] = data;

	return header;
}
