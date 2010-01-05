#pragma once

#ifdef _DEBUG
   #define _CRTDBG_MAP_ALLOC
#endif

#define STRICT
#define WIN32_LEAN_AND_MEAN
#pragma warning(disable: 4509) /* nonstandard extension used: use of SEH and 'Error' has destructor */
#pragma warning(disable: 4214) /* nonstandard extension used : bit field types other than int */
#pragma warning(disable: 4201) /* nonstandard extension used : nameless struct/union */
#pragma warning(disable: 4710) /* function not expanded */
#include <Windows.h>
#include <WindowsX.h>
#include "ObjIdl.h"
#pragma warning(disable: 4201) /* nonstandard extension used : nameless struct/union */
#include <winver.h>
#include <WinSock2.h>
#include <ShlObj.h>
#include <ShellAPI.h>
#include <CommCtrl.h>
#include <CommDlg.h>
#pragma warning(disable: 4701) /* local variable may be used without having been initialized */
#include <strsafe.h>
#pragma warning(default: 4701)
#include <Psapi.h>
#include <DbgHelp.h>
#include <stdlib.h>
#include <rpc.h>
#ifdef _DEBUG
#ifndef _CRTDBG_MAP_ALLOC
#error undefined _CRTDBG_MAP_ALLOC 
#endif
#include <crtdbg.h>
#endif
#include <typeinfo.h>
#pragma warning(disable: 4514) /* unreferenced inline function has been removed */
#pragma warning(disable: 4710) /* function not expanded */
#pragma warning(disable: 4711) /* function selected for automatic inline expansion */
#pragma warning(disable: 4100) /* unreferenced formal parameter */

#pragma warning(disable: 4505) /* unreferenced local function has been removed */
#pragma warning(disable: 4057) /* differs in indirection to slightly different base types */
#pragma warning(disable: 4512) /* assignment operator could not be generated */

#pragma warning(default: 4189) /* unreferenced local variable vc++ 5.0 does not support it */

#pragma intrinsic(memcpy, memset, memcmp, abs)

#undef interface

#define null NULL
typedef __int64 longlong;
typedef unsigned __int64 ulonglong;
typedef unsigned char   byte;
typedef unsigned char   u1;
typedef unsigned short  u2;
typedef unsigned int    u4;
typedef unsigned _int64 u8;
typedef WCHAR uchar;
typedef unsigned short word;
typedef unsigned int uint;
typedef unsigned long dword;
#define countof(a) (sizeof(a)/sizeof((a)[0]))
#define interface struct __declspec(novtable)
#define extends : public virtual
#define implements : public virtual
#define and_implements , public virtual
#define unused(exp) (void)(exp)
#define instanceof(exp, iface) (dynamic_cast<iface>(exp) != null)

longlong getTimeInMicroseconds();

struct Object;
void trace(const Object* o);
void traceln(const Object* o);
void trace(const Object& o);
void traceln(const Object& o);
void trace(const uchar* fmt, ...);
void traceln(const uchar* fmt, ...);
void trace(const char* fmt, ...);
void traceln(const char* fmt, ...);
void traceError();

#undef assert
#ifdef _DEBUG
#define assert(exp) (void)( (exp) || (_assert(#exp, __FILE__, __LINE__), 0) )
#else
#define assert(exp)
#endif

int _assert(const char* exp, const char* file, int line);

#define check(exp) (void)( (exp) || (_assert(#exp, __FILE__, __LINE__), 0) )

inline int  ustrlen(const uchar* s) { return wcslen(s); }
inline void ustrcpy(uchar* d, int dstsize, const uchar* s) { StringCbCopyW(d, dstsize, s); }
inline int  ustrcmp(const uchar* l, const uchar* r) { return wcscmp(l, r); }
inline int  ustricmp(const uchar* l, const uchar* r) { return wcsicmp(l, r); }
inline const uchar* ustrchr(const uchar* s, uchar ch) { return wcschr(s, ch); }
inline const uchar* ustrrchr(const uchar* s, uchar ch) { return wcsrchr(s, ch); }

#ifdef _DEBUG
void* operator new(unsigned int s, char* file, int line);
#define THIS_FILE __FILE__
#define DEBUG_NEW new(THIS_FILE, __LINE__)
#define new DEBUG_NEW
#else
void* operator new(unsigned int s);
#endif

// -----------------------------------------------------------------------------------

struct Object;
struct String;
struct ByteArray;

interface Comparable {
    virtual int compareTo(const Object* right) const = 0;
};

typedef void (*Runnable)(void* p);

// -----------------------------------------------------------------------------------

struct Object {
    virtual ~Object() { }
    virtual int hashCode() const { return (int)this; }
    virtual Object* clone() const { __asm { int 3 }; return null;  } // not supported
    virtual bool equals(const Object* o) const { return o == this;  }
    virtual String* toString() const;
    static bool equals(const Object* o1, const Object* o2) {
        assert(o1 == null || o2 == null || !o1->equals(o2) || o1->hashCode() == o2->hashCode());
        return o1 == null ? o2 == null : o1->equals(o2);
    }
    static bool equals(const Object& o1, const Object* o2) { return equals(&o1, o2); }
    static bool equals(const Object* o1, const Object& o2) { return equals(o1, &o2); }
    static bool equals(const Object& o1, const Object& o2) { return equals(&o1,&o2); }
    static inline Object* clone(const Object* o) {
        return o == null ? null : o->clone();
    }
    static const Object* nil; // == null but typed
};

// -----------------------------------------------------------------------------------

/* immutable string */

struct String extends Object and_implements Comparable {

    inline String() : len(0), val(new uchar[1]), hash(-1) { 
        *(uchar*)val = 0;
    }

    inline String(const String* other) : len(other->len), val(new uchar[len + 1]), hash(other->hash) {
        memcpy((uchar*)val, other->val, len * sizeof(uchar));
    }

    inline String(const String& other) : len(other.len), val(new uchar[len + 1]), hash(other.hash) {
        memcpy((uchar*)val, other.val, len * sizeof(uchar));
    }

    inline String(const uchar* s) : len(ustrlen(s)), val(new uchar[len + 1]), hash(-1) {
        memcpy((uchar*)val, s, (len + 1) * sizeof(uchar));
    }

    inline String(const uchar* s, int l) : len(l), val(new uchar[len + 1]), hash(-1) {
        memcpy((uchar*)val, s, len * sizeof(uchar));
    }

    inline String(const char* s) : len(strlen(s)), val(new uchar[len + 1]), hash(-1) {
        for (int i = 0; i <= len; i++) ((uchar*)val)[i] = s[i];
    }
    
    virtual ~String() {
        delete (uchar *)val;
        val = null;
        *((int*)&len) = 0;
    }

    inline int length() const { return len; }

    int hashCode() const {
        if (hash == -1) hash = hashCode(val, len);
        return hash;
    }

    static int hashCode(const uchar *val, int len) {
        int hash = -1;
        for (int i = 0; i < len; i++) hash = 31U * hash + val[i];
        enum { mask = ((1U << (sizeof(int)*8-1)) - 1) };
        hash = hash & (uint)mask;
        if (hash <= 0) hash = -hash;
        return hash;
    }

    Object* clone() const { return new String(val);  }

    bool equals(const Object* o) const { 
        return o == null ? false : (o == dynamic_cast<const Object*>(this) ? true :
              (instanceof(o, const String*) ? 
               ustrcmp(val, dynamic_cast<const String*>(o)->val) == 0 : false));
    }

    bool equals(const uchar* s) const { 
        return s == null ? false : ustrcmp(val, s) == 0;
    }

    static String* operator+(const uchar* s1, const String& s2) {
        return String(s1) + s2;
    }

    String* operator+(const uchar* s) const {
        assert(s != null);
        int n = ustrlen(s) + 1;
        uchar* buf = new uchar[len + n];
        memcpy(buf, val, len * sizeof(uchar));
        memcpy(buf + len, s, n * sizeof(uchar));
        String* r = new String(buf);
        delete buf;
        return r;
    }

    String* operator+(const String* s) const { return operator+(*s); }

    bool equalsIgnoreCase(const Object* o) const { 
        const String* s = dynamic_cast<const String*>(o);
        return o == null ? false : 
               (this == s ? true :
                (s == null ? false :
                 (s->len != len ? false :
                  CompareString(LOCALE_USER_DEFAULT, NORM_IGNORECASE, val, len, s->val, len) == CSTR_EQUAL)));
    }

    bool equalsIgnoreCase(const uchar* s) const { 
        return CompareString(LOCALE_USER_DEFAULT, NORM_IGNORECASE, val, len, s, ustrlen(s)) == CSTR_EQUAL;
    }
    
    int compareTo(const Object* s) const { 
        if (s == null || !instanceof(s, const String*)) return +1;
        return ustrcmp(val, dynamic_cast<const String*>(s)->val); 
    }

    int compareTo(const uchar* s) const { 
        assert(CSTR_LESS_THAN == 1 && CSTR_EQUAL == 2 && CSTR_GREATER_THAN == 3);
        return s == null ? +1 : 
            CompareString(LOCALE_USER_DEFAULT, 0, val, len, s, -1) - CSTR_EQUAL;
    }

    bool startsWith(const uchar* s) const { 
        int n = ustrlen(s);
        return n <= len ? memcmp(val, s, n * sizeof(uchar)) == 0 : false; 
    }

    bool startsWithIgnoreCase(const uchar* s) const { 
        int n = ustrlen(s);
        return n > len ? false :
            CompareString(LOCALE_USER_DEFAULT, NORM_IGNORECASE, val, n, s, n) == CSTR_EQUAL;
    }

    bool endsWith(const uchar* s) const {
        int n = ustrlen(s);
        return n > len ? false : ustrcmp(val + len - n, s) == 0;
    }

    bool endsWithIgnoreCase(const uchar* s) const {
        int n = ustrlen(s);
        return n > len ? false :
            CompareString(LOCALE_USER_DEFAULT, NORM_IGNORECASE, val + len - n, n, s, n) == CSTR_EQUAL;
    }

    String* toString() const; // see StringBuffer.h for the implementation

    inline operator const uchar*() const { 
        return val; 
    }

    inline const uchar charAt(int i) const { 
        assert(0 <= i && i < len);
        return val[i];
    }

    static String* guid(); // generates new GUID on each call

    static String* fromUTF8(const byte* utf8);
    static String* fromUTF8(const char* utf8) { return fromUTF8((const byte*)utf8); }
    static ByteArray* toUTF8(const uchar* s, bool bom = false); // ByteArray is NOT zero terminated
    static ByteArray* toUTF8(const String& s, bool bom = false) { return toUTF8((const uchar*)s, bom); }
    static ByteArray* toUTF8(const String* s, bool bom = false) { return toUTF8(*s, bom); }

    static byte* toUTF8(const uchar *s, /*out*/int &len, bool bom = false);

    static String* toString(const Object*);
    static String* toString(const Object& o) { return toString(&o); }
private:
    inline void operator=(const String& s) { assert(false); }
    const int len;
    uchar const* val;
    mutable int hash;
};

