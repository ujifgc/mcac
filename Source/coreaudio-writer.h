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

typedef void *AudioConverterPtr;
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
	AudioConverterPtr inAudioConverter,
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

#define kAudioChannelLayoutTag_Unknown 0xFFFF0000
enum
{
	kAudioChannelLayoutTag_UseChannelDescriptions = (0L << 16) | 0,         // use the array of AudioChannelDescriptions to define the mapping.
	kAudioChannelLayoutTag_UseChannelBitmap = (1L << 16) | 0,               // use the bitmap to define the mapping.

	kAudioChannelLayoutTag_Mono = (100L << 16) | 1,                         // a standard mono stream
	kAudioChannelLayoutTag_Stereo = (101L << 16) | 2,                       // a standard stereo stream (L R) - implied playback
	kAudioChannelLayoutTag_StereoHeadphones = (102L << 16) | 2,             // a standard stereo stream (L R) - implied headphone playbac
	kAudioChannelLayoutTag_MatrixStereo = (103L << 16) | 2,                 // a matrix encoded stereo stream (Lt, Rt)
	kAudioChannelLayoutTag_MidSide = (104L << 16) | 2,                      // mid/side recording
	kAudioChannelLayoutTag_XY = (105L << 16) | 2,                           // coincident mic pair (often 2 figure 8's)
	kAudioChannelLayoutTag_Binaural = (106L << 16) | 2,                     // binaural stereo (left, right)
	kAudioChannelLayoutTag_Ambisonic_B_Format = (107L << 16) | 4,           // W, X, Y, Z
	kAudioChannelLayoutTag_Quadraphonic = (108L << 16) | 4,                 // front left, front right, back left, back right
	kAudioChannelLayoutTag_Pentagonal = (109L << 16) | 5,                   // left, right, rear left, rear right, center
	kAudioChannelLayoutTag_Hexagonal = (110L << 16) | 6,                    // left, right, rear left, rear right, center, rear
	kAudioChannelLayoutTag_Octagonal = (111L << 16) | 8,                    // front left, front right, rear left, rear right, front center, rear center, side left, side right
	kAudioChannelLayoutTag_Cube = (112L << 16) | 8,                         // left, right, rear left, rear right, top left, top right, top rear left, top rear right

	//  MPEG defined layouts
	kAudioChannelLayoutTag_MPEG_1_0 = kAudioChannelLayoutTag_Mono,          //  C
	kAudioChannelLayoutTag_MPEG_2_0 = kAudioChannelLayoutTag_Stereo,        //  L R
	kAudioChannelLayoutTag_MPEG_3_0_A = (113L << 16) | 3,                   //  L R C
	kAudioChannelLayoutTag_MPEG_3_0_B = (114L << 16) | 3,                   //  C L R
	kAudioChannelLayoutTag_MPEG_4_0_A = (115L << 16) | 4,                   //  L R C Cs
	kAudioChannelLayoutTag_MPEG_4_0_B = (116L << 16) | 4,                   //  C L R Cs
	kAudioChannelLayoutTag_MPEG_5_0_A = (117L << 16) | 5,                   //  L R C Ls Rs
	kAudioChannelLayoutTag_MPEG_5_0_B = (118L << 16) | 5,                   //  L R Ls Rs C
	kAudioChannelLayoutTag_MPEG_5_0_C = (119L << 16) | 5,                   //  L C R Ls Rs
	kAudioChannelLayoutTag_MPEG_5_0_D = (120L << 16) | 5,                   //  C L R Ls Rs
	kAudioChannelLayoutTag_MPEG_5_1_A = (121L << 16) | 6,                   //  L R C LFE Ls Rs
	kAudioChannelLayoutTag_MPEG_5_1_B = (122L << 16) | 6,                   //  L R Ls Rs C LFE
	kAudioChannelLayoutTag_MPEG_5_1_C = (123L << 16) | 6,                   //  L C R Ls Rs LFE
	kAudioChannelLayoutTag_MPEG_5_1_D = (124L << 16) | 6,                   //  C L R Ls Rs LFE
	kAudioChannelLayoutTag_MPEG_6_1_A = (125L << 16) | 7,                   //  L R C LFE Ls Rs Cs
	kAudioChannelLayoutTag_MPEG_7_1_A = (126L << 16) | 8,                   //  L R C LFE Ls Rs Lc Rc
	kAudioChannelLayoutTag_MPEG_7_1_B = (127L << 16) | 8,                   //  C Lc Rc L R Ls Rs LFE    (doc: IS-13818-7 MPEG2-AAC Table 3.1)
	kAudioChannelLayoutTag_MPEG_7_1_C = (128L << 16) | 8,                   //  L R C LFE Ls Rs Rls Rrs
	kAudioChannelLayoutTag_Emagic_Default_7_1 = (129L << 16) | 8,           //  L R Ls Rs C LFE Lc Rc
	kAudioChannelLayoutTag_SMPTE_DTV = (130L << 16) | 8,                    //  L R C LFE Ls Rs Lt Rt     (kAudioChannelLayoutTag_ITU_5_1 plus a matrix encoded stereo mix)

