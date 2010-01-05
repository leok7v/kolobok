#include "ExifReader.h"

ExifReader::TAG ExifReader::tags[] = {
    {kExifIFDPointer, 0, LONG, 1, fIfdPointer},
    {kGPSInfoIFDPointer, 0, LONG, 1, fIfdPointer},
    {kInteroperabilityIFDPointer, kExifIFDPointer, LONG, 1, fIfdPointer},

    // Image data structure
    {kImageWidth, 0, INT, 1, 0},
    {kImageLength, 0, INT, 1, 0},
    {kBitsPerSample, 0, SHORT, 3, 0},
    {kCompression, 0, SHORT, 1, 0},
    {kPhotometricInterpretation, 0, SHORT, 1, 0},
    {kOrientation, 0, SHORT, 1, 0},
    {kSamplesPerPixel, 0, SHORT, 1, 0},
    {kPlanarConfiguration, 0, SHORT, 1, 0},
    {kYCbCrSubSampling, 0, SHORT, 2, 0},
    {kYCbCrPositioning, 0, SHORT, 1, 0},
    {kXResolution, 0, RATIONAL, 1, 0},
    {kYResolution, 0, RATIONAL, 1, 0},
    {kResolutionUnit, 0, SHORT, 1, 0},
    
    // Recording offset
    {kStripOffsets, 0, INT, 0, 0},
    {kRowsPerStrip, 0, INT, 1, 0},
    {kStripByteCounts, 0, INT, 0, 0},
    {kJPEGInterchangeFormat, 0, LONG, 1, fSpecial},
    {kJPEGInterchangeFormatLength, 0, LONG, 1, fSpecial},

    // Image data characteristics
    {kTransferFunction, 0, SHORT, 3*256, 0},
    {kWhitePoint, 0, RATIONAL, 2, 0},
    {kPrimaryChromaticities, 0, RATIONAL, 6, 0},
    {kYCbCrCoefficients, 0, RATIONAL, 3, 0},
    {kReferenceBlackWhite, 0, RATIONAL, 6, 0},

    // Other tags
    {kDateTime, 0, ASCII, 20, 0},
    {kImageDescription, 0, ASCII, 0, 0},
    {kMake, 0, ASCII, 0, 0},
    {kModel, 0, ASCII, 0, 0},
    {kSoftware, 0, ASCII, 0, 0},
    {kArtist, 0, ASCII, 0, 0},
    {kCopyright, 0, ASCII, 0, 0},

    // Exif IFD Attribute information
    {kExifVersion, kExifIFDPointer, UNDEFINED, 4, 0},
    {kFlashpixVersion, kExifIFDPointer, UNDEFINED, 4, 0},
    {kColorSpace, kExifIFDPointer, SHORT, 1, 0},
    {kComponentsConfiguration, kExifIFDPointer, UNDEFINED, 4, 0},
    {kCompressedBitsPerPixel, kExifIFDPointer, RATIONAL, 1, 0},
    {kPixelXDimension, kExifIFDPointer, INT, 1, 0},
    {kPixelYDimension, kExifIFDPointer, INT, 1, 0},
    {kMakerNote, kExifIFDPointer, UNDEFINED, 0, 0},
    {kUserComment, kExifIFDPointer, UNDEFINED, 0, 0},
    {kRelatedSoundFile, kExifIFDPointer, ASCII, 13, 0},
    {kDateTimeOriginal, kExifIFDPointer, ASCII, 20, 0},
    {kDateTimeDigitized, kExifIFDPointer, ASCII, 20, 0},
    {kSubSecTime, kExifIFDPointer, ASCII, 0, 0},
    {kSubSecTimeOriginal, kExifIFDPointer, ASCII, 0, 0},
    {kSubSecTimeDigitized, kExifIFDPointer, ASCII, 0, 0},
    {kImageUniqueID, kExifIFDPointer, ASCII, 33, 0},

    {kExposureTime, kExifIFDPointer, RATIONAL, 1, 0},
    {kFNumber, kExifIFDPointer, RATIONAL, 1, 0},
    {kExposureProgram, kExifIFDPointer, SHORT, 1, 0},
    {kSpectralSensitivity, kExifIFDPointer, ASCII, 0, 0},
    {kISOSpeedRatings, kExifIFDPointer, SHORT, 0, 0},
    {kOECF, kExifIFDPointer, UNDEFINED, 0, 0},
    {kShutterSpeedValue, kExifIFDPointer, SRATIONAL, 1, 0},
    {kApertureValue, kExifIFDPointer, RATIONAL, 1, 0},
    {kBrightnessValue, kExifIFDPointer, SRATIONAL, 1, 0},
    {kExposureBiasValue, kExifIFDPointer, SRATIONAL, 1, 0},
    {kMaxApertureValue, kExifIFDPointer, RATIONAL, 1, 0},
    {kSubjectDistance, kExifIFDPointer, RATIONAL, 1, 0},
    {kMeteringMode, kExifIFDPointer, SHORT, 1, 0},
    {kLightSource, kExifIFDPointer, SHORT, 1, 0},
    {kFlash, kExifIFDPointer, SHORT, 1, 0},
    {kFocalLength, kExifIFDPointer, RATIONAL, 1, 0},
    {kSubjectArea, kExifIFDPointer, SHORT, 2, fSpecial},
    {kFlashEnergy, kExifIFDPointer, RATIONAL, 1, 0},
    {kSpatialFrequencyResponse, kExifIFDPointer, UNDEFINED, 0, 0},
    {kFocalPlaneXResolution, kExifIFDPointer, RATIONAL, 1, 0},
    {kFocalPlaneYResolution, kExifIFDPointer, RATIONAL, 1, 0},
    {kFocalPlaneResolutionUnit, kExifIFDPointer, SHORT, 1, 0},
    {kSubjectLocation, kExifIFDPointer, SHORT, 2, 0},
    {kExposureIndex, kExifIFDPointer, RATIONAL, 1, 0},
    {kSensingMethod, kExifIFDPointer, SHORT, 1, 0},
    {kFileSource, kExifIFDPointer, UNDEFINED, 1, 0},
    {kSceneType, kExifIFDPointer, UNDEFINED, 1, 0},
    {kCFAPattern, kExifIFDPointer, UNDEFINED, 0, 0},
    {kCustomRendered, kExifIFDPointer, SHORT, 1, 0},
    {kExposureMode, kExifIFDPointer, SHORT, 1, 0},
    {kWhiteBalance, kExifIFDPointer, SHORT, 1, 0},
    {kDigitalZoomRatio, kExifIFDPointer, RATIONAL, 1, 0},
    {kFocalLengthIn35mmFilm, kExifIFDPointer, SHORT, 1, 0},
    {kSceneCaptureType, kExifIFDPointer, SHORT, 1, 0},
    {kGainControl, kExifIFDPointer, RATIONAL, 1, 0},
    {kContrast, kExifIFDPointer, SHORT, 1, 0},
    {kSaturation, kExifIFDPointer, SHORT, 1, 0},
    {kSharpness, kExifIFDPointer, SHORT, 1, 0},
    {kDeviceSettingDescription, kExifIFDPointer, UNDEFINED, 0, 0},
    {kSubjectDistanceRange, kExifIFDPointer, SHORT, 1, 0},

/*
    // GPS Attribute Information
    {kGPSVersionID, kGPSInfoIFDPointer, BYTE, 4, 0},
    {kGPSLatitudeRef, kGPSInfoIFDPointer, ASCII, 2, 0},
    {kGPSLatitude, kGPSInfoIFDPointer, RATIONAL, 3, 0},
    {kGPSLongitudeRef, kGPSInfoIFDPointer, ASCII, 2, 0},
    {kGPSLongitude, kGPSInfoIFDPointer, RATIONAL, 3, 0},
    {kGPSAltitudeRef, kGPSInfoIFDPointer, BYTE, 1, 0},
    {kGPSAltitude, kGPSInfoIFDPointer, RATIONAL, 1, 0},
    {kGPSTimeStamp, kGPSInfoIFDPointer, RATIONAL, 3, 0},
    {kGPSSatellites, kGPSInfoIFDPointer, ASCII, 0, 0},
    {kGPSStatus, kGPSInfoIFDPointer, ASCII, 2, 0},
    {kGPSMeasureMode, kGPSInfoIFDPointer, ASCII, 2, 0},
    {kGPSDOP, kGPSInfoIFDPointer, RATIONAL, 1, 0},
    {kGPSSpeedRef, kGPSInfoIFDPointer, ASCII, 2, 0},
    {kGPSSpeed, kGPSInfoIFDPointer, RATIONAL, 1, 0},
    {kGPSTrackRef, kGPSInfoIFDPointer, ASCII, 2, 0},
    {kGPSTrack, kGPSInfoIFDPointer, RATIONAL, 1, 0},
    {kGPSImgDirectionRef, kGPSInfoIFDPointer, ASCII, 2, 0},
    {kGPSImgDirection, kGPSInfoIFDPointer, RATIONAL, 1, 0},
    {kGPSMapDatum, kGPSInfoIFDPointer, ASCII, 0, 0},
    {kGPSDestLatitudeRef, kGPSInfoIFDPointer, ASCII, 2, 0},
    {kGPSDestLatitude, kGPSInfoIFDPointer, RATIONAL, 3, 0},
    {kGPSDestLongitudeRef, kGPSInfoIFDPointer, ASCII, 2, 0},
    {kGPSDestLongitude, kGPSInfoIFDPointer, RATIONAL, 3, 0},
    {kGPSDestBearingRef, kGPSInfoIFDPointer, ASCII, 2, 0},
    {kGPSDestBearing, kGPSInfoIFDPointer, RATIONAL, 1, 0},
    {kGPSDestDistanceRef, kGPSInfoIFDPointer, ASCII, 2, 0},
    {kGPSDestDistance, kGPSInfoIFDPointer, RATIONAL, 1, 0},
    {kGPSProcessingMethod, kGPSInfoIFDPointer, UNDEFINED, 0, 0},
    {kGPSAreaInformation, kGPSInfoIFDPointer, UNDEFINED, 0, 0},
    {kGPSDateStamp, kGPSInfoIFDPointer, ASCII, 11, 0},
    {kGPSDifferential, kGPSInfoIFDPointer, SHORT, 1, 0},
*/
    // Interoperability IFD Attribute Information
    {kInteroperabilityIndex, kInteroperabilityIFDPointer, ASCII, 0, 0},
    {kInteroperabilityVersion, kInteroperabilityIFDPointer, UNDEFINED, 4, 0},
    {kRelatedImageFileFormat, kInteroperabilityIFDPointer, SHORT, 0, 0},
    {kRelatedImageWidth, kInteroperabilityIFDPointer, INT, 1, 0},
    {kRelatedImageLength, kInteroperabilityIFDPointer, INT, 1, 0}
};


