#pragma once
#include "tiny.h"

struct ExifReader {

    enum {
        FF  = 0xFF,
        SOI = 0xD8, // Start Of Image
        APP1= 0xE1, // Application Segment 1
        APP2= 0xE2, // Application Segment 2
        DQT = 0xDB, // Define Quantization Table
        DHT = 0xC4, // Define Huffmann Table
        DRI = 0xDD, // Define Restart Interoperability
        SOF = 0xC0, // Start Of Frame
        SOS = 0xDA, // Start Of Scan
        EOI = 0xD9, // End Of Image
        magic = 42
    };

    enum {
        BYTE = 1,
        ASCII = 2,
        SHORT = 3,
        LONG = 4,
        RATIONAL = 5,
        UNDEFINED = 7,
        SLONG = 9,
        SRATIONAL = 10,

        INT = 11,           // SHORT or LONG
    };

    enum {
        /* keys defined by EXIF 2.2 */
        kExifIFDPointer = 0x8769,               // ???
        kGPSInfoIFDPointer = 0x8825,            // ??? Need samples
        kInteroperabilityIFDPointer = 0xA005,   // ???

        // Image data structure
        kImageWidth = 0x100,                    // SHORT or LONG
        kImageLength = 0x101,                   // SHORT or LONG
        kBitsPerSample = 0x102,                 // SHORT * 3
        kCompression = 0x103,                   // SHORT
        kPhotometricInterpretation = 0x106,     // SHORT
        kOrientation = 0x112,                   // SHORT
        kSamplesPerPixel = 0x115,               // SHORT
        kPlanarConfiguration = 0x11C,           // SHORT
        kYCbCrSubSampling = 0x212,              // SHORT * 2
        kYCbCrPositioning = 0x213,              // SHORT
        kXResolution = 0x11A,                   // RATIONAL
        kYResolution = 0x11B,                   // RATIONAL
        kResolutionUnit = 0x128,                // SHORT
        
        // Recording offset
        kStripOffsets = 0x111,                  // SHORT or LONG * S
        kRowsPerStrip = 0x116,                  // SHORT or LONG
        kStripByteCounts = 0x117,               // SHORT or LONG * S
        kJPEGInterchangeFormat = 0x201,         // LONG
        kJPEGInterchangeFormatLength = 0x202,   // LONG

        // Image data characteristics
        kTransferFunction = 0x12D,              // SHORT * 3 * 256
        kWhitePoint = 0x13E,                    // RATIONAL * 2
        kPrimaryChromaticities = 0x13F,         // RATIONAL * 6
        kYCbCrCoefficients = 0x211,             // RATIONAL * 3
        kReferenceBlackWhite = 0x214,           // RATIONAL * 6

        // Other tags
        kDateTime = 0x132,                      // ASCII * 20
        kImageDescription = 0x10E,              // ASCII * Any
        kMake = 0x10F,                          // ASCII * Any
        kModel = 0x110,                         // ASCII * Any
        kSoftware = 0x131,                      // ASCII * Any
        kArtist = 0x13B,                        // ASCII * Any
        kCopyright = 0x8298,                    // ASCII * Any

        // Exif IFD Attribute information
        kExifVersion = 0x9000,                  // UNDEFINED * 4
        kFlashpixVersion = 0xA000,              // UNDEFINED * 4
        kColorSpace = 0xA001,                   // SHORT
        kComponentsConfiguration = 0x9101,      // UNDEFINED * 4
        kCompressedBitsPerPixel = 0x9102,       // RATIONAL
        kPixelXDimension = 0xA002,              // SHORT or LONG
        kPixelYDimension = 0xA003,              // SHORT or LONG
        kMakerNote = 0x927C,                    // UNDEFINED * Any
        kUserComment = 0x9286,                  // UNDEFINED * Any
        kRelatedSoundFile = 0xA004,             // ASCII * 13
        kDateTimeOriginal = 0x9003,             // ASCII * 20
        kDateTimeDigitized = 0x9004,            // ASCII * 20
        kSubSecTime = 0x9290,                   // ASCII * Any
        kSubSecTimeOriginal = 0x9291,           // ASCII * Any
        kSubSecTimeDigitized = 0x9292,          // ASCII * Any
        kImageUniqueID = 0xA420,                // ASCII * 33

