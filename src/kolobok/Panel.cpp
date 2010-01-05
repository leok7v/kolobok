#include "Panel.h"
#include "Application.h"
#include "BorderLayout.h"
#include "Skin.h"

Panel::Panel() : hwnd(null) {
    MessageQueue::addBroadcastListener(this);
}

Panel::Panel(HWND parent) : hwnd(null) {
    MessageQueue::addBroadcastListener(this);
    // create WS_CHILDWINDOW with parent == null fails:
    hwnd = Window::create(GetDesktopWindow(), 
                          WS_EX_COMPOSITED, WS_CHILDWINDOW, _wndProc, 
                          main.getSkin().getBrush(L"background"), 
                          (Panel*)this);
    SetParent(hwnd, parent);
    assert(hwnd != null);
}

Panel::~Panel() {
    assert(hwnd == null);
}

Panel* Panel::getPanel(HWND hwnd) {
    assert(hwnd != null);
    const Int* that = dynamic_cast<const Int*>(Window::get(hwnd, L"this"));
    return that == null ? null : (Panel*)that->intValue();
}

void Panel::eventReceived(const Message& msg) {
}

int Panel::doLayout() {
    return BorderLayout::doLayout(hwnd);
}

static int contextMenu(HWND hwnd, const Point &pt) {
    const String* name = dynamic_cast<const String*>(Window::get(hwnd, L"name"));
    if (name != null && main.getSkin().hasPopupMenu(*name)) {
	    main.getSkin().trackPopupMenu(hwnd, *name, pt);
    }
    return 0;
}

int Panel::destroy() { 
    MessageQueue::removeBroadcastListener(this);
    if (!IsWindow(hwnd)) {
        traceln("Panel::destroy() IsWindow(0x%04X) == false", hwnd);
    }
    else {
        Window::put(hwnd, L"this", null);
    }
    hwnd = null; 
    return 0; 
}

int Panel::wndProc(UINT msg, WPARAM wp, LPARAM lp) {
    switch (msg) {
        case WM_CONTEXTMENU: return contextMenu(hwnd, Point(LOWORD(lp), HIWORD(lp)));
	case WM_WINDOWPOSCHANGED: return doLayout();
    }
    return DefWindowProc(hwnd, msg, wp, lp);
}

LRESULT CALLBACK Panel::_wndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
    Panel* panel = getPanel(hwnd);
    if (msg == WM_NCCREATE) {
        assert(panel == null);
	CREATESTRUCT* cs = (CREATESTRUCT*)lp;
	Window::put(hwnd, L"this", (int)cs->lpCreateParams);
        panel = (Panel*)cs->lpCreateParams;
        panel->hwnd = hwnd; // need as early as possible
    }
    int result = panel != null ? panel->wndProc(msg, wp, lp) : 
                 DefWindowProc(hwnd, msg, wp, lp);
    if (msg == WM_DESTROY && panel != null) {
        Window::put(hwnd, L"this", null);
    }
    return result;
}
