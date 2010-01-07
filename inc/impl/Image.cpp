#include "tiny.h"
extern "C" {
#include "png.h"
#define  XMD_H
#undef   FAR
#include "jerror.h"
#include "jpeglib.h"
}

static int calcBytesPerLine(int nWidth, int nBitsPerPixel) {
    return ( (nWidth * nBitsPerPixel + 31) & (~31) ) / 8;
}

static BITMAPV4HEADER bh0 = { sizeof(BITMAPV4HEADER) };

static void guid(Image& i) {
    String* g = String::guid();
    i.getProperties().put(L"guid", g);
    delete g;
}

Image::Image() : 
    hdc(null), getCount(0), error(null), file(null), bits(null), bitmap(null), 
    compression(0), bytesPerLine(0), unselect(null) {
    bi = bh0;
    guid(*this);
}

Image::Image(const uchar* fname) : 
    hdc(null), getCount(0), error(null), file(null), bits(null), bitmap(null),
    compression(0), bytesPerLine(0), unselect(null)  {
    bi = bh0;
    guid(*this);
    int w = 0, h = 0;
    check(load(fname, w, h));
}

Image::Image(InputStream* s) : 
    hdc(null), getCount(0), error(null), file(null), bits(null), bitmap(null), 
    compression(0), bytesPerLine(0), unselect(null)  {
    bi = bh0;
    guid(*this);
    int w, h;
    check(load(s, w, h));
}

Image::Image(int w, int h, int bits) : 
    hdc(null), getCount(0), error(null), file(null), bits(null), bitmap(null), 
    compression(0), bytesPerLine(0), unselect(null)  {
    guid(*this);
    bi = bh0;
    check(create(w, h, bits));
}

Image::~Image() {
    if (bitmap != null) DeleteObject(bitmap);
    delete error;
    delete file;
    assert(hdc == null);
    assert(unselect == null);
    assert(getCount == 0);
}

bool Image::create(int w, int h, int bitcount) {
    assert(w > 0 && h > 0);
    bi.bV4Size = sizeof(BITMAPV4HEADER);
    bi.bV4Width = w;
    bi.bV4Height = h;
    bi.bV4Planes = 1;
    bi.bV4BitCount = bitcount;
    bi.bV4V4Compression = BI_BITFIELDS;
    bi.bV4RedMask = 0x00FF0000;
    bi.bV4GreenMask = 0x0000FF00;
    bi.bV4BlueMask = 0x000000FF;
    bi.bV4AlphaMask = 0xFF000000;
    HDC hdc = GetDC(::GetDesktopWindow());
    bitmap = CreateDIBSection(hdc, (BITMAPINFO*)&bi, DIB_RGB_COLORS, (void**)&bits, null, 0);
    ReleaseDC(GetDesktopWindow(), hdc);
    bytesPerLine = calcBytesPerLine(bi.bV4Width, bi.bV4BitCount);
    if (bi.bV4SizeImage == 0) {
        bi.bV4SizeImage = bytesPerLine * bi.bV4Height;
    }
    return bitmap != null;
}

/* Verified AlphaBlend on both ATI Mobility Radeon 9600 and NVIDIA GeForce 6600 
   AlphaBlen failes with with < 8 when blending on device context surface.
   Workaround is simple but annoying.
 */

static void alphaBlend(HDC hdc, int dx, int dy, int dw, int dh, 
                       HDC mem, int sx, int sy, int sw, int sh) {
    BLENDFUNCTION bf = {AC_SRC_OVER, 0, 0xFF, AC_SRC_ALPHA};
    check(AlphaBlend(hdc, dx, dy, dw, dh, mem, sx, sy, sw, sh, bf));
}

void Image::alphaBlend(HDC hdc, int dx, int dy, int dw, int dh, 
                                int sx, int sy, int sw, int sh) const {
    if (dw < 8) {
        Image g(8, dh, 32);
        alphaBlend(g, 0, 0, dw, dh, sx, sy, sw, sh);
        HDC mem = g.getDC();
        ::alphaBlend(hdc, dx, dy, 8, dh, mem, 0, 0, 8, dh);
        g.releaseDC();
    }
    else {
        HDC mem = ((Image*)this)->getDC();
        ::alphaBlend(hdc, dx, dy, dw, dh, mem, sx, sy, sw, sh);
        ((Image*)this)->releaseDC();
    }
}

