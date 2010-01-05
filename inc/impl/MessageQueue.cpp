#include "tiny.h"

const uint INVOKE_LATER = RegisterWindowMessage(L"invokeLater");
const uint BROADCAST = RegisterWindowMessage(L"broadcast");
const DWORD DISPATCH_THREAD_ID = GetCurrentThreadId();

LinkedList MessageQueue::listeners;
HACCEL MessageQueue::accel;
bool MessageQueue::shutdown;
int  MessageQueue::shutdownDelay;

HashSet hooked;

// Modal Dialogs, Popup and Top level menus all "swallow" messages
// from message queue on a secondary dispatch loop. Such irresponsible 
// consumption of our Broadcast messages will lead to messages being
// not broadcasted and not deleted at the end of dispatch cycle.
// Also invokeLater messages will never be invoked.

// The only way I found to fix this is to install GetMessage hook
// and dispatch our messages when they are removed from the queue.

static void hookWindow(HWND hwnd);
static void unhookWindow(HWND hwnd);

static HHOOK hookWndProc();
static HHOOK hookWndProcRet();
static HHOOK hookGetMessageProc();

static HHOOK hookWP  = hookWndProc();
static HHOOK hookWPR = hookWndProcRet();
static HHOOK hookGM  = hookGetMessageProc();

static void checkHookedWindows() {
    if (hooked.size() > 0) {
        for (HashSet::Iterator i(hooked); i.hasNext(); ) {
            const Long& wnd = *dynamic_cast<const Long*>(i.next());
            uchar buf[256] = {0};
            if (!GetClassName((HWND)wnd.intValue(), buf, countof(buf))) {
                ustrcpy(buf, countof(buf), L"<invalid hwnd>");
            }
            // if you see this message chances are that you've ran to
            // something similar to "OleMainThreadWndClass" or "CLIPBRDWNDCLASS"
            // (search down to see special treatment of those).
            traceln(L"Warning: haven't heard WM_DESTROY hwnd=0x%08X class=%s", wnd.intValue(), buf);
        }
    }
    assert(hooked.size() == 0);
}

static LRESULT CALLBACK hookedGetMessage(int code, WPARAM wp, LPARAM lp) {
    if (code == HC_ACTION && wp == PM_REMOVE) {
        MSG& msg = *(MSG*)lp;
        if (msg.message == WM_QUIT && hookGM != null) {
            int r = CallNextHookEx(hookGM, code, wp, lp);
            UnhookWindowsHookEx(hookGM); hookGM = null;
            checkHookedWindows();
            return r;
        }
        else if (msg.message == INVOKE_LATER) {
            ((Runnable)msg.wParam)((void*)msg.lParam);
        }
        else if (msg.message == BROADCAST) {
            Message* m = dynamic_cast<Message*>((Object*)msg.lParam);
            m->dispatch();
        }
    }
    return CallNextHookEx(hookGM, code, wp, lp);
} 

HHOOK hookGetMessageProc() {
    HHOOK h = SetWindowsHookEx(WH_GETMESSAGE, hookedGetMessage, GetModuleHandle(null), GetCurrentThreadId());
    assert(h != null);
    return h;
}

static LRESULT CALLBACK hookedWndProc(int code, WPARAM wp, LPARAM lp) {
    if (code == HC_ACTION) {
        CWPSTRUCT& msg = *(CWPSTRUCT*)lp;
        if (msg.message == WM_CREATE) {
            hookWindow(msg.hwnd);
        }
        if (msg.message == WM_QUIT && hookWP != null) {
            int r = CallNextHookEx(hookWP, code, wp, lp);
            UnhookWindowsHookEx(hookWP); hookWP = null;
            return r;
        }
    }
    return CallNextHookEx(hookWP, code, wp, lp);
} 

HHOOK hookWndProc() {
    HHOOK h = SetWindowsHookEx(WH_CALLWNDPROC, hookedWndProc, GetModuleHandle(null), GetCurrentThreadId());
    assert(h != null);
    return h;
}

// for dialog boxes DestroyWindow() does not post WM_DESTROY on the queue
// but calls window proc directly. We need destroy notification to free
// our resources. This is why WndProcReturn is hooked too.

static LRESULT CALLBACK hookedWndProcRet(int code, WPARAM wp, LPARAM lp) {
    if (code == HC_ACTION) {
        CWPRETSTRUCT& msg = *(CWPRETSTRUCT*)lp;
        if (msg.message == WM_QUIT && hookWPR != null) {
            int r = CallNextHookEx(hookWPR, code, wp, lp);
            UnhookWindowsHookEx(hookWPR); hookWPR = null;
            return r;
        }
    }
    return CallNextHookEx(hookWPR, code, wp, lp);
} 