inline String* Object::toString() const {
    uchar buf[16];
    StringCchPrintf(buf, countof(buf), L"Obj0x%08X", this);
    return new String(buf);
}

// -----------------------------------------------------------------------------------

/* Mutable string.
   append operator+= and appending printf
*/

struct StringBuffer extends Object and_implements Comparable {

    StringBuffer() : len(0), val(new uchar[1]) {
        *val = 0;
    }

    StringBuffer(int n) : len(0), val(new uchar[n]) {
        *val = 0;
    }

    StringBuffer(const uchar* s) : len(ustrlen(s)), val(new uchar[len + 1]) {
        memcpy(val, s, (len + 1) * sizeof(uchar));
    }

    virtual ~StringBuffer() {
        delete val;
        val = null;
        len = 0;
    }
    
    int length() const { return len; }
    
    void clear() {
        delete val;
        val = new uchar[1];
        *val = 0;
        len = 0;
    }

    int hashCode() const { 
        int hash = 0;
        for (int i = 0; i < len; i++) hash = 31U * hash + val[i];
        enum { mask = ((1U << (sizeof(int)*8-1)) - 1) };
        hash = hash & (uint)mask;
        return hash <= 0 ? -hash : hash;
    }

    Object* clone() const { return new StringBuffer(val);  }

    bool equals(const Object* o) const { 
        return o == null ? false : (o == dynamic_cast<const Object*>(this) ? true :
              (instanceof(o, const StringBuffer*) ? 
               ustrcmp(val, dynamic_cast<const StringBuffer*>(o)->val) == 0 : false));
    }

    String* toString() const { return new String(val); }

    uchar get(int i) const { 
        assert(0 <= i && i < len);
        return val[i]; 
    }

    void set(int i, uchar ch) { 
        assert(0 <= i && i < len);
        val[i] = ch;
    }

    operator const uchar*() const { return val; }

    int compareTo(const Object* s) const { 
        if (s == null || !instanceof(s, const StringBuffer*)) return +1;
        return ustrcmp(val, dynamic_cast<const StringBuffer*>(s)->val);
    }

    void operator +=(const uchar* s) {
        int n = ustrlen(s);
        uchar* v = new uchar[n + len + 1];
        memcpy(v, val, len * sizeof(uchar));
        memcpy(&v[len], s, (n + 1) * sizeof(uchar));
        len += n;
        delete val;
        val = v;
    }

    void operator +=(const String& s) {
        *this += (const uchar*)s;
    }

    void operator +=(const String* s) {
        *this += (const uchar*)*s;
    }

    void operator +=(const uchar ch) {
        uchar s[2];
        s[0] = ch;
        s[1] = 0;
        *this += (const uchar*)s;
    }

    void printf(const uchar* fmt, ...) { // always appends
        va_list vl;
        va_start(vl, fmt);
        vprintf(fmt, vl);
        va_end(vl);
    }

    void vprintf(const uchar* fmt, va_list vl) { // always appends
        int n = 64;
        uchar* buf = null;
        HRESULT hr = STRSAFE_E_INSUFFICIENT_BUFFER;
        while (hr == STRSAFE_E_INSUFFICIENT_BUFFER) {
            delete buf;
            n = n + n;
            buf = new uchar[n];
            hr = StringCchVPrintf(buf, n, fmt, vl);
        }
        *this += buf;
        delete buf;
    }

private:
    int len;
    uchar* val;
};

inline String* String::toString() const { 
    StringBuffer sb(L"\"");
    sb += val;
    sb += L"\"";
    return new String(sb);
}

// -----------------------------------------------------------------------------------

/* 32-bit signed integer value */

struct Int extends Object and_implements Comparable {
    
    enum {
        MAX_VALUE = 0x7FFFFFFF,
        MIN_VALUE = 0x80000000
    };
    
    inline Int(int v) : val(v) { }
    
    virtual ~Int() { }
    
    int hashCode() const { 
        return val & 0x7FFFFFFF; // positive
    }
    
    Object* clone() const { return new Int(val);  }
    
    bool equals(const Object* o) const { 
        return o == null ? false : (instanceof(o, const Int*) ? 
            val == dynamic_cast<const Int*>(o)->val : false);
    }

    String* toString() const {
        uchar buf[32];
        StringCchPrintf(buf, countof(buf), L"%I64d", val);
        return new String(buf);
    }
    
    inline int intValue() const { return (int)val; }
    

    int compareTo(const Object* o) const {
        if (o == null || !instanceof(o, const Int*)) return +1;
        else {
            const Int* r = dynamic_cast<const Int*>(o);
            return val == r->val ? 0 : (val < r->val ? -1 : +1);
        }
    }

    static inline int decode(const uchar* s) { return _wtoi(s); }
    
private:
    int val;
};

// -----------------------------------------------------------------------------------

/* 64-bit signed integer value */

struct Long extends Object and_implements Comparable {
    
    inline Long(longlong v) : val(v) { }
    
    virtual ~Long() { }
    
    int hashCode() const { 
        int i = (int)val + (int)((val >> 32) & 0xFFFFFFFF);
        return i < 0 ? -i : i;
    }
    
    Object* clone() const { return new Long(val);  }
    
    bool equals(const Object* o) const { 
        return o == null ? false : (instanceof(o, const Long*) ? 
            val == dynamic_cast<const Long*>(o)->val : false);
    }

    String* toString() const {
        uchar buf[32];
        StringCchPrintf(buf, countof(buf), L"%I64d", val);
        return new String(buf);
    }
    
    inline int intValue() const { return (int)val; }
    
    inline longlong longValue() const { return val; }

    int compareTo(const Object* o) const {
        if (o == null || !instanceof(o, const Long*)) return +1;
        else {
            const Long* r = dynamic_cast<const Long*>(o);
            return val == r->val ? 0 : (val < r->val ? -1 : +1);
        }
    }

    static inline longlong decode(const uchar* s) { return _wtoi64(s); }
    
private:
    longlong val;
};

// -----------------------------------------------------------------------------------

/* 64-bit signed rational value */

struct Rational extends Object and_implements Comparable {
    
    inline Rational(longlong _n, longlong _d) : n(_n), d(_d == 0 ? 1 : _d) { }
    
    virtual ~Rational() { }
    
    int hashCode() const { 
        int i = (int)n + (int)((n >> 32) & 0xFFFFFFFF) +
            (int)d + (int)((d >> 32) & 0xFFFFFFFF);
        return i < 0 ? -i : i;
    }

    Object* clone() const { return new Rational(n, d);  }

    bool equals(const Object* o) const { 
        return o == null ? false : (instanceof(o, const Rational*) ? 
            n == dynamic_cast<const Rational*>(o)->n &&
            d == dynamic_cast<const Rational*>(o)->d : false);
    }

    String* toString() const {
        uchar buf[65];
        StringCchPrintf(buf, countof(buf), L"%I64d/%I64d", n, d);
        return new String(buf);
    }
    
    inline double doubleValue() const { return (double)n / (double)d; }
    inline longlong numerator() const { return n; }
    inline longlong denominator() const { return d; }

    int compareTo(const Object* o) const {
        if (o == null || !instanceof(o, const Rational*)) return +1;
        else {
            const Rational* r = dynamic_cast<const Rational*>(o);
            if (n == r->n && d == r->d)
                return 0;
            return doubleValue() < r->doubleValue() ? -1 : +1;
        }
    }

    static Rational* decode(const uchar *s) {
        longlong num = 0;
        longlong den = 0;
        swscanf(s, L"%I64d/%I64d", &num, &den);
        return new Rational(num, den);
    }

private:
    longlong n, d;
};

// -----------------------------------------------------------------------------------

/* 64-bit double value */

struct Double extends Object and_implements Comparable {
    
    inline Double(double v) : val(v) { }
    
    virtual ~Double() { }
    
    int hashCode() const {
        longlong* v = (longlong*)&val;
        int i = (int)*v + (int)((*v >> 32) & 0xFFFFFFFF);
        return i < 0 ? -i : i;
    }
    
    Object* clone() const { return new Double(val);  }
    
    bool equals(const Object* o) const { 
        return o == null ? false : (instanceof(o, const Double*) ? 
            val == dynamic_cast<const Double*>(o)->val : false);
    }

    String* toString() const {
        uchar buf[64];
        StringCchPrintf(buf, countof(buf), L"%.13g", val);
        return new String(buf);
    }
    
    inline double doubleValue() const { return val; }
        
    int compareTo(const Object* o) const {
        if (o == null || !instanceof(o, const Double*)) return +1;
        else {
            const Double* r = dynamic_cast<const Double*>(o);
            return val == r->val ? 0 : (val < r->val ? -1 : +1);
        }
    }

private:
    double val;
};

// -----------------------------------------------------------------------------------

/* object wrapper for Win-32 POINT. MUTABLE! */

struct Point extends Object {
    
#pragma pack(push, 1)
    public: LONG x;
    public: LONG y;
#pragma pack(pop)

    inline Point() : x(0), y(0) { }
    inline Point(const Point& pt) : x(pt.x), y(pt.y) { }
    inline Point(int xc, int yc) : x(xc), y(yc) { }
    
    virtual ~Point() { }

    const Point& operator=(const Point& pt) { x = pt.x; y = pt.y; return *this; }
    
    int hashCode() const {
        int h = x;
        h += ~(h << 9U) + y;
        h ^=  ((uint)h >> 14U);
        h +=  (h << 4U);
        h ^=  ((uint)h >> 10U);
        return h < 0 ? -h : h;
    }
    
    Object* clone() const { return new Point(x, y);  }
    
    bool equals(const Object* o) const { 
        if (o == null || !instanceof(o, const Point*)) return false;
        const Point& pt = *dynamic_cast<const Point*>(o);
        return pt.x == x && pt.y == y;
    }

    String* toString() const {
        StringBuffer sb;
        sb.printf(L"Point@0x%08X(%d,%d)", this, x, y);
        return sb.toString();
    }
    
    inline operator POINT*() const { return (POINT*)&x; }
    inline operator POINT() const { return *(POINT*)&x; }
        
};

// -----------------------------------------------------------------------------------

struct Rect extends Object {

#pragma pack(push, 1)
    public: LONG left;
    public: LONG top;
    public: LONG right;
    public: LONG bottom;
#pragma pack(pop)

    inline Rect() : left(0), top(0), right(0), bottom(0) { }
    inline Rect(int l, int t, int r, int b) : left(l), top(t), right(r), bottom(b) { }
    inline Rect(const Rect& s) : left(s.left), top(s.top), right(s.right), bottom(s.bottom) { }
    
    inline void operator=(const Rect& s) { left = s.left; top = s.top; right = s.right; bottom = s.bottom; }
    
    virtual ~Rect() { }
    
    int hashCode() const {
        int h = left;
        h += ~(h << 9U) + top;
        h ^=  ((uint)h >> 14U) + bottom;
        h +=  (h << 4U) + right;
        h ^=  ((uint)h >> 10U);
        return h < 0 ? -h : h;
    }
    
