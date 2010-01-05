#include "resource.h"
#include "Skin.h"

static int menu_cmd_id = 42000;

Skin::Skin(String& ver) : version(ver), skin(null), ui(null) {
    load();
}

Skin::~Skin() {
    for (HashMap::ValueIterator i(menus); i.hasNext(); ) {
        const Int &m = *dynamic_cast<const Int*>(i.next());
        destroyMenu((HMENU)m.intValue());
    }
    clearCache();
    delete skin;
    delete ui;
}

void Skin::load() {
    delete skin;
    delete ui;
    clearCache();
    String* s = null;
    File skin_xml(L"skin.xml");
    File skin_png(L"skin.png");
    s = Resources::getString(ID_SKIN_XML);
    skin = dynamic_cast<HashMap*>(XMLDecoder::decode(s));
    ui = Resources::getImage(ID_SKIN_PNG);
    delete s; s = null;
    if (skin_xml.exists() && skin_png.exists()) {
        s = skin_xml.readUTF8();
        HashMap* fskin = dynamic_cast<HashMap*>(XMLDecoder::decode(s));
        delete s; s = null;
        const String* fv = fskin->getString(L"version");
        const String* rv =  skin->getString(L"version");
        // if versions are the same favor file based skin:
        if (Object::equals(fv, rv)) { 
            delete skin;
            skin = fskin;
            delete ui;
            ui = new Image(skin_png);
        }
        else {
            delete fskin;
        }
    }
}

void Skin::eventReceived(const Message& msg) {
    if (ustrcmp(msg.id, L"commandHelpPackSkin") == 0)
        pack();
    else if (ustrcmp(msg.id, L"commandHelpUnpackSkin") == 0)
        unpack();
    else if (ustrcmp(msg.id, L"commandHelpReloadSkin") == 0) {
        load();
        InvalidateRect(getApplicationMainWindow(), null, false);
    }
    else if (ustrcmp(msg.id, L"updateCommandsState") == 0) {
        HashMap& map = *(HashMap*)dynamic_cast<const HashMap*>(msg.param);
        map.put(L"commandHelpReloadSkin", true);
        map.put(L"commandHelpPackSkin", canPack());
        map.put(L"commandHelpUnpackSkin", !canPack());
    }
}

void Skin::clearCache() {
    for (HashMap::ValueIterator i(cache); i.hasNext(); ) {
        const Int &m = *dynamic_cast<const Int*>(i.next());
        delete (Image*)m.intValue(); 
    }
    cache.clear();
    for (HashMap::ValueIterator b(brushes); b.hasNext(); ) {
        const Int &brush = *dynamic_cast<const Int*>(b.next());
        DeleteObject((HBRUSH)brush.intValue());
    }
    brushes.clear();
    for (HashMap::ValueIterator k(icons); k.hasNext(); ) {
        const Int &icon = *dynamic_cast<const Int*>(k.next());
        DeleteObject((HICON)icon.intValue());
    }
    icons.clear();
}
    
const String* Skin::getText(const uchar* id) {
    const HashMap& texts = *dynamic_cast<const HashMap*>(skin->get(L"Text"));
    return dynamic_cast<const String*>(texts.get(id));
}

const HashMap* Skin::getTextMap(const uchar* id) {
    const HashMap& texts = *dynamic_cast<const HashMap*>(skin->get(L"Text"));
    return dynamic_cast<const HashMap*>(texts.get(id));
}

const String* Skin::getToolTipText(const uchar* id) {
    const HashMap& tooltips = *dynamic_cast<const HashMap*>(skin->get(L"ToolTips"));
    return dynamic_cast<const String*>(tooltips.get(id));
}

struct Accel extends Object {
    ACCEL a;
    Accel() { ACCEL a0 = {0}; a = a0; }
    Accel(const ACCEL &a0) { a = a0; }
    Object* clone() const { return new Accel(a); }
};

