#include "resource.h"
#include "Database.h"
#include "sqlite3.h"


struct IDataProvider {
    virtual int close() = 0;

    virtual int open(const uchar *filename, bool create) = 0;

    // Media and Metadata
    virtual int add(const uchar *guid, const uchar *file) = 0;
    virtual int updateMetadata(const uchar *guid, const HashMap& info) = 0;
    virtual int updateThumbnail(const uchar *guid, const ByteArray& blob) = 0;
    virtual int count(int& count) = 0;
    virtual int getIds(int first, int count, ArrayList** ids) = 0;
    virtual int get(const uchar *guid, String** file, HashMap** info, ByteArray** blob) = 0;
    virtual int remove(const uchar *guid) = 0;

    // Background processing Stacks
    virtual int pushStackItem(int stackId, const uchar *guid) = 0;
    virtual int countStackItems(int stackId, int& count) = 0;
    virtual int removeStackItems(int stackId, const ArrayList& list) = 0;
    // move items to the queue front
    virtual int bumpStackItems(int stackId, const ArrayList& list) = 0;

    // this is blocking, must only be called by worker thread
    virtual int popStackItem(int stackId, String **guid) = 0;
    virtual int getFilename(const uchar *g, String **f) = 0;

    virtual ~IDataProvider() {}
};


// TODO: sqlite IO: system file cache vs. sqlite data cache vs. write-through
struct SQLiteProvider implements IDataProvider {

    SQLiteProvider() : sqlite(null) {
        memset(sql, 0, sizeof(sql));
        memset(sql_stack, 0, sizeof(sql_stack));
    }

    enum {
        SQL_ADD = 0,
        SQL_GET,
        SQL_MEDIA_ID,
        SQL_ADD_META,
        SQL_DEL,
        SQL_DEL_META,
        SQL_GET_META,
        SQL_UPDATE_BLOB,
        SQL_MEDIA_GET_FILENAME,
        SQL_MAX
    };

    enum {
        SQL_STACK_ADD = 0,
        SQL_STACK_COUNT,
        SQL_STACK_REMOVE,
        SQL_STACK_GET,
        SQL_STACK_DELETE,
        SQL_STACK_MAX
    };

// TODO: Handle SQLITE_BUSY in a reasonable way.
    enum { TIMEOUT = 100 };

    enum {
        typeString = 1,
        typeInt = 2,
        typeLong = 3,
        typeRational = 4,
        typeByteArray = 5,
        typeRationalArray = 6,
        typeIntArray = 7,
        typeLongArray = 8
    };

    sqlite3* sqlite;
    sqlite3_stmt* sql[SQL_MAX];
    sqlite3_stmt* sql_stack[SQL_STACK_MAX];

    static const char* SQL[SQL_MAX];
    static const char* SQL_STACK[SQL_STACK_MAX];

    static void byte_dtor(void *pv) {
        byte *pb = (byte *)pv;
        delete[] pb;
    }

    int bind_string(sqlite3_stmt *st, int index, const uchar *s) {
        int cb = 0;
        byte* utf = String::toUTF8(s, cb, false);
        return sqlite3_bind_text(st, index, (char *)utf, cb, byte_dtor);
    }

    int bind_object(sqlite3_stmt *st, int index, Object &o) {
        String &s = *(XMLEncoder::encode(&o));
        int cb = 0;
        byte *utf = String::toUTF8((const uchar *)s, cb, false);
        delete &s;
        return sqlite3_bind_text(st, index, (char *)utf, cb, byte_dtor);
    }

    int bind_blob(sqlite3_stmt *st, int index, const ByteArray& ba) {
        byte *pb = new byte[ba.size()];
        memcpy(pb, (byte *)ba, ba.size());
        return sqlite3_bind_blob(st, index, pb, ba.size(), byte_dtor);
    }

    int report(int e) {
        if (e != SQLITE_OK)
            traceln("SQLite error: %d - %s", e, sqlite3_errmsg16(sqlite));
        return e;
    }

    int open(const uchar *filename, bool create) {
        static const char *code[] = {
            "CREATE TABLE media (id INTEGER PRIMARY KEY, guid TEXT UNIQUE NOT NULL, file TEXT, thumbnail BLOB);",
            "CREATE TABLE metadata (id INTEGER PRIMARY KEY, media_id INTEGER NOT NULL, key TEXT NOT NULL, type INTEGER NOT NULL, value TEXT);",
            "CREATE INDEX meta_media_key ON metadata (media_id, key);",
            "CREATE TABLE stacks (id INTEGER PRIMARY KEY,stack INTEGER NOT NULL,media TEXT NOT NULL);"
        };

        if (create && GetFileAttributes(filename) != 0xFFFFFFFF)
            DeleteFile(filename);

        int cb = 0;
        byte *utf = String::toUTF8(filename, cb, false);
        int e = sqlite3_open((char *)utf, &sqlite);
        delete[] utf;
        if (!e) e = sqlite3_busy_timeout(sqlite, TIMEOUT);

        if (create) {
            for (int i = 0; !e && i < countof(code); ++i)
                e = sqlite3_exec(sqlite, code[i], null, 0, null);
        }
        for (int i = 0; i < SQL_MAX && !e; ++i)
            e = sqlite3_prepare(sqlite, SQL[i], -1, &(sql[i]), null);
        for (int j = 0; j < SQL_STACK_MAX && !e; ++j)
            e = sqlite3_prepare(sqlite, SQL_STACK[j], -1, &(sql_stack[j]), null);
        return e;
    }

    int close() {
        for (int i = 0; i < SQL_MAX; ++i) {
            sqlite3_finalize(sql[i]);
            sql[i] = null;
        }
        for (int j = 0; j < SQL_STACK_MAX; ++j) {
            sqlite3_finalize(sql_stack[j]);
            sql_stack[j] = null;
        }
        if (sqlite != null) {
            sqlite3_close(sqlite);
            sqlite = null;
        }
        return SQLITE_OK;
    }