void Image::alphaBlend(Image& dst, int dx, int dy, int dw, int dh,
                                   int sx, int sy, int sw, int sh) const {
    HDC mdc = ((Image*)this)->getDC();
    HDC dc = dst.getDC();
    ::alphaBlend(dc, dx, dy, dw, dh, mdc, sx, sy, sw, sh);
    dst.releaseDC();
    ((Image*)this)->releaseDC();
}

void Image::bitBlt(HDC hdc, int dx, int dy, int dw, int dh, 
                             int sx, int sy, int sw, int sh, int op) const {
    HDC mem = ((Image*)this)->getDC();
    if (dw == sw && dh == sh) 
        check(BitBlt(hdc, dx, dy, dw, dh, mem, sx, sy, op));
    else {
        int mode = SetStretchBltMode(hdc, HALFTONE);
        check(StretchBlt(hdc, dx, dy, dw, dh, mem, sx, sy, sw, sh, op));
        SetStretchBltMode(hdc, mode);
    }
    ((Image*)this)->releaseDC();
}

void Image::bitBlt(Image& dst, int dx, int dy, int dw, int dh,
                    int sx, int sy, int sw, int sh, int op) const {
    HDC dc = dst.getDC();
    bitBlt(dc, dx, dy, dw, dh, sx, sy, sw, sh, op);
    dst.releaseDC();
}

HDC Image::getDC() {
    if (hdc != null) { getCount++; return hdc; }
    assert(unselect == null);
    hdc = CreateCompatibleDC(null);
    SetStretchBltMode(hdc, HALFTONE);
    unselect = (HBITMAP)SelectObject(hdc, bitmap);
    assert(unselect != null);
    getCount = 1;
    return hdc;
}

void Image::releaseDC() {
    assert(getCount > 0);
    if (--getCount == 0) {
        assert(unselect != null);
        HBITMAP b = (HBITMAP)SelectObject(hdc, unselect);
        assert(b == bitmap);
        unused(b);
        unselect = null;
        DeleteDC(hdc);
        hdc = null;
    }
}

Image* Image::clone(const Rect& rc) const {
    if (rc.left < 0 || rc.right > getWidth() ||
        rc.top < 0 || rc.bottom > getHeight())
        return NULL;
    assert(bi.bV4BitCount == 32); // TODO: implement RGB and BW
    Image* i = new Image(rc.getWidth(), rc.getHeight(), bi.bV4BitCount);
    for (int y = rc.top; y < rc.bottom; y++) {
        int* s = &((int*)getLine(y))[rc.left];
        int* d = (int*)i->getLine(y - rc.top);
        int n = rc.right - rc.left;
        while (n-- > 0) *d++ = *s++;
    }
    return i;
}

Object* Image::clone() const {
    Rect rc(0, 0, getWidth(), getHeight());
    return clone(rc);
}

bool Image::load(const uchar* fname, int &w, int &h, bool headeronly) {
    HANDLE hf = CreateFile(fname, GENERIC_READ, FILE_SHARE_READ, null,
                           OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN, null);
    assert(hf != INVALID_HANDLE_VALUE);
    FileInputStream fis(hf, fname);
    return load(&fis, w, h, headeronly);
}

static bool isJPG(InputStream* input) {
   byte buf[6];
   int n = input->read(buf, sizeof buf);
   return n == sizeof(buf) && memcmp(&buf[2], "JFIF", 4) == 0;
}

static bool isPNG(InputStream* input) {
   byte png_signature[4] = {137, 80, 78, 71};
   byte buf[sizeof(png_signature)];
   int n = input->read(buf, sizeof buf);
   return n == sizeof buf && memcmp(buf, png_signature, sizeof buf) == 0;
}

bool Image::load(InputStream* input, int &w, int &h, bool headeronly) {
    delete file;
    file = dynamic_cast<String*>(input->getName()->clone());
    if (file->endsWithIgnoreCase(L".png")) {
        return loadpng(headeronly, w, h, input, null);
    }
    else if (file->endsWithIgnoreCase(L".jpg")) {
        return loadjpg(headeronly, w, h, input, null);
    }
    else {
        if (isPNG(input)) {
            input->reset(); // TODO: rewind to marker instead of reset
            if (loadpng(headeronly, w, h, input, null)) return true;
            input->reset(); // TODO: rewind to marker instead of reset
        }
        if (isJPG(input)) {
            input->reset(); // TODO: rewind to marker instead of reset
            if (loadjpg(headeronly, w, h, input, null)) return true;
            input->reset(); // TODO: rewind to marker instead of reset
        }
        StringBuffer sb;
        sb.printf(L"cannot read image from file %s - unsupported format", (const uchar*)*file);
        setError((const uchar*)sb);
        return false;
    }
}