	//  ITU defined layouts
	kAudioChannelLayoutTag_ITU_1_0 = kAudioChannelLayoutTag_Mono,           //  C
	kAudioChannelLayoutTag_ITU_2_0 = kAudioChannelLayoutTag_Stereo,         //  L R
	kAudioChannelLayoutTag_ITU_2_1 = (131L << 16) | 3,                      //  L R Cs
	kAudioChannelLayoutTag_ITU_2_2 = (132L << 16) | 4,                      //  L R Ls Rs
	kAudioChannelLayoutTag_ITU_3_0 = kAudioChannelLayoutTag_MPEG_3_0_A,     //  L R C
	kAudioChannelLayoutTag_ITU_3_1 = kAudioChannelLayoutTag_MPEG_4_0_A,     //  L R C Cs
	kAudioChannelLayoutTag_ITU_3_2 = kAudioChannelLayoutTag_MPEG_5_0_A,     //  L R C Ls Rs
	kAudioChannelLayoutTag_ITU_3_2_1 = kAudioChannelLayoutTag_MPEG_5_1_A,   //  L R C LFE Ls Rs
	kAudioChannelLayoutTag_ITU_3_4_1 = kAudioChannelLayoutTag_MPEG_7_1_C,   //  L R C LFE Ls Rs Rls Rrs

	// DVD defined layouts
	kAudioChannelLayoutTag_DVD_0 = kAudioChannelLayoutTag_Mono,             // C (mono)
	kAudioChannelLayoutTag_DVD_1 = kAudioChannelLayoutTag_Stereo,           // L R
	kAudioChannelLayoutTag_DVD_2 = kAudioChannelLayoutTag_ITU_2_1,          // L R Cs
	kAudioChannelLayoutTag_DVD_3 = kAudioChannelLayoutTag_ITU_2_2,          // L R Ls Rs
	kAudioChannelLayoutTag_DVD_4 = (133L << 16) | 3,                        // L R LFE
	kAudioChannelLayoutTag_DVD_5 = (134L << 16) | 4,                        // L R LFE Cs
	kAudioChannelLayoutTag_DVD_6 = (135L << 16) | 5,                        // L R LFE Ls Rs
	kAudioChannelLayoutTag_DVD_7 = kAudioChannelLayoutTag_MPEG_3_0_A,       // L R C
	kAudioChannelLayoutTag_DVD_8 = kAudioChannelLayoutTag_MPEG_4_0_A,       // L R C Cs
	kAudioChannelLayoutTag_DVD_9 = kAudioChannelLayoutTag_MPEG_5_0_A,       // L R C Ls Rs
	kAudioChannelLayoutTag_DVD_10 = (136L << 16) | 4,                       // L R C LFE
	kAudioChannelLayoutTag_DVD_11 = (137L << 16) | 5,                       // L R C LFE Cs
	kAudioChannelLayoutTag_DVD_12 = kAudioChannelLayoutTag_MPEG_5_1_A,      // L R C LFE Ls Rs
	// 13 through 17 are duplicates of 8 through 12.
	kAudioChannelLayoutTag_DVD_13 = kAudioChannelLayoutTag_DVD_8,           // L R C Cs
	kAudioChannelLayoutTag_DVD_14 = kAudioChannelLayoutTag_DVD_9,           // L R C Ls Rs
	kAudioChannelLayoutTag_DVD_15 = kAudioChannelLayoutTag_DVD_10,          // L R C LFE
	kAudioChannelLayoutTag_DVD_16 = kAudioChannelLayoutTag_DVD_11,          // L R C LFE Cs
	kAudioChannelLayoutTag_DVD_17 = kAudioChannelLayoutTag_DVD_12,          // L R C LFE Ls Rs
	kAudioChannelLayoutTag_DVD_18 = (138L << 16) | 5,                       // L R Ls Rs LFE
	kAudioChannelLayoutTag_DVD_19 = kAudioChannelLayoutTag_MPEG_5_0_B,      // L R Ls Rs C
	kAudioChannelLayoutTag_DVD_20 = kAudioChannelLayoutTag_MPEG_5_1_B,      // L R Ls Rs C LFE