    int getValueType(const Object *o) {
        if (instanceof(o, const String*)) return typeString;
        if (instanceof(o, const Int*)) return typeInt;
        if (instanceof(o, const Long*)) return typeLong;
        if (instanceof(o, const Rational*)) return typeRational;
        if (instanceof(o, const ByteArray*)) return typeByteArray;
        if (instanceof(o, const ArrayList*)) {
            const ArrayList* al = dynamic_cast<const ArrayList *> (o);
            if (al->size() < 1)
                return 0;
            const Object *v = al->get(0);
            if (instanceof(v, const Int*)) return typeIntArray;
            if (instanceof(v, const Long*)) return typeLongArray;
            if (instanceof(v, const Rational*)) return typeRationalArray;
        }
        return 0;
    }

    bool isValidType(const Object *o) {
        return getValueType(o) != 0;
    }

    // TODO: Currently all array values are stored as XML.
    //       We might want to have some other formatting for them...
    String* formatValue(const Object *o, int type) {
        switch(type) {
        case typeString:
        case typeInt:
        case typeLong:
        case typeRational:
            return o->toString();
        case typeByteArray:
        case typeIntArray:
        case typeLongArray:
        case typeRationalArray:
            return XMLEncoder::encode(o);
        }
        return null;
    }

    Object* valueFromString(String* s, int type) {
        switch(type) {
        case typeString: return s->clone();
        case typeInt: return new Int(Int::decode(*s));
        case typeLong: return new Long(Long::decode(*s));
        case typeRational: return Rational::decode(*s);
        case typeByteArray:
        case typeIntArray:
        case typeLongArray:
        case typeRationalArray:
            return XMLDecoder::decode(s);
        }
        return null;
    }

    int getRowid(sqlite3_stmt *st, const uchar *guid, longlong &id) {
        int e = sqlite3_reset(st);
        if (!e) e = bind_string(st, 1, guid);
        if (!e) {
            e = sqlite3_step(st);
            if (e == SQLITE_ROW) {
                id = sqlite3_column_int64(st, 0);
                e = sqlite3_step(st);
                if (e == SQLITE_DONE) e = SQLITE_OK;
            }
        }
        return e;
    }

    int add(const uchar *guid, const uchar *file) {
        int e = sqlite3_exec(sqlite, "BEGIN;", null, 0, null);
        if (!e) e = sqlite3_reset(sql[SQL_ADD]);
        if (!e) e = bind_string(sql[SQL_ADD], 1, guid);
        if (!e) e = bind_string(sql[SQL_ADD], 2, file);
        if (!e) {
            e = sqlite3_step(sql[SQL_ADD]);
            if (e == SQLITE_DONE) e = SQLITE_OK;
        }
        if (!e) e = sqlite3_exec(sqlite, "COMMIT;", null, 0, null);
        else        sqlite3_exec(sqlite, "ROLLBACK;", null, 0, null);
        return e;
    }

    int count(int &count) {
        char **result = null;
        int rows=0, cols=0;
        int e = sqlite3_get_table(sqlite, "SELECT COUNT(*) FROM media;", &result, &rows, &cols, null);
        if (!e) check(sscanf(result[1], "%d", &count) == 1);
        sqlite3_free_table(result);
        return e;
    }

    int getIds(int first, int count, ArrayList** ids) {
        char **result = null;
        int rows = 0/*, cols=0*/;

        StringBuffer cmd;
        cmd.printf(L"SELECT guid FROM media LIMIT %d OFFSET %d;", count, first);
        int cb = 0;
        byte *utf = String::toUTF8((const uchar *)cmd, cb, false);
        int e = sqlite3_get_table(sqlite, (char*)utf, &result, &rows, null/*&cols*/, null);
        delete utf;
        if (!e) {
            *ids = new ArrayList();
            for (int i = 0; i < rows; ++i) {
                String *s = String::fromUTF8(result[i+1]);
                (*ids)->add(s);
                delete s;
            }
        }
        sqlite3_free_table(result);
        return e;
    }

    int get(const uchar *guid, String **file, HashMap **info, ByteArray **blob) {
        int e = sqlite3_exec(sqlite, "BEGIN;", null, 0, null);
        if (!e) e = sqlite3_reset(sql[SQL_GET]);
        if (!e) e = bind_string(sql[SQL_GET], 1, guid);
        if (!e) e = sqlite3_step(sql[SQL_GET]);
        longlong rowid = 0;
        if (e == SQLITE_ROW) {
            rowid = sqlite3_column_int64(sql[SQL_GET], 0);
            *file = String::fromUTF8(sqlite3_column_text(sql[SQL_GET], 1));
            *blob = new ByteArray((byte *)sqlite3_column_blob(sql[SQL_GET], 2), sqlite3_column_bytes(sql[SQL_GET], 2));
            check(sqlite3_step(sql[SQL_GET]) == SQLITE_DONE);
            e = SQLITE_OK;
        }
        if (!e) e = sqlite3_reset(sql[SQL_GET_META]);
        if (!e) e = sqlite3_bind_int64(sql[SQL_GET_META], 1, rowid);
        if (!e) e = sqlite3_step(sql[SQL_GET_META]);
        while (e == SQLITE_ROW) {
            if (*info == null)
                *info = new HashMap();
            String *key = String::fromUTF8(sqlite3_column_text(sql[SQL_GET_META], 0));
            int type = sqlite3_column_int(sql[SQL_GET_META], 1);
            String *value = String::fromUTF8(sqlite3_column_text(sql[SQL_GET_META], 2));
            Object *data = valueFromString(value, type);
            (*info)->put(key, data);
            delete key;
            delete value;
            delete data;
            e = sqlite3_step(sql[SQL_GET_META]);
        }
        if (e == SQLITE_DONE)
            e = SQLITE_OK;
        sqlite3_exec(sqlite, "ROLLBACK;", null, 0, null);
        return e;
    }