bool Image::save(const uchar* fname) {
    String file(fname);
    if (!file.endsWithIgnoreCase(L".png") && !file.endsWithIgnoreCase(L".jpg")) return false;
    HANDLE hf = CreateFile(fname, GENERIC_WRITE, 0, null,
                           OPEN_ALWAYS, FILE_FLAG_SEQUENTIAL_SCAN, null);
    assert(hf != INVALID_HANDLE_VALUE);
    FileOutputStream fos(hf, fname);
    return file.endsWithIgnoreCase(L".png") ? savepng(&fos) : savejpg(&fos);
}


static void checkError(Image* image) {
    if (image->getError() == null || image->getError()->length() == 0) {
        int err = ::GetLastError();
        StringBuffer sb;
        sb.printf(L"Failed to read file '%s'; GetLastError = %d [0x%08X]", 
                  (const uchar*)*image->getFile(), err, err);
        image->setError((const uchar*)sb);
    }
}

// ............................  JPEG  ......................................

enum { IMAGE_EXCEPTION = 0x42424242 };

inline void swaprgb(byte* p) {
    byte t = *p; *p = p[2]; p[2] = t;
}

inline void greyscale(byte* p, byte v) {
    *p++ = v;  *p++ = v;  *p++ = v;
}

void jpeg_error_exit(void* vcinfo) {
    j_common_ptr cinfo = (j_common_ptr)vcinfo;
    char buf[JMSG_LENGTH_MAX] = { 0 };
    (*cinfo->err->format_message)(cinfo, buf);
    Image* image = (Image*)cinfo->client_data;
    int n = strlen(buf);
    uchar* error = new uchar[n + 1];
    for (int i = 0; i < n + 1; i++) error[i] = buf[i];
    image->setError(error);
    delete error;
    RaiseException(IMAGE_EXCEPTION, 0, 0, null);
}

struct Source : public jpeg_source_mgr {
    const uchar*  filename;
    InputStream*  infile;
    OutputStream* outfile;
    byte*   buffer;
    boolean start_of_file;
};

enum { IO_BUF_SIZE = 64*1024 };

static void init_source(j_decompress_ptr cinfo) {
    Source* src = (Source*)cinfo->src;
    src->start_of_file = true;
}

static boolean fill_and_copy_input_buffer(j_decompress_ptr cinfo) {
    Source* src = (Source*) cinfo->src;
    int nbytes = src->infile->read(src->buffer, IO_BUF_SIZE);
    if (nbytes < 0) {
        if (src->start_of_file) { // empty input file is an error
            cinfo->err->msg_code = JERR_INPUT_EMPTY;
            cinfo->err->error_exit((j_common_ptr)cinfo);
        }
        WARNMS(cinfo, JWRN_JPEG_EOF);
        src->buffer[0] = (byte) 0xFF; // add EOI marker
        src->buffer[1] = (byte) JPEG_EOI;
        nbytes = 2;
    }
    else if (src->outfile != null) {
        int written = src->outfile->write(src->buffer, nbytes);
        if (written != nbytes) {
            cinfo->err->msg_code = JERR_FILE_WRITE;
            cinfo->err->error_exit((j_common_ptr)cinfo);
        }
    }
    src->next_input_byte = src->buffer;
    src->bytes_in_buffer = nbytes;
    src->start_of_file = false;
    return true;
}

static void skip_input_data(j_decompress_ptr cinfo, long num_bytes) {
    Source* src = (Source*) cinfo->src;
    if (num_bytes > 0) {
        while (num_bytes > (long) src->bytes_in_buffer) {
            num_bytes -= (long) src->bytes_in_buffer;
            (void)fill_and_copy_input_buffer(cinfo);
        }
        src->next_input_byte += (size_t) num_bytes;
        src->bytes_in_buffer -= (size_t) num_bytes;
    }
}

static void term_source(j_decompress_ptr cinfo) { }

typedef JMETHOD(void, JPEG_ERROR_EXIT, (j_common_ptr cinfo));

