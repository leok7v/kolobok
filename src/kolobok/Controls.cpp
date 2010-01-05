#include "Controls.h"
#include "Application.h"
#include "Panel.h"
#include "Skin.h"
#include "ToolTips.h"

struct ButtonPanel extends Panel {

    enum { 
        NORESIZE = 0,
        HRESIZE = 1,
        VRESIZE = 2 
    };

    ButtonPanel(const uchar* image, const uchar* command, int resize) : Panel(null), 
        iid(image), cmd(command), btn(null), hot(false), tt(null), resizeable(resize) { 
        assert((resize & VRESIZE) == 0); // TODO: not implemented yet
        SetParent(hwnd, null);
        int w = 0, h = 0;
        const Image& i = *getImage(L"_normal");
        w = i.getWidth();
        h = i.getHeight();
        btn = CreateWindowEx(0, L"Button", null,
                            WS_CHILD|WS_VISIBLE|WS_CLIPSIBLINGS|BS_OWNERDRAW|BS_PUSHBUTTON,
                            0, 0, w, h, hwnd, null, GetModuleHandle(null), null);
        assert(btn != null);
        check(SetWindowPos(hwnd, null, -1, -1, w, h,
                           SWP_NOZORDER|SWP_NOACTIVATE|SWP_NOMOVE|SWP_SHOWWINDOW));
        Window::put(btn, L"command", cmd);
        const String* s = main.getSkin().getToolTipText(iid);
        if (s != null) ToolTips::add(btn, *(tt = dynamic_cast<String*>(s->clone())));
    }

private:

    void checkHoverOver() {
        Rect rc = Window::getBounds(btn);
        Point pt;
        check(GetCursorPos(pt));
        check(ScreenToClient(btn, pt));
        bool b = PtInRect(rc, pt) != false;
        if (b != hot) {
            hot = b;
            InvalidateRect(btn, null, false);
        }
    }
    
    void eventReceived(const Message& m) {
        // updateCommandsState is used as 1/10 of a second timer
        if (ustrcmp(m.id, L"updateCommandsState") == 0) {
            checkHoverOver();
        }
        else if (ustrcmp(m.id, L"updateCommandsState") == 0) {
            // NOTE: "d" in "updated"! This message comes after everybody
            // has contributed to updateCommandsState... See Skin.cpp
            const HashMap& map = *dynamic_cast<const HashMap*>(m.param);
            bool b = IsWindowEnabled(btn) != false;
            const Int* state = map.getInt(cmd);
            bool e = state != null && state->intValue() != 0;
            if (b != e) EnableWindow(btn, e);
        }
    }

    virtual ~ButtonPanel() {
        traceln("~ButtonPanel %s", (const uchar*)iid);
        delete tt;
    }
    
    const Image* getImage(const uchar* suffix) { return getImage(iid, suffix); }
    
    static const Image* getImage(const String& iid, const uchar* suffix) {
        const Image* i = main.getSkin().getImage(iid, suffix);
        return i;
    }
    
    void drawItem(DRAWITEMSTRUCT& dis) {
        Rect rc;
        GetClientRect(dis.hwndItem, rc);
        FillRect(dis.hDC, rc, main.getSkin().getBrush(L"background"));
        const uchar* suffix = L"_normal";
        if (dis.itemState & ODS_DISABLED) suffix = L"_disabled";
        else if (dis.itemState & ODS_DEFAULT) suffix = L"_default";
        else if (dis.itemState & ODS_SELECTED) suffix = L"_pressed";
        else if (hot) suffix = L"_hot";
        assert(dis.hwndItem == btn);
        const Image& i = *getImage(suffix);
        if (resizeable == HRESIZE) {
            int l = i.getProperties().getInt(L"left")->intValue();
            int c = i.getProperties().getInt(L"center")->intValue();
            int r = i.getProperties().getInt(L"right")->intValue();
            int w = rc.right - r;
            int x = i.getWidth() - r;
            i.alphaBlend(dis.hDC, 0, 0, l, i.getHeight(),
                                  0, 0, l, i.getHeight());
            i.alphaBlend(dis.hDC, w, 0, r, i.getHeight(),
                                  x, 0, r, i.getHeight());
            int cx = (i.getWidth() - c) / 2;
            for (x = l; x < w; x += c) {
                int cw = x + c < w ? c : w - x;
                assert(cw > 0);
                i.alphaBlend(dis.hDC, x, 0, cw, i.getHeight(),
                                     cx, 0, cw, i.getHeight());
            }
        }
        else {
            i.alphaBlend(dis.hDC, 0, 0, i.getWidth(), i.getHeight(),
                              0, 0, i.getWidth(), i.getHeight());
        }
    }