static void decodeAccelerator(const String& acl, int id, ArrayList &at) {
    String* str = dynamic_cast<String*>(acl.clone()); 
    Accel a;
    a.a.cmd = (WORD)id;
    a.a.fVirt |= FVIRTKEY;
    for (;;) {
        if (str->startsWithIgnoreCase(L"Alt+")) {
            a.a.fVirt |= FALT;
            String* tail = new String(&((const uchar*)*str)[4], str->length() - 4);
            delete str;
            str = tail;
        }
        else if (str->startsWithIgnoreCase(L"Ctrl+")) {
            a.a.fVirt |= FCONTROL;
            String* tail = new String(&((const uchar*)*str)[5], str->length() - 5);
            delete str;
            str = tail;
        }
        else if (str->startsWithIgnoreCase(L"Shift+")) {
            a.a.fVirt |= FSHIFT;
            String* tail = new String(&((const uchar*)*str)[6], str->length() - 6);
            delete str;
            str = tail;
        }
        else {
            break;
        }
    }
    if (str->length() > 1 && str->charAt(0) == 'F' &&
        '1' <= str->charAt(1) && str->charAt(1) <= '9') {
        const uchar* s = *str;
        a.a.key = (WORD)(VK_F1 + (int)Int::decode(&s[1]));
    }
    else {
        assert(str->length() == 1);
        uchar ch = towupper(str->charAt(0));
        assert(L'A' <= ch && ch <= L'Z' || L'0' <= ch && ch <= L'9');
        a.a.key = str->charAt(0);
    }
    delete str; str = null;
    at.add(&a);
}

static void insertItem(HMENU menu, int pos, const String& key, const String& acl, 
                       const String& cmd, int &id, ArrayList &at) {
    StringBuffer title(key);
    if (acl.length() > 0) {
        title.printf(L"\t%s", (const uchar*)acl);
        decodeAccelerator(acl, id, at);
    }
    MENUITEMINFO mii = {sizeof(MENUITEMINFO)};
    mii.fMask = MIIM_TYPE|MIIM_ID|MIIM_DATA;
    mii.dwTypeData = (uchar*)(const uchar*)title;
    mii.cch = title.length();
    mii.wID = id++;
    mii.dwItemData = (ULONG_PTR)dynamic_cast<String*>(cmd.clone());
    BOOL b = InsertMenuItem(menu, pos, true, &mii);
    if (!b) traceError();
    assert(b);
}

static HMENU getSubMenu(const LinkedList* list, int &id, ArrayList &at) {
    int count = 0;
    HMENU menu = CreateMenu();
    for (LinkedList::Iterator i(list); i.hasNext(); ) {
        const String& key = *dynamic_cast<const String*>(i.next());
        const String& acl = *dynamic_cast<const String*>(i.next());
        const String& cmd = *dynamic_cast<const String*>(i.next());
        insertItem(menu, ++count, key, acl, cmd, id, at);
    }
    return menu;
}

void Skin::getTopLevelMenu(HMENU &menu, HACCEL &accel) {
    menu = getMenu(L"TopLevelMenu", true, accel);
}

