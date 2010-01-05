#include "tiny.h"

ObjectPipe::ObjectPipe(HANDLE h) : pipe(h) {
    pos = 0;
    len = 0;
    eof = false;
    memset(&overlapped_read, 0, sizeof(overlapped_read));
    memset(&overlapped_write, 0, sizeof(overlapped_write));
    overlapped_read.hEvent = CreateEvent(null, true, true, null);
    overlapped_write.hEvent = CreateEvent(null, true, true, null);
    if (overlapped_read.hEvent == null) error();
    if (overlapped_write.hEvent == null) error();
}

ObjectPipe::~ObjectPipe() {
    if (!CloseHandle(overlapped_read.hEvent)) error();
    if (!CloseHandle(overlapped_write.hEvent)) error();
}

BOOL ObjectPipe::readBuffer() {
    assert(pos >= len);
    if (eof) return false;
    DWORD rsz = 0;
    if (!ReadFile(pipe, buf, sizeof(buf), &rsz, &overlapped_read)) {
        switch (GetLastError()) { 
        case ERROR_IO_PENDING: 
            switch (WaitForSingleObject(overlapped_read.hEvent, 60000)) {
            case WAIT_OBJECT_0:
                if (!GetOverlappedResult(pipe, &overlapped_read, &rsz, false)) {
                    switch (GetLastError()) {
                    case ERROR_PIPE_NOT_CONNECTED:
                        eof = true;
                        return false;
                    default:
                        error();
                        break;
                    }
                }
                break;
            case WAIT_TIMEOUT:
                error();
                break;
            default:
                error();
                break;
            }
            break; 
        default:
            error();
            break;
        }
    }
    assert(rsz > 0);
    len = rsz;
    pos = 0;
    return true;
}

uchar ObjectPipe::readChar() {
    if (pos >= len && !readBuffer()) return 0;
    uchar ch = (uchar)buf[pos++];
    if (pos >= len && !readBuffer()) return 0;
    return (uchar)(ch + 256 * (uchar)buf[pos++]);
}

String* ObjectPipe::readString() {
    StringBuffer sb;
    for (;;) {
        uchar ch = readChar();
        if (ch == 0) break;
        sb += ch;
    }
    if (sb.length() == 0 && eof) return null;
    return sb.toString();
}

Object* ObjectPipe::readObject() {
    String* xml = readString();
    // TODO ambiguity: return null can be EOF or Object null
    if (xml == null) return null;
    assert(xml->length() > 0);
    Object* obj = XMLDecoder::decode(xml);
    delete xml;
    return obj;
}

void ObjectPipe::writeStr(const String* str) {
    DWORD wsz = 0;
    const uchar* buf = (const uchar*)(*str);
    DWORD buf_size = (ustrlen(buf) + 1) * sizeof(uchar);
    if (!WriteFile(pipe, buf, buf_size, &wsz, &overlapped_write)) {
        switch (GetLastError()) { 
        case ERROR_IO_PENDING: 
            switch (WaitForSingleObject(overlapped_write.hEvent, 60000)) {
            case WAIT_OBJECT_0:
                if (!GetOverlappedResult(pipe, &overlapped_write, &wsz, false)) error();
                break;
            case WAIT_TIMEOUT:
                error();
                break;
            default:
                error();
                break;
            }
            break; 
        default:
            error();
            break;
        }
    }
    assert(wsz == buf_size);
    if (!FlushFileBuffers(pipe)) error();
}

void ObjectPipe::writeObject(const Object* obj) {
    String* str = XMLEncoder::encode(obj);
    writeStr(str);
    delete str;
}

void ObjectPipe::error() {
    traceError();
    ExitProcess(1);
}