    int getFilename(const uchar *guid, String **file) {
        int e = sqlite3_reset(sql[SQL_MEDIA_GET_FILENAME]);
        if (!e) e = bind_string(sql[SQL_MEDIA_GET_FILENAME], 1, guid);
        if (!e) e = sqlite3_step(sql[SQL_MEDIA_GET_FILENAME]);
        if (e == SQLITE_ROW) {
            *file = String::fromUTF8(sqlite3_column_text(sql[SQL_MEDIA_GET_FILENAME], 0));
            check(sqlite3_step(sql[SQL_MEDIA_GET_FILENAME]) == SQLITE_DONE);
            e = SQLITE_OK;
        }
        return report(e);
    }
    
    int remove(const uchar *guid) {
        longlong id = 0;
        int e = getRowid(sql[SQL_MEDIA_ID], guid, id);
        if (!e) e = sqlite3_reset(sql[SQL_DEL]);
        if (!e) e = bind_string(sql[SQL_DEL], 1, guid);
        if (!e) e = sqlite3_step(sql[SQL_DEL]);
        if (e == SQLITE_DONE) e = SQLITE_OK;
        if (!e) e = sqlite3_reset(sql[SQL_DEL_META]);
        if (!e) e = sqlite3_bind_int64(sql[SQL_DEL_META], 1, id);
        if (!e) e = sqlite3_step(sql[SQL_DEL_META]);
        if (e == SQLITE_DONE) e = SQLITE_OK;
        return e;
    }

    int updateThumbnail(const uchar *guid, const ByteArray& blob) {
        int e = sqlite3_reset(sql[SQL_UPDATE_BLOB]);
        if (!e) e = bind_blob  (sql[SQL_UPDATE_BLOB], 1, blob);
        if (!e) e = bind_string(sql[SQL_UPDATE_BLOB], 2, guid);
        if (!e) {
            e = sqlite3_step(sql[SQL_UPDATE_BLOB]);
            if (e == SQLITE_DONE) e = SQLITE_OK;
        }
        return e;
    }

    int add2metadata(longlong id, const Object *key, const Object *value) {
        String *k = key->toString();
        int type = getValueType(value);
        String *v = formatValue(value, type);;
        int e = sqlite3_reset(sql[SQL_ADD_META]);
        if (e == SQLITE_OK) e = sqlite3_bind_int64(sql[SQL_ADD_META], 1, id);
        if (e == SQLITE_OK) e = bind_string(sql[SQL_ADD_META], 2, *k);
        if (e == SQLITE_OK) e = sqlite3_bind_int(sql[SQL_ADD_META], 3, type);
        if (e == SQLITE_OK) e = bind_string(sql[SQL_ADD_META], 4, *v);
        if (e == SQLITE_OK) {
            e = sqlite3_step(sql[SQL_ADD_META]);
            if (e == SQLITE_DONE) e = SQLITE_OK;
        }
        delete k;
        delete v;
        return e;
    }

    int updateMetadata(const uchar *guid, const HashMap& info) {
        int e = sqlite3_exec(sqlite, "BEGIN;", null, 0, null);
        longlong id = 0;
        if (!e) e = getRowid(sql[SQL_MEDIA_ID], guid, id);
        if (!e) e = sqlite3_reset(sql[SQL_DEL_META]);
        if (!e) e = sqlite3_bind_int64(sql[SQL_DEL_META], 1, id);
        if (!e) e = sqlite3_step(sql[SQL_DEL_META]);
        if (e == SQLITE_DONE) e = SQLITE_OK;
        if (!e) {
            HashMap::EntryIterator i(info);
            while (i.hasNext() && !e) {
                const HashMap::Entry *entry = i.nextEntry();
                if (!isValidType(entry->getKey()) || !isValidType(entry->getVal())) {
                    e = SQLITE_MISUSE;
                    sqlite3_exec(sqlite, "ROLLBACK;", null, 0, null);
                    break;
                }
                e = add2metadata(id, entry->getKey(), entry->getVal());
            }
        }
        return sqlite3_exec(sqlite, e == SQLITE_OK ? "COMMIT;" : "ROLLBACK;", null, 0, null);
    }

    bool hasStacks() {
        return true;
        /*
        char **result = null;
        int rows = 0;
        int e = sqlite3_get_table(sqlite,
                    "SELECT name FROM sqlite_master where type='table' and name='stacks';",
                    &result, &rows, null, null);
        if (!e) sqlite3_free_table(result);
        return (!e && rows == 1);
        */
    }
    
    int ensureStacks() {
        return SQLITE_OK;
        /*
        char **result = null;
        int rows = 0;
        int e = sqlite3_get_table(sqlite, "SELECT name FROM sqlite_master where type='table' and name='stacks';",
            &result, &rows, null, null);
        if (!e) sqlite3_free_table(result);
        if (!e && rows == 0) {
            e = sqlite3_exec(sqlite,
                    "CREATE TABLE stacks ("
                    "id INTEGER PRIMARY KEY, "
                    "stack INTEGER NOT NULL, "
                    "media TEXT NOT NULL);",
                    null, 0, null);
        }
        if (!e) {
            for (int i = 0; i < SQL_STACK_MAX && !e; ++i) {
                e = sqlite3_prepare(sqlite, SQL_STACK[i], -1, &(sql_stack[i]), null);
                if (e) {
                    traceln("SQLite error: %s", sqlite3_errmsg16(sqlite));
                }
            }
        }
        return e;
        */
    }

