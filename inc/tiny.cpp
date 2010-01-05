#include "tiny.h"

const Object* Object::nil = null;

int  _assert(const char* exp, const char* file, int line) {
    if (GetLastError() != 0) traceError();
    trace(L"\n%S(%d): assertion \"%S\" failed\n", file, line, exp);
    __asm int 3 /* DebugBreak(); */
    return 0;
}

static void ods(const uchar* s) {
    OutputDebugString(s);
    DWORD mode = 0;
    if (GetConsoleMode(GetStdHandle(STD_ERROR_HANDLE), &mode)) { // is console attached?
        fputws(s, stderr);
    }
}

static void trace(const uchar* fmt, va_list vl) {
    /* experimental knowledge OutputDebugStringW is limited to 1023 */
    enum { N = 1023 }; 
    StringBuffer sb;
    sb.vprintf(fmt, vl);
    int n = sb.length();
    const uchar* p = sb;
    while (n > 0) {
        if (n < N) {
            ods(p);
            n = 0; 
        }
        else {
            uchar chunk[N + 1];
            memcpy(chunk, p, sizeof chunk);
            chunk[N] = 0;
            ods(chunk);
            p += N;
            n -= N;
        }
    }
}

void trace(const uchar* fmt, ...) {
    va_list vl;
    va_start(vl, fmt);
    trace(fmt, vl);
    va_end(vl);
}

void _cdecl traceln(const uchar* fmt, ...) {
    va_list vl;
    va_start(vl, fmt);
    trace(fmt, vl);
    va_end(vl);
    ods(L"\n");
}

void trace(const char* fmt, ...) {
    va_list vl;
    va_start(vl, fmt);
    String s(fmt);
    trace((const uchar*)s, vl);
    va_end(vl);
}

void traceln(const char* fmt, ...) {
    va_list vl;
    va_start(vl, fmt);
    String s(fmt);
    trace((const uchar*)s, vl);
    va_end(vl);
    ods(L"\n");
}

void trace(const Object* o) {
    if (o == null) trace(L"null");
    else {
/*
        const char* type = typeid(o).name();
        if (strstr(type, "struct ") != null) type += 7;
        int len = strlen(type);
        if (len > 8 && strstr(type + len - 8, " const *") != null) {
            len -= 8;
        }
*/
        String* s = o->toString();
//        trace(L"%.*S@0x%x%s", len, type, o, (const uchar*)*s);
        trace(L"%s", (const uchar*)*s);
        delete s;
    }
}

void traceln(const Object* o) { 
    trace(o);
    trace(L"\n");
}

void trace(const Object& o) {
    trace(&o);
}

void traceln(const Object& o) {
    traceln(&o);
}

void traceError() {
    DWORD err = GetLastError();
    uchar * msg = null;
    FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
                  null, err, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                  (uchar*)&msg,	0, null);
    if (msg != null) {
        trace(L"GetLastError(): %d [0x%08X] %s", err, err, msg);
        LocalFree(msg);
    }
    else {
        trace(L"GetLastError(): %d [0x%08X].\n", err, err);
    }
}

static longlong queryPerformanceCounter() {
    LARGE_INTEGER pc = {0}, cps = {0};
    QueryPerformanceCounter(&pc);
    return *(longlong*)&pc;
}

static longlong cps;  // counts per second

static longlong queryPerformanceFrequency() { // counts per second
    LARGE_INTEGER cps = {0};
    QueryPerformanceFrequency(&cps); 
    return *(longlong*)&cps;
}

longlong getTimeInMicroseconds() {
    if (cps == 0) cps = queryPerformanceFrequency();
    return queryPerformanceCounter() * 1000000 / cps;
}


// keep code below at the end of manifest.cpp file
// so calls to ::new from the manifest file are still
// trackable in debug and zero fill memory.

#undef new 
#ifdef _DEBUG
void* operator new(unsigned int s, char* file, int line) { 
    void* a = ::operator new(s, _NORMAL_BLOCK, file, line);
    // TODO: implement low memeory warning and safety pool here.
    memset(a, 0, s);
    return a;
}
#else
void* operator new(unsigned int s) { 
    void* a = malloc(s);
    // TODO: implement low memeory warning and safety pool here.
    memset(a, 0, s);
    return a;
}
#endif

int __stdcall WinMain(HINSTANCE instance, HINSTANCE, char*, int show) {
#ifdef _DEBUG
    /* will call _CrtDumpMemoryLeaks() after global destuctors and fini */
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF|_CRTDBG_LEAK_CHECK_DF); 
#endif
    ApplicationTester::init();
    DnD::init();
    INITCOMMONCONTROLSEX iccex = { sizeof(INITCOMMONCONTROLSEX),
                                   ICC_WIN95_CLASSES };
    check(InitCommonControlsEx(&iccex));
    check(OleInitialize(null) == S_OK);
    LPWSTR envw = GetEnvironmentStringsW();
    int argc = 0;
    LPWSTR* argvw = CommandLineToArgvW(GetCommandLineW(), &argc);
    int xc = winMain(instance, argc, argvw, envw, show);
    ApplicationTester::fini();
    DnD::fini();
    return xc;
}
