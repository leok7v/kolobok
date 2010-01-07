#include "tiny.h"

static HANDLE pipe = INVALID_HANDLE_VALUE;
static HANDLE pipe_reader = INVALID_HANDLE_VALUE;
static const int MAX_HISTORY = 256;
static ObjectPipe* object_pipe = null;
struct HistoryMessage;
static HistoryMessage* history[MAX_HISTORY];
static int history_len = 0;
static HANDLE history_mutex = null;
static HANDLE history_event = null;

struct HistoryMessage : Message {
    String* str;
    Object* res;
    HistoryMessage(String* cmd, Object* obj, Object* r) : Message((const uchar*)(*cmd), obj), str(cmd), res(r) {
    }
    virtual ~HistoryMessage() {
        delete str;
        delete res;
    }
};

static void error(void) {
    trace(L"Test Server: ");
    traceError();
    ExitProcess(1);
}

static void error(const uchar* fmt, ...) {
    DWORD err = GetLastError();
    StringBuffer bf(L"Test Server: ");
    va_list vl;
    va_start(vl, fmt);
    bf.vprintf(fmt, vl);
    va_end(vl);
    traceln(&bf);
    if (err != ERROR_SUCCESS) {
        trace(L"  GetLastError(): ");
        traceError();
    }
    ExitProcess(1);
}

static void post(const uchar* cmd, const Object* obj) {
    String str(cmd);
    object_pipe->writeStr(&str);
    object_pipe->writeObject(obj);
}

static HistoryMessage* waitForBroadcast(const uchar* cmd) {
    for (;;) {
        HistoryMessage* msg = null;
        switch (WaitForSingleObject(history_event, 300000)) {
        case WAIT_OBJECT_0:
            break;
        case WAIT_TIMEOUT:
            SetLastError(ERROR_SUCCESS);
            error(L"Timeout waiting for broadcast message '%s'", cmd);
            break;
        default:
            error(L"Error waiting for broadcast message");
            break;
        }
        switch (WaitForSingleObject(history_mutex, 10000)) {
        case WAIT_OBJECT_0:
            break;
        case WAIT_TIMEOUT:
            SetLastError(ERROR_SUCCESS);
            error(L"Timeout trying to access broadcast message history");
            break;
        default:
            error(L"Error trying to access broadcast message history");
            break;
        }
        assert(history_len > 0);
        int i = 0;
        while (i < history_len) {
            traceln(L"Broadcast: %s", history[i]->id);
            if (ustrcmp(history[i]->id, cmd) == 0) {
                msg = history[i++];
                break;
            }
            delete history[i++];
        }
        assert(i > 0);
        for (int j = 0; j < MAX_HISTORY; j++) {
            if (j < history_len - i) history[j] = history[j + i];
            else history[j] = null;
        }
        history_len -= i;
        if (history_len == 0) {
            if (!ResetEvent(history_event)) error();
        }
        if (!ReleaseMutex(history_mutex)) error();
        if (msg != null) return msg;
    }
}

static Object* send(uchar* cmd, Object* obj) {
    post(cmd, obj);
    HistoryMessage* msg = waitForBroadcast(cmd);
    Object* res = msg->res;
    msg->res = null;
    delete msg;
    return res;
}

static LRESULT sendWM(Object* wnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    HashMap map;
    if (wnd != null) map.put(L"wnd", wnd);
    Int l0(msg), l1(wParam), l2(lParam);
    map.put(L"msg", &l0);
    map.put(L"wParam", &l1);
    map.put(L"lParam", &l2);
    post(L"testerSendWM", &map);
    HistoryMessage* his_msg = waitForBroadcast(L"testerSendWM");
    LRESULT lRes = (dynamic_cast<const Int*>(his_msg->res))->intValue();
    delete his_msg;
    return lRes;
}

static void sendVK(int vk) {
    HashMap map;
    Int l(vk);
    map.put(L"vk", &l);
    post(L"testerSendVK", &map);
    HistoryMessage* his_msg = waitForBroadcast(L"testerSendVK");
    assert((dynamic_cast<const Int*>(his_msg->res))->intValue() == 1);
    delete his_msg;
}

static void test_connection() {
    Object* objs[] = {
        null,
        new HashMap(),
        new ArrayList(),
        new LinkedList(),
        new String(),
        new String(L"x"),
        new String(L"y"),
        new StringBuffer(),
        new StringBuffer(L"x"),
        new StringBuffer(L"y"),
        new HashSet(),
        new Double(153.),
        new Double(334.),
        new Long(153),
        new Long(334),
        new Long(335),
        new Long(336),
        new Long(337),
        new Long(338),
        new Point(123, 321),
        new Rect(1,2,3,4),
        null,
        null
    };
    int i;
    // Create HashSet and HashMap with content
    HashSet* test_set = new HashSet();
    HashMap* test_map = new HashMap();
    for (i = 0; i < countof(objs); i++) {
        if (objs[i] == null) continue;
        test_set->add(objs[i]);
        test_map->put(objs[i], objs[i]);
    }
    objs[countof(objs) - 2] = test_set;
    objs[countof(objs) - 1] = test_map;
    // Send test messages
    for (i = 0; i < countof(objs); i++) {
        post(L"testerTestConnection", objs[i]);
    }
    // Receive test messages
    for (i = 0; i < countof(objs); i++) {
        Message* msg = waitForBroadcast(L"testerTestConnection");
        assert(ustrcmp(msg->id, L"testerTestConnection") == 0);
        if (objs[i] == null) {
            assert(msg->param == null);
        }
        else {
            assert(msg->param != null && msg->param->equals(objs[i]));
        }
        delete msg;
        delete objs[i];
    }
}