    int checkDeleteStacks() {
        return SQLITE_OK;
        /*
        char **result = null;
        int rows=0, count=0;
        int e = sqlite3_get_table(sqlite, "SELECT COUNT(*) FROM stacks;", &result, &rows, null, null);
        if (!e) check(sscanf(result[1], "%d", &count) == 1);
        sqlite3_free_table(result);

        if (!e && count == 0) {
            e = sqlite3_exec(sqlite, "DROP TABLE stacks;", null, 0, null);
            if (e == SQLITE_DONE) e = SQLITE_OK;
            for (int i = 0; i < SQL_STACK_MAX; ++i) {
                sqlite3_finalize(sql_stack[i]);
                sql_stack[i] = null;
            }
        }
        return e;
        */
    }

    int pushStackItem(int stack, const uchar *guid) {
        int e = sqlite3_exec(sqlite, "BEGIN;", null, 0, null);
        if (!e) e = ensureStacks();
        if (!e) e = sqlite3_reset(sql_stack[SQL_STACK_ADD]);
        if (!e) e = sqlite3_bind_int(sql_stack[SQL_STACK_ADD], 1,stack);
        if (!e) e = bind_string(sql_stack[SQL_STACK_ADD], 2, guid);
        if (!e) {
            e = sqlite3_step(sql_stack[SQL_STACK_ADD]);
            if (e == SQLITE_DONE) e = SQLITE_OK;
        }
        return sqlite3_exec(sqlite, e == SQLITE_OK ? "COMMIT;" : "ROLLBACK;", null, 0, null);
    }

    int countStackItems(int stack, int &count) {
        if (!hasStacks()) {
            count = 0;
            return SQLITE_OK;
        }
        int e = sqlite3_reset(sql_stack[SQL_STACK_COUNT]);
        if (!e) e = sqlite3_bind_int(sql_stack[SQL_STACK_COUNT], 1,stack);
        if (!e) {
            e = sqlite3_step(sql_stack[SQL_STACK_COUNT]);
            if (e == SQLITE_ROW) {
                count = sqlite3_column_int(sql_stack[SQL_STACK_COUNT], 0);
                e = sqlite3_step(sql_stack[SQL_STACK_COUNT]);
            }
            if (e == SQLITE_DONE) e = SQLITE_OK;
        }
        return e;
    }

    int removeStackItems(int stack, const ArrayList& list) {
        if (!hasStacks()) {
            return SQLITE_OK;
        }
        int e = sqlite3_exec(sqlite, "BEGIN;", null, 0, null);
        for(int i = 0; i < list.size() && !e; ++i) {
            e = sqlite3_reset(sql_stack[SQL_STACK_REMOVE]);
            if (!e) e = sqlite3_bind_int(sql_stack[SQL_STACK_REMOVE], 1,stack);
            const String* val = dynamic_cast<const String*> (list.get(i));
            if (!e) e = bind_string(sql_stack[SQL_STACK_REMOVE], 2, *val);
            if (!e) {
                e = sqlite3_step(sql_stack[SQL_STACK_REMOVE]);
                if (e == SQLITE_DONE) e = SQLITE_OK;
            }
        }
        if (!e) checkDeleteStacks();
        return sqlite3_exec(sqlite, e == SQLITE_OK ? "COMMIT;" : "ROLLBACK;", null, 0, null);
    }

    int bumpStackItems(int stack, const ArrayList& list) {
        int e = sqlite3_exec(sqlite, "BEGIN;", null, 0, null);
        if (!e && hasStacks()) {
            for(int i = 0; i < list.size() && !e; ++i) {
                const String* val = dynamic_cast<const String*> (list.get(i));
                e = sqlite3_reset(sql_stack[SQL_STACK_REMOVE]);
                if (!e) e = sqlite3_bind_int(sql_stack[SQL_STACK_REMOVE], 1, stack);
                if (!e) e = bind_string(sql_stack[SQL_STACK_REMOVE], 2, *val);
                if (!e) {
                    sqlite3_step(sql_stack[SQL_STACK_REMOVE]);
                    e = SQLITE_OK;
                }
            }
        }
        if (!e) e = ensureStacks();
        for (int i = 0; i < list.size() && !e; ++i) {
            const String* val = dynamic_cast<const String*> (list.get(i));
            if (!e) e = sqlite3_reset(sql_stack[SQL_STACK_ADD]);
            if (!e) e = sqlite3_bind_int(sql_stack[SQL_STACK_ADD], 1, stack);
            if (!e) e = bind_string(sql_stack[SQL_STACK_ADD], 2, *val);
            if (!e) {
                e = sqlite3_step(sql[SQL_STACK_ADD]);
                if (e == SQLITE_DONE) e = SQLITE_OK;
            }
        }
        return sqlite3_exec(sqlite, e == SQLITE_OK ? "COMMIT;" : "ROLLBACK;", null, 0, null);
    }

    int popStackItem(int stack, String** guid) {
        if (!hasStacks())
            return SQLITE_ERROR;
        int e = sqlite3_exec(sqlite, "BEGIN;", null, 0, null);
        if (!e) e = sqlite3_reset(sql_stack[SQL_STACK_GET]);
        if (!e) e = sqlite3_bind_int(sql_stack[SQL_STACK_GET], 1, stack);
        longlong rowid = 0;
        *guid = null;
        if (!e) {
            e = sqlite3_step(sql_stack[SQL_STACK_GET]);
            if (e == SQLITE_ROW) {
                rowid = sqlite3_column_int64(sql_stack[SQL_STACK_GET], 0);
                *guid = String::fromUTF8(sqlite3_column_text(sql_stack[SQL_STACK_GET], 1));
                e = sqlite3_step(sql_stack[SQL_STACK_GET]);
                if (e == SQLITE_DONE) e = SQLITE_OK;
            }
        }
        if (!e) e = sqlite3_reset(sql_stack[SQL_STACK_DELETE]);
        if (!e) e = sqlite3_bind_int64(sql_stack[SQL_STACK_DELETE], 1, rowid);
        if (!e) {
            e = sqlite3_step(sql_stack[SQL_STACK_DELETE]);
            if (e == SQLITE_DONE) e = SQLITE_OK;
        }
        if (!e) checkDeleteStacks();
        return sqlite3_exec(sqlite, e == SQLITE_OK ? "COMMIT;" : "ROLLBACK;", null, 0, null);
    }

};