    Object* clone() const { return new Rect(left, top, right, bottom);  }
    
    bool equals(const Object* o) const { 
        if (o == null || !instanceof(o, const Rect*)) return false;
        const Rect& rc = *dynamic_cast<const Rect*>(o);
        return rc.left == left && rc.top == top && rc.right == right && rc.bottom == bottom;
    }

    String* toString() const {
        StringBuffer sb;
        sb.printf(L"Rect@0x%08X(%d,%d,%d,%d)", this, left, top, right, bottom);
        return sb.toString();
    }

    inline getWidth() const { return right - left; }
    inline getHeight() const { return bottom - top; }
    
    inline operator RECT*() const { return (RECT*)&left; }
        
};

// -----------------------------------------------------------------------------------

/* object wrapper for Win-32 COLORREF */

struct Color extends Object and_implements Comparable {
    
    public: COLORREF c;

    inline Color() : c(0) { }
    inline Color(int r, int g, int b) : c(RGB(r, g, b)) { }
    inline Color(COLORREF other) : c(other) { }

    virtual ~Color() { }
    
    int hashCode() const {
        return c;
    }
    
    Object* clone() const { return new Color(getRed(), getGreen(), getBlue());  }

    inline int getRed() const { return GetRValue(c); }
    inline int getGreen() const { return GetGValue(c); }
    inline int getBlue() const { return GetBValue(c); }
    
    bool equals(const Object* o) const { 
        if (o == null || !instanceof(o, const Color*)) return false;
        const Color& i = *dynamic_cast<const Color*>(o);
        return i.c == c;
    }

    String* toString() const {
        uchar buf[64];
        StringCchPrintf(buf, countof(buf), L"Color(%d,%d,%d)", getRed(), getGreen(), getBlue());
        return new String(buf);
    }
    
    int compareTo(const Object* o) const {
        if (o == null || !instanceof(o, const Color*)) return +1;
        else {
            const Color* r = dynamic_cast<const Color*>(o);
            return c == r->c ? 0 : (c < r->c ? -1 : +1);
        }
    }

    inline operator COLORREF() const { return c; }
        
};


// -----------------------------------------------------------------------------------

/* LinkedList is deep cloning container. It always clones elements 
   and deletes elements in destructor.

   TODO: implement other methods on as needed basis
   TODO: implement removeFirst, removeLast on as needed basis
*/

struct LinkedList extends Object {

    LinkedList() : count(0), head(null), modified(0) { }
    
    virtual ~LinkedList() { clear(); }

private:
    struct Entry {
        Entry(const Object* d) : next(null), prev(null), data((Object*)d) { }
        Entry* next;
        Entry* prev;
        Object* data;
    };

public:
    struct Iterator { // failfast 

        // TODO: implement remove() w/ concurent modification check

        Iterator(const LinkedList &list) : that(&list), mc(list.modified) {
            e = that->head;
            pos = 0;
        }
        
        Iterator(const LinkedList* list) : that(list), mc(list->modified) {
            e = that->head;
            pos = 0;
        }

        bool hasNext() const { return pos < that->count; }

        const Object* next() { 
            assert(mc == that->modified); /* concurent modification? */
            const Entry* r = e;
            e = e->next;
            pos++;
            return r->data;
        }

    private:
        const LinkedList* that;
        const Entry* e;
        mutable int pos;
        const int mc;
    };
    
    void add(const Object* o) { // appends to list
        Entry* e = new Entry(Object::clone(o));
        if (head == null) {
            e->next = e->prev = e;
            head = e;
        }
        else {
            e->next = head;
            e->prev = head->prev;
            e->prev->next = e;
            e->next->prev = e;
        }
        count++;
        modified++;
    }

    void add(int before, const Object* o) { // insert before pos
        if (before == count) { add(o); return; }
        Entry* p = getEntry(before);
        Entry* e = new Entry(Object::clone(o));
        e->next = p;
        e->prev = p->prev;
        e->next->prev = e;
        e->prev->next = e;
        if (head == p) head = e;
        count++;
        modified++;
    }
    
    bool remove(const Object* o) {
        assert(o != null);
        Entry* e = head;
        for (int i = 0; i < count && !Object::equals(o, e->data); i++) {
            e = e->next;
        }
        if (i >= count) return false;
        removeEntry(e); 
        return true; 
    }

    // TODO: do we need it? we may be better of with removeFirst removeLast
    void removeAt(int ix) { 
        assert(0 <= ix && ix < count);
        Entry* e = head;
        while (ix-- > 0) e = e->next;
        removeEntry(e);
    }

    void clear() {
        Entry* e = head;
        for (int i = 0; i < count; i++) {
            Entry* next = e->next;
            delete e->data;
            delete e;
            e = next;
        }
        head = null;
        count = 0;
        modified++;
    }

    const Object* get(int i) const { 
        return getEntry(i)->data;
    }

    const void set(int i, const Object* o) const { 
        Entry* e = getEntry(i);
        delete e->data;
        e->data = Object::clone(o);
    }
    
    inline int size() const { return count; }

    int hashCode() const { 
        int hash = 0;
        Entry* e = head;
        for (int i = 0; i < count; i++) {
            hash += e->data != null ? e->data->hashCode() : 0;
            e = e->next;
        }
        return hash;
    }

    Object* clone() const {
        LinkedList* list = new LinkedList();
        Entry* e = head;
        for (int i = 0; i < count; i++, e = e->next) {
            list->add(e->data);
        }
        return list;
    }

    bool equals(const Object* o) const { 
        if (o == null) return false;
        const LinkedList* list = dynamic_cast<const LinkedList*>(o);
        if (list == null) return false;
        if (list == this) return true;
        if (count != list->count) return false;
        Entry* e1 = head;
        Entry* e2 = list->head;
        for (int i = 0; i < count; i++) {
            if (!Object::equals(e1->data, e2->data)) return false;
            e1 = e1->next;
            e2 = e2->next;
        }
        return true;
    }

    String* toString() const { 
        StringBuffer sb;
        sb.printf(L"LinkedList@0x%08X{", this);
        Entry* e = head;
        for (int i = 0; i < count; i++) {
            String* s = String::toString(e->data);
            sb += s;
            delete s;
            if (i < count - 1) sb += L", ";
            e = e->next;
        }
        sb += L"}";
        return sb.toString();
    }

    void add(int n) { add(Int(n)); }
    void add(longlong n) { add(Long(n)); }
    void add(double d) { add(Double(d)); }
    void add(const uchar* s) { add(String(s)); }
    void add(const Object& o) { add(&o); }

    void add(int before, int n) { add(before, Long(n)); }
    void add(int before, longlong n) { add(before, Long(n)); }
    void add(int before, double d) { add(before, Double(d)); }
    void add(int before, const uchar* s) { add(before, String(s)); }
    void add(int before, const Object& o) { add(before, &o); }

    bool remove(const Object& o) { return remove(&o); }
    bool remove(const uchar* s) { return remove(String(s)); }
    bool remove(int n) { return remove(Int(n)); }
    bool remove(longlong n) { return remove(Long(n)); }
    bool remove(double d) { return remove(Double(d)); }

private:

    Entry* getEntry(int ix) const {
        assert(0 <= ix && ix < count);
        Entry* e = head;
        for (int i = 0; i < ix; i++) e = e->next;
        return e;
    }

    void removeEntry(Entry* e) {
        if (count == 1) { clear(); return; }
        if (e == head) head = e->next;
        e->next->prev = e->prev;
        e->prev->next = e->next;
        delete e->data;
        delete e;
        count--;
        modified++;
    }

    friend Iterator;
    int count;
    Entry* head;
    int modified;
};

// -----------------------------------------------------------------------------------

struct ByteArray extends Object {

    ByteArray() : len(0), capacity(0), data(null), modified(0) { }
    
    ByteArray(const byte* b, int size) : len(size), capacity(size), data(new byte[size]), modified(0) {
        memcpy(data, b, size);
    }

    ByteArray(const ByteArray &o) : len(o.len), capacity(o.len), data(new byte[o.len]), modified(0) {
        memcpy(data, o.data, len);
    }
    
    ByteArray(int size) : len(size), capacity(size), data(new byte[size]), modified(0) {
    }

    virtual ~ByteArray() { clear(); }

    void add(byte b) {
        if (len + 1 >= capacity) grow();
        assert(len < capacity);
        data[len++] = b;
        modified++;
    }
    
    void insert(int index, byte b) {
        if (len + 1 >= capacity) grow();
        assert(len < capacity);
        for (int i = len; i > index; i--) data[i] = data[i - 1];
        data[index] = b;
        len++;
        modified++;
    }
    
    const byte get(int i) {
        assert(0 <= i && i < len);
        return data[i];
    }

    void set(int i, byte b) {
        assert(0 <= i && i < len);
        data[i] = b;
    }

    void removeAt(int ix) { 
        assert(0 <= ix && ix < len);
        memcpy(&data[ix], &data[ix+1], len - ix - 1);
        data[--len] = 0;
        modified++;
        if (len < capacity / 2) shrink();
    }
    
    void clear() {
        delete data; data = null;
        len = 0;
        capacity = 0;
        modified++;
    }

    inline int size() const { return len; }

    int hashCode() const { 
        int hash = 0;
        for (int i = 0; i < len; i++) hash = 31U * hash + data[i];
        enum { mask = ((1U << (sizeof(int)*8-1)) - 1) };
        return hash <= 0 ? -hash : hash;
    }

    Object* clone() const {
        ByteArray* a = new ByteArray();
        a->data = new byte[len];
        memcpy(a->data, data, len);
        a->len = a->capacity = len;
        return a;
    }

    bool equals(const Object* o) const {
        if (o == null) return false;
        const ByteArray* a = dynamic_cast<const ByteArray*>(o);
        if (a == null) return false;
        if (o == a) return true;
        if (len != a->len) return false;
        for (int i = 0; i < len; i++) {
            if (data[i] != a->data[i]) return false;
        }
        return true;
    }

    inline operator byte*() const { 
        return data; 
    }

    String* toString() const { 
        StringBuffer sb(L"[");
        for (int i = 0; i < len; i++) {
            uchar s[8];
            StringCchPrintf(s, countof(s), L"0x%02X", data[i]);
            sb += s;
            if (i < len - 1) sb += L", ";
        }
        sb += L"]";
        return sb.toString();
    }

private:

    void grow() {
        capacity = capacity == 0 ? 4 : capacity * 2;
        reallocData();
    }

    void shrink() {
        capacity = len;
        if (len == 0) clear(); 
        else reallocData();
    }

    void reallocData() {
        byte* d = new byte[capacity];
        assert(len <= capacity);
        memcpy(d, data, len);
        delete[] data;
        data = d;
    }

    int len;
    int capacity;
    byte* data;
    int modified;
};

// -----------------------------------------------------------------------------------

/* ArrayList is deep cloning container. It always clones elements.
   Note:
       1. add(o) and a[i] = o clone the object passed as a parameter
       2. a[i] returns access to the object not a clone

   Because of this correct code that implements e.g. swap is:

        const Object* o = a.get(i);
        Object* c = o->clone();
        a.set(i, a.get(j)); // this will call delete and "o" is not valid after that!
        a.set(j, c);
        delete c;
    
   Exercise extreme caution.

   TODO: implement swap and possibly other methods on as needed basis

*/