static void test_xyz() {
    sendWM(null, WM_NULL, 0, 0);
    Object* wnd = send(L"testerGetMainWindow", null);
    int i;
    for (i = 0; i < 2; i++) {
        sendWM(wnd, WM_SYSCOMMAND, SC_MAXIMIZE, 0);
        sendWM(wnd, WM_SYSCOMMAND, SC_RESTORE, 0);
        // TODO SC_MINIMIZE puts window to background, 
        // and it stays there until user clicks the window :-(
        // sendWM(wnd, WM_SYSCOMMAND, SC_MINIMIZE, 0);
        sendWM(wnd, WM_SYSCOMMAND, SC_RESTORE, 0);
    }
    delete send(L"commandHelpAbout", null);
    // check event handlig works when dialog box is open
    test_connection();
    sendWM(null, WM_CLOSE, 0, 0);

    sendVK(VK_RETURN);
    delete send(L"commandHelpAbout", null);
    sendVK(VK_ESCAPE);

    sendWM(wnd, WM_SYSCOMMAND, SC_MAXIMIZE, 0);
    delete send(L"testerSetForegroundWindow", wnd);
    for (i = 0; i < 2; i++) {
        delete send(L"commandEditCopy", null);
    }

    // Test page selection 
    for (int n = 0; n < 6; n++) {
        // Test different modes
        switch (n % 2) {
        case 0:
            delete send(L"commandViewThumbs", null);
            break;
        case 1:
            delete send(L"commandViewPicture", null);
            break;
        }
/*
        // Test selection by events
        HashMap m;
        Int src((int)&m), l0(0), l2(2), lm1(-1);
        m.put(L"source", &src);
        m.put(L"page", &l0);
        delete send(L"selectCurrentPage", &m);
        m.put(L"page", &l2);
        delete send(L"selectCurrentPage", &m);
        m.put(L"page", &l0);
        delete send(L"selectCurrentPage", &m);
        m.put(L"page", &lm1);
        delete send(L"selectCurrentPage", &m);

        // Test selection by keyboard
        Object* res = null;
        HistoryMessage* msg = null;
        const HashMap* param = null;
        String book_view_name(L"BookView"), book_nav_name(L"BookNavigationView");
        Object* book_view = send(L"testerGetWindow", &book_view_name);
        Object* book_nav = send(L"testerGetWindow", &book_nav_name);
        assert((dynamic_cast<const Int*>(book_view))->intValue() != 0);
        assert((dynamic_cast<const Int*>(book_nav))->intValue() != 0);
        res = send(L"testerSetFocus", book_nav);
        assert((dynamic_cast<const Int*>(res))->intValue() != 0);
        delete res;
        delete send(L"commandViewZoomBestFit", null);
        sendVK(VK_HOME);
        msg = waitForBroadcast(L"selectCurrentPage");
        param = dynamic_cast<const HashMap*>(msg->param);
        assert((dynamic_cast<const Int*>(param->get(L"page")))->intValue() == 0);
        delete msg;
        for (i = 0; i < 5; i++) {
            sendVK(VK_RIGHT);
            msg = waitForBroadcast(L"selectCurrentPage");
            param = dynamic_cast<const HashMap*>(msg->param);
            assert((dynamic_cast<const Int*>(param->get(L"page")))->intValue() == (i + 1) * 2);
            delete msg;
        }
        sendVK(VK_END);
        msg = waitForBroadcast(L"selectCurrentPage");
        param = dynamic_cast<const HashMap*>(msg->param);
        int page = (dynamic_cast<const Int*>(param->get(L"page")))->intValue();
        delete msg;
        delete send(L"commandViewZoom400", null);
        sendVK(VK_RIGHT);
        while (page > 0) {
            page -= 2;
            sendVK(VK_LEFT);
            msg = waitForBroadcast(L"selectCurrentPage");
            param = dynamic_cast<const HashMap*>(msg->param);
            assert((dynamic_cast<const Int*>(param->get(L"page")))->intValue() == page);
            delete msg;
        }
        delete send(L"commandViewZoomBestFit", null);
        delete book_view;
        delete book_nav;
*/
    }
    // TODO implement more kolobok test here

    delete wnd;
}

