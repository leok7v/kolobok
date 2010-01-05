#pragma once
#include "tiny.h"
#include "Panel.h"

/* Main Application Window */

struct Frame extends Panel {
    Frame(HWND parent);
    ~Frame();
    void eventReceived(const Message& message);
private:
    HashMap views;
    int wndProc(UINT msg, WPARAM wp, LPARAM lp);
    int destroy();
};