	// These layouts are recommended for AudioUnit usage
	// These are the symmetrical layouts
	kAudioChannelLayoutTag_AudioUnit_4 = kAudioChannelLayoutTag_Quadraphonic,
	kAudioChannelLayoutTag_AudioUnit_5 = kAudioChannelLayoutTag_Pentagonal,
	kAudioChannelLayoutTag_AudioUnit_6 = kAudioChannelLayoutTag_Hexagonal,
	kAudioChannelLayoutTag_AudioUnit_8 = kAudioChannelLayoutTag_Octagonal,
	// These are the surround-based layouts
	kAudioChannelLayoutTag_AudioUnit_5_0 = kAudioChannelLayoutTag_MPEG_5_0_B,       // L R Ls Rs C
	kAudioChannelLayoutTag_AudioUnit_6_0 = (139L << 16) | 6,                        // L R Ls Rs C Cs
	kAudioChannelLayoutTag_AudioUnit_7_0 = (140L << 16) | 7,                        // L R Ls Rs C Rls Rrs
	kAudioChannelLayoutTag_AudioUnit_5_1 = kAudioChannelLayoutTag_MPEG_5_1_A,       // L R C LFE Ls Rs
	kAudioChannelLayoutTag_AudioUnit_6_1 = kAudioChannelLayoutTag_MPEG_6_1_A,       // L R C LFE Ls Rs Cs
	kAudioChannelLayoutTag_AudioUnit_7_1 = kAudioChannelLayoutTag_MPEG_7_1_C,       // L R C LFE Ls Rs Rls Rrs
	kAudioChannelLayoutTag_AudioUnit_7_1_Front = kAudioChannelLayoutTag_MPEG_7_1_A, // L R C LFE Ls Rs Lc Rc

	kAudioChannelLayoutTag_AAC_3_0 = kAudioChannelLayoutTag_MPEG_3_0_B,             // C L R
	kAudioChannelLayoutTag_AAC_Quadraphonic = kAudioChannelLayoutTag_Quadraphonic,  // L R Ls Rs
	kAudioChannelLayoutTag_AAC_4_0 = kAudioChannelLayoutTag_MPEG_4_0_B,             // C L R Cs
	kAudioChannelLayoutTag_AAC_5_0 = kAudioChannelLayoutTag_MPEG_5_0_D,             // C L R Ls Rs
	kAudioChannelLayoutTag_AAC_5_1 = kAudioChannelLayoutTag_MPEG_5_1_D,             // C L R Ls Rs Lfe
	kAudioChannelLayoutTag_AAC_6_0 = (141L << 16) | 6,                              // C L R Ls Rs Cs
	kAudioChannelLayoutTag_AAC_6_1 = (142L << 16) | 7,                              // C L R Ls Rs Cs Lfe
	kAudioChannelLayoutTag_AAC_7_0 = (143L << 16) | 7,                              // C L R Ls Rs Rls Rrs
	kAudioChannelLayoutTag_AAC_7_1 = kAudioChannelLayoutTag_MPEG_7_1_B,             // C Lc Rc L R Ls Rs Lfe
	kAudioChannelLayoutTag_AAC_Octagonal = (144L << 16) | 8,                        // C L R Ls Rs Rls Rrs Cs

