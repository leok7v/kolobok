#include "resource.h"
#include "Log.h"

Log::Log(HWND parent) : Panel(parent) {
}

Log::~Log() {
    assert(hwnd == null);
}

void Log::eventReceived(const Message& msg) {
}

int Log::destroy() {
    return Panel::destroy();
}

int Log::paint() {
    PAINTSTRUCT ps = {0};
    BeginPaint(hwnd, &ps);
    RECT rc;
    GetClientRect(hwnd, &rc);
    EndPaint(hwnd, &ps);
    return 0;
}

int Log::wndProc(UINT msg, WPARAM wp, LPARAM lp) {
    switch (msg) {
        case WM_PAINT: return paint();
        default:
            return Panel::wndProc(msg, wp, lp);
    }
}
