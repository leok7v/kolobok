#pragma once
#include "svnrev.h"
#include "tiny.h"

struct Database;
struct Frame;
struct Skin;
struct MetadataReader;

struct Application {

    Application();
    ~Application();

    inline Skin& getSkin() const { return *skin; }
    inline String& getVersion() const { return *version; }
    inline HINSTANCE getInstance() const { return instance; }
    inline Database& getDatabase() const { return *db; }

    void setDatabase(Database *d);

private:
    HINSTANCE instance;
    String* version;
    Skin* skin;
    Frame* frame;
    Database *db;
    MetadataReader *mr;
    
    void getFileVersionInfo();
    static void createFrame(void* vpshow);
    static int  run(HINSTANCE instance, int argc, uchar* argv[], uchar* env, int show);
    friend static LRESULT CALLBACK wndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);
    friend extern HWND getApplicationMainWindow();
    friend extern int winMain(HINSTANCE instance, int argc, uchar* argv[], uchar* env, int show);
};

extern Application main;