const char* SQLiteProvider::SQL[SQLiteProvider::SQL_MAX] = {
    "INSERT INTO media (guid, file) VALUES (?1,?2);",
    "SELECT id,file,thumbnail FROM media WHERE (guid=?1);",
    "SELECT id FROM media WHERE guid=?1;",
    "INSERT INTO metadata (media_id, key, type, value) VALUES (?1,?2,?3,?4);",
    "DELETE FROM media WHERE guid=?1;",
    "DELETE FROM metadata WHERE media_id=?1;",
    "SELECT key,type,value FROM metadata WHERE (media_id=?1);",
    "UPDATE media SET thumbnail=?1 WHERE guid=?2;",
    "SELECT file FROM media WHERE (guid=?1);"
};

const char* SQLiteProvider::SQL_STACK[SQLiteProvider::SQL_STACK_MAX] = {
    "INSERT INTO stacks (stack,media) VALUES (?1,?2);",
    "SELECT COUNT(*) FROM stacks WHERE stack=?1;",
    "DELETE FROM stacks WHERE stack=?1 AND media=?2;",
    "SELECT id,media FROM stacks WHERE stack=?1 ORDER BY id DESC LIMIT 1;",
    "DELETE FROM stacks WHERE id=?1;",
};

///////////////////////////////////////////////////////////////////////////////
// DatabaseJob and Database proper stuff

struct DatabaseJob {
    DatabaseJob *next, *prev;
    DatabaseJob() : next(null), prev(null) {}

    virtual void run() = 0;
    virtual void dispatch() = 0;
    virtual ~DatabaseJob() {}

    static void _dispatch(void *pv) {
        DatabaseJob *dj = (DatabaseJob *)pv;
        dj->dispatch();
    }
};

struct BlockingDatabaseJob extends DatabaseJob {
    BlockingDatabaseJob() : event(0) {
        event = CreateEvent(null, false, false, null);
    }

    virtual void dispatch() {
        SetEvent(event);
    }

    virtual ~BlockingDatabaseJob() {
        CloseHandle(event);
    }

    int error;
    HANDLE event;
};

void Database::addJob(DatabaseJob *e) {
    EnterCriticalSection(&cs);
    if (head == null)
        head = e->next = e->prev = e;
    else {
        e->next = head;
        e->prev = head->prev;
        e->next->prev = e->prev->next = e;
    }
    LeaveCriticalSection(&cs);
    SetEvent(job);
}

int Database::addJobAndWait(BlockingDatabaseJob *e) {
    addJob(e);
    WaitForSingleObject(e->event, INFINITE);
    int error = e->error;
    delete e;
    return error;
}

DatabaseJob *Database::removeJob() {
    DatabaseJob *e = null;
    if (head != null) {
        EnterCriticalSection(&cs);
        e = head;
        head = head->next;
        if (head == e) head = null;
        e->prev->next = e->next;
        e->next->prev = e->prev;
        LeaveCriticalSection(&cs);
    }
    return e;
}

Database::Database(const uchar *n) : provider(null), peer(null), head(null), name(n) {
    InitializeCriticalSection(&cs);
    MessageQueue::addBroadcastListener(this);
}

Database::~Database() {
    if (peer != null)
        finalize();
    assert(peer == null);
    assert(job == null);
    delete provider;
    MessageQueue::removeBroadcastListener(this);
    DeleteCriticalSection(&cs);
}

dword WINAPI Database::worker(void *pv) {
    Database *db = (Database *)pv;
    while (WaitForSingleObject(db->job, INFINITE) == WAIT_OBJECT_0) {
        while (db->head != null) {
            db->removeJob()->run();
        }
    }
    return 0;
}

int Database::init_peer() {
    assert(peer == null);   assert(job == null);
    check((job = CreateEvent(null, false, false, null)) != null);
    check((peer = CreateThread(null, 0, worker, this, 0, null)) != null);
    return 0;
}

