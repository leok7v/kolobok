#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <Windows.h>

typedef unsigned char byte;

enum {
    DQT = 0xDB,
    DHT = 0xC4,
    DRI = 0xDD,
    SOF = 0xC0,
    kFourtyTwo = 0x2A,

    kImageWidth = 0x100,
    kImageLength = 0x101,
    kBitsPerSample = 0x102, // x 3
    kOrientation = 0x112,
    kSamplesPerPixel = 0x115,
    kJPEGInterchangeFormat = 0x201, // Offset to JPEG SOI 
    kJPEGInterchangeFormatLength = 0x202, // Bytes of JPEG data 

    kDateTime = 0x132, // File change date and time 
    kImageDescription = 0x10E,
    kMake = 0x10F, // Image input equipment manufacturer
    kModel = 0x110, // Image input equipment model 
    kSoftware = 0x131, // Software used 
    kArtist = 0x13B, // Person who created the image 
    kCopyright = 0x8298 // Copyright holder 
};

class CExifThumbnail {

public:
    CExifThumbnail() : 
        littleEndian_(false),
        ImageWidth_(-1),
        ImageLength_(-1),
        samples_(0),
        Orientation_(-1),
        SamplesPerPixel_(-1),
        JPEGInterchangeFormat_(-1),
        JPEGInterchangeFormatLength_(-1),
        DateTime_(NULL),
        ImageDescription_(NULL),
        Make_(NULL),
        Model_(NULL),
        Software_(NULL),
        Artist_(NULL),
        Copyright_(NULL) { 
    }

    ~CExifThumbnail() { }

    bool extract(const byte* data, int data_size) {
        const byte* jpeg_header = data;
        if (jpeg_header[0] != 0xFF || jpeg_header[1] != 0xD8) return false;
        int len = -1;
        const byte* app1 = NULL;
        int offset = 2;
        data_size -= 2;
        while (app1 == NULL && data_size > 0) {
            const byte* header = data + offset;
            if (header[1] == DQT || header[1] == DHT || 
                header[1] == SOF || header[1] == DRI) {
                break;
            }
            offset += 2;
            data_size -= 2;
            len = data[offset++] * 256;
            len |= data[offset++];
            if (len < 2) return false;
            len -= 2;
            const byte* chunk = data + offset;
            offset += len;
            data_size -= 2 + len;
            if (header[0] == 0xFF && header[1] == 0xE1) { // APP1
                app1 = chunk;
            }
        }
        if (app1 == NULL || len < 6 || memcmp(app1, "Exif\0\0", 6) != 0) return false;
        data_size -= 6;
        offset = 6;
        littleEndian_ = app1[offset++] == 0x49;
        byte next = app1[offset++];
        if (next != app1[offset - 2]) return false;
        int magic = readInt(app1, offset, 2);
        if (magic != kFourtyTwo) return false;
        offset += 2;
        data_size -= 2;
        int ifdOffset = readInt(app1, offset, 4);
        data_size -= 4;
        while (ifdOffset != 0 && ifdOffset < data_size) {
            int n = readInt(app1, 6 + ifdOffset, 2);
            data_size -= 2;
            offset = 6 + ifdOffset + 2;
            readIFD(app1, offset, n);
            offset += n * 12;
            data_size -= n * 12 + 4;
            ifdOffset = readInt(app1, offset, 4);
        }
        if (offset <= JPEGInterchangeFormat_ && 
            JPEGInterchangeFormat_ + JPEGInterchangeFormatLength_ < len) {
            byte* thumb = new byte[JPEGInterchangeFormatLength_];
            if (thumb!= NULL) memcpy(thumb, app1 + JPEGInterchangeFormat_, JPEGInterchangeFormatLength_);
        }
        return true;
    }
    
    bool ExtractThumbnail(const char* filename) {
        HANDLE file = CreateFile(filename, GENERIC_READ, FILE_SHARE_READ, 
            NULL, OPEN_EXISTING, NULL, NULL);
        assert(file != INVALID_HANDLE_VALUE);
        DWORD file_size = GetFileSize(file, NULL);
        int mapsize = file_size > 64*1024 ? 64*1024 : file_size;
        HANDLE mapping = CreateFileMapping(file, NULL, PAGE_READONLY/*|SEC_NOCACHE*/,
                                           0, mapsize, NULL); 
        assert(mapping != NULL);
        void* map = MapViewOfFile(mapping, FILE_MAP_READ, 0, 0, mapsize);
        assert(map != NULL);
        bool b = extract((byte*)map, mapsize);
        UnmapViewOfFile(map);
        CloseHandle(mapping);
        CloseHandle(file);
        return b;
    }
private:

