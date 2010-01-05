#pragma once
#include "tiny.h"

struct Help implements BroadcastListener {
    void eventReceived(const Message& m);
    static Help help;
    HWND dlgAbout;
};


