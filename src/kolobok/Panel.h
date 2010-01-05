#pragma once
#include "tiny.h"

/* base container for all panels */

struct Panel extends Object and_implements BroadcastListener {

    Panel(HWND parent);
    virtual ~Panel();
    
    inline HWND getWnd() const { return hwnd; }
    void eventReceived(const Message& message);
    virtual int destroy();
    virtual int doLayout();
    virtual int wndProc(UINT msg, WPARAM wp, LPARAM lp);
    static Panel* getPanel(HWND hwnd);

protected: 
    Panel(); // only to be used in Frame (check usages and correct if not so).
    HWND hwnd;
private: 
    static LRESULT CALLBACK _wndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);
};

