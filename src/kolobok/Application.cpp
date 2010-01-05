#include "Application.h"
#include "Skin.h"
#include "Help.h"
#include "Frame.h"
#include "Database.h"
#include "MetadataReader.h"

Application main;

Application::Application() : version(null), skin(null), db(null), mr(null) {
}

Application::~Application() {
    finiSampleDatabase();
    delete version; version = null;
    delete skin;    skin = null;
    delete mr;      mr = null;
    delete db;      db = null;
    delete frame;   frame = null;
}

static void setWorkingDirectory() {
    uchar fnm[1024];
    check(GetModuleFileName(null, fnm, countof(fnm)));
    int i = ustrlen(fnm);
    while (i > 0 && fnm[i - 1] != L'/' && fnm[i - 1] != L'\\') i--;
    fnm[i] = 0;
    check(SetCurrentDirectory(fnm));
}

void Application::getFileVersionInfo() {
    uchar fname[1024];
    check(GetModuleFileName(null, fname, countof(fname)));
    DWORD handle = 0;
    DWORD size = GetFileVersionInfoSize(fname, &handle);
    assert(size > 0);
    byte* buf = new byte[size];
    check(GetFileVersionInfo(fname, handle, size, buf));
    VS_FIXEDFILEINFO* v = null;
    uint len = 0;
    check(VerQueryValue(buf, L"\\", (void**)&v, &len));
    assert(len == sizeof(*v));
    StringBuffer sb;
    sb.printf(L"%d.%d.%d.%d", v->dwFileVersionMS >> 16, v->dwFileVersionMS & 0xFFFF,
                              v->dwFileVersionLS >> 16, v->dwFileVersionLS & 0xFFFF);
    version = sb.toString();
    trace("version "); traceln(version);
    delete buf;
}

void Application::setDatabase(Database *d) {
    // TODO: for now we just close old DB.
    //       In the future we should open new top-level frame for new DB.
    if (db != null) {
        delete mr; mr = null;
        delete db; db = null;
    }
    db = d;
    mr = new MetadataReader(db);
    MessageQueue::post(L"databaseChanged", null);
}

void Application::createFrame(void* vpshow) {
    int show = (int)vpshow;
    MessageQueue::addBroadcastListener(&main.getSkin());
    MessageQueue::addBroadcastListener(&Help::help);
    main.frame = new Frame(GetDesktopWindow());
    initSampleDatabase();
    ShowWindow(main.frame->getWnd(), show);
}

static void option(uchar* o) {
/*
    if (ustrcmp(o, L"-g") == 0) main.debug = 1;
    if (ustrcmp(o, L"-t") == 0) main.test = true;
*/
}

int Application::run(HINSTANCE instance, int argc, uchar* argv[], uchar* env, int show) {
    main.instance = instance;
    setWorkingDirectory();
    main.getFileVersionInfo();
    int i = 1;
    while (i < argc && argv[i][0] == L'-') {
        option(argv[i]);
        i++;
    }
    main.skin = new Skin(main.getVersion());
    MSG msg;
    PeekMessage(&msg, null, 0, 0, PM_NOREMOVE);
    MessageQueue::invokeLater(Application::createFrame, (void*)show);
    return MessageQueue::messageLoop();
}

HWND getApplicationMainWindow() {
    return main.frame->getWnd();
}

int winMain(HINSTANCE instance, int argc, uchar* argv[], uchar* env, int show) {
    return Application::run(instance, argc, argv, env, show);
}