	kAudioChannelLayoutTag_TMH_10_2_std = (145L << 16) | 16,                        // L R C Vhc Lsd Rsd Ls Rs Vhl Vhr Lw Rw Csd Cs LFE1 LFE2
	kAudioChannelLayoutTag_TMH_10_2_full = (146L << 16) | 21,                       // TMH_10_2_std plus: Lc Rc HI VI Haptic

	kAudioChannelLayoutTag_DiscreteInOrder = (147L << 16) | 0,                      // needs to be ORed with the actual number of channels  

	kAudioChannelLayoutTag_AudioUnit_7_0_Front = (148L << 16) | 7,                  // L R Ls Rs C Lc Rc

	kAudioChannelLayoutTag_AC3_1_0_1 = (149L << 16) | 2,                            // C LFE
	kAudioChannelLayoutTag_AC3_3_0 = (150L << 16) | 3,                              // L C R
	kAudioChannelLayoutTag_AC3_3_1 = (151L << 16) | 4,                              // L C R Cs
	kAudioChannelLayoutTag_AC3_3_0_1 = (152L << 16) | 4,                            // L C R LFE
	kAudioChannelLayoutTag_AC3_2_1_1 = (153L << 16) | 4,                            // L R Cs LFE
	kAudioChannelLayoutTag_AC3_3_1_1 = (154L << 16) | 5,                            // L C R Cs LFE
	kAudioChannelLayoutTag_EAC_6_0_A = (155 << 16) | 6,                             // L C R Ls Rs Cs
	kAudioChannelLayoutTag_EAC_7_0_A = (156 << 16) | 7,                             // L C R Ls Rs Rls Rrs
	kAudioChannelLayoutTag_EAC3_6_1_A = (157 << 16) | 7,                            // L C R Ls Rs LFE Cs
	kAudioChannelLayoutTag_EAC3_6_1_B = (158 << 16) | 7,                            // L C R Ls Rs LFE Ts
	kAudioChannelLayoutTag_EAC3_6_1_C = (159 << 16) | 7,                            // L C R Ls Rs LFE Vhc
	kAudioChannelLayoutTag_EAC3_7_1_A = (160 << 16) | 8,                            // L C R Ls Rs LFE Rls Rrs
	kAudioChannelLayoutTag_EAC3_7_1_B = (161 << 16) | 8,                            // L C R Ls Rs LFE Lc Rc
	kAudioChannelLayoutTag_EAC3_7_1_C = (162 << 16) | 8,                            // L C R Ls Rs LFE Lsd Rsd
	kAudioChannelLayoutTag_EAC3_7_1_D = (163 << 16) | 8,                            // L C R Ls Rs LFE Lw Rw
	kAudioChannelLayoutTag_EAC3_7_1_E = (164 << 16) | 8,                            // L C R Ls Rs LFE Vhl Vhr
	kAudioChannelLayoutTag_EAC3_7_1_F = (165 << 16) | 8,                            // L C R Ls Rs LFE Cs Ts
	kAudioChannelLayoutTag_EAC3_7_1_G = (166 << 16) | 8,                            // L C R Ls Rs LFE Cs Vhc
	kAudioChannelLayoutTag_EAC3_7_1_H = (167 << 16) | 8,                            // L C R Ls Rs LFE Ts Vhc

