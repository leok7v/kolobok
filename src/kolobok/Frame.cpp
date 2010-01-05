#include "resource.h"
#include "Frame.h"
#include "Application.h"
#include "BorderLayout.h"
#include "Database.h"
#include "Controls.h"
#include "Skin.h"
#include "FolderIterator.h"

static const uchar className[] = L"KolobokApplication";

static void createChildren(void*);

Frame::Frame(HWND parent) : Panel() {
    const int cs = CS_HREDRAW|CS_VREDRAW|CS_DROPSHADOW;
    WNDCLASS wc = { cs, Panel::_wndProc, 0, 0, main.getInstance(), null, 
                    (HCURSOR)LoadCursor(null, IDC_ARROW),
                    main.getSkin().getBrush(L"application_background"),
                    null, className };
    ATOM a = RegisterClass(&wc);
    assert(a != null);
    HMENU menu = null;
    HACCEL at = null;
    main.getSkin().getTopLevelMenu(menu, at);
    MessageQueue::setAcceleratorTable(at);
    const String &title = *main.getSkin().getText(L"application_title_russian");
    hwnd = CreateWindowEx(WS_EX_COMPOSITED, className, 
                          (const uchar *)title, WS_OVERLAPPEDWINDOW, 
                          10, 10, 800, 600, parent, 
                          menu, main.getInstance(), (Panel*)this);
    assert(hwnd != null);
    HICON icon = LoadIcon(main.getInstance(), MAKEINTRESOURCE(IDI_MAIN));
    PostMessage(hwnd, WM_SETICON, ICON_BIG, (LPARAM)icon);
    SetTimer(hwnd, 0xFFFF, 100, null);
    MessageQueue::invokeLater(createChildren, null);
}

Frame::~Frame() {
    assert(hwnd == null);
}

static void commandFileOpen() {
    String* s = File::getSpecialFolder(CSIDL_MYPICTURES);
    assert(s != null);
    String mypictures(s);
    delete s; s = null;    
    OPENFILENAME ofn = {sizeof(OPENFILENAME)};
    // should be about enough for 1000 files
    const int MAX_RETURN = 256*1024;
    uchar* file = new uchar[MAX_RETURN];
    ofn.hwndOwner = getApplicationMainWindow();
    ofn.lpstrFile = file;
    ofn.nMaxFile = MAX_RETURN;
    ofn.lpstrFilter = L"All\0*.*\0Pictures\0*.jpg\0Collections\0*.klb\0";
    ofn.nFilterIndex = 1;
    ofn.lpstrFileTitle = file;
    ofn.nMaxFileTitle = MAX_RETURN;
    ofn.nMaxFileTitle = 0;
    ofn.lpstrInitialDir = mypictures;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_ALLOWMULTISELECT | 
                OFN_EXPLORER | OFN_ENABLESIZING | OFN_NOCHANGEDIR;
    if (GetOpenFileName(&ofn)) {
        ArrayList files;
        // double zero terminated list. Folder name is first
        uchar* p = file;
        String folder(file);
//      traceln(folder);
        p += ustrlen(p) + 1;
        while (p[0] != 0 && p[1] != null) {
            String* fname = folder + p;
            files.add(fname);
//          traceln(*fname);
            delete fname;
            p += ustrlen(p) + 1;
        }
        // special case signle file name - than folder is this filename:
        if (files.size() == 0) files.add(folder);
        traceln(files);
    }
    delete file;
}

static void newDatabaseCallback(void *pv, int error, Database *db) {
    if (error) {
        trace(L"Error creating database: %d\n", error);
    }
    else {
        main.setDatabase(db);
    }
}