// This can be optimized if need be
const ExifReader::TAG* ExifReader::findTag(int tag, int ifd) {
    if (ifd == 1) ifd = 0;
    for (int i = 0; i < countof(tags); ++i) {
        if (tags[i].tag == tag && tags[i].ifd == ifd)
            return &tags[i];
    }
    return null;
}

bool ExifReader::read(const uchar* filename, HashMap **pInfo, ByteArray **pBlob) {
    fn = filename;
    info = new HashMap();
    blob = null;

    HANDLE hf = CreateFile(filename, GENERIC_READ, FILE_SHARE_READ, null, OPEN_EXISTING, null, null);
    assert(hf != INVALID_HANDLE_VALUE && hf != NULL);
    DWORD cb = GetFileSize(hf, null);
    int mapsize = cb > 64*1024 ? 64*1024 : cb;
    HANDLE mapping = CreateFileMapping(hf, null, PAGE_READONLY/*|SEC_NOCACHE*/, 0, mapsize, null); 
    assert(mapping != null);
    const byte* map = (const byte*)MapViewOfFile(mapping, FILE_MAP_READ, 0, 0, mapsize);
    assert(map != null);

    bool b = parseJpeg(map, mapsize);

    fn = null;
    UnmapViewOfFile(map);
    CloseHandle(mapping);
    CloseHandle(hf);
    if (blob == null) b = false;
    if (b) { *pInfo = info; *pBlob = blob; } else { delete info; delete blob; }
    return b;
}

