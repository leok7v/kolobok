#include "tiny.h"

String* String::guid() {
    uchar* buf;
    UUID uuid = {0};
    UuidCreate(&uuid);
    UuidToString(&uuid, reinterpret_cast<RPC_WSTR *>(&buf));
    String* s = new String(buf);
    RpcStringFree(&buf);
    return s;
}

String* String::fromUTF8(const byte* data) {
    // Notepad.exe reads both UTF-8 with or without BOM. However
    // it alsways prepends BOM to UTF-8 text files. Unixes may
    // not be happy about it... Code below treats all:
    char* utf8 = (char*)data;
    if (data[0] == 0xFF && data[1] == 0xFE) { // UTF-16 BOM
        return new String((const uchar*)data + 1);
    }
    else if (data[0] == 0xEF && data[1] == 0xBB && data[2] == 0xBF)  {  // UTF-8 BOM
        utf8 += 3;
    }
    else {
        // treat anything else as UTF-8
    }
    int slen = strlen(utf8);
    int n = MultiByteToWideChar(CP_UTF8, 0, utf8, slen, null, 0);
    uchar* s = new uchar[n + 1];
    MultiByteToWideChar(CP_UTF8, 0, (const char*)utf8, slen, s, n + 1);
    String* str = new String(s , n);
    delete s;
    return str;
}

byte* String::toUTF8(const uchar* s, int &len, bool bom) {
    int ulen = ustrlen(s);
    int n = WideCharToMultiByte(CP_UTF8, 0, s, ulen, null, 0, null, null);
    len = bom ? n + 3 : n;
    char* utf = new char[len + 1];
    WideCharToMultiByte(CP_UTF8, 0, s, ulen, bom ? utf + 3 : utf, len + 1, null, null);
    if (bom) { utf[0] = (byte)0xEF; utf[1] = (byte)0xBB; utf[2] = (byte)0xBF; }
    utf[len] = 0;
    return (byte *)utf;
}

ByteArray* String::toUTF8(const uchar* s, bool bom) {
    int len = 0;
    byte *utf = String::toUTF8(s, len, bom);
    ByteArray* ba = new ByteArray(utf, len);
    delete utf;
    return ba;
}

String* String::toString(const Object* o) {
    if (o == null) return new String(L"null");
    else if (instanceof(o, const String*)) {
        StringBuffer sb;
        sb.printf(L"\"%s\"", (const uchar*)*dynamic_cast<const String*>(o));
        return sb.toString();
    }
    else if (instanceof(o, const StringBuffer*)) {
        StringBuffer sb;
        sb.printf(L"`%s`", (const uchar*)*dynamic_cast<const StringBuffer*>(o));
        return sb.toString();
    }
    else {
        return o->toString();
    }
}