	kAudioChannelLayoutTag_DTS_3_1 = (168 << 16) | 4,                               // C L R LFE
	kAudioChannelLayoutTag_DTS_4_1 = (169 << 16) | 5,                               // C L R Cs LFE
	kAudioChannelLayoutTag_DTS_6_0_A = (170 << 16) | 6,                             // Lc Rc L R Ls Rs
	kAudioChannelLayoutTag_DTS_6_0_B = (171 << 16) | 6,                             // C L R Rls Rrs Ts
	kAudioChannelLayoutTag_DTS_6_0_C = (172 << 16) | 6,                             // C Cs L R Rls Rrs
	kAudioChannelLayoutTag_DTS_6_1_A = (173 << 16) | 7,                             // Lc Rc L R Ls Rs LFE
	kAudioChannelLayoutTag_DTS_6_1_B = (174 << 16) | 7,                             // C L R Rls Rrs Ts LFE
	kAudioChannelLayoutTag_DTS_6_1_C = (175 << 16) | 7,                             // C Cs L R Rls Rrs LFE
	kAudioChannelLayoutTag_DTS_7_0 = (176 << 16) | 7,                               // Lc C Rc L R Ls Rs
	kAudioChannelLayoutTag_DTS_7_1 = (177 << 16) | 8,                               // Lc C Rc L R Ls Rs LFE    
	kAudioChannelLayoutTag_DTS_8_0_A = (178 << 16) | 8,                             // Lc Rc L R Ls Rs Rls Rrs
	kAudioChannelLayoutTag_DTS_8_0_B = (179 << 16) | 8,                             // Lc C Rc L R Ls Cs Rs
	kAudioChannelLayoutTag_DTS_8_1_A = (180 << 16) | 9,                             // Lc Rc L R Ls Rs Rls Rrs LFE
	kAudioChannelLayoutTag_DTS_8_1_B = (181 << 16) | 9,                             // Lc C Rc L R Ls Cs Rs LFE
	kAudioChannelLayoutTag_DTS_6_1_D = (182 << 16) | 7,                             // C L R Ls Rs LFE Cs
	kAudioChannelLayoutTag_AAC_7_1_B = (183 << 16) | 8,                             // C L R Ls Rs Rls Rrs LFE
	kAudioChannelLayoutTag_AAC_7_1_C = (184 << 16) | 8,                             // C L R Ls Rs LFE Vhl Vhr
};

typedef OSStatus (*AudioConverterNew_t)(
	const AudioStreamBasicDescription *inSourceFormat,
	const AudioStreamBasicDescription *inDestinationFormat,
	AudioConverterPtr *outAudioConverter);

typedef OSStatus (*AudioConverterDispose_t)(
	AudioConverterPtr inAudioConverter);

typedef OSStatus (*AudioConverterReset_t)(
	AudioConverterPtr inAudioConverter);

typedef OSStatus (*AudioConverterGetProperty_t)(
	AudioConverterPtr inAudioConverter,
	AudioConverterPropertyID inPropertyID,
	UInt32 *ioPropertyDataSize,
	void *outPropertyData);

typedef OSStatus (*AudioConverterGetPropertyInfo_t)(
	AudioConverterPtr inAudioConverter,
	AudioConverterPropertyID inPropertyID,
	UInt32 *outSize,
	Boolean *outWritable);

typedef OSStatus (*AudioConverterSetProperty_t)(
	AudioConverterPtr inAudioConverter,
	AudioConverterPropertyID inPropertyID,
	UInt32 inPropertyDataSize,
	const void *inPropertyData);

typedef OSStatus (*AudioConverterFillComplexBuffer_t)(
	AudioConverterPtr inAudioConverter,
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
    if (sampleRate == 44100) return 4;
    if (sampleRate == 48000) return 3;

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

	header[0] = 0xFF;

	uint8_t kFieldId = 0;
	uint8_t kMpegLayer = 0;
	uint8_t kProtectionAbsense = 1;
	header[1] = 0xF0 | (kFieldId << 3) | (kMpegLayer << 1) | kProtectionAbsense;

	uint8_t kProfileCode = OMX_AUDIO_AACObjectLC - 1;
	uint8_t kSampleFreqIndex = getSampleRateTableIndex(mSampleRate);
	uint8_t kPrivateStream = 0;
	uint8_t kChannelConfigCode = mChannelCount;
	header[2] = (kProfileCode << 6) | (kSampleFreqIndex << 2) | (kPrivateStream << 1) | (kChannelConfigCode >> 2);

	// 4 bits from originality to copyright start
	uint8_t kCopyright = 0;
	uint32_t kFrameLength = ADTS_PACKET_HEADER_LENGTH + packetLength;
	header[3] = ((kChannelConfigCode & 3) << 6) | (kCopyright << 2) | ((kFrameLength & 0x1800) >> 11);

	header[4] = (uint8_t)((kFrameLength & 0x07F8) >> 3);

	uint32_t kBufferFullness = 0x7FF;  // VBR
	header[5] = ((kFrameLength & 0x07) << 5) | (uint8_t)((kBufferFullness & 0x07C0) >> 6);

	uint8_t kFrameCount = 0;
	header[6] = ((kBufferFullness & 0x03F) << 2) | kFrameCount;

	return header;
}