bool ExifReader::parseJpeg(const byte *data, int cb) {
    // SOI
    if (cb < 2 || data[0] != FF || data[1] != SOI) return false;

    const byte *pb = data, *end = data + cb;
    const byte *app1 = null, *sof = null;
    int cb_app1 = 0;
    while (pb + 2 <= end && (app1 == null || sof == null)) {
        if (pb[0] != FF) return false;
        if (pb[1] == SOI) {
            pb += 2;
            continue;
        }
        if (pb[1] == EOI)
            break;
        if (pb + 4 > end)
            return false;
        int len = ((pb[2] << 8) | pb[3]);
        if (pb[1] == APP1) {
            app1 = pb + 4;
            cb_app1 = len;
        }
        else if (pb[1] == SOF) {
            sof = pb + 4;
        }
        pb = pb + 2 + len;
    }
    // Note: missing APP1 is not an error, some metadata can be salvaged from SOF
    if (app1 != null && cb_app1 >= 6 && memcmp(app1, "Exif\0\0", 6) == 0 && !parseApp1(app1 + 6, cb_app1 - 6))
        return false;
    return sof != null &&  parseSof(sof);
}

bool ExifReader::parseApp1(const byte *exif, int cb) {
    const byte *pch = exif;
    if (pch[0] != pch[1])
        return false;
    littleEndian_ = (pch[0] == 0x49);    pch += 2;
    if (readInt(pch, 2) != magic)
        return false;
    int offset = readInt(pch, 4);
    if (offset == 0 || offset + 2 > cb)
        return false;
    pch = exif + offset;
    if (!parseIfd(exif, cb, pch, 0))
        return false;
    // optional thumbnail IFD may follow
    if (exif + cb - pch < 4)
        return true;
    offset = readInt(pch, 4);
    if (offset == 0 || offset + 2 > cb)
        return true;
    pch = exif + offset;
    if (!parseIfd(exif, cb, pch, 1))
        return false;
    if (JIFOffset_ > 0 && JIFLength_ > 0)
        blob = new ByteArray(exif + JIFOffset_, JIFLength_);
    return true;
}