struct ArrayList extends Object {

    ArrayList() : len(0), capacity(0), data(null), modified(0) { }
    
    virtual ~ArrayList() { 
        clear(); 
    }

    void add(const Object* o) {
        if (len + 1 >= capacity) grow();
        assert(len < capacity);
        data[len++] = Object::clone(o);
        modified++;
    }
    
    void add(int before, const Object* o) {
        if (len + 1 >= capacity) grow();
        assert(len < capacity);
        for (int i = len; i > before; i--) data[i] = data[i - 1];
        data[before] = Object::clone(o);
        len++;
        modified++;
    }
    
    const Object* get(int i) const {
        assert(0 <= i && i < len);
        return data[i];
    }

    void set(int i, const Object* o) {
        assert(0 <= i && i < len);
        delete data[i];
        data[i] = Object::clone(o);
    }

    bool remove(const Object* o) {
        int i = 0;
        while (i < len && !Object::equals(o, data[i])) i++;
        if (i < len) { removeAt(i); return true; }
        return false;
    }

    void removeAt(int ix) { 
        assert(0 <= ix && ix < len);
        Object* r = data[ix];
        memcpy(&data[ix], &data[ix+1], (len - ix - 1) * sizeof(Object*));
        data[--len] = null;
        delete r;
        modified++;
        if (len < capacity / 2) shrink();
    }
    
    void clear() {
        for (int i = 0; i < len; i++) {
            delete data[i];
            data[i] = null;
        }
        delete[] data; data = null;
        len = 0;
        capacity = 0;
        modified++;
    }

    inline int size() const { return len; }

    int hashCode() const { 
        int hash = 0;
        for (int i = 0; i < len; i++) 
            hash += data[i] == null ? 0 : data[i]->hashCode();
        return hash;
    }

    Object* clone() const {
        ArrayList* a = new ArrayList();
        a->data = new Object*[len];
        for (int i = 0; i < len; i++) a->data[i] = Object::clone(data[i]);
        a->len = a->capacity = len;
        return a;
    }

    bool equals(const Object* o) const { 
        if (o == null) return false;
        const ArrayList* a = dynamic_cast<const ArrayList*>(o);
        if (a == null) return false;
        if (a == this) return true;
        if (len != a->len) return false;
        for (int i = 0; i < len; i++) {
            if (!Object::equals(data[i], a->data[i])) return false;
        }
        return true;
    }

    String* toString() const { 
        StringBuffer sb;
        sb.printf(L"ArrayList@0x%08X[", this);
        for (int i = 0; i < len; i++) {
            String* s = String::toString(data[i]);
            sb += s;
            delete s;
            if (i < len - 1) sb += L", ";
        }
        sb += L"]";
        return sb.toString();
    }

    void add(const Object& o) { add(&o); }
    void add(int n) { add(Int(n)); }
    void add(longlong n) { add(Long(n)); }
    void add(double d) { add(Double(d)); }
    void add(const uchar* s) { add(String(s)); }
    
    void add(int before, const Object& o) { add(before, o); }
    void add(int before, int n) { add(before, Int(n)); }
    void add(int before, longlong n) { add(before, Long(n)); }
    void add(int before, double d) { add(before, Double(d)); }
    void add(int before, const uchar* s) { add(before, String(s)); }
    
    void set(int i, const Object& o) { set(i, &o); }
    void set(int i, int n) { set(i, Int(n)); }
    void set(int i, longlong n) { set(i, Long(n)); }
    void set(int i, double d) { set(i, Double(d)); }
    void set(int i, const uchar* s) { set(i, String(s)); }

    bool remove(const Object& o) { return remove(&o); }
    bool remove(int n) { return remove(Int(n)); }
    bool remove(longlong n) { return remove(Long(n)); }
    bool remove(double d) { return remove(Double(d)); }
    bool remove(const uchar* s) { return remove(String(s)); }

private:

    void grow() {
        capacity = capacity == 0 ? 4 : capacity * 2;
        reallocData();
    }

    void shrink() {
        capacity = len;
        if (len == 0) clear(); 
        else reallocData();
    }

    void reallocData() {
        Object** d = new Object*[capacity];
        assert(len <= capacity);
        memcpy(d, data, len * sizeof(Object*));
        delete[] data;
        data = d;
    }

    int len;
    int capacity;
    Object** data;
    int modified;
};

// -----------------------------------------------------------------------------------

/* HashMap is deep cloning hash table container. It always clones keys and values.
   Keys MUST be immutable.

   Singnificant difference from java.utils.HashMap (because of deep cloning):
      const Object* o = map.get("key");
      if (o == null) map.put("key", o = new Object());
      // here o does NOT points on what in the map!!!
   code above produces memory leak and any further operations
   with object do not modify object clone that resides in the map.
   Instead use:
      const Object* o = map.get("key");
      if (o == null) o = map.put("key", Object());
   this produces correct results.

   TODO: implement other methods on as needed basis
*/

struct HashMap extends Object {

    HashMap() : count(0), capacity(0), entries(null), modified(0) { }
    HashMap(const HashMap &o) : count(0), capacity(0), entries(null), modified(0) {
        for (EntryIterator i(o); i.hasNext(); ) {
            const Entry* e = i.nextEntry();
            put(e->key, e->val);
        }
    }
    
    virtual ~HashMap() { clear(); }

    int size() const { return count; }

    struct EntryIterator;

    struct Entry {

        Entry(Object* k, Object* v, Entry* n) : key(k), val(v), next(n) { }
        
        virtual ~Entry() {
            key = val = null;
            next = null;
        }

        inline Object* getKey() const { return key; }
        inline Object* getVal() const { return val; }
        
    private:
        friend struct HashMap;
        friend struct EntryIterator;
        Object* key;
        Object* val;
        Entry* next;
    };

    struct EntryIterator { // failfast 

        // TODO: implement remove() w/ concurent modification check

        EntryIterator(const HashMap& map) : that(&map), pos(-1), mc(map.modified), i(null) {
            nextPos();
        }
        
        EntryIterator(const HashMap* map) : that(map), pos(-1), mc(map->modified), i(null) {
            nextPos();
        }

        virtual ~EntryIterator() { delete i; }

        bool hasNext() const { return i != null; }

        const Entry* nextEntry() { 
            check(mc == that->modified); /* concurent modification? */
            check(hasNext());
            const Entry* e = i;
            nextPos();
            return e;
        }

    private:
        void nextPos() {
            if (i != null) i = i->next;
            if (i == null && pos < that->capacity - 1) {
                do { pos++; } while (pos < that->capacity && that->entries[pos] == null);
                i = pos < that->capacity ? that->entries[pos] : null;
            }
        }

        const HashMap* that;
        int pos;
        Entry* i;
        const int mc;
    };

    struct KeyIterator extends EntryIterator {
        KeyIterator(const HashMap& map) : EntryIterator(map) { }
        KeyIterator(const HashMap* map) : EntryIterator(map) { }
        const Object* next() { return EntryIterator::nextEntry()->getKey(); }
    };

    struct ValueIterator extends EntryIterator {
        ValueIterator(const HashMap& map) : EntryIterator(map) { }
        ValueIterator(const HashMap* map) : EntryIterator(map) { }
        const Object* next() { return EntryIterator::nextEntry()->getVal(); }
    };
    
    Object* put(const Object* key, const Object* val) {
        if (capacity < (count + 1) * 4 / 3) grow();
        int h = hash(key) % capacity;
        Entry* e = count == 0 ? null : getEntry(key, h);
        if (e != null) {
            if (e->val == val) return e->val; // not modified
            delete e->val;
            e->val = Object::clone(val);
            modified++;
            return e->val;
        }
        entries[h] = new Entry(Object::clone(key), Object::clone(val), entries[h]);
        count++;
        modified++;
        return entries[h]->val;
    }

    // TODO: add put(double, ...) helpers if ever needed

    Entry* getEntry(const Object* key) const {
        if (count == 0) return null;
        int h = hash(key) % capacity;
        return getEntry(key, h);
    }    

    bool containsKey(const Object* key) const {
        Entry* e = getEntry(key);
        return e != null;
    }


    const Object* get(const Object* key) const {
        if (count == 0) return null;
        Entry* e = getEntry(key);
        return e == null ? null : e->val;
    }
    
    bool remove(const Object* key) {
        if (count == 0) return false;
        int h = hash(key) % capacity;
        Entry** prev = &entries[h];
        for (Entry* list = entries[h]; list != null; ) {
            Entry* next = list->next;
            if (Object::equals(list->key, key)) {
                *prev = list->next;
                deleteEntry(list);
                modified++;
                count--;
                if (capacity > 7 && count < capacity / 3) shrink();
                return true;
            }
            prev = &list->next;
            list = next;
        }
        return false;
    }

    void clear() {
        for (int i = 0; i < capacity; i++) {
            for (Entry* list = entries[i]; list != null; ) {
                Entry* next = list->next;
                deleteEntry(list);
                list = next;
            }
        }
        delete[] entries;  entries = null;
        count = capacity = 0;
        modified++;
    }

    Object* clone() const {
        HashMap* c = new HashMap();
        for (EntryIterator i(this); i.hasNext(); ) {
            const Entry* e = i.nextEntry();
            c->put(e->key, e->val);
        }
        return c;
    }

    bool equals(const Object* o) const { 
        if (o == null || !instanceof(o, const HashMap*)) return false;
        const HashMap& map = *dynamic_cast<const HashMap*>(o);
        if (&map == this) return true;
        if (count != map.count) return false;
        for (EntryIterator i(this); i.hasNext(); ) {
            const Entry* e = i.nextEntry();
            if (!Object::equals(map.get(e->key), e->val)) return false;
        }
        assert(hashCode() == map.hashCode());
        return true;
    }

    int hashCode() const { return count; }

    String* toString() const { 
        int k = 0;
        StringBuffer sb;
        sb.printf(L"HashMap@0x%08X{", this);
        for (EntryIterator i(this); i.hasNext(); ) {
            const Entry* e = i.nextEntry();
            String* sk = String::toString(e->key);
            String* sv = String::toString(e->val);
            sb.printf((++k < count ? L"%s=%s, " : L"%s=%s"),
                      (const uchar*)*sk, (const uchar*)*sv);
            delete sk;
            delete sv;
        }
        sb += L"}";
        return sb.toString();
    }

    // Convenient methods:

    inline bool containsKey(const uchar* k) const { 
        return k != null ? containsKey(String(k)) : containsKey(Object::nil);
    }

    inline bool containsKey(const Object& k) const { return containsKey(&k); }
    inline bool containsKey(longlong k) const { return containsKey(Long(k)); }
    inline bool containsKey(int k) const { return containsKey(Int(k)); }