struct jpeg_decompress_struct_ex : public jpeg_decompress_struct {
    Image* image;

    jpeg_decompress_struct_ex(Image* i, jpeg_error_mgr *jerr, 
                              const uchar* fname, InputStream* infile, OutputStream* outfile) {
        image = i;
        err = jpeg_std_error(jerr);
        err->error_exit = (JPEG_ERROR_EXIT)jpeg_error_exit;
        client_data = image;
        jpeg_create_decompress(this);
        mem->max_memory_to_use = 1024*1024*1024; // 1GB 
        Source* s = (Source*)src;
        if (s == null) {
            s = (Source*)mem->alloc_small((j_common_ptr)this, JPOOL_PERMANENT, sizeof(Source));
            s->buffer = (byte*)mem->alloc_small((j_common_ptr)this, JPOOL_PERMANENT, IO_BUF_SIZE);
            src = (jpeg_source_mgr*)s;
        }
        s->init_source = init_source;
        s->fill_input_buffer = fill_and_copy_input_buffer;
        s->skip_input_data = skip_input_data;
        s->resync_to_restart = jpeg_resync_to_restart; // use default method
        s->term_source = term_source;
        s->filename = fname;
        s->infile  = infile;
        s->outfile = outfile;
        s->bytes_in_buffer = 0; // forces fill_input_buffer on first read
        s->next_input_byte = null; // until buffer loaded
    }
};

bool Image::loadjpg(bool headeronly, int  &w, int  &h, 
                     InputStream* input, OutputStream* output) {
    bool result = true;
    jpeg_error_mgr jerr;
    jpeg_decompress_struct_ex cinfo(this, &jerr, (const uchar*)file, input, output);
    __try {
        jpeg_read_header(&cinfo, true);
        assert(cinfo.num_components == 3 || cinfo.num_components == 1);
        w = cinfo.image_width;
        h = cinfo.image_height;
        if (!headeronly) {
            jpeg_start_decompress(&cinfo);
            assert(cinfo.output_components == 3 || cinfo.output_components == 1);
            // original compression quality guesswork for jpeg. Based on a constant
            // quant table std_chrominance_quant_tbl and code in:
            //  jpeg_add_quant_table and 
            //  jpeg_quality_scaling
            int n = cinfo.output_components != 1;
            int v = cinfo.quant_tbl_ptrs[n]->quantval[63];
            int base = 99; // original value for std_chrominance_quant_tbl[63]
            if (v == 255) {
                v = cinfo.quant_tbl_ptrs[n]->quantval[0];
                base = 17; // original value for std_chrominance_quant_tbl[0]
            }
            if (v < base) compression = (base - v / 2);
            else          compression = (5000 * base / v) / 100;
            compression++;
            if (compression < 0)   compression = 1;
            if (compression > 100) compression = 100;
            int nRowSize = cinfo.output_width * cinfo.output_components;
            JSAMPARRAY pBuffer = (*cinfo.mem->alloc_sarray)
                      ((j_common_ptr)&cinfo, JPOOL_IMAGE, nRowSize, 1);
            result = createRGBA(cinfo.output_width, cinfo.output_height);
            if (result) {
                int W4 = cinfo.output_width * 4;
                byte* p = bits;
                int   n = bytesPerLine;
                p += n * cinfo.output_height;
                while (cinfo.output_scanline < cinfo.output_height) {
                    p -= n;
                    jpeg_read_scanlines(&cinfo, pBuffer, 1);
                    if (cinfo.output_components == 3) {
                        byte* b = pBuffer[0];
                        for (int i = 0, j = 0; i < W4; i += 4, j += 3) {
                            p[i + 0] = b[j + 2];
                            p[i + 1] = b[j + 1];
                            p[i + 2] = b[j + 0];
                            p[i + 3] = 0xFF;
                        }
                    }
                    else {
                        byte* b = pBuffer[0];
                        for (int i = 0; i < W4; i += 4) greyscale(p + i, *b++);
                    }
                }
            }
            jpeg_finish_decompress(&cinfo);
        }
    }
    __except(_exception_code() == IMAGE_EXCEPTION ? 
             EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH) {
        checkError(this);
        result = false;
    }
    jpeg_destroy_decompress(&cinfo);
    if (output != null) output->flush();
    return result;
}

bool Image::savejpg(OutputStream* output) {
    assert(false); // TODO: not implemented yet
    return false;
}