static DWORD WINAPI pipe_reader_func(LPVOID arg) {
    traceln(L"Test Server: Client connected.");
    for (;;) {
        String* cmd = object_pipe->readString();
        if (cmd == null) break;
        Object* obj = object_pipe->readObject();
        Object* res = object_pipe->readObject();
        HistoryMessage* msg = new HistoryMessage(cmd, obj, res);
        if (WaitForSingleObject(history_mutex, 10000) != WAIT_OBJECT_0) error();
        if (history_len >= MAX_HISTORY) {
            delete history[0];
            for (int i = 0; i < MAX_HISTORY - 1; i++) {
                history[i] = history[i + 1];
            }
            history_len = MAX_HISTORY - 1;
        }
        history[history_len++] = msg;
        if (!SetEvent(history_event)) error();
        if (!ReleaseMutex(history_mutex)) error();
    }
    traceln(L"Test Server: Client disconnected.");
    return 0;
}

void test_app(int argc, char ** argv) {
    BOOL loop = false;
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--server-loop") == 0) loop = true;
    }
    pipe = CreateNamedPipe(L"\\\\.\\pipe\\kolobok-tester",
        PIPE_ACCESS_DUPLEX | FILE_FLAG_OVERLAPPED, PIPE_TYPE_BYTE,
        1, 0x1000, 0x1000, INFINITE, null);
    if (pipe == INVALID_HANDLE_VALUE) error();
    HANDLE process = INVALID_HANDLE_VALUE;
    if (!loop) {
        // Start kolobok process
        STARTUPINFO si;
        PROCESS_INFORMATION pi;
        memset(&si, 0, sizeof(si));
        si.cb = sizeof(si);
        memset(&pi, 0, sizeof(pi));
        BOOL res = CreateProcess(null, L"../../bin/kolobok-g.exe -t",
            null, null, true, CREATE_NO_WINDOW, null, null, &si, &pi);
        if (!res) {
            trace(L"Cannot start Kolobok:");
            traceError();
            traceln(L"Warning: kolobok tests are skipped.");
            if (!CloseHandle(pipe)) error();
            return;
        }
        if (!CloseHandle(pi.hThread)) error();
        process = pi.hProcess;
    }
    do {
        OVERLAPPED pipe_overlapped;
        memset(&pipe_overlapped, 0, sizeof(pipe_overlapped));
        pipe_overlapped.hEvent = CreateEvent(null, true, true, null);
        if (pipe_overlapped.hEvent == null) error();
        // Connect to test client
        traceln(L"Test Server: Waiting for client to connect.");
        DWORD rsz = 0;
        BOOL res = ConnectNamedPipe(pipe, &pipe_overlapped);
        if (res) error();
        switch (GetLastError()) { 
        case ERROR_IO_PENDING: 
            switch (WaitForSingleObject(pipe_overlapped.hEvent, loop ? INFINITE : 4000)) {
            case WAIT_OBJECT_0:
                if (!GetOverlappedResult(pipe, &pipe_overlapped, &rsz, false)) error();
                break;
            case WAIT_TIMEOUT:
                SetLastError(ERROR_SUCCESS);
                error(L"Timeout waiting for test client to connect");
                break;
            default:
                error(L"Error waiting for test client to connect");
                break;
            }
            break; 
        case ERROR_PIPE_CONNECTED: 
            break; 
        default:
            error(L"Cannot connect to test client");
            break;
        }
        if (!CloseHandle(pipe_overlapped.hEvent)) error();
        // Start client listening thread
        history_mutex = CreateMutex(null, false, null);
        if (history_mutex == null) error();
        history_event = CreateEvent(null, true, false, null);
        if (history_event == null) error();
        history_len = 0;
        object_pipe = new ObjectPipe(pipe);
        pipe_reader = CreateThread(null, 0, pipe_reader_func, null, 0, null);
        if (pipe_reader == INVALID_HANDLE_VALUE) error();
        // Run tests
        test_connection();
        test_xyz();
        post(L"commandFileExit", null);
        delete waitForBroadcast(L"commandFileExit");
        // Disconnect client
        if (!DisconnectNamedPipe(pipe)) error(L"Cannot disconnect from test client");
        WaitForSingleObject(pipe_reader, INFINITE);
        // Cleanup server state
        delete object_pipe;
        object_pipe = null;
        if (!CloseHandle(pipe_reader)) error();
        if (!CloseHandle(history_mutex)) error();
        if (!CloseHandle(history_event)) error();
        pipe_reader = INVALID_HANDLE_VALUE;
        history_mutex = null;
        history_event = null;
        for (int i = 0; i < MAX_HISTORY; i++) {
            delete history[i];
            history[i] = 0;
        }
        history_len = 0;
    }
    while (loop);
    if (process != INVALID_HANDLE_VALUE) {
        WaitForSingleObject(process, INFINITE);
        if (!CloseHandle(process)) error();
    }
    if (!CloseHandle(pipe)) error();
}

HWND getApplicationMainWindow() {
    return null;
}

int winMain(HINSTANCE instance, int argc, uchar* argv[], uchar* env, int show) {
    assert(false);
    return -1;
}