HMENU Skin::getMenu(const uchar* name, bool toplevel, HACCEL &accel) {
    const Int* m = dynamic_cast<const Int*>(menus.get(name));
    if (m != null) return (HMENU)m->intValue();
    ArrayList at;
    int count = 0;
    HMENU menu = toplevel ? CreateMenu() : CreatePopupMenu();
    menus.put(name, (int)menu);
    const HashMap&  menus = *dynamic_cast<const HashMap*>(skin->get(L"Menus"));
    const LinkedList* list = dynamic_cast<const LinkedList*>(menus.get(name));
    if (list == null) return null;
    for (LinkedList::Iterator i(list); i.hasNext(); ) {
        const String&  key = *dynamic_cast<const String*>(i.next());
        const Object* val = dynamic_cast<const Object*>(i.next());
        if (instanceof(val, const LinkedList*)) {
            const LinkedList* sub = dynamic_cast<const LinkedList*>(val);
            MENUITEMINFO mii = {sizeof(MENUITEMINFO)};
            mii.fMask = MIIM_SUBMENU|MIIM_TYPE;
            mii.fType = MFT_STRING;
            mii.dwTypeData = (uchar*)(const uchar*)key;
            mii.cch = key.length();
            mii.hSubMenu = getSubMenu(sub, menu_cmd_id, at);
            check(InsertMenuItem(menu, ++count, true, &mii));
        }
        else {
            const String& acl = *dynamic_cast<const String*>(val);
            const String& cmd = *dynamic_cast<const String*>(i.next());
            insertItem(menu, ++count, key, acl, cmd, menu_cmd_id, at);
        }
    }
    if (toplevel) {
        ACCEL* t = new ACCEL[at.size()];
        for (int i = 0; i < at.size(); i++) {
            t[i] = dynamic_cast<const Accel*>(at.get(i))->a;
        }
        accel = CreateAcceleratorTable(t, at.size());
        delete t;
    }
    return menu;
}

static void destroyMenuItem(HMENU menu, int i) {
    MENUITEMINFO mii = {sizeof(MENUITEMINFO)};
    mii.fMask = MIIM_DATA;
    check(GetMenuItemInfo(menu, i, true, &mii));
    String* cmd = (String*)mii.dwItemData;
    delete cmd;
}

void Skin::destroyMenu(HMENU menu) {
    for (int i = 0; i < GetMenuItemCount(menu); i++) {
        HMENU sub = GetSubMenu(menu, i);
        if (sub == null) destroyMenuItem(menu, i);
        else {
            for (int j = 0; j < GetMenuItemCount(sub); j++) 
                destroyMenuItem(sub, j);
        }
    }
}

static void dispatchMenuItem(HMENU menu, int i, int id) {
    MENUITEMINFO mii = {sizeof(MENUITEMINFO)};
    mii.fMask = MIIM_ID|MIIM_DATA;
    check(GetMenuItemInfo(menu, i, true, &mii));
    if ((int)mii.wID == id) {
        String& cmd = *(String*)mii.dwItemData;
        MessageQueue::post(cmd, null);
    }
}

int Skin::dispatchMenu(HMENU menu, int id) {
    for (int i = 0; i < GetMenuItemCount(menu); i++) {
        HMENU sub = GetSubMenu(menu, i);
        if (sub == null) dispatchMenuItem(menu, i, id);
        else {
            for (int j = 0; j < GetMenuItemCount(sub); j++)
                dispatchMenuItem(sub, j, id);
        }
    }
    return 0;
}

static const String* getMenuItemCommand(HMENU menu, int i) {
    MENUITEMINFO mii = {sizeof(MENUITEMINFO)};
    mii.fMask = MIIM_DATA;
    check(GetMenuItemInfo(menu, i, true, &mii));
    assert(mii.dwItemData != 0);
    return (String*)mii.dwItemData;
}

struct UpdateCommandsStateMessage extends Message {
    
    UpdateCommandsStateMessage(HMENU m, const uchar* i, const HashMap* p) : menu(m), Message(i, p) { }

    virtual ~UpdateCommandsStateMessage() { 
        const HashMap* map = dynamic_cast<const HashMap*>(param);
        for (int i = 0; i < GetMenuItemCount(menu); i++) {
            HMENU sub = GetSubMenu(menu, i);
            if (sub == null) {
                int enabled = dynamic_cast<const Int*>(map->get(getMenuItemCommand(menu, i)))->intValue();
                int flags = enabled ? MF_ENABLED : (MF_DISABLED|MF_GRAYED);
                EnableMenuItem(menu, i, MF_BYPOSITION | flags);
            }
            else {
                for (int j = 0; j < GetMenuItemCount(sub); j++) {
                    int enabled = dynamic_cast<const Int*>(map->get(getMenuItemCommand(sub, j)))->intValue();
                    int flags = enabled ? MF_ENABLED : (MF_DISABLED|MF_GRAYED);
                    EnableMenuItem(sub, j, MF_BYPOSITION | flags);
                }
            }
        }
        (new Message(L"updateCommandsState", map->clone()))->post();
    }

