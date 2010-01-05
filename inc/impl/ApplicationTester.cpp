#include "tiny.h"

static ApplicationTester* tester;

ApplicationTester::ApplicationTester(HANDLE h) : pipe(h), object_pipe(h) {
    pipe_reader = INVALID_HANDLE_VALUE;
}

static HWND findWindow(const HWND parent, const uchar* name) {
    HWND wnd = GetWindow(parent, GW_CHILD);
    for (;;) {
        if (wnd == null) break;
        const String* str = dynamic_cast<const String*>(Window::get(wnd, L"name"));
        if (str != null) {
            if (ustrcmp(*str, name) == 0) return wnd;
            HWND res = findWindow(wnd, name);
            if (res != null) return res;
        }
        wnd = GetWindow(wnd, GW_HWNDNEXT);
    }
    return null;
}

void ApplicationTester::eventReceived(const Message& msg) {
    Object* result = null;
    if (ustrcmp(msg.id, L"testerSendWM") == 0) {
        HWND wnd = null;
        UINT Msg = 0;
        WPARAM wParam = 0;
        LPARAM lParam = 0;
        if (msg.param != null) {
            const HashMap* map = dynamic_cast<const HashMap*>(msg.param);
            const Int* lw = dynamic_cast<const Int*>(map->get(L"wnd"));
            if (lw != null) wnd = (HWND)lw->intValue();
            else wnd = GetActiveWindow();
            const Int* l0 = dynamic_cast<const Int*>(map->get(L"msg"));
            const Int* l1 = dynamic_cast<const Int*>(map->get(L"wParam"));
            const Int* l2 = dynamic_cast<const Int*>(map->get(L"lParam"));
            if (l0 != null) Msg = l0->intValue();
            if (l1 != null) wParam = l1->intValue();
            if (l2 != null) lParam = l2->intValue();
            LRESULT lRes = 0;
            if (wnd != null) lRes = SendMessage(wnd, Msg, wParam, lParam);
            result = new Int(lRes);
        }
    }
    else if (ustrcmp(msg.id, L"testerSendVK") == 0) {
        const HashMap* map = dynamic_cast<const HashMap*>(msg.param);
        const Int* vk = dynamic_cast<const Int*>(map->get(L"vk"));
        INPUT inp = { INPUT_KEYBOARD };
        inp.ki.wVk = (WORD)vk->intValue();
        inp.ki.wScan = (WORD)MapVirtualKey(inp.ki.wVk, 0);
        UINT n = SendInput(1, &inp, sizeof(inp));
        if (n == 1) {
            inp.ki.dwFlags |= KEYEVENTF_KEYUP;
            n = SendInput(1, &inp, sizeof(inp));
        }
        result = new Int(n);
    }
    else if (ustrcmp(msg.id, L"testerGetMainWindow") == 0) {
        result = new Int((DWORD)getApplicationMainWindow());
    }
    else if (ustrcmp(msg.id, L"testerGetWindow") == 0) {
        const String* name = dynamic_cast<const String*>(msg.param);
        result = new Int((DWORD)findWindow(getApplicationMainWindow(), *name));
    }
    else if (ustrcmp(msg.id, L"testerSetFocus") == 0) {
        const Int* wnd = dynamic_cast<const Int*>(msg.param);
        result = new Int((DWORD)SetFocus((HWND)wnd->intValue()));
    }
    else if (ustrcmp(msg.id, L"testerSetForegroundWindow") == 0) {
        HWND wnd = getApplicationMainWindow();
        if (msg.param != null) {
            const Int* lw = dynamic_cast<const Int*>(msg.param);
            if (lw != null) wnd = (HWND)lw->intValue();
        }
        result = new Int(SetForegroundWindow(wnd));
    }
    String* cmd= new String(msg.id);
    object_pipe.writeStr(cmd);
    object_pipe.writeObject(msg.param);
    object_pipe.writeObject(result);
    delete cmd;
    delete result;
}

DWORD WINAPI ApplicationTester::pipeReaderThread(LPVOID arg) {
    for (;;) {
        String* cmd = tester->object_pipe.readString();
        if (cmd == null) break;
        Object* obj = tester->object_pipe.readObject();
        // Cannot use 'cmd' in MessageQueue::post because
        // 'cmd' needs to be deleted now to prevent memory leak
        const String* id = dynamic_cast<const String*>(tester->str_map.get(cmd));
        if (id == null) {
            tester->str_map.put(cmd, cmd);
            id = dynamic_cast<const String*>(tester->str_map.get(cmd));
            assert(id != null);
        }
//      trace("pipeReaderThread "); trace(*id); trace(L" ");  traceln(obj);
        MessageQueue::post(*id, obj);
        delete cmd;
    }
    return 0;
}

void ApplicationTester::init() {
    HANDLE pipe = CreateFile(L"\\\\.\\pipe\\kolobok-tester",
        GENERIC_READ | GENERIC_WRITE,
        0, null, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, null);
    if (pipe == INVALID_HANDLE_VALUE) {
        traceln(L"Warning: automated tester connection cannot be established.");
    }
    else {
        tester = new ApplicationTester(pipe);
        tester->pipe_reader = CreateThread(null, 0, pipeReaderThread, null, 0, null);
        MessageQueue::addBroadcastListener(tester);
    }
}

void ApplicationTester::fini() {
    if (tester == null) return;
    MessageQueue::removeBroadcastListener(tester);
    WaitForSingleObject(tester->pipe_reader, INFINITE);
    CloseHandle(tester->pipe);
    CloseHandle(tester->pipe_reader);
    delete tester;
}
