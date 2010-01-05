#include "BorderLayout.h"
#include "Application.h"
#include "Skin.h"

int getPreferredWidth(HWND child) {
    const Point* pt = dynamic_cast<const Point*>(Window::get(child, L"preferred"));
    if (pt != null)
        return pt->x;
    RECT rc = {0};
    GetClientRect(child, &rc);
    return rc.right - rc.left;
}

int getPreferredHeight(HWND child) {
    const Point* pt = dynamic_cast<const Point*>(Window::get(child, L"preferred"));
    if (pt != null)
        return pt->y;
    RECT rc = {0};
    GetClientRect(child, &rc);
    return rc.bottom - rc.top;
}

static void setWindowPos(HWND hwnd, int x, int y, int w, int h) {
    Rect rc;
    GetWindowRect(hwnd, rc);
    HWND parent = GetParent(hwnd);
    assert(parent != null);
    ScreenToClient(parent, (POINT*)&rc.left);
    ScreenToClient(parent, (POINT*)&rc.right);
    if (x != rc.left || y != rc.top || w != rc.getWidth() || h != rc.getHeight()) {
        static const int flags = SWP_NOZORDER|SWP_NOACTIVATE;
        SetWindowPos(hwnd, null, x, y, w, h, flags);
        traceln("setWindowPos(%d %d %d %d) was (%d %d %d %d)", x, y, w, h, rc.left, rc.top, rc.getWidth(), rc.getHeight());
    }
}

int BorderLayout::doLayout(HWND hwnd) {
    HashMap children;
    HWND child = GetWindow(hwnd, GW_CHILD);
    while (child != null) {
        if (IsWindowVisible(child) && GetProp(child, L"dialog") == null && 
            Window::getStr(child, L"layout") != null) {
            String layout(Window::getStr(child, L"layout"));
            assert(!children.containsKey(&layout));
            children.put(layout, (int)child);
        }
        child = GetWindow(child, GW_HWNDNEXT);
    }
    if (children.size() == 0) {
        return 0;
    }
    RECT rc = {0};
    GetClientRect(hwnd, &rc);
    if (children.size() == 1) {
        child = GetWindow(hwnd, GW_CHILD);
        setWindowPos(child, 0, 0, rc.right, rc.bottom);
    }
    else {
        const int gap = 0;
        RECT rc1 = rc;
        rc1.left += gap; rc1.right -= gap;
        rc1.top += gap; rc1.bottom -= gap;

        if (children.get(L"west") != null) {
            child = (HWND)((dynamic_cast<const Int*>(children.get(L"west")))->intValue());
            int w = getPreferredWidth(child);
            setWindowPos(child, rc1.left, rc1.top, w, rc1.bottom-rc1.top);
            rc1.left += w + gap;
        }
        if (children.get(L"east") != null) {
            child = (HWND)((dynamic_cast<const Int*>(children.get(L"east")))->intValue());
            int w = getPreferredWidth(child);
            setWindowPos(child, rc1.right - w, rc1.top, w, rc1.bottom-rc1.top);
            rc1.right -= w + gap;
        }
        if (children.get(L"north") != null) {
            child = (HWND)((dynamic_cast<const Int*>(children.get(L"north")))->intValue());
            int h = getPreferredHeight(child);
            setWindowPos(child, rc1.left, rc1.top, rc1.right-rc1.left, h);
            rc1.top += h + gap;
        }
        if (children.get(L"south") != null) {
            child = (HWND)((dynamic_cast<const Int*>(children.get(L"south")))->intValue());
            int h = getPreferredHeight(child);
            setWindowPos(child, rc1.left, rc1.bottom - h, rc1.right-rc1.left, h);
            rc1.bottom -= h + gap;
        }
        if (children.get(L"center") != null) {
            child = (HWND)((dynamic_cast<const Int *>(children.get(L"center")))->intValue());
            setWindowPos(child, rc1.left, rc1.top, rc1.right-rc1.left, rc1.bottom-rc1.top);
        }
    }
    return 0;
}