    virtual void dispatch() {
        Message::dispatch();
    }

    HMENU menu;
};

int Skin::updateCommandsState(HMENU menu) {
    HashMap* map = new HashMap();
    for (int i = 0; i < GetMenuItemCount(menu); i++) {
        HMENU sub = GetSubMenu(menu, i);
        if (sub == null) {
            map->put(getMenuItemCommand(menu, i), false);
        }
        else {
            for (int j = 0; j < GetMenuItemCount(sub); j++) {
                map->put(getMenuItemCommand(sub, j), false);
            }
        }
    }
    (new UpdateCommandsStateMessage(menu, L"updateCommandsState", map))->post();
    return 0;
}

bool Skin::hasPopupMenu(const uchar* id) {
    HACCEL ha = null;
    return getMenu(id, false, ha) != null;
}

int Skin::trackPopupMenu(HWND hwnd, const uchar* id, const Point& pt) {
    HACCEL ha = null;
    HMENU popup = getMenu(id, false, ha);
    int cmd = TrackPopupMenu(popup, TPM_RETURNCMD|TPM_NONOTIFY, pt.x, pt.y, 0, hwnd, null);
    return dispatchMenu(popup, cmd);
}

/* from Windows XP GUI guidelines:
  Franklin Gothic is used only for text over 14 point. Franklin Gothic is often used for headers, 
  and should never be used for body text.
  Tahoma is used as the system's default font. Tahoma should be used at 8, 9 or 11 point sizes.
  Verdana is used only for title bars of tear-off/floating palettes- Verdana Bold, 8 point.
  Trebuchet MS is used only for the title bars of windows- Trebuchet MS Bold, 10 point.
*/

static int getHAlign(const String& a) {
    if (a.equalsIgnoreCase(L"center")) return TA_CENTER;
    else if (a.equalsIgnoreCase(L"left")) return TA_LEFT;
    else if (a.equalsIgnoreCase(L"right")) return TA_RIGHT;
    else assert(false);
    return 0;
}

static int getVAlign(const String& a) {
    if (a.equalsIgnoreCase(L"top")) return TA_TOP;
    else if (a.equalsIgnoreCase(L"bottom")) return TA_BOTTOM;
    else if (a.equalsIgnoreCase(L"baseline")) return TA_BASELINE;
    else assert(false);
    return 0;
}

static void renderText(Image& image, const Point location, 
                       const String& halign, const String& valign, 
                       const String& s, const String& fontname,
                       int points, int weight, COLORREF color) {
    const int w = image.getWidth(), h = image.getHeight();
    HDC mdc = CreateCompatibleDC(GetDC(null));
    HBITMAP bm = CreateCompatibleBitmap(GetDC(null), w, h);
    HFONT font = CreateFont(points, 0, 0, 0, weight, false, false, false,
        ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        ANTIALIASED_QUALITY, DEFAULT_PITCH, fontname);
    Rect rc(0, 0, w, h);
    HBITMAP ob = (HBITMAP)SelectObject(mdc, bm);
    check(ob != null);
    FillRect(mdc, rc, (HBRUSH)GetStockObject(BLACK_BRUSH));
    HFONT of = (HFONT)SelectObject(mdc, font);
    check(of != null);
    SetBkMode(mdc, TRANSPARENT);
    SetTextColor(mdc, RGB(255, 255, 255));
    SetTextAlign(mdc, getHAlign(halign) | getVAlign(valign));
    TextOut(mdc, location.x, location.y, s, s.length()); // ??? x,y
    check((HFONT)SelectObject(mdc, of) == font);
    Image alpha(w, h, 32);
    check(GetDIBits(mdc, bm, 0, h, alpha.getLine(alpha.getHeight() - 1), alpha, DIB_RGB_COLORS));
    check((HBITMAP)SelectObject(mdc, ob) == bm);
    DeleteObject(font);
    DeleteObject(bm);
    for (int y = 0; y < h; y++) {
        byte* p = alpha.getLine(y);
        for (int x = 0; x < w; x++) {
            uint a = uint(p[0] + p[1] + p[2]) / 3U;
            if (a != 0) *(uint*)p = color | (a << 24U);
            p += 4;
        }
    }
    alpha.alphaBlend(image, 0, 0, w, h, 0, 0, w, h);
}

