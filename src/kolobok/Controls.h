#pragma once
#include "tiny.h"

struct Controls {

    static HWND createButton(const uchar* imageId, const uchar* broadcastCommand);
    // horizontaly resizable button
    static HWND createHorizontalButton(const uchar* imageId, const uchar* broadcastCommand);

    static HWND createDialog(int id, DLGPROC dlgProc);
    static void centerInParent(HWND dlg);
    static void skinDialog(HWND dlg);
};
