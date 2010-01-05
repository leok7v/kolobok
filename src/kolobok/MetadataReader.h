#include "tiny.h"
#include "Database.h"

struct MetadataReader implements BroadcastListener {

    MetadataReader(Database *d, int stackId = Database::METADATA);

    void ping() { SetEvent(job); }

    ~MetadataReader();
    void eventReceived(const Message& message);

private:
    Database *db;
    int stack;
    bool shutdown;
    HANDLE peer, job;

    static dword WINAPI worker(void *pv);
    void processMedia(String &guid);
};