static const uchar* hprops[] = {
    L"left",
    L"center",
    L"right"
};

static Image* constructLayered(Skin& skin, const LinkedList& list, const uchar* suffix) {
    // layered image must have at least 1 element
    // all list elements are maps
    // first element of the list must be reference
    Image* image = null;
    for (LinkedList::Iterator i(list); i.hasNext(); ) {
        HashMap* map = dynamic_cast<HashMap*>(i.next()->clone());
        if (map->containsKey(L"text")) {
            if (map->containsKey(L"ref")) {
                const String& ref = map->getString(L"ref");
                const HashMap& m = *skin.getTextMap(ref);
                for (HashMap::EntryIterator k(m); k.hasNext(); ) {
                    const HashMap::Entry& e = *k.nextEntry();
                    if (!map->containsKey(e.getKey())) map->put(e.getKey(), e.getVal());
                }
            }
            const String& text = map->getString(L"text");
            const String& font = map->getString(L"font");
            double weight = map->getDouble(L"weight")->doubleValue() * 1000;
            double height = map->getDouble(L"height")->doubleValue();
            const Point& location = *dynamic_cast<const Point*>(map->get(L"location"));
            const String& halign = map->getString(L"halign");
            const String& valign = map->getString(L"valign");
            const Color* color = suffix != null ? map->getColor(&suffix[1]) : null;
            if (color == null) color = map->getColor(L"color");
            renderText(*image, location, halign, valign, text, font, (int)height, int(weight), *color);
        }
        else if (map->containsKey(L"ref")) {
            const String& r = *map->getString(L"ref");
            const Image& ref = *skin.getImage(r, suffix);
            const int w = ref.getWidth(), h = ref.getHeight();
            if (image == null) image = new Image(w, h, 32);
            Point pt;
            const Point* loc = dynamic_cast<const Point*>(map->get(L"location"));
            if (loc != null) pt = *loc;
            ref.alphaBlend(image, pt.x, pt.y, w, h, 0, 0, w, h);
            // copy "left", "center", "right" to Image
            for (int i = 0; i < countof(hprops); i++) {
                if (ref.getProperties().containsKey(hprops[i]))
                    image->getProperties().put(hprops[i], ref.getProperties().get(hprops[i]));
                if (map->containsKey(hprops[i]))
                    image->getProperties().put(hprops[i], map->get(hprops[i]));
            }
        }
        else {
            trace(L"only \"text\" and \"ref\" supported. Unexpected tag");
            traceln(map);
            assert(false);
        }
        delete map;
    }
    return image;
}

const Object* Skin::getImageObject(const uchar* id, const uchar* suffix) const {
    const HashMap& images = *dynamic_cast<const HashMap*>(skin->get(L"Images"));
    const Object* v = images.get(id);
    if (v == null && suffix != null) {
        String* s = String(id) + suffix;
        v = images.get(*s);
        delete s;        
    }
    return v;
}