    const String* getString(const uchar *k) const { return dynamic_cast<const String*>(get(k)); }
    const String* getString(const Object *k) const { return dynamic_cast<const String*>(get(k)); }
    const String* getString(int k) const { return dynamic_cast<const String*>(get(k)); }
    const String* getString(longlong k) const { return dynamic_cast<const String*>(get(k)); }
    
    const HashMap* getMap(const uchar *k) const { return dynamic_cast<const HashMap*>(get(k)); }
    const HashMap* getMap(const Object *k) const { return dynamic_cast<const HashMap*>(get(k)); }

    const Int* getInt(const uchar *k) const { return dynamic_cast<const Int*>(get(k)); }
    const Int* getInt(const Object *k) const { return dynamic_cast<const Int*>(get(k)); }

    const Long* getLong(const uchar *k) const { return dynamic_cast<const Long*>(get(k)); }
    const Long* getLong(const Object *k) const { return dynamic_cast<const Long*>(get(k)); }

    const Double* getDouble(const uchar *k) const { return dynamic_cast<const Double*>(get(k)); }
    const Double* getDouble(const Object *k) const { return dynamic_cast<const Double*>(get(k)); }
    
    const Color* getColor(const uchar *k) const { return dynamic_cast<const Color*>(get(k)); }
    const Color* getColor(const Object *k) const { return dynamic_cast<const Color*>(get(k)); }

    const Object* get(const uchar* k) const { return k != null ? get(String(k)) : get(Object::nil); }

    const Object* get(const Object& k) const { return get(&k); }
    const Object* get(longlong k) const { return get(Long(k)); }
    const Object* get(int k) const { return get(Int(k)); }

    inline bool remove(const uchar* k) { 
        return k != null ? remove(String(k)) : remove(Object::nil);
    }

    inline bool remove(const Object& k) { return remove(&k); }
    inline bool remove(longlong k) { return remove(Long(k)); }
    inline bool remove(int k) { return remove(Int(k)); }

    inline Object* put(const Object& k, const Object& v) { return put(&k, &v); }
    inline Object* put(const Object* k, const Object& v) { return put( k, &v); }
    inline Object* put(const Object& k, const Object* v) { return put(&k,  v); }

    Object* put(const Object& k, const uchar* v)   { return put(&k, v); }
    Object* put(const Object& k, longlong v) { return put(&k, v); }
    Object* put(const Object& k, double v) { return put(&k, v); }
    Object* put(const Object& k, int v) { return put(&k, v); }

    Object* put(const uchar* k, const Object& v) { 
        return k != null ? put(k, &v) : put(Object::nil, &v);
    }

    Object* put(int k, const Object& v) { return put(k, &v); }
    Object* put(longlong k, const Object& v) { return put(k, &v); }

    Object* put(const Object* k, const uchar* v) {
        return v != null ? put(k, String(v)) : put(k, Object::nil);
    }

    Object* put(const Object* k, int v) { return put(k, Int(v)); }
    Object* put(const Object* k, longlong v) { return put(k, Long(v)); }
    Object* put(const Object* k, double v) { return put(k, Double(v)); }

    Object* put(const uchar* k, const uchar* v) {
        if (k == null && v == null) return put(Object::nil, Object::nil);
        else if (k == null) return put(Object::nil, String(v));
        else if (v == null) return put(String(k), Object::nil);
        else return put(String(k), String(v));
    }

    Object* put(const uchar* k, int v) { return put(k, Int(v)); }

    Object* put(const uchar* k, longlong v) {
        return k != null ? put(String(k), Long(v)) : put(Object::nil, Long(v));
    }

    Object* put(const uchar* k, double v) {
        return k != null ? put(String(k), Double(v)) : put(Object::nil, Double(v));
    }

    Object* put(const uchar* k, const Object* v) {
        return k != null ? put(String(k), v) : put(Object::nil, v);
    }

    Object* put(int k, const uchar* v) {
        return v != null ? put(Int(k), String(v)) : put(Int(k), Object::nil);
    }

    Object* put(int k, const Object* v) { return put(Int(k), v); }

    Object* put(longlong k, const uchar* v) {
        return v != null ? put(Long(k), String(v)) : put(Long(k), Object::nil);
    }

    Object* put(longlong k, int v) { return put(Long(k), Int(v)); }
    Object* put(longlong k, longlong v) { return put(Long(k), Long(v)); }
    Object* put(longlong k, double v) { return put(Long(k), Double(v)); }
    Object* put(longlong k, const Object* v) { return put(Long(k), v); }

private:

    friend EntryIterator;

    Entry* getEntry(const Object* key, int h) const {
        assert(count > 0);
        for (Entry* list = entries[h]; list != null; list = list->next) {
            if (Object::equals(key, list->key)) return list;
        }
        return null;
    }

    void grow() {
        int n = capacity == 0 ? 5 : capacity * 2;
        reallocData(n);
    }

    void shrink() {
        int n = capacity * 3 / 4;
        if (n < count) n = count;
        if (count == 0) clear(); 
        else reallocData(n);
    }

    void reallocData(int n) {
        assert(count <= capacity);
        Entry** e = new Entry*[n];
        for (int i = 0; i < capacity; i++) {
            for (Entry* list = entries[i]; list != null; ) {
                Entry* next = list->next;
                int h = hash(list->key) % n;
                e[h] = new Entry(list->key, list->val, e[h]);
                delete list;
                list = next;
            }
        }
        delete[] entries;
        entries = e;
        capacity = n;
        assert(count <= capacity);
    }

    static int hash(const Object* x) {
        if (x == null) return 0;
        int h = x->hashCode();
        h += ~(h << 9U);
        h ^=  ((uint)h >> 14U);
        h +=  (h << 4U);
        h ^=  ((uint)h >> 10U);
        return h < 0 ? -h : h;
    }

    static void deleteEntry(Entry* list) {
        delete list->key;
        delete list->val;
        delete list;
    }

    int count;
    int capacity;
    Entry** entries;
    int modified;
    static const Object* nil;
};

// -----------------------------------------------------------------------------------

/* HashSet is deep cloning container. It always clones keys.
   keys MUST be immutable
   TODO: implement other methods on as needed basis
*/

struct HashSet extends Object {

    HashSet() { }
    
    virtual ~HashSet() { }

    // not KeyIterator because of compiler bug
    struct Iterator extends HashMap::EntryIterator { 
        Iterator(const HashSet &set) : EntryIterator(set.map) { }
        Iterator(const HashSet* set) : EntryIterator(set->map) { }
        const Object* next() { return EntryIterator::nextEntry()->getKey(); }
    };

    int size() const { return map.size(); }

    Object* add(const Object* e) { return map.put(e, Object::nil); }
    Object* add(const Object& e) { return add(&e); }
    Object* add(int n) { return add(Int(n)); }
    Object* add(longlong n) { return add(Long(n)); }
    Object* add(const uchar* s) { return add(String(s)); }

    bool remove(const Object* o) { return map.remove(o); }
    bool remove(const Object& o) { return remove(&o); }
    bool remove(int v) { return remove(Int(v)); }
    bool remove(longlong v) { return remove(Long(v)); }
    bool remove(const uchar* s) { return remove(String(s)); }
    
    bool contains(const Object* e) const { return map.containsKey(e); }
    bool contains(const Object& e) const { return contains(&e); }
    bool contains(const uchar* s) const { return contains(String(s)); }
    bool contains(int n) const { return contains(Int(n)); }
    bool contains(longlong n) const { return contains(Long(n)); }

    void clear() { map.clear(); }

    Object* clone() const {
        HashSet* c = new HashSet();
        for (HashMap::KeyIterator i(map); i.hasNext(); ) c->add(i.next());
        return c;
    }

    String* toString() const { 
        int k = 0, count = map.size();
        StringBuffer sb;
        sb.printf(L"HashSet@0x%08X{", this);
        for (HashMap::KeyIterator i(map); i.hasNext(); ) {
            String* key = String::toString(i.next());
            sb.printf((++k < count ? L"%s, " : L"%s"), (const uchar*)*key);
            delete key;
        }
        sb += L"}";
        return sb.toString();
    }

    bool equals(const Object* o) const { 
        if (o == null) return false;
        const HashSet* set = dynamic_cast<const HashSet*>(o);
        if (set == null) return false;
        if (set == this) return true;
        if (size() != set->size()) return false;
        return map.equals(&set->map);
    }

    int hashCode() const { return map.hashCode(); }

private:
    friend Iterator;
    HashMap map;
};

// -----------------------------------------------------------------------------------

struct InputStream extends Object {

    InputStream() { }
    virtual ~InputStream() { }

    virtual int available() const = 0;
    virtual void close() = 0;
    virtual int read(byte* b, int len) = 0;
    virtual void reset() = 0;
    virtual const String* getName() const = 0;
};

// -----------------------------------------------------------------------------------

struct OutputStream extends Object {

    OutputStream() { }
    virtual ~OutputStream() { }

    virtual bool close() = 0;
    virtual int write(const byte* b, int len) = 0;
    virtual bool flush() = 0;
    virtual const String* getName() const = 0;
};

// -----------------------------------------------------------------------------------

struct ByteArrayInputStream extends InputStream {

    ByteArrayInputStream(const ByteArray* ba, const uchar* n = null) : 
        data(dynamic_cast<const ByteArray*>(ba->clone())), pos(0), name(null) { 
        if (n != null) {
            name = new String(n);
        }
        else {
            StringBuffer sb;
            sb.printf(L"ByteArrayInputStream(0x%08X)", (const byte*)*ba);
            name = sb.toString();
        }
    }

    virtual ~ByteArrayInputStream() {
        delete data;
        delete name;
    }

    int available() const { return data == null ? 0 : data->size() - pos; }

    void close() {
        delete data; data = null; pos = 0;
    }

    int read(byte* b, int len) {
        if (data == null || pos >= data->size()) return -1;
        int n = available();
        if (n > len) n = len;
        memcpy(b, ((const byte*)*data) + pos, n);
        pos += n;
        return n;
    }
    
    void reset() { pos = 0; }

    const String* getName() const { return name; }

    const ByteArray* data;
    String* name;
    int pos;
};

// -----------------------------------------------------------------------------------

/* LIMITATION: max file size 2GB-1 */

struct FileOutputStream extends OutputStream {

    FileOutputStream(HANDLE h, const uchar* fname) : handle(h), file(new String(fname)) {
        assert(handle != INVALID_HANDLE_VALUE);
    }

    FileOutputStream(const uchar* fname) : handle(INVALID_HANDLE_VALUE), file(new String(fname)) {
        handle = CreateFile(fname, GENERIC_WRITE, 0, null,
                               CREATE_ALWAYS, FILE_FLAG_SEQUENTIAL_SCAN, null);
        check(handle != INVALID_HANDLE_VALUE);
    }
    
    virtual ~FileOutputStream() {
        check(close());
        delete file;
    }

    bool close() {
        bool b = true;
        if (handle != INVALID_HANDLE_VALUE) {
            b = flush() && CloseHandle(handle);
        }
        handle = INVALID_HANDLE_VALUE;
        return b;
    }