    void forceSize(WINDOWPOS& ps) {
        if (!(ps.flags & SWP_NOSIZE)) { // resist size change
            const Image& i = *getImage(L"_normal");
            if ((resizeable & HRESIZE) == 0) ps.cx = i.getWidth();
            if ((resizeable & VRESIZE) == 0) ps.cy = i.getHeight();
             check(SetWindowPos(btn, null, -1, -1, ps.cx, ps.cy,
                                SWP_NOZORDER|SWP_NOACTIVATE|SWP_NOMOVE));
        }
    }

    int destroy() {
        if (tt != null) ToolTips::remove(btn);
        DestroyWindow(btn);
        MessageQueue::invokeLater(suicide, this);
        return Panel::destroy();
    }

    static void suicide(void* that) { delete (ButtonPanel*)that; }

    int wndProc(UINT msg, WPARAM wp, LPARAM lp) {
        switch (msg) 
        { 
            case WM_DRAWITEM: drawItem(*(DRAWITEMSTRUCT*)lp); return true;
            case WM_WINDOWPOSCHANGING: forceSize(*(WINDOWPOS*)lp); return 0;
            case WM_COMMAND: (new Message(cmd, null))->post(); return 0;
            case WM_DESTROY: return destroy();
        }
        return Panel::wndProc(msg, wp, lp);
    }

    HWND btn;
    bool hot;
    String  cmd;
    String  iid; // image id
    String* tt;  // tooltip
    int  resizeable;
};

HWND Controls::createButton(const uchar* imageId, const uchar* cmd) {
    return (new ButtonPanel(imageId, cmd, ButtonPanel::NORESIZE))->getWnd();
}

HWND Controls::createHorizontalButton(const uchar* imageId, const uchar* cmd) {
    return (new ButtonPanel(imageId, cmd, ButtonPanel::HRESIZE))->getWnd();
}

static void replaceText(HWND hwnd) {
    String text = Window::getText(hwnd);
    if (text.startsWith(L"text_")) {
        const String* s = main.getSkin().getText(String(text + ustrlen(L"text_")));
        if (s != null) SetWindowText(hwnd, *s);
    }
}

static void replaceIcon(HWND hwnd) {
    String text = Window::getText(hwnd);
    if (text.startsWith(L"icon_")) {
        HICON icon = main.getSkin().getIcon(String(text + ustrlen(L"icon_")));
        if (icon != null) {
            int style = GetWindowLong(hwnd, GWL_STYLE);
            SetWindowLong(hwnd, GWL_STYLE, style | SS_ICON);
            SendMessage(hwnd, STM_SETIMAGE, IMAGE_ICON, (int)icon);
        }
    }
}

static bool replaceControl(HWND parent, HWND hwnd) {
    uchar buf[64] = {0};
    GetClassName(hwnd, buf, countof(buf));
    String text = Window::getText(hwnd);
    if (text.startsWith(L"ctrl_")) {
        String name(text + ustrlen(L"ctrl_"));
        if (String(buf).equalsIgnoreCase(L"Button") && main.getSkin().containsImage(name)) {
            Rect rc = Window::getBounds(hwnd);
            int style = GetWindowLong(hwnd, GWL_STYLE);
            style = style & !(BS_ICON|BS_BITMAP|BS_OWNERDRAW);
            int id = GetWindowLong(hwnd, GWL_ID);
            HWND parent = GetParent(hwnd);
            DestroyWindow(hwnd);
            String* cmd = String(L"commandButton") + name;
            hwnd = Controls::createButton(name, *cmd);
            SetParent(hwnd, parent);
            Rect nr = Window::getBounds(hwnd);
            // keep new control centered at the center of old control
            rc.left += (rc.getWidth() - nr.getWidth()) / 2;
            rc.top  += (rc.getHeight() - nr.getHeight()) / 2;
            SetParent(hwnd, parent);
            SetWindowLong(hwnd, GWL_ID, id);
            SetWindowLong(hwnd, GWL_ID, style);
            SetWindowPos(hwnd, null, rc.left, rc.top, -1, -1,
                         SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOSIZE);
            delete cmd;
            return true;
        }
    }
    return false;
}

