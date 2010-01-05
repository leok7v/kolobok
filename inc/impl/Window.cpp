#include "tiny.h"

void Window::addListener(HWND hwnd, Window::Listener* sink) {
    assert(hwnd != null);
    Int listener((int)sink);
    HashMap* map = (HashMap*)GetProp(hwnd, L"system");
    if (map == null) SetProp(hwnd, L"system", map = new HashMap());
    HashSet* set = (HashSet*)dynamic_cast<const HashSet*>(map->get(L"listeners"));
    if (set == null) {
        map->put(L"listeners", HashSet());
        set = (HashSet*)dynamic_cast<const HashSet*>(map->get(L"listeners"));
    }
    else if (set->contains(&listener)) {
        traceln(L"Warning: addListener(hwnd:0x%08X, sink:0x%08X) called twice", hwnd, sink);
    }
    set->add(&listener);
}

void Window::removeListener(HWND hwnd, Window::Listener* sink) {
    assert(hwnd != null);
    Int listener((int)(void*)sink);
    HashMap* map = (HashMap*)GetProp(hwnd, L"system");
    if (map != null) {
        HashSet* set = (HashSet*)dynamic_cast<const HashSet*>(map->get(L"listeners"));
        if (set->contains(&listener)) {
            set->remove(&listener);
            if (set->size() > 0) map->put(L"listeners", set);
            else map->remove(L"listeners");
            return;
        }
    }
    traceln(L"Warning: removeListener(hwnd:0x%08X, sink:0x%08X) is excesive", hwnd, sink);
}

void Window::put(HWND hwnd, const uchar* id, const Object* o) {
    HashMap* map = (HashMap*)GetProp(hwnd, L"user");
    if (map == null) SetProp(hwnd, L"user", map = new HashMap());
    map->put(id, o);
}

void Window::put(HWND hwnd, const uchar* id, const uchar* s) {
    put(hwnd, id, String(s));
}

void Window::put(HWND hwnd, const uchar* id, int n) {
    put(hwnd, id, Int(n));
}

const Object* Window::get(HWND hwnd, const uchar* id) {
    HashMap* map = (HashMap*)GetProp(hwnd, L"user");
    if (map == null) return null;
    return map->get(id);
}

void Window::notifyListeners(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
    HashMap* map = (HashMap*)GetProp(hwnd, L"system");
    if (map != null) {
        const HashSet* set = dynamic_cast<const HashSet*>(map->get(L"listeners"));
        if (set != null) {
            assert(set->size() > 0);
            HashSet* clone = dynamic_cast<HashSet*>(set->clone());
            for (HashSet::Iterator i(clone); i.hasNext(); ) {
                const Int& pv = *dynamic_cast<const Int*>(i.next());
                ((Window::Listener*)pv.intValue())->eventReceived(hwnd, msg, wp, lp);
            }
            delete clone;
        }
    }
}

void Window::notifyDestroy(HWND hwnd) {
    // can be called multiple times
    delete (HashMap*)RemoveProp(hwnd, L"user");
    delete (HashMap*)RemoveProp(hwnd, L"system");
}

static const uchar className[] = L"Window";
static HashMap atoms;

HWND Window::create(HWND parent, int styleEx, int style, WNDPROC wndProc, HBRUSH brush, void* that) {
    const int cs = CS_HREDRAW|CS_VREDRAW; /*|CS_DROPSHADOW; */
    const String* classname = atoms.getString((int)brush);
    if (classname == null) {
        String* bs = Int((int)brush).toString();
        String* cn = String(className) + bs;
        WNDCLASS wc = { cs, wndProc, 0, 0, GetModuleHandle(null), null,
                        (HCURSOR)LoadCursor(null, IDC_ARROW),
                        brush, null, *cn };
        ATOM atom = RegisterClass(&wc);
        assert(atom != null);
        atoms.put((int)brush, cn);
        delete bs;
        delete cn;
        classname = atoms.getString((int)brush);
    }
    return CreateWindowEx(styleEx, *classname, null, style, 
            0, 0, 0, 0, parent, null, GetModuleHandle(null), that);
}

Rect Window::getBounds(HWND hwnd) {
    HWND parent = GetParent(hwnd);
    assert(parent != null);
    Rect rc;
    GetWindowRect(hwnd, rc);
    POINT* lt = (POINT*)&rc.left;
    POINT* rb = (POINT*)&rc.right;
    ScreenToClient(parent, lt);
    ScreenToClient(parent, rb);
    return rc;
}

String Window::getText(HWND hwnd) {
    int n = 1024;
    uchar* buf = new uchar[n];
    int k = GetWindowText(hwnd, buf, n);
    while (n < 1024*1024 && GetWindowText(hwnd, buf, n) == n - 1) {
        delete buf;
        n = n * 2;
        buf = new uchar[n];
    }
    String s(buf);
    delete buf;
    return s;
}