// ............................  PNG  ......................................

struct png_copy {
    InputStream* input;
    OutputStream* output;
    const uchar* fname;
    png_copy(const uchar* fn, InputStream* i, OutputStream* o) : fname(fn), input(i), output(o) { }
};

static void user_warning_fn(png_structp png_ptr, png_const_charp warning_msg) {
//  traceln(L"libpng warning: %S", warning_msg); // ASCII, thus capital "%S"
}

static void png_user_error_fn(void* png_vptr, const char* error_msg) {
    png_structp png_ptr = (png_structp)png_vptr;
//  traceln(L"libpng error: %S", error_msg); // ASCII, thus capital "%S"
    Image* image = (Image*)png_ptr->error_ptr;
    int n = strlen(error_msg);
    uchar* error = new uchar[n + 1];
    for (int i = 0; i <= n; i++) error[i] = error_msg[i];
    image->setError(error);
    delete error;
    RaiseException(IMAGE_EXCEPTION, 0, 0, null);
}

static void user_read_fn(png_structp png_ptr, png_bytep data, png_size_t length) {
    png_copy &files = *(png_copy*)png_get_io_ptr(png_ptr);
    int n = files.input->read(data, length);
    if (n != (int)length) {
        png_error(png_ptr, "failed to read file");
    }
    if (files.output != null) {
        int written = files.output->write(data, n);
        if (written != n) {
            png_error(png_ptr, "failed to copy file");
        }
    }
}

static void user_write_fn(png_structp png_ptr, png_bytep data, png_size_t length) {
    png_copy &files = *(png_copy*)png_get_io_ptr(png_ptr);
    int written = files.output->write(data, length);
    if (written != (int)length) {
        png_error(png_ptr, "failed to write file");
    }
}

static void user_flush_fn(png_structp png_ptr) {
    png_copy &files = *(png_copy*)png_get_io_ptr(png_ptr);
    files.output->flush();
}

bool Image::loadpng(bool headeronly, int &w, int &h, 
                    InputStream* input, OutputStream* output) {
    // Create and initialize the png_struct with the desired error handler
    // functions.  If you want to use the default stderr and longjump method,
    // you can supply null for the last three parameters.  We also check that
    // the header file is compatible with the library version.
    png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING,
                                     (void*)this, 
                                     (png_error_ptr)png_user_error_fn, 
                                     user_warning_fn);
    if (png_ptr == null) return false;
    png_infop info_ptr = png_create_info_struct(png_ptr);
    if (info_ptr == null) {
        png_destroy_read_struct(&png_ptr, null, null);
        return false;
    }
    bool result = true;
    png_bytep* row_pointers = null;
    __try
    {
        png_copy files((const uchar*)file, input, output);
        png_set_read_fn(png_ptr, (void*)&files, user_read_fn);
        png_read_info(png_ptr, info_ptr);
        if (info_ptr->color_type == PNG_COLOR_TYPE_PALETTE)
            png_set_expand(png_ptr);
        if (info_ptr->color_type == PNG_COLOR_TYPE_GRAY && info_ptr->bit_depth < 8)
            png_set_expand(png_ptr);
        if (info_ptr->color_type == PNG_COLOR_TYPE_GRAY || 
            info_ptr->color_type == PNG_COLOR_TYPE_GRAY_ALPHA) 
            png_set_gray_to_rgb(png_ptr); 
        if (info_ptr->valid & PNG_INFO_tRNS)
            png_set_expand(png_ptr);
        // tell libpng to strip 16 bit/color files down to 8 bits/color
        if (info_ptr->bit_depth == 16)
            png_set_strip_16(png_ptr);
        // invert monocrome files to have 0 as white and 1 as black
        if (info_ptr->bit_depth == 1 && info_ptr->color_type == PNG_COLOR_TYPE_GRAY)
            png_set_invert_mono(png_ptr);
        // Add alpha channel if present 
        if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS)) 
            png_set_tRNS_to_alpha(png_ptr); 
        // flip the rgb pixels to bgr
        if (info_ptr->color_type == PNG_COLOR_TYPE_RGB ||
            info_ptr->color_type == PNG_COLOR_TYPE_RGB_ALPHA)
            png_set_bgr(png_ptr);
        if (info_ptr->color_type == PNG_COLOR_TYPE_RGB)
            png_set_add_alpha(png_ptr, 0xFF, PNG_FILLER_AFTER);
        // swap bytes of 16 bit files to least significant bit first
        if (info_ptr->bit_depth == 16)
            png_set_swap(png_ptr);
        // do NOT call png_set_filler!
        png_read_update_info(png_ptr, info_ptr);
        w = png_get_image_width(png_ptr,  info_ptr);
        h = png_get_image_height(png_ptr, info_ptr);
        if (headeronly) {
            png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp)null);
            return true;
        }
        // allocate the memory to hold the image using the fields of png_info.
        // the easiest way to read the image
        if (!createRGBA(w, h)) {
            png_destroy_read_struct(&png_ptr, (png_infopp)null, (png_infopp)null);
            return false;
        }
        row_pointers = new png_bytep[h];
        if (row_pointers == null) {
            png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp)null);
            return false;
        }
        memset(row_pointers, 0, h * sizeof png_bytep);
        byte* dp = bits + bytesPerLine * h;
        assert(bytesPerLine >= (int)info_ptr->rowbytes);
        for (int row = 0; row < h; row++) {
            dp -= bytesPerLine;
            row_pointers[row] = dp;
        }
        png_read_image(png_ptr, row_pointers);
        // read the rest of the file, getting any additional chunks in info_ptr
        png_read_end(png_ptr, info_ptr);
        if (info_ptr->channels == 1) {
            for (int row = 0; row < h; row++) {
                byte* p = row_pointers[row];
                for (int i = w-1; i >= 0; i--) {
                    p[(i<<2)+0] = 
                    p[(i<<2)+1] = 
                    p[(i<<2)+2] = p[i];
                }
            }
        }
    }
    __except(_exception_code() == IMAGE_EXCEPTION ? 
             EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH) {
        checkError(this);
        result = false;
    }
    png_destroy_read_struct(&png_ptr, &info_ptr, null);
    if (row_pointers != null) {
        memset(row_pointers, 0, h * sizeof png_bytep);
        delete []row_pointers;
    }
    if (output != null) output->flush();
    return result;
}