        kExposureTime = 0x829A,                 // RATIONAL
        kFNumber = 0x829D,                      // RATIONAL
        kExposureProgram = 0x8822,              // SHORT
        kSpectralSensitivity = 0x8824,          // ASCII * Any
        kISOSpeedRatings = 0x8827,              // ASCII * Any
        kOECF = 0x8828,                         // UNDEFINED * Any
        kShutterSpeedValue = 0x9201,            // SRATIONAL
        kApertureValue = 0x9202,                // RATIONAL
        kBrightnessValue = 0x9203,              // SRATIONAL
        kExposureBiasValue = 0x9204,            // SRATIONAL
        kMaxApertureValue = 0x9205,             // RATIONAL
        kSubjectDistance = 0x9206,              // RATIONAL
        kMeteringMode = 0x9207,                 // SHORT
        kLightSource = 0x9208,                  // SHORT
        kFlash = 0x9209,                        // SHORT
        kFocalLength = 0x920A,                  // RATIONAL
        kSubjectArea = 0x9214,                  // SHORT * [2|3|4]
        kFlashEnergy = 0xA20B,                  // RATIONAL
        kSpatialFrequencyResponse = 0xA20C,     // UNDEFINED * Any
        kFocalPlaneXResolution = 0xA20E,        // RATIONAL
        kFocalPlaneYResolution = 0xA20F,        // RATIONAL
        kFocalPlaneResolutionUnit = 0xA210,     // SHORT
        kSubjectLocation = 0xA214,              // SHORT * 2
        kExposureIndex = 0xA215,                // RATIONAL
        kSensingMethod = 0xA217,                // SHORT
        kFileSource = 0xA300,                   // UNDEFINED
        kSceneType = 0xA301,                    // UNDEFINED
        kCFAPattern = 0xA302,                   // UNDEFINED * Any
        kCustomRendered = 0xA401,               // SHORT
        kExposureMode = 0xA402,                 // SHORT
        kWhiteBalance = 0xA403,                 // SHORT
        kDigitalZoomRatio = 0xA404,             // RATIONAL
        kFocalLengthIn35mmFilm = 0xA405,        // SHORT
        kSceneCaptureType = 0xA406,             // SHORT
        kGainControl = 0xA407,                  // RATIONAL
        kContrast = 0xA408,                     // SHORT
        kSaturation = 0xA409,                   // SHORT
        kSharpness = 0xA40A,                    // SHORT
        kDeviceSettingDescription = 0xA40B,     // UNDEFINED * Any
        kSubjectDistanceRange = 0xA40C,         // SHORT

/*
GPS information has independent tag numbering scheme.
No support for GPS in the first version.

        // GPS Attribute Information
        kGPSVersionID = 0x0000,                 // BYTE * 4
        kGPSLatitudeRef = 0x0001,               // ASCII * 2
        kGPSLatitude = 0x0002,                  // RATIONAL * 3
        kGPSLongitudeRef = 0x0003,              // ASCII * 2
        kGPSLongitude = 0x0004,                 // RATIONAL * 3
        kGPSAltitudeRef = 0x0005,               // BYTE
        kGPSAltitude = 0x0006,                  // RATIONAL
        kGPSTimeStamp = 0x0007,                 // RATIONAL * 3
        kGPSSatellites = 0x0008,                // ASCII * Any
        kGPSStatus = 0x0009,                    // ASCII * 2
        kGPSMeasureMode = 0x000A,               // ASCII * 2
        kGPSDOP = 0x000B,                       // RATIONAL
        kGPSSpeedRef = 0x000C,                  // ASCII * 2
        kGPSSpeed = 0x000D,                     // RATIONAL
        kGPSTrackRef = 0x000E,                  // ASCII * 2
        kGPSTrack = 0x000F,                     // RATIONAL
        kGPSImgDirectionRef = 0x0010,           // ASCII * 2
        kGPSImgDirection = 0x0011,              // RATIONAL
        kGPSMapDatum = 0x0012,                  // ASCII * Any
        kGPSDestLatitudeRef = 0x0013,           // ASCII * 2
        kGPSDestLatitude = 0x0014,              // RATIONAL * 3
        kGPSDestLongitudeRef = 0x0015,          // ASCII * 2
        kGPSDestLongitude = 0x0016,             // RATIONAL * 3
        kGPSDestBearingRef = 0x0017,            // ASCII * 2
        kGPSDestBearing = 0x0018,               // RATIONAL
        kGPSDestDistanceRef = 0x0019,           // ASCII * 2
        kGPSDestDistance = 0x001A,              // RATIONAL
        kGPSProcessingMethod = 0x001B,          // UNDEFINED * Any
        kGPSAreaInformation = 0x001C,           // ANDEFINED * Any
        kGPSDateStamp = 0x001D,                 // ASCII * 11
        kGPSDifferential = 0x001E,              // SHORT
*/
        // Interoperability IFD Attribute Information
        kInteroperabilityIndex = 0x0001,        // ASCII * Any
        kInteroperabilityVersion = 0x0002,      // UNDEFINED * Any
        kRelatedImageFileFormat = 0x1000,       // ??? SHORT
        kRelatedImageWidth = 0x1001,            // ??? SHORT or LONG
        kRelatedImageLength = 0x1002            // ??? SHORT or LONG
    };

    ExifReader() : littleEndian_(false), JIFOffset_(-1), JIFLength_(-1), info(null), blob(null) { }

    bool read(const uchar* filename, HashMap **pInfo, ByteArray **pBlob);

private:
    
    enum {
        fIfdPointer = 1,
        fSpecial = 2
    };

    typedef struct TAG {
        int tag;
        int ifd;        // expected container IFD, 0 works for both 0 and 1
        int type;
        int count;      // expected count, 0 = any
        int flags;
    };

    const TAG *findTag(int tag, int ifd);

    bool parseJpeg(const byte *data, int cb);
    bool parseApp1(const byte *exif, int cb);
    bool parseSof(const byte *sof);
    bool parseIfd(const byte *exif, int cb, const byte* &pch, int ifdId);
    int  readInt(const byte* &src, int len);

    bool littleEndian_;
    int JIFOffset_, JIFLength_;
    const uchar *fn;
    HashMap *info;
    ByteArray *blob;

    static TAG tags[];
};
