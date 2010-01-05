#include "tiny.h"

// FolderIterator:-
// Recursively iterates through files in a folder.

struct FolderIterator {

    FolderIterator(const uchar *f_) : folder(null), hff(null), buf(null), cch(0),
            valid(false) {
        memset(&fd, 0, sizeof(fd));
        String base(f_);
        queue.add(base);
        getNext();
    }

    ~FolderIterator() {
        if (hff != null)
            FindClose(hff);
        delete[] buf;
        delete folder;
    }

    bool hasNext() {
        return valid;
    }

    String next(WIN32_FIND_DATA *pfd) {
        if (!valid)
            return String(L"");
        if (pfd != null)
            *pfd = fd;
        String result(name());
        getNext();
        return result;
    }

private:
    // NOTE: destroys previous content of buf!!!
    const uchar *name() {
        assert(folder != null);
        int count = folder->length() + 1 + ustrlen(fd.cFileName) + 1;
        if (count > cch) {
            if (buf != null) delete[] buf;
            cch = count;
            buf = new uchar[cch];
        }
        // REVIEW: ustrcpy should use number of characters, not number of bytes!!!
        ustrcpy(buf, cch*sizeof(uchar), (const uchar *)(*folder));
        ustrcpy(buf+folder->length(), cch*sizeof(uchar), L"\\");
        ustrcpy(buf+folder->length()+1, cch*sizeof(uchar), fd.cFileName);
        return buf;
    }

    bool getNext() {
        valid = false;
        do {
            while (hff == null || hff == INVALID_HANDLE_VALUE) {
                hff = null;
                if (queue.size() <= 0)
                    return false;
                if (folder != null)
                    delete folder;
                // TODO: folder = dynamic_cast<String*> (queue.removeFirst());
                folder = dynamic_cast<String*> ((dynamic_cast<const String *> (queue.get(0)))->clone());
                queue.removeAt(0);
                StringBuffer pattern;
                pattern.printf(L"\\\\?\\%s\\*", (const uchar*)(*folder));
                hff = FindFirstFile((const uchar*)pattern, &fd);
                valid = (hff != INVALID_HANDLE_VALUE);
            }
            assert(hff != null);
            while (hff != null && (!valid || (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0)) {
                if (valid && ustrcmp(L".", fd.cFileName) != 0 && ustrcmp(L"..", fd.cFileName) != 0) {
                    queue.add(name());
                }
                valid = (FindNextFile(hff, &fd) != 0);
                if (!valid) {
                    FindClose(hff);
                    hff = null;
                }
            }
        }
        while (!valid || (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0);
        return true;
    }

    uchar* buf;
    int cch;
    bool valid;
    String* folder;
    LinkedList queue;
    HANDLE hff;
    WIN32_FIND_DATA fd;
};


/*****************************************************************************
Intended usage:

    FolderIterator fs(L"C:\\My Documents"); // no trailing "\\"
    while (fs.hasNext()) {
        WIN32_FIND_DATA fd;
        String file = fs.next(&fd);
        trace(DateTime(fd.ftLastWriteTime)); trace(L" "); traceln(file);
    }

******************************************************************************/