    int write(const byte* b, int len) {
        if (handle == INVALID_HANDLE_VALUE) return -1;
        dword res = (dword)-1;
        if (!WriteFile(handle, b, len, &res, null)) {
            return -1;
        }
        else {
            return (int)res;
        }
    }

    bool flush() {
        return FlushFileBuffers(handle) != FALSE;
    }

    const String* getName() const { return file; }

    private: HANDLE handle;
    private: String* file;
};

// -----------------------------------------------------------------------------------

/* LIMITATION: max file size 2GB-1 */

struct FileInputStream extends InputStream {

    FileInputStream(HANDLE h, const uchar* fname) : handle(h), file(new String(fname)) {
        assert(handle != INVALID_HANDLE_VALUE);
        size = GetFileSize(h, null);
    }

    virtual ~FileInputStream() {
        close();
        delete file;
    }

    int available() const { 
        if (handle == INVALID_HANDLE_VALUE) return -1;
        dword pos = SetFilePointer(handle, 0, null, FILE_CURRENT);
        return size - (int)pos;
    }

    void close() {
        if (handle != INVALID_HANDLE_VALUE) CloseHandle(handle);
        handle = INVALID_HANDLE_VALUE;
    }

    int read(byte* b, int len) {
        if (handle == INVALID_HANDLE_VALUE) return -1;
        dword res = (dword)-1;
        if (!ReadFile(handle, b, len, &res, null)) {
            return -1;
        }
        else {
            return (int)res;
        }
    }

    void reset() {
        dword pos = SetFilePointer(handle, 0, null, FILE_BEGIN);
        assert(pos != 0xFFFFFFFF);
    }

    const String* getName() const { return file; }

    private: HANDLE handle;
    private: String* file;
    private: int size;
};

// -----------------------------------------------------------------------------------

struct File extends Object and_implements Comparable {

    File(const uchar* fn) : file(fn) { }

    File(const String* fn) : file(fn) { }

    File(const String& fn) : file(fn) { }

    int length() const { return file.length(); }
    
    bool exists() const { return GetFileAttributes(file) != 0xFFFFFFFF; }

    longlong getSize() const {
        WIN32_FILE_ATTRIBUTE_DATA a = {0};
        if (!GetFileAttributesEx(file, GetFileExInfoStandard, &a)) return -1;
        return (((longlong)a.nFileSizeHigh) << 32U) | a.nFileSizeLow;
    }

    bool isDirectory() const {
        WIN32_FILE_ATTRIBUTE_DATA a = {0};
        if (!GetFileAttributesEx(file, GetFileExInfoStandard, &a)) return false;
        return (a.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
    }

    operator const uchar*() const { return (const uchar*)file; }

    const uchar * getBasename() {
        const uchar *base_f = ustrrchr(file, '/');
        const uchar *base_r = ustrrchr(file, '\\');

        if (base_f != null && base_r != null)
            return (base_f > base_r ? base_r : base_f) + 1;
        else if (base_f != null)
            return base_f + 1;
        else if (base_r != null)
            return base_r + 1;
        else
            return file;
    }

    ByteArray* readBinary() const {
        HANDLE f = CreateFile(file, GENERIC_READ, FILE_SHARE_READ, null,
                              OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN, null);
        if (f == INVALID_HANDLE_VALUE) return null;
        FileInputStream s(f, file);
        int n = (int)getSize();
        ByteArray* b = new ByteArray(n);
        if (s.read(*b, n) != n) {
            delete b;
            return null;
        }
        return b;
    }

    String* readUTF8() const {
        ByteArray* b = readBinary();
        if (b == null) return null;
        String* s = String::fromUTF8((const char*)(byte*)*b);
        delete b;
        return s;
    }

    static String* File::getSpecialFolder(int csidl) { 
        // CSIDL_MYPICTURES, CSIDL_APPDATA, CSIDL_PERSONAL, CSIDL_DESKTOP
        ITEMIDLIST* iil = null;
        if (FAILED(SHGetSpecialFolderLocation(::GetDesktopWindow(), csidl, &iil))) {
            return null;
        }
        assert(iil != null);
        uchar buf[16*1024];
        if (!SHGetPathFromIDList(iil, buf)) return null;
        IMalloc* piMalloc = null;
        check(SUCCEEDED(SHGetMalloc(&piMalloc)));
        check(piMalloc != null);
        piMalloc->Free(iil);
        piMalloc->Release();
        return new String(buf);
    }

    int hashCode() const { return file.hashCode(); }

    Object* clone() const { return new File(file);  }

    bool equals(const Object* o) const { 
        if (o == null || !instanceof(o, const File*)) return false;
        const File& f = *dynamic_cast<const File*>(o);
        return file.equals(&f.file);
    }

    int compareTo(const Object* o) const { 
        if (o == null || !instanceof(o, const File*)) return +1;
        const File& f = *dynamic_cast<const File*>(o);
        return file.compareTo(&f.file); 
    }

    String* toString() const { 
        StringBuffer sb;
        sb.printf(L"File@0x%08X\"%s\"", this, (const uchar*)file);
        return sb.toString();
    }

private:
    String file;
};


// -----------------------------------------------------------------------------------

struct XMLDecoder {

    static Object* decode(const String& s) { return decode(&s); }

    static Object* decode(const String* s) {
        int pos = 0;
        return decode(s, pos);
    }

    static Object* decode(const String* s, int &pos) {
        String* tag = nextTag(s, pos);
        bool b = tag->compareTo(L"<?xml version=\"1.0\" encoding=\"UTF-8\"?>") == 0;
        check(b); // compiler bug: cannot use escaped string as assert macro param
        delete tag;
        tag = nextTag(s, pos);
        assert(tag->startsWith(L"<XMLEncoder"));
        delete tag;
        tag = nextTag(s, pos);
        Object* o = decodeObject(s, pos, tag);
        delete tag;
        tag = nextTag(s, pos);
        assert(tag->startsWith(L"</XMLEncoder"));
        delete tag;
        return o;
    }
    
    static Object* decodeObject(const String* s, int &pos, const String* tag) {
        Object* r = null;
        assert(tag->startsWith(L"<"));
        if (tag->compareTo(L"<object>") == 0) {
            String* t = nextValue(s, pos);
            assert(t->compareTo(L"null") == 0);
            delete t;
            checkEndTag(s, pos, L"</object>");
        }
        else if (tag->compareTo(L"<int>") == 0) {
            String* t = nextValue(s, pos);
            int v = _wtoi((const uchar*)*t);
            delete t;
            r = new Int(v);
            checkEndTag(s, pos, L"</int>");
        }
        else if (tag->compareTo(L"<long>") == 0) {
            String* t = nextValue(s, pos);
            longlong v = _wtoi64((const uchar*)*t);
            delete t;
            r = new Long(v);
            checkEndTag(s, pos, L"</long>");
        }
        else if (tag->compareTo(L"<point>") == 0) {
            String* t = nextValue(s, pos);
            Point* pt = new Point();
            swscanf((const uchar*)*t, L"%d,%d", &pt->x, &pt->y);
            delete t;
            r = pt;
            checkEndTag(s, pos, L"</point>");
        }
        else if (tag->compareTo(L"<rect>") == 0) {
            String* t = nextValue(s, pos);
            Rect* rc = new Rect();
            swscanf((const uchar*)*t, L"%d,%d,%d,%d", &rc->left, &rc->top, 
                &rc->right, &rc->bottom);
            delete t;
            r = rc;
            checkEndTag(s, pos, L"</rect>");
        }
        else if (tag->compareTo(L"<color>") == 0) {
            String* t = nextValue(s, pos); 
            assert(t->startsWithIgnoreCase(L"0x"));
            const uchar* n = *t;
            Color* c = new Color();
            swscanf(n + 2, L"%X", &c->c);
            delete t;
            r = c;
            checkEndTag(s, pos, L"</color>");
        }
        else if (tag->compareTo(L"<double>") == 0) {
            String* t = nextValue(s, pos);
            uchar* last = null;
            double v = wcstod((const uchar*)*t, &last);
            delete t;
            r = new Double(v);
            checkEndTag(s, pos, L"</double>");
        }
        else if (tag->compareTo(L"<string>") == 0) {
            String* t = nextValue(s, pos);
            if (t->compareTo(L"</string>") == 0) { // empty string
                delete t;
                r = new String();
            }
            else {
                r = t;
                checkEndTag(s, pos, L"</string>");
            }
        }
        else if (tag->compareTo(L"<file>") == 0) {
            String* t = nextValue(s, pos);
            if (t->compareTo(L"</file>") == 0) { // empty string
                r = new File(L"");
            }
            else {
                r = new File(t);
                checkEndTag(s, pos, L"</file>");
            }
            delete t;
        }
        else if (tag->compareTo(L"<StringBuffer>") == 0) {
            String* t = nextValue(s, pos);
            if (t->compareTo(L"</StringBuffer>") == 0) { // empty string
                r = new StringBuffer();
            }
            else {
                r = new StringBuffer((const uchar*)*t);
                checkEndTag(s, pos, L"</StringBuffer>");
            }
            delete t;
        }
        else if (tag->compareTo(L"<rational>") == 0) {
            String* t = nextValue(s, pos);
            if (t->compareTo(L"</rational>") == 0) {
                r = new Rational(0, 1);
            }
            else {
                longlong n=0, d=1;
                swscanf((const uchar*)*t, L"%I64d/%I64d", &n, &d);
                r = new Rational(n, d == 0 ? 1 : d);
                checkEndTag(s, pos, L"</rational>");
            }
            delete t;
        }
        else if (tag->compareTo(L"<list>") == 0) {
            LinkedList* list = new LinkedList();
            String* t = nextTag(s, pos);
            while (t->compareTo(L"</list>") != 0) {
                Object* e = decodeObject(s, pos, t);
                list->add(e);
                delete e;  e = null;
                delete t;  t = null;
                t = nextTag(s, pos);
            }
            delete t;
            r = list;
        }
        else if (tag->compareTo(L"<array>") == 0) {
            ArrayList* array = new ArrayList();
            String* t = nextTag(s, pos);
            while (t->compareTo(L"</array>") != 0) {
                Object* e = decodeObject(s, pos, t);
                array->add(e);
                delete e; e = null;
                delete t; t = null;
                t = nextTag(s, pos);
            }
            delete t;
            r = array;
        }
        else if (tag->compareTo(L"<set>") == 0) {
            HashSet* set = new HashSet();
            String* t = nextTag(s, pos);
            while (t->compareTo(L"</set>") != 0) {
                Object* e = decodeObject(s, pos, t);
                set->add(e);
                delete e;  e = null;
                delete t;  t = null;
                t = nextTag(s, pos);
            }
            delete t;
            r = set;
        }
        else if (tag->compareTo(L"<map>") == 0) {
            HashMap* map = new HashMap();
            String* k = nextTag(s, pos);
            while (k->compareTo(L"</map>") != 0) {
                Object* key = decodeObject(s, pos, k);
                String* v = nextTag(s, pos);
                Object* val = decodeObject(s, pos, v);
                delete v; v = null;
                map->put(key, val);
                delete key; key = null;
                delete val; val = null;
                delete k; k = null;
                k = nextTag(s, pos);
            }
            delete k;
            r = map;
        }
        else {
            trace(L"Unknown tag \"%s\" in pos %d\n", (const uchar*)*tag, pos);
            assert(false); // unknown tag
        }
        return r;
    }