bool Image::savepng(OutputStream* output) {
    assert(bi.bV4BitCount == 32);
    assert(bi.bV4Width > 0 && bi.bV4Height > 0);
    bool result = true;
    png_structp png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, 
                                                  (void*)this,
                                                  (png_error_ptr)png_user_error_fn, 
                                                  user_warning_fn);
    if (png_ptr == null)
        return false;
    png_infop info_ptr = png_create_info_struct(png_ptr);
    if (info_ptr == null) {
        png_destroy_write_struct(&png_ptr, NULL);
        return false;
    }
    png_bytep* row_pointers = null;
    __try
    {
        png_copy files((const uchar*)file, null, output);
        png_set_write_fn(png_ptr, (void*)&files, user_write_fn, user_flush_fn);
        row_pointers = new png_bytep[bi.bV4Height];
        if (row_pointers == null) {
            png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp)null);
            return false;
        }
        memset(row_pointers, 0, bi.bV4Height * sizeof png_bytep);
        byte* dp = bits + bytesPerLine * bi.bV4Height;
        assert(bytesPerLine >= (int)info_ptr->rowbytes);
        for (int row = 0; row < bi.bV4Height; row++) {
            dp -= bytesPerLine;
            row_pointers[row] = dp;
        }
        png_set_compression_level(png_ptr, Z_BEST_COMPRESSION);
        png_set_IHDR(png_ptr, info_ptr, bi.bV4Width, bi.bV4Height,
                     8, PNG_COLOR_TYPE_RGB_ALPHA, PNG_INTERLACE_NONE,
                     PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
        png_color_16 background = {0, 0, 0, 0};
        png_set_bKGD(png_ptr, info_ptr, &background);
        png_write_info(png_ptr, info_ptr);
        png_set_packing(png_ptr);
        png_set_bgr(png_ptr);
        png_write_image(png_ptr, row_pointers);
        png_write_end(png_ptr, null);
    }
    __except(_exception_code() == IMAGE_EXCEPTION ? 
             EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH) {
        checkError(this);
        result = false;
    }
    png_destroy_write_struct(&png_ptr, &info_ptr);
    if (row_pointers != null) {
        memset(row_pointers, 0, bi.bV4Height * sizeof png_bytep);
        delete []row_pointers;
    }
    delete png_ptr;
    delete info_ptr;
    output->flush();
    return result;
}
