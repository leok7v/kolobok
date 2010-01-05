#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <Windows.h>

#define null NULL
#define countof(a) (sizeof(a)/sizeof((a)[0]))
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
    kJIFOffset = 0x201, // Offset to JPEG SOI 
    kJIFLength = 0x202, // Bytes of JPEG data 

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
        base(null),
        max_offset(0),
        ImageWidth_(-1),
        ImageLength_(-1),
        samples_(0),
        Orientation_(-1),
        SamplesPerPixel_(-1),
        JIFOffset_(-1),
        JIFLength_(-1),
        DateTime_(null),
        ImageDescription_(null),
        Make_(null),
        Model_(null),
        Software_(null),
        Artist_(null),
        Copyright_(null),
        clone(null) { 
        memset(BitsPerSample_, 0, sizeof(BitsPerSample_));
    }

    ~CExifThumbnail() { delete clone; }

    bool Extract(const char* filename) {
        delete [] clone;
        HANDLE file = CreateFile(filename, GENERIC_READ, FILE_SHARE_READ, 
            null, OPEN_EXISTING, null, null);
        assert(file != INVALID_HANDLE_VALUE);
        DWORD file_size = GetFileSize(file, null);
        int mapsize = file_size > 64*1024 ? 64*1024 : file_size;
        HANDLE mapping = CreateFileMapping(file, null, PAGE_READONLY/*|SEC_NOCACHE*/,
                                           0, mapsize, null); 
        assert(mapping != null);
        const byte* map = (const byte*)MapViewOfFile(mapping, FILE_MAP_READ, 0, 0, mapsize);
        assert(map != null);
        bool b = ExtractThumbnail((byte*)map, mapsize);
        if (b) {
            clone = new byte[max_offset];
            memcpy(clone, map, max_offset);
            b = ExtractThumbnail(clone, max_offset);
            assert(b);
        }
        UnmapViewOfFile(map);
        CloseHandle(mapping);
        CloseHandle(file);
        map = null;
        return b;
    }

    const byte* GetThumbnail() const { return thumb_; }
    int GetThumbnailSize() const { return JIFLength_; }