const Image* Skin::getImage(const uchar* id, const uchar* suffix) {
    String* full_name;
    if(suffix != null)
        full_name = String(id) + suffix;
    else
        full_name = new String(id);
    const Int* i = dynamic_cast<const Int*>(cache.get(*full_name));
    if (i != null) {
        delete full_name;
        return (Image*)i->intValue();
    }
    const Object* v = getImageObject(id, suffix);
    if (v == null) {
        traceln(L"WARNING: missing reqired image \"%s\"\n", id);
        return null;
    }
    Image* c = null;
    if (instanceof(v, const Rect*)) {
        const Rect& rect = *dynamic_cast<const Rect*>(v);
        c = ui->clone(rect);
    }
    else if (instanceof(v, const File*)) {
        c = new Image(*dynamic_cast<const File*>(v));
    }
    else if (instanceof(v, const LinkedList*)) {
        c = constructLayered(*this, *dynamic_cast<const LinkedList*>(v), suffix);
    }
    else {
        trace(L"unxepected node type: "); traceln(v);
        assert(false);
        return null;
    }
    cache.put(*full_name, (int)(void*)c);
    delete full_name;
    return c;
}

HICON Skin::getIcon(const uchar* id) {
    const Int* i = dynamic_cast<const Int*>(icons.get(id));
    if (i != null) return (HICON)i->intValue();
    const Image* image = getImage(id);
    if (image == null) return null;
    ICONINFO ii = {0};
    ii.fIcon = TRUE;
    ii.hbmMask = image->getBitmap();
    ii.hbmColor = image->getBitmap();
    HICON icon = CreateIconIndirect(&ii);
    assert(icon != 0);
    icons.put(id, (int)icon);
    return icon;
}

Color Skin::getColor(const uchar* id) const {
    const HashMap& colors = *dynamic_cast<const HashMap*>(skin->get(L"Colors"));
    return *colors.getColor(id);
}

HBRUSH Skin::getBrush(const uchar* id) {
    const Int* brush = brushes.getInt(id);
    if (brush != null) return (HBRUSH)brush->intValue();
    HBRUSH hb = CreateSolidBrush(getColor(id));
    brushes.put(id, (int)hb);
    return hb;
}

bool Skin::canPack() {
    return File(*skin->getString(L"name")).isDirectory();
}

static void saveSkin(const HashMap* skin) {
    const uchar* skin_xml = L"skin.xml";
    String* encoded =  XMLEncoder::encode(skin);
    ByteArray* utf8 = String::toUTF8(encoded, false);
    delete encoded;
    FileOutputStream fos(skin_xml);
    check(fos.write(*utf8, utf8->size()) == utf8->size());
    check(fos.close());
    delete utf8;
}