void Database::eventReceived(const Message& msg) {
    if (ustrcmp(msg.id, L"shutdown") == 0) {
        if (head != null) {
            MessageQueue::delayShutdown(20);
        }
        else if (peer != null) {
            // This ensures that "done" notification from last job will get posted and dispatched
            finalize();
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
// Jobs

struct JobOpen extends DatabaseJob {

    JobOpen(const uchar *n, bool c, Database *r, DBDoneOpen d, void *p, IDataProvider* _db) :
        error(0), done(d), parm(p), result(r), db(_db), name(n), create(c) {
    }

    ~JobOpen() { }

    void run() {
        error = db->open(name, create);
        MessageQueue::invokeLater(DatabaseJob::_dispatch, (DatabaseJob *)this);
    }

    void dispatch() {
        if (done != null) done(parm, error, result);
        delete this;
    }

    int error;
    DBDoneOpen done;
    void *parm;
    IDataProvider *db;

    Database *result;
    String name;
    bool create;
};

void Database::open(const uchar *name, bool create, DBDoneOpen done, void *pv) {
    Database *db = new Database(name);
    int e = db->init_peer();
    db->provider = new SQLiteProvider();
    if (e != 0 || db->provider == null) { delete db; done(pv, e, null); return; }
    db->addJob(new JobOpen(name, create, db, done, pv, db->provider));
}


struct JobClose extends DatabaseJob {

    JobClose(DBDone d, void *p, IDataProvider *_db) : error(0), done(d), param(p), db(_db) {
    }

    ~JobClose() { }

    void run() {
        error = db->close();
        MessageQueue::invokeLater(DatabaseJob::_dispatch, (DatabaseJob *)this);
    }

    void dispatch() {
        if (done != null) done(param, error);
        done = null;
        delete this;
    }

    int error;
    DBDone done;
    void *param;
    IDataProvider *db;
};

void Database::close(DBDone done, void *pv) {
    if (peer != null)
        addJob(new JobClose(done, pv, provider));
    else
        done(pv, SQLITE_ERROR);
}


struct JobFinalize extends DatabaseJob {
    JobFinalize(IDataProvider *_db, HANDLE e) : db(_db), event(e) { }
    ~JobFinalize() { }

    void run() {
        db->close();
        CloseHandle(event);
        delete this;
    }

    void dispatch() { }

    IDataProvider *db;
    HANDLE event;
};

void Database::finalize() {
    addJob(new JobFinalize(provider, job));
    check(WaitForSingleObject(peer, INFINITE) == WAIT_OBJECT_0);
    peer = null;
    job = null;
}


struct JobAdd extends DatabaseJob {
    JobAdd(const uchar *g, const String &f, DBDone d, void *p, IDataProvider *_db) :
        error(0), done(d), param(p), db(_db), guid(g), file(f) {
    }

    ~JobAdd() { }

    void run() {
        error = db->add(guid, file);
        MessageQueue::invokeLater(DatabaseJob::_dispatch, (DatabaseJob *)this);
    }

    void dispatch() {
        if (done != null) done(param, error);
        done = null;
        delete this;
    }

    int error;
    DBDone done;
    void *param;
    IDataProvider *db;

    String guid;
    String file;
};

void Database::add(const uchar *guid, const String &file, DBDone done, void *p) {
    if (peer != null)
        addJob(new JobAdd(guid, file, done, p, provider));
    else
        done(p, SQLITE_ERROR);
    }


struct JobCount extends DatabaseJob {

    JobCount(DBDoneCount d, void *p, IDataProvider *_db) :
        error(0), done(d), param(p), db(_db), count(0) {
    }

    ~JobCount() { }

    void run() {
        error = db->count(count);
        MessageQueue::invokeLater(DatabaseJob::_dispatch, (DatabaseJob *)this);
    }

    void dispatch() {
        if (done != null) done(param, error, count);
        delete this;
    }

    int error;
    DBDoneCount done;
    void *param;
    IDataProvider* db;

    int count;
};

void Database::count(DBDoneCount done, void *p) {
    if (peer != null)
        addJob(new JobCount(done, p, provider));
    else
        done(p, SQLITE_ERROR, 0);
}


struct JobGetIds extends DatabaseJob {

    JobGetIds(int f, int c, DBDoneGetIds d, void *p, IDataProvider *_db) : 
        error(0), done(d), param(p), db(_db), first(f), count(c), ids(null) {
    }

    ~JobGetIds() { }

    void run() {
        error = db->getIds(first, count, &ids);
        MessageQueue::invokeLater(DatabaseJob::_dispatch, (DatabaseJob *)this);
    }

    void dispatch() {
        if (done != null) done(param, error, ids);
        delete this;
    }

    int error;
    DBDoneGetIds done;
    void *param;
    IDataProvider *db;

    int first;
    int count;
    ArrayList *ids;
};

void Database::getIds(int first, int count, DBDoneGetIds done, void *p) {
    if (peer != null)
        addJob(new JobGetIds(first, count, done, p, provider));
    else
        done(p, SQLITE_ERROR, null);
}


struct JobGet extends DatabaseJob {
    JobGet(const uchar *g, DBDoneGet d, void *p, IDataProvider *_db) :
        error(0), done(d), param(p), db(_db), guid(g), file(null), info(null), blob(null) {
    }

    ~JobGet() { }

    void run() {
        error = db->get(guid, &file, &info, &blob);
        MessageQueue::invokeLater(DatabaseJob::_dispatch, (DatabaseJob *)this);
    }

    void dispatch() {
        if (done != null) done(param, error, guid, file, info, blob);
        delete this;
    }

    int error;
    DBDoneGet done;
    void *param;
    IDataProvider *db;

    String guid;
    String *file;
    HashMap *info;
    ByteArray *blob;
};

void Database::get(const uchar *guid, DBDoneGet done, void *p) {
    if (peer != null)
        addJob(new JobGet(guid, done, p, provider));
    else {
        String g(guid);
        done(p, SQLITE_ERROR, g, null, null, null);
    }
}


struct JobGetFilename extends BlockingDatabaseJob {
    JobGetFilename(const uchar *g, String **f, IDataProvider* _db) :
        db(_db), guid(g), file(f) {
    }
    ~JobGetFilename() { }

    void run() {
        error = db->getFilename(guid, file);
        dispatch();
    }

    IDataProvider *db;
    String guid;
    String** file;
};

int Database::getFilename(const uchar *guid, String **file) {
    assert(!MessageQueue::isDispatchThread());
    if (peer != null)
        return addJobAndWait(new JobGetFilename(guid, file, provider));
    else
        return SQLITE_ERROR;
}


struct JobRemove extends DatabaseJob {
    JobRemove(const uchar *g, DBDone d, void *p, IDataProvider *_db) :
        error(0), done(d), param(p), db(_db), guid(g) {
    }

    ~JobRemove() { }

    void run() {
        error = db->remove(guid);
        MessageQueue::invokeLater(DatabaseJob::_dispatch, (DatabaseJob *)this);
    }

    void dispatch() {
        if (done != null) done(param, error);
        delete this;
    }

    int error;
    DBDone done;
    void *param;
    IDataProvider* db;

    String guid;
};

void Database::remove(const uchar *guid, DBDone done, void *p) {
    if (peer != null)
        addJob(new JobRemove(guid, done, p, provider));
    else
        done(p, SQLITE_ERROR);
}


struct JobUpdateThumbnail extends DatabaseJob {
    JobUpdateThumbnail(const uchar *g, const ByteArray& b, DBDone d, void *p, IDataProvider *_db) :
        error(0), done(d), param(p), db(_db), guid(g), blob(b) {
    }

    void run() {
        error = db->updateThumbnail(guid, blob);
        MessageQueue::invokeLater(DatabaseJob::_dispatch, (DatabaseJob *)this);
    }

    void dispatch() {
        if (done != null) done(param, error);
        delete this;
    }

    int error;
    DBDone done;
    void *param;
    IDataProvider *db;

    String guid;
    ByteArray blob;
};

void Database::updateThumbnail(const uchar *guid, const ByteArray& blob, DBDone done, void *p) {
    if (peer != null)
        addJob(new JobUpdateThumbnail(guid, blob, done, p, provider));
    else
        done(p, SQLITE_ERROR);
}


struct JobUpdateMetadata extends DatabaseJob {
    JobUpdateMetadata(const uchar *g, const HashMap& i, DBDone d, void *p, IDataProvider *_db) :
        error(0), done(d), param(p), db(_db), guid(g), info(i) {
    }

    ~JobUpdateMetadata() { }

    void run() {
        error = db->updateMetadata(guid, info);
        MessageQueue::invokeLater(DatabaseJob::_dispatch, (DatabaseJob *)this);
    }

    void dispatch() {
        if (done != null) done(param, error);
        delete this;
    }

    int error;
    DBDone done;
    void *param;
    IDataProvider *db;

    String guid;
    HashMap info;
};

void Database::updateMetadata(const uchar *guid, const HashMap& info, DBDone done, void *p) {
    if (peer != null) 
        addJob(new JobUpdateMetadata(guid, info, done, p, provider));
    else
        done(p, SQLITE_ERROR);
}


struct JobPushStackItem extends DatabaseJob {
    JobPushStackItem(int s, const uchar *g, DBDone d, void *p, IDataProvider* _db) :
        error(0), done(d), param(p), db(_db), stack(s), guid(g) {
    }
    ~JobPushStackItem() { }

    void run() {
        error = db->pushStackItem(stack, guid);
        MessageQueue::invokeLater(DatabaseJob::_dispatch, (DatabaseJob *)this);
    }

    void dispatch() {
        if (done != null) done(param, error);
        delete this;
    }

    int error;
    DBDone done;
    void *param;
    IDataProvider *db;

    int stack;
    String guid;
};

void Database::pushStackItem(int stack, const uchar *guid, DBDone done, void *p) {
    if (peer != null)
        addJob(new JobPushStackItem(stack, guid, done, p, provider));
    else
        done(p, SQLITE_ERROR);
}


struct JobCountStackItems extends DatabaseJob {
    JobCountStackItems(int s, DBDoneCount d, void *p, IDataProvider* _db) :
        error(0), done(d), param(p), db(_db), stack(s) {
    }
    ~JobCountStackItems() { }

    void run() {
        error = db->countStackItems(stack, count);
        MessageQueue::invokeLater(DatabaseJob::_dispatch, (DatabaseJob *)this);
    }

    void dispatch() {
        if (done != null) done(param, error, count);
        delete this;
    }

    int error;
    DBDoneCount done;
    void *param;
    IDataProvider *db;

    int stack;
    int count;
};

void Database::countStackItems(int stack, DBDoneCount done, void *p) {
    if (peer != null)
        addJob(new JobCountStackItems(stack, done, p, provider));
    else
        done(p, SQLITE_ERROR, 0);
}


struct JobRemoveStackItems extends DatabaseJob {
    JobRemoveStackItems(int s, const ArrayList& l, DBDone d, void *p, IDataProvider* _db) :
        error(0), done(d), param(p), db(_db), stack(s), list(l) {
    }
    ~JobRemoveStackItems() { }

    void run() {
        error = db->removeStackItems(stack, list);
        MessageQueue::invokeLater(DatabaseJob::_dispatch, (DatabaseJob *)this);
    }

    void dispatch() {
        if (done != null) done(param, error);
        delete this;
    }

    int error;
    DBDone done;
    void *param;
    IDataProvider *db;

    int stack;
    ArrayList list;
};

void Database::removeStackItems(int stack, const ArrayList& list, DBDone done, void *p) {
    if (peer != null)
        addJob(new JobRemoveStackItems(stack, list, done, p, provider));
    else
        done(p, SQLITE_ERROR);
}


struct JobBumpStackItems extends DatabaseJob {
    JobBumpStackItems(int s, const ArrayList& l, DBDone d, void *p, IDataProvider* _db) :
        error(0), done(d), param(p), db(_db), stack(s), list(l) {
    }
    ~JobBumpStackItems() { }

    void run() {
        error = db->bumpStackItems(stack, list);
        MessageQueue::invokeLater(DatabaseJob::_dispatch, (DatabaseJob *)this);
    }

    void dispatch() {
        if (done != null) done(param, error);
        delete this;
    }

    int error;
    DBDone done;
    void *param;
    IDataProvider *db;

    int stack;
    ArrayList list;
};

void Database::bumpStackItems(int stack, const ArrayList& list, DBDone done, void *p) {
    if (peer != null)
        addJob(new JobBumpStackItems(stack, list, done, p, provider));
    else
        done(p, SQLITE_ERROR);
}


struct JobPopStackItem extends BlockingDatabaseJob {
    JobPopStackItem(int s, String **g, IDataProvider* _db) :
        db(_db), stack(s), guid(g) {
    }
    ~JobPopStackItem() { }

    void run() {
        error = db->popStackItem(stack, guid);
        dispatch();
    }

    IDataProvider *db;
    int stack;
    String** guid;
};

int Database::popStackItem(int stack, String **guid) {
    assert(!MessageQueue::isDispatchThread());
    if (peer != null)
        return addJobAndWait(new JobPopStackItem(stack, guid, provider));
    else
        return SQLITE_ERROR;
}


///////////////////////////////////////////////////////////////////////////////
// Debugging/testing development code

#include "Application.h"
#include "ExifReader.h"

static void newDatabaseCallback(void *pv, int error, Database *db) {
    if (error) {
        trace(L"Error creating database: %d\n", error);
    }
    else {
        main.setDatabase(db);
    }
}

static void getCallback(void *p, int error, String &guid, String *file, HashMap *info, ByteArray *blob) {
    if (error) {
        trace(L"Error retrieving record {%s}: %d\n", (const uchar *)guid, error);
    }
    else {
        trace("Image {%s}: file=\"%s\" ", (const uchar *)guid, (const uchar *)*file);
        if (info != null) trace(" info.size()=%d", info->size()); else trace(" info=null");
        if (blob != null) trace(" blob.size()=%d", blob->size()); else trace(" blob=null");
        traceln("");
        delete file;
        delete info;
        delete blob;
    }
}

static void getIdsCallback(void *p, int error, ArrayList *ids) {
    if (error) {
        trace(L"Error retrieving record IDS: %d\n", error);
    }
    else {
//        trace(L"Listing IDS:\n");
        for (int i = 0; i < ids->size(); ++i) {
            const Object *o = ids->get(i);
            assert(instanceof(o, const String *));
            const String *s = dynamic_cast<const String *> (o);
//            traceln(L"%d: {%s}", i, (const uchar *)*s);
            main.getDatabase().get(*s, getCallback, null);
        }
        delete ids;
    }
}

static void addImageCallback(void *pv, int error) {
    String& guid = *(String *)pv;
    if (error) {
        trace(L"Error adding image {%s}: %d\n", (const uchar *)guid, error);
        delete &guid;
    }
    else {
        MessageQueue::post(L"imageAdded", &guid);
    }
}

static void updateMetadataCallback(void *pv, int error) {
    String *guid = (String *)pv;
    if (error) {
        trace(L"Error updating metadata for image {%s}: %d\n", (const uchar *)(*guid), error);
        delete guid;
    }
    else {
        MessageQueue::post(L"metadataUpdated", guid);
    }
}

static void updateThumbnailCallback(void *pv, int error) {
    String *guid = (String *)pv;
    if (error) {
        trace(L"Error updating thumbnail for image {%s}: %d\n", (const uchar *)(*guid), error);
        delete guid;
    }
    else {
        MessageQueue::post(L"thumbnailUpdated", guid);
    }
}

static void countCallback(void *p, int error, int count) {
    if (error) {
        trace(L"Error querying record count: %d\n", error);
    }
    else {
        trace(L"Current record count %d\n", count);
        main.getDatabase().getIds(0, count, getIdsCallback, null);
    }
}

struct DatabaseDriver implements BroadcastListener {

    DatabaseDriver() : count(0) {
    }
    
    void eventReceived(const Message &msg) {
//        traceln(L"DatabaseDriver::eventReceived: %s(%s)", msg.id, instanceof(msg.param, const String *) ? 
//            (const uchar *)*(dynamic_cast<const String *> (msg.param)) :  msg.param == null ? L"null" : L"param");

        if (ustrcmp(msg.id, L"createSampleDatabase") == 0) {
            Database::open(L"c:\\kolobok.klb", true, newDatabaseCallback, null);
        }
        else if (ustrcmp(msg.id, L"databaseChanged") == 0) {
            File file(main.getDatabase().getName());
            if (ustrcmp(file.getBasename(), L"kolobok.klb") == 0) {
                traceln(L"Populating database with sample images...");
                const uchar* images[] = {
                    L"olympus-d320l-APP12.jpg",
                    L"sony-powershota5-CIFF-APP0.jpg",
                    L"sony-d700.jpg",
                    L"kodak-dc210.jpg",
                    L"Olympus C920Z,D450Z,v873-75.jpg",
                    L"Fuji Film FinePix S602 Zoom.jpg",
                    L"canon-powershot-sd300.jpg",
                    L"nikon-e950.jpg",
                    L"canon-ixus.jpg",
                    L"fujifilm-dx10.jpg",
                    L"fujifilm-finepix40i.jpg",
                    L"fujifilm-mx1700.jpg",
                    L"kodak-dc240.jpg",
                    L"olympus-c960.jpg",
                    L"ricoh-rdc5300.jpg",
                    L"sanyo-vpcg250.jpg",
                    L"sanyo-vpcsx550.jpg",
                    L"sony-cybershot.jpg"
                };

                for (int i = 0; i < countof(images); ++i) {
                    StringBuffer buf;
                    buf.printf(L"../externals/EXIF/%s", images[i]);
                    MessageQueue::post(L"addImage", buf.toString());
                }
            }
        }
        else if (ustrcmp(msg.id, L"addImage") == 0) {
            // TODO: this needs to be a proper command handler
            assert(instanceof(msg.param, const String *));
            ExifReader er;
            HashMap *info = null;
            ByteArray *blob = null;
            const String *filename = dynamic_cast<const String *> (msg.param);
            if (er.read(*filename, &info, &blob)) {
                String *guid = String::guid();
                main.getDatabase().add(*guid, *filename, addImageCallback, guid);

//                g = new String(guid);
//                main.getDatabase().updateMetadata(*g, *info, updateMetadataCallback, g);
//                g = new String(guid);
//                main.getDatabase().updateThumbnail(*g, *blob, updateThumbnailCallback, g);
                delete info;
                delete blob;
            }
            else {
                traceln(L"Error reading EXIF from image \"%s\"", (const uchar *)*filename);
            }
        }
        else if (ustrcmp(msg.id, L"imageAdded") == 0) {
            if (++count == 7)
                main.getDatabase().count(countCallback, null);
        }
        else {
        }
    }
    int count;
};

static DatabaseDriver *dd;

void initSampleDatabase() {
    dd = new DatabaseDriver();
    MessageQueue::addBroadcastListener(dd);
}

void finiSampleDatabase() {
    if (dd != null) {
        MessageQueue::removeBroadcastListener(dd);
        delete dd;
    }
}