int CALLBACK commonDlgProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
    DLGPROC dialogProc = (DLGPROC)Window::getInt(hwnd, L"dialogProc", null);
    switch (msg) {
        case WM_INITDIALOG: {
            SetProp(hwnd, L"dialog", (void*)true);
            dialogProc = (DLGPROC)lp;
            Window::put(hwnd, L"dialogProc", (int)dialogProc);
            return dialogProc(hwnd, msg, wp, lp);
        }
        case WM_SETFONT: case WM_NCDESTROY: {
            // first WM_SETFONT arrives before WM_INITDIALOG
            // it is not clear what to do with it.
            // WM_NCDESTROY comes after DESTROY
            if (dialogProc == null) return 0; // ignore both
            break;
        }
        case WM_CTLCOLORMSGBOX: case WM_CTLCOLORDLG: {
            HDC hdc = (HDC)wp;
            SetBkColor(hdc, main.getSkin().getColor(L"background"));
            return (int)main.getSkin().getBrush(L"background");
        }
        case WM_CTLCOLORSTATIC:
        {
            HDC hdc = (HDC)wp;
            SetTextColor(hdc, main.getSkin().getColor(L"static_text"));
            SetBkMode(hdc, TRANSPARENT);
            return (int)main.getSkin().getBrush(L"background");
        }
        case WM_COMMAND: 
            if (LOWORD(wp) == IDOK || LOWORD(wp) == IDCANCEL) PostMessage(hwnd, WM_CLOSE, 0, 0);
            break;
        case WM_CLOSE: {
            SetWindowPos(getApplicationMainWindow(), HWND_TOP, -1, -1, -1, -1, SWP_NOSIZE|SWP_NOMOVE); 
            ShowWindow(hwnd, SW_HIDE);
            DestroyWindow(hwnd);
            break;
        }
    }
    assert(dialogProc != null);
    return dialogProc(hwnd, msg, wp, lp);
}

void Controls::centerInParent(HWND hwnd) {
    RECT rc0 = {0}, rc1 = {0};
    GetWindowRect(getApplicationMainWindow(), &rc0);
    GetWindowRect(hwnd, &rc1);
    int x = rc0.left + (rc0.right - rc0.left) / 2 - (rc1.right - rc1.left) / 2;
    int y = rc1.top  + (rc0.bottom - rc0.top) / 2 - (rc1.bottom - rc1.top) / 2;
    SetParent(hwnd, null);
    SetWindowPos(hwnd, HWND_TOP, x, y, -1, -1, SWP_NOSIZE|SWP_SHOWWINDOW); 
    SetFocus(hwnd);
    EnableWindow(getApplicationMainWindow(), false);
}

void Controls::skinDialog(HWND dlg) {
    assert(GetProp(dlg, L"dialog"));
    replaceText(dlg);
    // replaceControl may change order of siblings. 
    // This is why double loop with "done"...
    bool done = false;
    while (!done) {
        done = true;
        HWND child = GetWindow(dlg, GW_CHILD);
        while (child != null && done) {
            if (replaceControl(dlg, child)) done = false;
            replaceText(child);
            replaceIcon(child);
            child = GetWindow(child, GW_HWNDNEXT);
        }
    }
}

HWND Controls::createDialog(int id, DLGPROC dlgProc) {
    HWND dlg = CreateDialogParam(main.getInstance(), MAKEINTRESOURCE(id), 
                                 getApplicationMainWindow(), 
                                 commonDlgProc, (LPARAM)dlgProc);
    assert(dlg != null);
    skinDialog(dlg);
    centerInParent(dlg);
    return dlg;
}