void Skin::pack() {
    assert(canPack());
    HourglassCursor wait;
    *skin->put(L"version", version);
    const String& skinName = *skin->getString(L"name");
    HashMap files;
    ArrayList fileNames;
    StringBuffer sb;
    sb.printf(L"%s\\*.png", (const uchar*)skinName);
    WIN32_FIND_DATA fd = {0};
    HANDLE find = FindFirstFile(sb, &fd);
    while (find != INVALID_HANDLE_VALUE) {
        String name(fd.cFileName, ustrlen(fd.cFileName) - 4);
        sb.clear();
        sb.printf(L"%s\\%s.png", (const uchar*)skinName, (const uchar*)name);
        Image i;
        int w = 0, h = 0;
        check(i.load(sb, w, h, true));
        Rect rc(0, 0, w, h);
        files.put(&name, &rc);
        fileNames.add(&name);
        if (!FindNextFile(find, &fd)) {
            FindClose(find);
            find = INVALID_HANDLE_VALUE;
        }
    }
    // sort by square or image rectangle
    int n = fileNames.size();
    int* ix = new int[n];
    for (int j = 0; j < n; j++) ix[j] = j;
    bool swap = true;
    while (swap) {
        swap = false;
        for (int i = 0; i < n - 1; i++) {
            const Rect& rc0 = *dynamic_cast<const Rect*>(files.get(fileNames.get(ix[i])));
            const Rect& rc1 = *dynamic_cast<const Rect*>(files.get(fileNames.get(ix[i + 1])));
            longlong s0 = rc0.right * rc0.bottom;
            longlong s1 = rc1.right * rc1.bottom;
            if (s0 > s1) {
                swap = true;
                int t = ix[i]; ix[i] = ix[i + 1]; ix[i + 1] = t;
            }
        }
    }
    // pack them in width <= MAX_WIDTH rectangle:
    const int MAX_WIDTH = 1280;
    int maxW = 0, maxH = 0, x = 1, y = 1;
    for (int i = 0; i < n; i++) {
        Rect& rc = *(Rect*)dynamic_cast<const Rect*>(files.get(fileNames.get(ix[i])));
        if (x > 0 && x + rc.getWidth() > MAX_WIDTH) {
            if (maxW < x + 1) maxW = x + 1;
            x = 0;
            y += maxH;
        }
        OffsetRect(rc, x, y);
        x += rc.getWidth() + 1;
        if (maxW < x + 1) maxW = x + 1;
        if (maxH < rc.getHeight() + 1) maxH = rc.getHeight() + 1;
    }
    Image packed(maxW, x > 0 ? y + maxH : y, 32);
    HashMap& images = (HashMap&)*dynamic_cast<const HashMap*>(skin->get(L"Images"));
    HashMap* rect_images = dynamic_cast<HashMap*>(images.clone());
    for (i = 0; i < n; i++) {
        const String& name = *dynamic_cast<const String*>(fileNames.get(ix[i]));
        const Rect& rc = *dynamic_cast<const Rect*>(files.get(&name));
        int w = 0, h = 0;
        sb.clear();
        sb.printf(L"%s\\%s.png", (const uchar*)skinName, (const uchar*)name);
        Image e;
        check(e.load(sb, w, h));
        rect_images->put(name, rc);
        assert(w == rc.getWidth() && h == rc.getHeight());
        e.alphaBlend(packed, rc.left, rc.top, w, h, 0, 0, w, h);
        images.put(&name, &rc);
        check(DeleteFile(sb));
    }
    delete ix;
    check(RemoveDirectory(skinName));
    const uchar* skin_png = L"skin.png";
    check(packed.save(skin_png));
    clearCache();
    delete ui;
    ui = new Image(skin_png);
    skin->put(L"Images", rect_images);
    delete rect_images;
    saveSkin(skin);
    load();
}

void Skin::unpack() {
    assert(!canPack());
    HourglassCursor wait;
    const String& skinName = *skin->getString(L"name");
    check(CreateDirectory(skinName, null));
    const HashMap& images = *dynamic_cast<const HashMap*>(skin->get(L"Images"));
    HashMap* file_images = dynamic_cast<HashMap*>(images.clone());
    for (HashMap::EntryIterator i(images); i.hasNext(); ) {
        const HashMap::Entry& e = *i.nextEntry();
        const String& name = *dynamic_cast<const String*>(e.getKey());
        if (instanceof(e.getVal(), const Rect*)) {
            const Rect& rc = *dynamic_cast<const Rect*>(e.getVal());
            Image* i = ui->clone(rc);
            if (i != NULL) {
                StringBuffer sb;
                sb.printf(L"%s\\%s.png", (const uchar*)skinName, (const uchar*)name);
                check(i->save(sb));
                file_images->put(name, File(sb));
                delete i;
            }
        }
        else if (instanceof(e.getVal(), const LinkedList*)) {
            // multilayered image
//          traceln(e.getVal());
        }
        else {
            traceln(e.getVal());
            assert(false); // unknown image type
        }
    }
    skin->put(L"Images", file_images);
    delete file_images;
    saveSkin(skin);
    DeleteFile(L"skin.png"); // ignore result
    load();
}