    void readIFD(const byte* ifd, int ifdOffset, int n) {
        const char* app1 = (const char*)ifd + 6;
        int offset = ifdOffset;
        for (int i = 0; i < n ; i++) {
            int tag = readInt(ifd, offset, 2);
            offset += 2;
            // There's no way to know ahead of time how many tags the ExifIFD will contain,
            // but luckily it's always terminated by 8 0's.
printf("tag=0x%02X\n", tag);
            if (tag == 0) break;
            int type = readInt(ifd, offset, 2);
            offset += 2;
            int count = readInt(ifd, offset, 4);
            offset += 4;
            int val = readInt(ifd, offset, 2);
            offset += 4;
            switch (tag) {
                case kBitsPerSample: 
                    if (samples_ < 3) BitsPerSample_[samples_++] = val;
                    break;
                case kImageWidth: ImageWidth_ = val; break;
                case kImageLength: ImageLength_ = val; break;
                case kOrientation: Orientation_ = val; break;
                case kSamplesPerPixel: SamplesPerPixel_ = val; break;
                case kJPEGInterchangeFormat: JPEGInterchangeFormat_ = val; break;
                case kJPEGInterchangeFormatLength: JPEGInterchangeFormatLength_ = val; break;
                case kDateTime: DateTime_ = app1 + val; break;
                case kImageDescription: ImageDescription_ = app1 + val; break;
                case kMake: Make_ = app1 + val; break;
                case kModel: Model_ = app1 + val; break;
                case kSoftware: Software_ = app1 + val; break;
                case kArtist: Artist_ = app1 + val; break;
                case kCopyright: Copyright_ = app1 + val; break;
                default: break;
            }
        }
    }

    int readInt(const byte* src, int offset, int len) {
        int r = 0;
        if (littleEndian_) {
            for (int i = 0; i < len; i++) r |= src[offset + i] << (8 * i);
        }
        else {
            for (int i = 0; i < len; i++) 
                r |= src[offset + i] << (8 * (len - i - 1));
        }
        return r;
    }

    bool littleEndian_;
    int ImageWidth_;
    int ImageLength_;
    int samples_;
    int BitsPerSample_[3];
    int Orientation_;
    int SamplesPerPixel_;
    int JPEGInterchangeFormat_;
    int JPEGInterchangeFormatLength_;

    const char* DateTime_; // "2005:12:31 23:59:59"
    const char* ImageDescription_;
    const char* Make_;
    const char* Model_;
    const char* Software_;
    const char* Artist_;
    const char* Copyright_;
};

int main(int argc, char** argv) {
    if (argc <= 1) return 1;
    CExifThumbnail xth;
    if (xth.ExtractThumbnail("test.jpg")) {
/*
        FILE* out = fopen("C:\\thumb.jpg", "w+b");
        fwrite(xth.GetData(), xth.GetLength(), 1, out);
        fclose(out);
*/
    }
    return 0;
}

