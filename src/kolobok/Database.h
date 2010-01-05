#pragma once
#include "tiny.h"

struct Database;
struct DatabaseJob;
struct BlockingDatabaseJob;
struct IDataProvider;

// TODO: one callback with void* p, int error, HashMap results
//       a little problem is ownership of the map and cloning of values
//       e.g. 'info'
typedef void (*DBDone)(void* p, int error);
typedef void (*DBDoneOpen)(void* p, int error, Database *db);
typedef void (*DBDoneCount)(void* p, int error, int count);
typedef void (*DBDoneGetIds)(void* p, int error, ArrayList *ids);
typedef void (*DBDoneGet)(void* p, int error, String &guid, String *file, HashMap *info, ByteArray *blob);

struct Database implements BroadcastListener
{
    static void open(const uchar *name, bool create, DBDoneOpen done, void *p);
    void close(DBDone done, void *p);
    const uchar *getName() { return name; }

    // Media and Metadata
    void add(const uchar *guid, const String &file, DBDone done, void *p);
    void updateMetadata(const uchar *guid, const HashMap& info, DBDone done, void *p);
    void updateThumbnail(const uchar *guid, const ByteArray& blob, DBDone done, void *p);
    void count(DBDoneCount done, void *p);
    void getIds(int first, int count, DBDoneGetIds done, void *p);
    void get(const uchar *guid, DBDoneGet done, void *p);
    void remove(const uchar *guid, DBDone done, void *p);

    // Background processing Stacks
    void pushStackItem(int stackId, const uchar *guid, DBDone done, void *p);
    void countStackItems(int stackId, DBDoneCount done, void *p);
    void removeStackItems(int stackId, const ArrayList& list, DBDone done, void *p);
    // move items to the queue front
    void bumpStackItems(int stackId, const ArrayList& list, DBDone done, void *p);

    // Blocking operations, must only be called by worker threads
    int getFilename(const uchar *guid, String **file);
    int popStackItem(int stackId, String **guid);

    ~Database();
    void eventReceived(const Message& message);

    // Stack numbers
    enum {
        METADATA = 1,
        THUMBNAIL
    };

private:
    Database(const uchar *name);

    int init_peer();
    void finalize();

    static dword WINAPI worker(void *pv);

    void addJob(DatabaseJob *job);
    int  addJobAndWait(BlockingDatabaseJob *e);
    DatabaseJob *removeJob();

    String name;
    IDataProvider *provider;
    HANDLE peer, job;
    CRITICAL_SECTION cs;

    DatabaseJob *head;
};

extern void initSampleDatabase();
extern void finiSampleDatabase();
