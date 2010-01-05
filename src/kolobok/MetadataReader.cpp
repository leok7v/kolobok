#include "MetadataReader.h"
#include "ExifReader.h"

MetadataReader::MetadataReader(Database *d, int stackId) : db(d), stack(stackId), shutdown(false) {
    MessageQueue::addBroadcastListener(this);
    assert(peer == null);   assert(job == null);
    check((job = CreateEvent(null, false, false, null)) != null);
    check((peer = CreateThread(null, 0, worker, this, 0, null)) != null);
}

MetadataReader::~MetadataReader() {
    MessageQueue::removeBroadcastListener(this);
}

static void pushMetadataItemCallback(void *pv, int error) {
    MetadataReader* mr = (MetadataReader*)pv;
    traceln("MetadataReader::pushMetadataItemCallback - %d", error);
    if (error == 0)
        mr->ping();
}

void MetadataReader::eventReceived(const Message& msg) {
    if (ustrcmp(msg.id, L"imageAdded") == 0) {
        const String &guid = *dynamic_cast<const String*>(msg.param);
        if (db != null)
            db->pushStackItem(stack, guid, pushMetadataItemCallback, this);
        ping();
    }
    else if (ustrcmp(msg.id, L"shutdown") == 0) {
        shutdown = true;
        ping();
        WaitForSingleObject(peer, INFINITE);
    }
}

dword WINAPI MetadataReader::worker(void *pv) {
    MetadataReader *mr = (MetadataReader*)pv;
    while (!mr->shutdown && WaitForSingleObject(mr->job, INFINITE) == WAIT_OBJECT_0) {
        String *guid = null;
        while (!mr->shutdown && mr->db->popStackItem(mr->stack, &guid) == 0 && guid != null) {
            mr->processMedia(*guid);
            delete guid;
        }
    }
    return 0;
}

static void pushThumbnailItemCallback(void *pv, int error) {
    String& guid = *(String*)pv;
    if (error != 0)
        traceln("MetadataReader::pushThumbnailItemCallback(%s) - %d", (const uchar*)guid, error);
    delete &guid;
    // TODO: (John) ping rescaler (when implmented)
}

static void updateThumbnailCallback(void *pv, int error) {
    String& guid = *(String*)pv;
    if (error != 0) {
        traceln("MetadataReader::updateThumbnailCallback(%s) - %d", (const uchar*)guid, error);
        delete &guid;
    }
    else
        MessageQueue::post(L"imageThumbnailChanged", &guid);
}

static void updateMetadataCallback(void *pv, int error) {
    String& guid = *(String*)pv;
    if (error != 0) {
        traceln("MetadataReader::updateMetadataCallback(%s) - %d", (const uchar*)guid, error);
        delete &guid;
    }
    else
        MessageQueue::post(L"imageMetadataChanged", &guid);
}

void MetadataReader::processMedia(String &guid) {
    String *file = null;
    db->getFilename(guid, &file);

    HashMap *info = null;
    ByteArray *blob = null;
    ExifReader er;
    if (er.read(*file, &info, &blob)) {
        db->updateMetadata(guid, *info, updateMetadataCallback, new String(guid));
        if (blob != null)
            db->updateThumbnail(guid, *blob, updateThumbnailCallback, new String(guid));
        else
            db->pushStackItem(Database::THUMBNAIL, guid, pushThumbnailItemCallback, new String(guid));
        delete info;
        delete blob;
    }
    else {
        traceln("Error reading EXIF for \"%s\"", (const uchar *)(*file));
    }
    delete file;
}
