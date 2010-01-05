#include "ToolTips.h"
#include "Application.h"
#include "Skin.h"

static HWND tt;
static HashMap map;

static void createToolTipControl() {
    assert(tt == null);
    tt = CreateWindowEx(WS_EX_TOPMOST, TOOLTIPS_CLASS, null,
                        WS_POPUP | TTS_NOPREFIX | TTS_ALWAYSTIP | TTS_BALLOON,
                        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
                        getApplicationMainWindow(), null, GetModuleHandle(null), null);
    assert(tt != null);
    const static int flags = SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE;
    check(SetWindowPos(tt, HWND_TOPMOST, -1, -1, -1, -1, flags));
    SendMessage(tt, TTM_SETMAXTIPWIDTH, 0, 200);
    SendMessage(tt, TTM_SETDELAYTIME, TTDT_AUTOMATIC, 1500);
}

void ToolTips::add(HWND hwnd, const String& text) {
    if (tt == null) createToolTipControl();
    if (map.containsKey((int)hwnd)) return;
    String* s = dynamic_cast<String*>(map.put((int)hwnd, text));
    // sizeof(TOOLINFO) is 0x30 and TootTip Ctrl does not work with it.
    TOOLINFO ti = { 0x2C }; 
    ti.uFlags = TTF_SUBCLASS | TTF_IDISHWND | TTF_TRANSPARENT;
    ti.hwnd   = getApplicationMainWindow();
    ti.uId    = (UINT)hwnd;
    ti.hinst  = GetModuleHandle(null);
    ti.lpszText  = (uchar*)(const uchar*)*s;
    ti.rect.left = ti.rect.top = 0;
    ti.rect.bottom = ti.rect.right = 100;
    check(SendMessage(tt, TTM_ADDTOOL, 0, (LPARAM)&ti));
}

void ToolTips::remove(HWND hwnd) {
    assert(map.containsKey((int)hwnd));
    map.remove((int)hwnd);
    // sizeof(TOOLINFO) is 0x30 and TootTip Ctrl does not work with it.
    TOOLINFO ti = { 0x2C }; 
    ti.uFlags = TTF_IDISHWND;
    ti.hwnd   = getApplicationMainWindow();
    ti.uId    = (UINT)hwnd;
    SendMessage(tt, TTM_DELTOOL, 0, (LPARAM)&ti);
}