    static void checkEndTag(const String* s, int &pos, const uchar* val) {
        String* close = nextTag(s, pos);
        assert(close->compareTo(val) == 0);
        delete close;
    }

    static String* nextTag(const String* s, int& pos) {
        skipBlank(s, pos);
        bool stop = false;
        uchar ch = decodeChar(s, pos, stop);
        if (ch == '<') {
            StringBuffer sb;
            sb += ch;
            do {
                ch = s->charAt(pos++);
                sb += ch;
            }
            while (ch != '>');
            return sb.toString();
        }
        else {
            return nextValue(s, pos, ch, true);
        }
    }

    static String* nextValue(const String* s, int& pos, uchar ch = 0, bool prepend = false) {
        StringBuffer sb;
        bool stop = false;
        if (!prepend) ch = decodeChar(s, pos, stop);
        while (!stop && pos < s->length()) {
            sb += ch;
            ch = decodeChar(s, pos, stop);
        }
        if (stop && ch == '<') pos--;
        return sb.toString();
    }

    static void skipBlank(const String* s, int& pos) {
        uchar ch = s->charAt(pos);
        while (ch == L' ' || ch == L'\t' || ch == L'\r' || ch == L'\n') {
            pos++;
            ch = s->charAt(pos);
        }
    }
    
    static uchar decodeChar(const String* s, int& pos, bool &stop) {
        uchar ch = s->charAt(pos++);
        stop = ch == '<';
        if (ch != L'&') return ch;
        int n = pos;
        while (s->charAt(n) != ';') n++;
        uchar buf[16] = {0};
        assert(n - pos < countof(buf));
        int i, j = 0;
        for (i = pos; i < n; i++) buf[j++] = s->charAt(i);
        pos = n + 1;
        if (ustrcmp(buf, L"amp") == 0) return L'&';
        else if (ustrcmp(buf, L"lt") == 0) return L'<';
        else if (ustrcmp(buf, L"gt") == 0) return L'>';
        else if (ustrcmp(buf, L"apos") == 0) return L'\'';
        else if (ustrcmp(buf, L"quot") == 0) return L'\"';
        else if (buf[0] == L'#') {
            uint c = 0;
            if (buf[1] == L'x') {
                for (i = 2; i < 6; i++) {
                    uchar ch = towlower(buf[i]);
                    assert(L'0' <= ch && ch <= '9' || L'a' <= ch && ch <= 'f');
                    c = c * 16U + (ch <= '9' ? (ch - L'0') : (ch - L'a' + 10));
                }
            }
            else {
                for (i = 1; i < 5; i++) {
                    uchar ch = towlower(buf[i]);
                    assert(L'0' <= ch && ch <= '9');
                    c = c * 10U + (ch - L'0');
                }
            }
            return (uchar)c;
        }
        else {
            assert(false); // don't know how to decode
            return ' ';
        }
    }

};

// -----------------------------------------------------------------------------------

struct XMLEncoder {

    static String* encode(const Object& o) { return encode(&o); }

    static String* encode(const Object* o) {
        StringBuffer sb;
        sb += L"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\r\n";
        sb += L"<XMLEncoder xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\">\r\n";
        encode(sb, o, 0);
        sb += L"</XMLEncoder>";
        return sb.toString();
    }

private:

    static void encode(StringBuffer& sb, const Object* o, int depth) {
        if (depth >= 0) tabify(sb, depth);
        if (o == null) {
            sb += L"<object>null</object>";
        }
        else if (instanceof(o, const String*)) {
            sb += L"<string>";
            const String& s = *dynamic_cast<const String*>(o);
            encodeString(sb, (const uchar*)s, s.length());
            sb += L"</string>";
        }
        else if (instanceof(o, const StringBuffer*)) {
            sb += L"<StringBuffer>";
            const StringBuffer& s = *dynamic_cast<const StringBuffer*>(o);
            encodeString(sb, (const uchar*)s, s.length());
            sb += L"</StringBuffer>";
        }
        else if (instanceof(o, const File*)) {
            sb += L"<file>";
            const File& f = *dynamic_cast<const File*>(o);
            encodeString(sb, (const uchar*)f, f.length());
            sb += L"</file>";
        }
        else if (instanceof(o, const Int*)) {
            const Int& v = *dynamic_cast<const Int*>(o);
            sb.printf(L"<int>%d</int>", v.intValue());
        }
        else if (instanceof(o, const Long*)) {
            const Long& v = *dynamic_cast<const Long*>(o);
            sb.printf(L"<long>%I64d</long>", v.longValue());
        }
        else if (instanceof(o, const Double*)) {
            const Double& v = *dynamic_cast<const Double*>(o);
            sb.printf(L"<double>%.13g</double>", v.doubleValue());
        }
        else if (instanceof(o, const Point*)) {
            const Point& v = *dynamic_cast<const Point*>(o);
            sb.printf(L"<point>%d,%d</point>", v.x, v.y);
        }
        else if (instanceof(o, const Rect*)) {
            const Rect& v = *dynamic_cast<const Rect*>(o);
            sb.printf(L"<rect>%d,%d,%d,%d</rect>", v.left, v.top, v.right, v.bottom);
        }
        else if (instanceof(o, const Rational*)) {
            const Rational& v = *dynamic_cast<const Rational*>(o);
            sb.printf(L"<rational>%I64d/%I64d</rational>", v.numerator(), v.denominator());
        }
        else if (instanceof(o, const Color*)) {
            const Color& v = *dynamic_cast<const Color*>(o);
            sb.printf(L"<color>0x%08X</color>", v.c);
        }
        else if (instanceof(o, const LinkedList*)) {
            sb += L"<list>\r\n";
            const LinkedList* list = dynamic_cast<const LinkedList*>(o);
            for (LinkedList::Iterator i(list); i.hasNext(); ) {
                append(sb, i.next(), depth + 1);
            }
            tabify(sb, depth, L"</list>\r\n");
        }
        else if (instanceof(o, const ArrayList*)) {
            sb += L"<array>\r\n";
            const ArrayList* array = dynamic_cast<const ArrayList*>(o);
            int n = array->size();
            for (int i = 0; i < n; i++) {
                append(sb, array->get(i), depth + 1);
            }
            tabify(sb, depth, L"</array>\r\n");
        }
        else if (instanceof(o, const HashSet*)) {
            sb += L"<set>\r\n";
            const HashSet& set = *dynamic_cast<const HashSet*>(o);
            int k = 0, n = set.size();
            Object** a = new Object*[n];
            for (HashSet::Iterator i(set); i.hasNext(); ) a[k++] = (Object*)i.next();
            sort(a, n);
            for (k = 0; k < n; k++) append(sb, a[k], depth + 1);
            delete[] a;
            tabify(sb, depth, L"</set>\r\n");
        }
        else if (instanceof(o, const HashMap*)) {
            sb += L"<map>\r\n";
            const HashMap& map = *dynamic_cast<const HashMap*>(o);
            int k = 0, n = map.size();
            Object** a = new Object*[n];
            for (HashMap::KeyIterator i(map); i.hasNext(); ) a[k++] = (Object*)i.next();
            sort(a, n);
            for (k = 0; k < n; k++) {
                encode(sb, a[k], depth + 1);
                const Object* val = map.get(a[k]);
                if (!isCollection(val)) {
                    encode(sb, val, -1);
                    sb += L"\r\n";
                }
                else {
                    sb += L"\r\n";
                    encode(sb, val, depth+1);
                }
            }
            delete[] a;
            tabify(sb, depth, L"</map>\r\n");
        }
        else {
            // Do not know how to encode, encode null object instead:-
            sb += L"<object>null</object>";
//            assert(false); // do not know how to encode
        }
    }

    static void encodeString(StringBuffer &sb, const uchar* s, int len) {
        for (int i = 0; i < len; i++) {
            const uchar ch = s[i];
            if (ch == L'&') sb += L"&amp;";
            else if (ch == L'<') sb += L"&lt;";
            else if (ch == L'>') sb += L"&gt;";
            else if (ch == L'\'') sb += L"&apos;";
            else if (ch == L'\"') sb += L"&quot;";
            else if (ch <= 255 && (ch < 32 || ch > 127)) {
                uchar sc[16];
                StringCchPrintf(sc, countof(sc), L"&#x%04X;", ch);
                sb += sc;
            }
            else {
                sb += ch;
            }
        }
    }

    static void append(StringBuffer& sb, const Object* o, int depth) {
        encode(sb, o, depth + 1); 
        if (!isCollection(o)) sb += L"\r\n";
    }

    static void tabify(StringBuffer &sb, int depth) {
        for (int d = 0; d < depth; d++) sb += L" ";
    }

    static void tabify(StringBuffer &sb, int depth, const uchar* close) {
        for (int d = 0; d < depth; d++) sb += L" ";
        sb += close;
    }

    static bool isCollection(const Object* o) {
        return instanceof(o, const LinkedList*) || instanceof(o, const ArrayList*) || 
               instanceof(o, const HashMap*) || instanceof(o, const HashSet*);
    }

    static int __cdecl compareObjects(const void *e1, const void *e2) {
        Object* o1 = *(Object**)e1;
        Object* o2 = *(Object**)e2;
        if (o1 == null) {
            return o2 == null ? 0 : -1;
        }
        else if (o2 == null) {
            return o1 == null ? 0 : +1;
        }
        else if (instanceof(o1, Comparable*)) {
            return dynamic_cast<Comparable*>(o1)->compareTo(o2);
        }
        else if (instanceof(o2, Comparable*)) {
            return -dynamic_cast<Comparable*>(o2)->compareTo(o1);
        }
        else if (o1->equals(o2) || o1->hashCode() == o2->hashCode()) {
            return 0;
        }
        else {
            return o1->hashCode() < o2->hashCode() ? -1 : +1;
        }
    }

    static void sort(Object** a, int n) {
        if (n > 1) qsort(a, n, sizeof(Object*), compareObjects);
    }

};

// -----------------------------------------------------------------------------------

struct Image extends Object {

    Image();
    Image(const uchar* file);
    Image(InputStream* s);
    Image(int w, int h, int bits);
    virtual ~Image();

    bool create(int w, int h, int bitcount);
    inline bool createRGBA(int w, int h) { return create(w, h, 32); }
    inline bool createBW(int w, int h) { return create(w, h, 1); }
    inline bool createRGB(int w, int h) { return create(w, h, 24); }
    Image* clone(const Rect& rc) const;
    Image* clone(const Rect* rc) const { return clone(*rc); }
    Object* clone() const;
    HDC getDC();
    void releaseDC();

