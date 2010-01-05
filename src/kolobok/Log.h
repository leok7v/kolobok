#pragma once
#include "tiny.h"
#include "Panel.h"

/* Main Application Window */

struct Log extends Panel {
    Log(HWND parent);
    ~Log();
    void eventReceived(const Message& message);
private:
    int wndProc(UINT msg, WPARAM wp, LPARAM lp);
    int destroy();
    int paint();
};