private:

    bool ExtractThumbnail(const byte* data, int data_size) {
        base = data;
        const byte* end = data + data_size;
        if (*data++ != 0xFF || *data++ != 0xD8) return false;
        const byte* app1 = null;
        int len = -1;
        while (app1 == null) {
            const byte* header = data;
            data++;
            if (*data == DQT || *data == DHT || *data == SOF || *data == DRI) {
                break;
            }
            data++;
            len = *data++ << 8;
            len |= *data++;
            if (len <= 0) return false;
            len -= 2;
            const byte* chunk = data;
            if (header[0] == 0xFF && header[1] == 0xE1) { // APP1
                app1 = chunk;
            }
            data += len;
        }
        if (app1 == null || len < 6 || memcmp(app1, "Exif\0\0", 6) != 0) return false;
        const byte* exif = app1 + 6;
        const byte* p = exif;
        littleEndian_ = *p++ == 0x49;
        byte next = *p++;
        if (next != p[-2]) return false;
        int magic = readInt(p, 2);
        if (magic != kFourtyTwo) return false;
        int ifdOffset = readInt(p, 4);
        while (ifdOffset != 0 && exif + ifdOffset < end - 20) {
            p = exif + ifdOffset;
            int n = readInt(p, 2);
            readIFD(exif, p, n);
            ifdOffset = readInt(p, 4);
        }
        if (0 < JIFOffset_) {
            thumb_ = exif + JIFOffset_;
            assert(thumb_ + JIFLength_ <= end);
            int offset = thumb_ - base + JIFLength_;
            if (offset > max_offset) max_offset = offset;
            return true;
        }
        return false;
    }

    void setMax(const char* s) {
        int len = strlen(s);
        int offset = s - (const char*)base + len + 1;
        if (offset > max_offset) max_offset = offset;
    }

    void readIFD(const byte* base, const byte* &ifd, int n) {
        const char* exif = (const char*)base;
        for (int i = 0; i < n ; i++) {
            int tag = readInt(ifd, 2);
//printf("0x%02X\n", tag);
            int type = readInt(ifd, 2);
            int count = readInt(ifd, 4);
            int val = readInt(ifd, 4);
            switch (tag) {
                case 0: return;
                case kBitsPerSample: 
                    if (samples_ < 3) BitsPerSample_[samples_++] = val;
                    break;
                case kImageWidth: ImageWidth_ = val; break;
                case kImageLength: ImageLength_ = val; break;
                case kOrientation: Orientation_ = val; break;
                case kSamplesPerPixel: SamplesPerPixel_ = val; break;
                case kJIFOffset: JIFOffset_ = val; break;
                case kJIFLength: JIFLength_ = val; break;
                case kDateTime: setMax(DateTime_ = exif + val); break;
                case kImageDescription: setMax(ImageDescription_ = exif + val); break;
                case kMake: setMax(Make_ = exif + val); break;
                case kModel: setMax(Model_ = exif + val); break;
                case kSoftware: setMax(Software_ = exif + val); break;
                case kArtist: setMax(Artist_ = exif + val); break;
                case kCopyright: setMax(Copyright_ = exif + val); break;
                default: break;
            }
        }
    }

    int readInt(const byte* &src, int len) {
        int r = 0;
        if (littleEndian_) {
            for (int i = 0; i < len; i++) r |= (*src++) << (8 * i);
        }
        else {
            for (int i = 0; i < len; i++) r |= (*src++) << (8 * (len - i - 1));
        }
        return r;
    }

    bool littleEndian_;
    const byte* base;
    int max_offset;
    int ImageWidth_;
    int ImageLength_;
    int samples_;
    int BitsPerSample_[3];
    int Orientation_;
    int SamplesPerPixel_;
    int JIFOffset_;
    int JIFLength_;

    const char* DateTime_; // "2005:12:31 23:59:59"
    const char* ImageDescription_;
    const char* Make_;
    const char* Model_;
    const char* Software_;
    const char* Artist_;
    const char* Copyright_;
    const byte* thumb_;
    byte* clone;
};

const char* tests[] = {
    "Olympus C920Z,D450Z,v873-75.jpg",
    "Fuji Film FinePix S602 Zoom.jpg",
    "canon-powershot-sd300.jpg",
    "olympus-d320l-APP12.jpg",
    "sony-powershota5-CIFF-APP0.jpg",
    "nikon-e950.jpg",
    "canon-ixus.jpg",
    "fujifilm-dx10.jpg",
    "fujifilm-finepix40i.jpg",
    "fujifilm-mx1700.jpg",
    "kodak-dc210.jpg",
    "kodak-dc240.jpg",
    "olympus-c960.jpg",
    "ricoh-rdc5300.jpg",
    "sanyo-vpcg250.jpg",
    "sanyo-vpcsx550.jpg",
    "sony-cybershot.jpg",
    "sony-d700.jpg"
};

int main(int argc, char** argv) {
    if (argc <= 1) return 1;
    for (int i = 0; i < countof(tests); i++) {
        const char* fname = tests[i % countof(tests)];
        CExifThumbnail xth;
        if (!xth.Extract(fname)) {
            printf("failed to read thumb for %s\n", fname);
        }
        else {
            printf("%s OK\n", fname);
            char tname[1024] = {0};
            sprintf(tname, "thumb.%s", fname);
            FILE* out = fopen(tname, "w+b");
            fwrite(xth.GetThumbnail(), xth.GetThumbnailSize(), 1, out);
            fclose(out);
        }
    }
    for (i = 0; i < 1000*10; i++) {
        const char* fname = tests[i % countof(tests)];
        CExifThumbnail xth;
        xth.Extract(fname);
    }
    return 0;
}

/*
  Performance test:

    time 83 sec for 1,000,000 images (same image from file cache)

    KB
    Mem Peak PageFaults VM Size
    724 724  177        240
    748 760  3,000,183  256 (about right for 12KB exif x 1,000,000 / 4K)

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