static void commandFileNew() {
    String* s = File::getSpecialFolder(CSIDL_PERSONAL);
    assert(s != null);
    String mydocs(s);
    delete s; s = null;
    OPENFILENAME ofn = {sizeof(OPENFILENAME)};
    const int MAX_RETURN = MAX_PATH;
    uchar* file = new uchar[MAX_RETURN];
    ofn.hwndOwner = getApplicationMainWindow();
    ofn.lpstrFile = file;
    ofn.nMaxFile = MAX_RETURN;
    ofn.lpstrFilter = L"All\0*.*\0Collections\0*.klb\0";
    ofn.lpstrDefExt = L"klb";
    ofn.nFilterIndex = 2;
    ofn.lpstrFileTitle = file;
    ofn.nMaxFileTitle = MAX_RETURN;
    ofn.nMaxFileTitle = 0;
    ofn.lpstrInitialDir = mydocs;
    ofn.Flags = OFN_EXPLORER | OFN_ENABLESIZING | OFN_NOCHANGEDIR | 
        OFN_OVERWRITEPROMPT;
    if (GetSaveFileName(&ofn)) {
        Database::open(file, true, newDatabaseCallback, null);
    }
    delete file;
}

void Frame::eventReceived(const Message& msg) {
    if (ustrcmp(msg.id, L"updateCommandsState") != 0) {
        const Object* p = dynamic_cast<const Object*> (msg.param);
        String* s = p == null ? null : p->toString();
        traceln(L"Frame::eventReceived: %s(%s)", msg.id, 
                s == null ? L"null" : (const uchar*)*s);
        delete s;
    }
    if (ustrcmp(msg.id, L"commandFileExit") == 0)
        PostMessage(hwnd, WM_CLOSE, 0, 0);
    else if (ustrcmp(msg.id, L"commandFileOpen") == 0)
        commandFileOpen();
    else if (ustrcmp(msg.id, L"commandFileNew") == 0)
        commandFileNew();
    else if (ustrcmp(msg.id, L"commandEditCopy") == 0)
        traceln(L"commandEditCopy");
    else if (ustrcmp(msg.id, L"updateCommandsState") == 0) {
        HashMap& map = *(HashMap*)dynamic_cast<const HashMap*>(msg.param);
        map.put(L"commandFileExit", true);
        map.put(L"commandFileOpen", true);
        map.put(L"commandFileNew", true);
        map.put(L"commandEditCopy", true);
    }
    else if (ustrcmp(msg.id, L"databaseChanged") == 0) {
        const String &title = *(main.getSkin().getText(L"application_title_russian"));
        StringBuffer buf;
        File file(main.getDatabase().getName());
        buf.printf(L"%s - %s", (const uchar *)(title), file.getBasename());
        SetWindowText(hwnd, buf);
    }
    else if (ustrcmp(msg.id, L"commandImport") == 0) {
        MessageQueue::post(L"createSampleDatabase", null);
    }
    else if (ustrcmp(msg.id, L"commandAddToTray") == 0) {
        FolderIterator fs(L"C:\\xepec.com\\projects\\kolobok\\src");
        while (fs.hasNext()) {
            String file = fs.next(null);
            traceln(file);
        }
    }
    else {
//      traceln(msg.id);
    }
}

int Frame::destroy() {
    SetMenu(hwnd, null);
    MessageQueue::initiateShutdown();
    // delete all Panel children on destroy
    HWND child = GetWindow(hwnd, GW_CHILD);
    while (child != null) {
        Panel* p = getPanel(child);
        if (p != null) {
            p->destroy();
            delete p;
        }
        child = GetWindow(child, GW_HWNDNEXT);
    }
    return Panel::destroy();
}

int Frame::wndProc(UINT msg, WPARAM wp, LPARAM lp) {
    switch (msg) {
        case WM_CLOSE: check(DestroyWindow(hwnd)); return 0;
        case WM_DESTROY: return destroy();
        case WM_COMMAND: return main.getSkin().dispatchMenu(GetMenu(hwnd), LOWORD(wp));
        case WM_TIMER: return main.getSkin().updateCommandsState(GetMenu(hwnd));
        default:  
            return Panel::wndProc(msg, wp, lp);
    }
}