/*

  http://www.exif.org/Exif2-2.PDF
    page 22:

Table 3 TIFF Rev. 6.0 Attribute Information Used in Exif
Tag ID  Tag Name    Field Name  Dec Hex     Type Count
    A. Tags relating to image data structure
Image width ImageWidth                          256 100 SHORT or LONG 1
Image height ImageLength                        257 101 SHORT or LONG 1
Number of bits per component BitsPerSample      258 102 SHORT 3
Compression scheme Compression                  259 103 SHORT 1
Pixel composition PhotometricInterpretation     262 106 SHORT 1
Orientation of image Orientation                274 112 SHORT 1
Number of components SamplesPerPixel            277 115 SHORT 1
Image data arrangement PlanarConfiguration      284 11C SHORT 1
Subsampling ratio of Y to C YCbCrSubSampling    530 212 SHORT 2
Y and C positioning YCbCrPositioning            531 213 SHORT 1
Image resolution in width
direction XResolution                           282 11A RATIONAL 1
Image resolution in height
direction YResolution                           283 11B RATIONAL 1
Unit of X and Y resolution ResolutionUnit       296 128 SHORT 1
    B. Tags relating to recording offset
Image data location StripOffsets                273 111 SHORT or LONG *S
Number of rows per strip RowsPerStrip           278 116 SHORT or LONG 1
Bytes per compressed strip StripByteCounts      279 117 SHORT or LONG *S
Offset to JPEG SOI JPEGInterchangeFormat        513 201 LONG 1
Bytes of JPEG data JPEGInterchangeFormatLength  514 202 LONG 1
    C. Tags relating to image data characteristics
Transfer function TransferFunction              301 12D SHORT 3 * 256
White point chromaticity WhitePoint             318 13E RATIONAL 2
Chromaticities of primaries 
PrimaryChromaticities                           319 13F RATIONAL 6
Color space transformation
matrix coefficients YCbCrCoefficients           529 211 RATIONAL 3
Pair of black and white
reference values ReferenceBlackWhite            532 214 RATIONAL 6
    D. Other tags
File change date and time DateTime              306 132 ASCII 20
Image title ImageDescription                    270 10E ASCII Any
Image input equipment
manufacturer Make                               271 10F ASCII Any
Image input equipment model Model               272 110 ASCII Any
Software used Software                          305 131 ASCII Any
Person who created the image Artist             315 13B ASCII Any
Copyright holder Copyright                      33432 8298 ASCII Any

  http://www.exif.org/Exif2-2.PDF
    page 27:

  "Compressed thumbnails shall be recorded in no more than 64 Kbytes, including
all other data to be recorded in APP1."
*/

/*
        FILE* f = fopen(filename, "rb");
        if (f == NULL) return false;
        int len = 0;
        byte* buf = NULL;
        byte jpeg_header[2] = {0};
        if (fread(jpeg_header, 2, 1, f) == 1) {
            if (jpeg_header[0] == 0xFF && jpeg_header[1] == 0xD8) {
                byte header[2] = {0};
                if (fread(header, 2, 1, f) == 1) {
                    for (;;) {
                        if (header[1] == DQT || header[1] == DHT || 
                            header[1] == SOF || header[1] == DRI) {
                            break;
                        }
                        len = fgetc(f);
                        len = len * 256 + fgetc(f);
                        if (len < 2) {
                            break;
                        }
                        buf = new byte[len - 2];
                        if (fread(buf, len - 2, 1, f) != 1) {
                            delete [] buf;
                            buf = NULL;
                            break;
                        }
                        if (header[0] == 0xFF && header[1] == 0xE1) break; // APP1
                        delete buf; buf = NULL;
                        if (fread(header, 2, 1, f) != 1) {
                            break;
                        }
                    }
                }
            }
        }
        fclose(f);
        if (buf == NULL || len < 6 || memcmp(buf, "Exif\0\0", 6) != 0) return false;
        int offset = 6;
        littleEndian = buf[offset++] == 0x49;
        byte next = buf[offset++];
        if (next != buf[offset - 2]) {
            delete [] buf;
            return false;
        }
        int magic = readInt(buf, offset, 2);
        if (magic != kFourtyTwo) {
            delete [] buf;
            return false;
        }
        offset += 2;
        int ifdOffset = readInt(buf, offset, 4);
        int thumbOffset = -1;
        while (ifdOffset != 0) {
            int n = readInt(buf, 6 + ifdOffset, 2);
            offset = 6 + ifdOffset + 2;
            int thumb = readIFD(buf, offset, n);
            if (thumbOffset < 0 && thumb >= 0) {
                thumbOffset = thumb;
            }
            offset += n * 12;
            ifdOffset = readInt(buf, offset, 4);
        }
        if (offset <= thumbOffset && thumbOffset < len) {
            size_ = len - thumbOffset;
            thumb_ = new byte[size_];
            if (thumb_ != NULL) memcpy(thumb_, buf + thumbOffset, size_);
            else 
                size_ = 0;
        }
        delete [] buf;
        return thumb_ != NULL;
    }
*/