bool ExifReader::parseSof(const byte *sof) {
    // Not just yet
    return true;
}

bool ExifReader::parseIfd(const byte *exif, int cb, const byte* &pch, int ifdId) {
    int n = readInt(pch, 2);
    for (int i = 0; i < n ; i++) {
        if (exif + cb - pch < 12)
            return false;
        int tag   = readInt(pch, 2);
        int type  = readInt(pch, 2);
        int count = readInt(pch, 4);
        int val   = readInt(pch, 4);

        const TAG *ts = findTag(tag, ifdId);
        if (ts == null) {
            traceln(L"EXIF: unknown tag 0x%04X (type=%d,count=%d,val=%08X) in IFD 0x%04X of image %s", tag, type, count, val, ifdId, fn);
        }
        else {
            if ((ts->flags & fIfdPointer) != 0) {
                if (val <= 0 || val + 2 > cb)
                    return false;   // illegal pointer to outside of APP1
                const byte *ifdData = exif + val;
                if (!parseIfd(exif, cb, ifdData, tag))
                    return false;   // mis-formatted nested IFD
            }
            else if ((ts->flags * fSpecial) != 0) {
                switch(tag) {
                case kJPEGInterchangeFormat: JIFOffset_ = val; break;
                case kJPEGInterchangeFormatLength: JIFLength_ = val; break;
                default:
                    traceln(L"EXIF: dropping fSpecial tag %04X in ifd %04X in image %s", tag, ifdId, fn);
                    break;
                }
            }
            else {
                if (ts->type == type || (ts->type == INT && (type == SHORT || type == LONG))) {
                    switch (type) {
                    case BYTE:
                    case UNDEFINED:
                        if (count <= 4) {
                            info->put(tag, Int(val));
                        }
                        else {
                            const byte *pb = (const byte *)(exif + val);
                            info->put(tag, ByteArray(pb, count));
                        }
                        break;
                    case ASCII:
                        if (count <= 4) {
                            const char *buf = (const char *)&val;
                            info->put(tag, String(buf));
                        }
                        else {
                            const char *buf = (const char *)(exif + val);
                            info->put(tag, String(buf));
                        }
                        break;
                    case SHORT:
                        if (count == 1) {
                            info->put(tag, (int)(word)val);
                        }
                        else {
                            const byte* pb = count == 2 ? (pch - 4) : exif + val;
                            if (exif + cb < pb + count * 2)
                                return false;
                            ArrayList *al = new ArrayList();
                            for (int i = 0; i < count; ++i)
                                al->add(Int(readInt(pb, 2)));
                            info->put(tag, al);
                            delete al;
                        }
                        break;
                    case LONG:
                        if (count == 1) {
                            info->put(tag, (longlong)(dword)val);
                        }
                        else {
                            const byte* pb = exif + val;
                            if (exif + cb < pb + count * 4)
                                return false;
                            ArrayList *al = new ArrayList();
                            for (int i = 0; i < count; ++i)
                                al->add(Long((dword)readInt(pb, 4)));
                            info->put(tag, al);
                            delete al;
                        }
                        break;
                    case SLONG:
                        if (count == 1) {
                            info->put(tag, val);
                        }
                        else {
                            const byte* pb = exif + val;
                            if (exif + cb < pb + count * 4)
                                return false;
                            ArrayList *al = new ArrayList();
                            for (int i = 0; i < count; ++i)
                                al->add(Int(readInt(pb, 4)));
                            info->put(tag, al);
                            delete al;
                        }
                        break;
                    case SRATIONAL:
                        if (count == 1) {
                            if (val + 8 > cb)
                                return false;
                            const byte *pb = exif + val;
                            int num = readInt(pb, 4);
                            int den = readInt(pb, 4);
                            info->put(tag, Rational(num, den));
                        }
                        else {
                            const byte* pb = exif + val;
                            if (exif + cb < pb + count * 8)
                                return false;
                            ArrayList *al = new ArrayList();
                            for (int i = 0; i < count; ++i) {
                                int num = readInt(pb, 4);
                                int den = readInt(pb, 4);
                                al->add(Rational(num, den));
                            }
                            info->put(tag, al);
                            delete al;
                        }
                        break;
                    case RATIONAL:
                        if (count == 1) {
                            if (val + 8 > cb)
                                return false;
                            const byte *pb = exif + val;
                            dword num = (dword)readInt(pb, 4);
                            dword den = (dword)readInt(pb, 4);
                            info->put(tag, Rational(num, den));
                        }
                        else {
                            const byte* pb = exif + val;
                            if (exif + cb < pb + count * 8)
                                return false;
                            ArrayList *al = new ArrayList();
                            for (int i = 0; i < count; ++i) {
                                dword num = (dword)readInt(pb, 4);
                                dword den = (dword)readInt(pb, 4);
                                al->add(Rational(num, den));
                            }
                            info->put(tag, al);
                            delete al;
                        }
                        break;
                    default:
                        traceln(L"*** EXIF: unknown tag type %d in tag %04X in ifd %04X in image %s", type, tag, ifdId, fn);
                    }
                }
                else {
                    traceln(L"EXIF: type mismatch for tag 0x%04X (type=%d) in image %s", tag, type, fn);
                }
            }
        }
    }
    return true;
}

int ExifReader::readInt(const byte* &src, int len) {
    int r = 0;
    if (littleEndian_) {
        for (int i = 0; i < len; i++) r |= (*src++) << (8 * i);
    }
    else {
        for (int i = 0; i < len; i++) r |= (*src++) << (8 * (len - i - 1));
    }
    return r;
}
