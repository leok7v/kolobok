#pragma once
#include "tiny.h"

struct Skin implements BroadcastListener {

    Skin(String& version);
    virtual ~Skin();

    const String* getText(const uchar* textid);
    const String* getToolTipText(const uchar* textid);
    const HashMap* getTextMap(const uchar* textid);

    bool containsImage(const uchar* id, const uchar* suffix = null) const { 
        return getImageObject(id, suffix) != null; 
    }
    const Image* getImage(const String* id, const uchar* suffix = null) { return getImage(*id, suffix); }
    const Image* getImage(const uchar* imageid, const uchar* suffix = null);

    HICON getIcon(const uchar* imageid);
    HICON getIcon(const String* id) { return getIcon(*id); }

    Color getColor(const uchar* id) const;
    Color getColor(const String* id) const { return getColor(*id); }
    HBRUSH getBrush(const uchar* id);
    HBRUSH getBrush(const String* id) { return getBrush(*id); }
    
    void getTopLevelMenu(HMENU &menu, HACCEL &accel);
    bool hasPopupMenu(const uchar* menuid);
    int trackPopupMenu(HWND hwnd, const uchar* menuid, const Point& pt);
    int dispatchMenu(HMENU menu, int cmdid);
    int updateCommandsState(HMENU menu);

private:
    void eventReceived(const Message& msg);
    void load();
    void pack();
    void unpack();
    bool canPack();
    const Object* getImageObject(const uchar* id, const uchar* suffix) const;

    HashMap* skin;
    String version;
    Image* ui;
    HashMap menus;
    HashMap cache;
    HashMap icons;
    HashMap brushes;
    HMENU getMenu(const uchar* menuid, bool toplevel, HACCEL &accel);
    void destroyMenu(HMENU menu);
    void clearCache();
};