HHOOK hookWndProcRet() {
    HHOOK h = SetWindowsHookEx(WH_CALLWNDPROCRET, hookedWndProcRet, GetModuleHandle(null), GetCurrentThreadId());
    assert(h != null);
    return h;
}

/*
    WM_NCDESTROY is called directly from WM_DESTROY.
    There is no meaningful action application can do in response to it
    but delivering it to the listeners is hard-to-impossible.
    Thus for now it is just filtered out.
*/

static LRESULT CALLBACK windowHook(HWND hwnd, uint msg, WPARAM wp, LPARAM lp) {
    assert(MessageQueue::isDispatchThread());
    WNDPROC chain = (WNDPROC)Window::getInt(hwnd, L"hookedChain", null);
    if (msg != WM_NCDESTROY) Window::notifyListeners(hwnd, msg, wp, lp);
    LRESULT r = chain != null ? CallWindowProc(chain, hwnd, msg, wp, lp) : 0;
    if (msg == WM_NCDESTROY || msg == WM_DESTROY) {
        if ((WNDPROC)GetWindowLong(hwnd, GWL_WNDPROC) == windowHook) {
            unhookWindow(hwnd);
            Window::notifyDestroy(hwnd);
        }
        else {
            /*
            WNDPROC wndproc = (WNDPROC)GetWindowLong(hwnd, GWL_WNDPROC);
            WNDPROC chain = (WNDPROC)Window::getInt(hwnd, L"hookedChain");
            uchar buf[32] = {0};
            GetClassName(hwnd, buf, countof(buf));
            traceln(L"%s cannot unhook %s 0x%08X failed to unhook! chain "
                    L"0x%08X wndproc 0x%08X windowHook 0x%08X", 
                    msg == WM_NCDESTROY ? L"WM_NCDESTROY" : L"WM_DESTROY",
                    buf, hwnd, chain, wndproc, windowHook);
            */
            if (msg == WM_NCDESTROY) {
                Window::notifyDestroy(hwnd);
                /*
                    WinXP SP2: GetOpenFileName hooks "ScrollBar" and unhooks
                    it on WM_NCDESTROY (not on WM_DESTROY).
                    GetOpenFileName hooks "Edit" twice(!) and unhooks
                    first hook on WM_NCDESTROY (not on WM_DESTROY) and
                    never (even on hookWndProcRet (verified) unhooks second hook.
                    The hook chain cannot be restored correctly but this function
                    allready called the chained wndproc with NC_DESTROY and
                    hwnd will get out of existance upon return from this hook
                    chain - thus it does not matter anymore...
                    This is a suspected bug in GetOpenFileName and/or Shell32.dll
                    but when it is fixed it will not break hooking code around here.
                */
                hooked.remove((int)hwnd);
            }
        }
    }
    return r;
}

/*
    Hidden window of window class OleMainThreadWndClass is created by
    OLE32.dll and not destroyed yet when application receives WM_QUIT.
    For now just not to hook it. It is anticipated that some 3rd parties
    may create other windows that should not be hooked. 
    Add special cases on as-needed basis.
*/

void hookWindow(HWND hwnd) {
    assert(MessageQueue::isDispatchThread());
    assert(hwnd != null);
    uchar buf[32] = {0};
    GetClassName(hwnd, buf, countof(buf));
//  traceln("hwnd = %08X class=%s", hwnd, buf);
    // special cases: see above
    if (ustrcmp(buf, L"OleMainThreadWndClass") == 0) return; 
    if (ustrcmp(buf, L"CLIPBRDWNDCLASS") == 0) return; 
    if (ustrcmp(buf, L"CicMarshalWndClass") == 0) return;
    // IME == Input Method Editor:
    if (ustrcmp(buf, L"MSCTFIME UI") == 0) return;
    if (ustrcmp(buf, L"IME") == 0) return;
    // OpenFile Dialog
    if (ustrcmp(buf, L"WorkerW") == 0) return;
    WNDPROC chain = (WNDPROC)GetWindowLong(hwnd, GWL_WNDPROC);
    if (IsBadCodePtr((FARPROC)chain)) {
        // CicMarshalWndClass has wndproc that is not valid code ptr
        traceln(L"*** Bad wndproc hwnd=%08X wndproc=%08X class=%s", hwnd, chain, buf);
    }
    else {
        hooked.add((int)hwnd);
        SetWindowLong(hwnd, GWL_WNDPROC, (long)windowHook);
        Window::put(hwnd, L"hookedChain", (int)chain);
    }
}

void unhookWindow(HWND hwnd) {
    assert(MessageQueue::isDispatchThread());
    assert(hwnd != null);
    hooked.remove((int)hwnd);
    WNDPROC chain = (WNDPROC)Window::getInt(hwnd, L"hookedChain");
    SetWindowLong(hwnd, GWL_WNDPROC, (long)chain);
}