    void alphaBlend(HDC hdc, int dx, int dy, int dw, int dh, int sx, int sy, int sw, int sh) const;
    void alphaBlend(Image& dst, int dx, int dy, int dw, int dh, int sx, int sy, int sw, int sh) const;
    void alphaBlend(Image* dst, int dx, int dy, int dw, int dh, int sx, int sy, int sw, int sh) const { 
        alphaBlend(*dst, dx, dy, dw, dh, sx, sy, sw, sh);
    }

    void bitBlt(HDC hdc, int dx, int dy, int dw, int dh, int sx, int sy, int sw, int sh, int op = SRCCOPY) const;
    void bitBlt(Image& dst, int dx, int dy, int dw, int dh, int sx, int sy, int sw, int sh, int op = SRCCOPY) const;
    void bitBlt(Image* dst, int dx, int dy, int dw, int dh, int sx, int sy, int sw, int sh, int op = SRCCOPY) const { 
        bitBlt(*dst, dx, dy, dw, dh, sx, sy, sw, sh, op);
    }
    
    bool load(const uchar* fname, int &w, int &h, bool headeronly = false);
    bool load(InputStream* stream, int &w, int &h, bool headeronly = false);
    bool save(const uchar* fname); // for .png only RGBA images
    inline const String* getError() const { return error; }
    inline const String* getFile() const { return file; }
    inline void setError(const uchar* s) { delete error; error = new String(s); }

    inline HBITMAP getBitmap() const { return bitmap; }
    inline int getWidth() const { return bi.bV4Width; }
    inline int getHeight() const { return bi.bV4Height; }

    inline byte* getLine(int y) const {
        return bits + (bi.bV4Height - 1 - y) * bytesPerLine;
    }

    inline Color getPixel(int x, int y) const {
        byte* line = getLine(y);
        return Color(line[2], line[1], line[0]); // BRG
    }
    
    inline operator BITMAPINFO*() { return (BITMAPINFO*)&bi; }
    inline operator BITMAPV4HEADER*() { return &bi; }

    inline HashMap& getProperties() { return properties; }
    inline const HashMap& getProperties() const { return properties; }

private: 
    bool loadjpg(bool headeronly, int  &w, int  &h, 
                      InputStream* input, OutputStream* output);
    bool loadpng(bool headeronly, int &w, int &h, 
                          InputStream* input, OutputStream* output);
    bool savepng(OutputStream* output);
    bool savejpg(OutputStream* output);

    HDC hdc;
    int getCount;
    int compression;
    int bytesPerLine;
    HashMap properties;
    String* file;
    String* error;
    byte* bits;
    HBITMAP bitmap;
    HBITMAP unselect;
    BITMAPV4HEADER bi;
};

// -----------------------------------------------------------------------------------

extern const uint INVOKE_LATER;
extern const uint BROADCAST;
extern const dword DISPATCH_THREAD_ID;

#define WM_SYSTIMER 0x0118 // selected text scroll http://support.microsoft.com/?id=108938

struct Message extends Object {

    public: const uchar*  id;
    public: const Object* param;

    Message(const uchar* i, const Object* p) : id(i), param(p) { }

    virtual ~Message() { delete param; }

    virtual void dispatch(); // see implementation below

    /* special care should be taken to call broadcast ONLY with 
       L"..." messages. Message itself and Object* param will be 
       forcefully deleted after dispatching */

    void post() {
        check(PostThreadMessage(DISPATCH_THREAD_ID, BROADCAST, null, (LPARAM)this));
    }

};

interface BroadcastListener {
    virtual void eventReceived(const Message &msg) = 0;
};

struct MessageQueue {

    static LinkedList listeners;
    static HACCEL accel;
    static bool shutdown;
    static int  shutdownDelay;

    static bool isDispatchThread() { return GetCurrentThreadId() == DISPATCH_THREAD_ID; }

    static void invokeLater(Runnable r, void* p) {
        check(PostThreadMessage(DISPATCH_THREAD_ID, INVOKE_LATER, (WPARAM)r, (LPARAM) p));
    }

    /* special care should be taken to call broadcast ONLY with 
       L"..." messages. Object* param will be forcefully deleted
       after dispatching */

    static void post(const uchar* id) {
        (new Message(id, null))->post(); // compiler bug () around new are needed
    }

    static void post(const uchar* id, const Object* param) {
        (new Message(id, param))->post(); // compiler bug () around new are needed
    }
    
    static void addBroadcastListener(BroadcastListener* iface) {
        listeners.add((int)iface);
    }

    static void removeBroadcastListener(BroadcastListener* iface) {
        listeners.remove((int)iface);
    }

    static void setAcceleratorTable(HACCEL ha) {
        accel = ha;
    }

    static HWND getParentDialog(HWND hwnd) {
        while (hwnd != null) {
            if (GetProp(hwnd, L"dialog") != null) return hwnd;
            hwnd = GetParent(hwnd);
        }
        return null;
    }

    static inline int messageLoop() {
        MSG msg = {0};
        while (GetMessage(&msg, null, 0, 0)) {
            // TODO: add __try _except and logging here
            HWND dialog = getParentDialog(msg.hwnd);
            if (dialog != null && IsDialogMessage(dialog, &msg)) {
                /* do nothing */
            }
            else {
                if (msg.hwnd != null && !TranslateAccelerator(msg.hwnd, accel, &msg)) {
                    TranslateMessage(&msg);
                }
                DispatchMessage(&msg);
            }
            // Make sure any pending messages get dispatched
            if (shutdown) {
                if (shutdownDelay > 0) {
                    int milliseconds = shutdownDelay;
                    shutdownDelay = 0;
                    post(L"shutdown");
                    Sleep(milliseconds); // let background threads work
                }
                else {
                    if (PeekMessage(&msg, null, 0, 0, PM_NOREMOVE) == 0)
                        PostQuitMessage(0);
                }
            }
        }
        assert(msg.message == WM_QUIT);
        return msg.wParam;
    }

    static void notifyAll(const Message& msg) {
        LinkedList* clone = dynamic_cast<LinkedList*>(listeners.clone());
        for (LinkedList::Iterator i(clone); i.hasNext(); ) {
            const Int* iface = dynamic_cast<const Int*>(i.next());
            ((BroadcastListener*)iface->intValue())->eventReceived(msg);
        }
        delete clone;
    }

    static void initiateShutdown() {
        shutdown = true;
        post(L"shutdown");
    }

    static void delayShutdown(int quantInMilliseconds) {
        shutdownDelay = max(shutdownDelay, max(quantInMilliseconds, 1));
    }
};

inline void Message::dispatch() {
    MessageQueue::notifyAll(*this);
    delete this; // will delete param
}

// -----------------------------------------------------------------------------------

struct Window  {

    interface Listener {
        virtual void eventReceived(HWND hwnd, int msg, int wp, int lp) = 0;
    };

    static HWND create(HWND parent, int styleEx, int style, WNDPROC, HBRUSH, void* that);

    static void notifyListeners(HWND hwnd, UINT Msg, WPARAM wParam, LPARAM lParam);
    static void notifyDestroy(HWND hwnd);

    static void addListener(HWND hwnd, Window::Listener* sink);
    static void addListener(HWND hwnd, Window::Listener& sink) { addListener(hwnd, &sink); }
    static void removeListener(HWND hwnd, Window::Listener* sink);

    static Rect getBounds(HWND hwnd); // return bounding rectangle inside parent

    static const Object* get(HWND hwnd, const uchar* id);

    static int getInt(HWND hwnd, const uchar* id) {
        return (int)dynamic_cast<const Int&>(*get(hwnd, id)).intValue();
    }

    static int getInt(HWND hwnd, const uchar* id, int defvalue) {
        const Int* val = dynamic_cast<const Int*>(get(hwnd, id));
        return val != null ? val->intValue() : defvalue;
    }


    static const uchar* getStr(HWND hwnd, const uchar* id) {
        const Object* s = get(hwnd, id);
        return s == null ? null : (const uchar*)dynamic_cast<const String&>(*s);
    }
    static void put(HWND hwnd, const uchar* id, const Object& o) { put(hwnd, id, &o); }
    static void put(HWND hwnd, const uchar* id, const Object* o);
    static void put(HWND hwnd, const uchar* id, const uchar* s);
    static void put(HWND hwnd, const uchar* id, int v);

    static String getText(HWND hwnd); /* returns arbitrary length text */
};

// -----------------------------------------------------------------------------------

class ObjectPipe {
    HANDLE pipe;
    OVERLAPPED overlapped_read;
    OVERLAPPED overlapped_write;
    char buf[0x1000];
    int pos;
    int len;
    BOOL eof;
    BOOL readBuffer();
    uchar readChar();
public:
    ObjectPipe(HANDLE pipe);
    String* readString();
    Object* readObject();
    void writeStr(const String* obj);
    void writeObject(const Object* obj);
    void error();
    ~ObjectPipe();
};

// -----------------------------------------------------------------------------------

/* Ole DoDragDrop "swallows" messages. After extensive study of it
   the simplest solution is just to implement and use its' equvivalent
   that does not. At least for internal purposes.
   DropTargets that expect out-of-the application drop should still
   register with OLE32
*/

struct DnD {

static void init();

static HRESULT registerDragDrop(HWND hwnd, LPDROPTARGET pDropTarget);
static HRESULT revokeDragDrop(HWND hwnd);
static HRESULT doDragDrop(IDataObject *pDataObject, IDropSource* pDropSource,
                          dword dwOKEffect, dword *pdwEffect);
static void fini();

};

// -----------------------------------------------------------------------------------

class ApplicationTester implements BroadcastListener {
    HANDLE pipe;
    HANDLE pipe_reader;
    ObjectPipe object_pipe;
    HashMap str_map;
    Tester(HANDLE pipe);
    void eventReceived(const Message& message);
    static dword WINAPI pipeReaderThread(void* arg);
    ApplicationTester(HANDLE h);
public:
    static void init();
    static void fini();
};

// -----------------------------------------------------------------------------------

struct Resources {

    static String* getString(int id) {
        ByteArray* data = getData(id);
        String* s = String::fromUTF8((const char*)(byte*)*data);
        delete data;
        return s;
    }

    static Image* getImage(int id) {
        ByteArray* data = getData(id);
        ByteArrayInputStream s(data);
        Image* i = new Image(&s);
        delete data;
        return i;
    }

    static ByteArray* getData(int id) {
        HINSTANCE instance = GetModuleHandle(null);
        HRSRC h = FindResource(instance, MAKEINTRESOURCE(id), RT_RCDATA);
        HGLOBAL g = LoadResource(instance, h);
        int size = (int)SizeofResource(instance, h);
        byte* rc = (byte*)LockResource(g);
        return new ByteArray(rc, size);
    }
};

// -----------------------------------------------------------------------------------

struct HourglassCursor {

    HourglassCursor() {
        save = SetCursor(LoadCursor(null, IDC_WAIT));
    }

    virtual ~HourglassCursor() {
        SetCursor(save);
        save = null;
    }

    private: HCURSOR save;
};

// -----------------------------------------------------------------------------------

// These functions must be implemented by the application:

HWND getApplicationMainWindow();
int winMain(HINSTANCE instance, int argc, uchar* argv[], uchar* env, int show);
