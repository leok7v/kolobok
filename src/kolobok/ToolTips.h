#pragma once
#include "tiny.h"

struct ToolTips {
    static void add(HWND hwnd, const String& text);
    static void remove(HWND hwnd);
};