struct ToolBar extends Panel { // TODO: externalize me

    ToolBar(HWND frame) : Panel(frame) { 
        // xxx placeholder of xml driven dynamic toolbar layout
        HWND b0 = Controls::createButton(L"commandImport", L"commandImport");
        SetParent(b0, hwnd);
        ShowWindow(b0, SW_SHOW);

        HWND b1 = Controls::createButton(L"commandSearch", L"commandSearch");
        SetParent(b1, hwnd);
        SetWindowPos(b1, null, 96, 0, -1, -1, SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOSIZE);
        ShowWindow(b1, SW_SHOW);

        HWND b2 = Controls::createButton(L"commandAnnotation", L"commandAnnotation");
        SetParent(b2, hwnd);
        SetWindowPos(b2, null, 192, 0, -1, -1, SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOSIZE);
        ShowWindow(b2, SW_SHOW);

        HWND b4 = Controls::createButton(L"commandAddToTray", L"commandAddToTray");
        SetParent(b4, hwnd);
        SetWindowPos(b4, null, 256, 0, -1, -1, SWP_NOZORDER | SWP_NOACTIVATE);
        ShowWindow(b4, SW_SHOW);
        // xxx
        
        Window::put(hwnd, L"preferred", Point(Int.MAX_VALUE, 53));
        Window::put(hwnd, L"layout", L"north");
        ShowWindow(hwnd, SW_SHOW);
    }
};

struct StatusBar extends Panel { // TODO: externalize me

    StatusBar(HWND frame) : Panel(frame) { 
        Window::put(hwnd, L"preferred", Point(Int.MAX_VALUE, 24));
        Window::put(hwnd, L"layout", L"south");
        ShowWindow(hwnd, SW_SHOW);
    }

    int StatusBar::paint() {
        PAINTSTRUCT ps = {0};
        BeginPaint(hwnd, &ps);
        RECT rc;
        GetClientRect(hwnd, &rc);
        const Image* corner = main.getSkin().getImage(L"resize_corner");
        Color color = corner->getPixel(0, 0);
        HBRUSH brush = CreateSolidBrush(color);
        FillRect(ps.hdc, &rc, brush);
        DeleteObject(brush);
        int w = corner->getWidth();
        int h = corner->getHeight();
        corner->bitBlt(ps.hdc, rc.right - w, rc.bottom - h, w, h, 0, 0, w, h);

        EndPaint(hwnd, &ps);
        return 0;
    }

    int StatusBar::wndProc(UINT msg, WPARAM wp, LPARAM lp) {
        switch (msg) {
            case WM_PAINT: return paint();
            default:  
                return Panel::wndProc(msg, wp, lp);
        }
    }

};

struct Views extends Panel { // TODO: externalize me

    Views(HWND frame) : Panel(frame) { 
        Window::put(hwnd, L"layout", L"center");
        ShowWindow(hwnd, SW_SHOW);
    }

    int Views::paint() {
        PAINTSTRUCT ps = {0};
        BeginPaint(hwnd, &ps);
        Rect rc;
        GetClientRect(hwnd, rc);
        const Image& test = *main.getSkin().getImage(L"warning48x48");
        int w = test.getWidth();
        int h = test.getHeight();
        FillRect(ps.hdc, rc, main.getSkin().getBrush(L"application_background"));
        test.bitBlt(ps.hdc, 0, 0, w, h, 0, 0, w, h);
        EndPaint(hwnd, &ps);
        return 0;
    }

    int Views::wndProc(UINT msg, WPARAM wp, LPARAM lp) {
        switch (msg) {
            case WM_PAINT: return paint();
            default:  
                return Panel::wndProc(msg, wp, lp);
        }
    }

};

static void createChildren(void*) {
    HWND hwnd = getApplicationMainWindow();
    Panel* frame = Panel::getPanel(hwnd);
    new ToolBar(hwnd);
    new StatusBar(hwnd);
    new Views(hwnd);
    frame->doLayout();
}
