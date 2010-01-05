#include "tiny.h"

static void test_map(void) {
    HashMap* m = new HashMap();
    Object* keys[] = {
        new HashMap(),
        new ArrayList(),
        new LinkedList(),
        new String(),
        new String(L"x"),
        new String(L"y"),
        new StringBuffer(),
        new StringBuffer(L"x"),
        new StringBuffer(L"y"),
        new File(L"foo.bar"),
        new HashSet(),
        new Double(153.),
        new Double(334.),
        new Int(153),
        new Int(334),
        new Int(335),
        new Int(336),
        new Int(337),
        new Int(338),
        new Long(153),
        new Long(334),
        new Long(335),
        new Long(336),
        new Long(337),
        new Long(338),
        new Color(1,2,3),
        new Point(123, 321),
        new Rect(1,2,3,4)
    };
    int i, j;
    for (i = 0; i < countof(keys); i++) {
        for (j = 0; j < countof(keys); j++) {
            assert(keys[i]->equals(keys[j]) == (i == j));
        }
    }
    for (i = 0; i < countof(keys); i++) {
        assert(m->size() == i);
        const Long* l = new Long(i);
        assert(l->intValue() == i);
        m->put(keys[i], l);
        assert(m->get(keys[i]) != null);
        delete l;
    }
    for (i = 0; i < countof(keys); i++) {
        m->put(keys[i], m->get(keys[i]));
        const Long* l = dynamic_cast<const Long*>(m->get(keys[i]));
        assert(l != null);
        m->put(keys[i], l);
    }
    for (i = 0; i < countof(keys); i++) {
        Long * l = new Long(i);
        assert(m->get(keys[i]) != null);
        assert(m->get(keys[i])->equals(l));
        delete l;
    }
    for (i = 0; i < countof(keys); i++) {
        Long l(i * 2);
        m->put(keys[i], &l);
        assert(m->get(keys[i]) != null);
        assert(l.equals(m->get(keys[i])));
    }
    for (i = 0; i < countof(keys); i++) {
        assert(m->size() == (int)countof(keys) - i);
        assert(m->remove(keys[i]));
        assert(m->size() == (int)countof(keys) - i - 1);
        for (j = i + 1; j < countof(keys); j++) {
            assert(m->get(keys[j]) != null);
            assert(dynamic_cast<const Long*>(m->get(keys[j])) != null);
            assert((dynamic_cast<const Long*>(m->get(keys[j])))->intValue() == j * 2);
        }
    }
    assert(m->size() == 0);
    Long l(334);
    m->put(L"test", &l);
    assert(dynamic_cast<const Long*>(m->get(L"test")) != null);
    assert(l.equals(dynamic_cast<const Long*>(m->get(L"test"))));
    m->clear();
    assert(m->size() == 0);
    for (i = 0; i < countof(keys); i++) {
        delete keys[i];
        keys[i] = null;
    }
    delete m;
}

static void test_array(void) {
    ArrayList a;
    Object* objs[] = {
        new HashMap(),
        new ArrayList(),
        new LinkedList(),
        new String(),
        new String(L"x"),
        new String(L"y"),
        new StringBuffer(),
        new StringBuffer(L"x"),
        new StringBuffer(L"y"),
        new File(L"foo.bar"),
        new HashSet(),
        new Double(153.),
        new Double(334.),
        new Int(153),
        new Int(334),
        new Int(335),
        new Int(336),
        new Int(337),
        new Int(338),
        new Long(153),
        new Long(334),
        new Long(335),
        new Long(336),
        new Long(337),
        new Long(338),
        new Color(1,2,3),
        new Point(123, 321),
        new Rect(1,2,3,4)
    };
    int i, j;
    for (i = 0; i < countof(objs); i++) {
        Object* o = objs[i];
        a.add(o);
        assert(a.get(i) != o);
        assert(a.get(i)->equals(o));
        assert(a.size() == i + 1);
    }
    ArrayList * b = dynamic_cast<ArrayList *>(a.clone());
    assert(b->equals(&a));
    assert(a.equals(b));
    for (i = 0; i < countof(objs); i++) {
        for (j = 0; j < countof(objs); j++) {
            assert(objs[i]->equals(objs[j]) == (i == j));
            assert(objs[i]->equals(a.get(j)) == (i == j));
            assert(objs[i]->equals(b->get(j)) == (i == j));
        }
    }
    assert(a.size() == (int)countof(objs));
    for (i = 0; i < countof(objs); i++) {
        assert(a.get(0)->equals(objs[i]));
        a.removeAt(0);
        assert(a.size() == (int)countof(objs) - i - 1);
        assert(!b->equals(&a));
        assert(!a.equals(b));
    }
    for (i = 0; i < countof(objs); i++) {
        Object* o = objs[countof(objs) - i - 1];
        a.add(0, o);
        assert(a.get(0) != o);
        assert(a.get(0)->equals(o));
        a.set(0, o);
        assert(a.get(0) != o);
        assert(a.get(0)->equals(o));
        assert(a.size() == i + 1);
    }
    assert(b->equals(&a));
    assert(a.equals(b));
    for (i = 0; i < countof(objs); i++) {
        delete objs[i];
        objs[i] = null;
    }
    delete b;
}
    
void test_XMLEncodeDecode() {
    Object* nill = null;
    String* s = XMLEncoder::encode(nill);
    assert(XMLDecoder::decode(s) == null);
    delete s;  s = null;
    Object* objects[] = {
        new HashSet(),      // 0  do not move around
        new HashMap(),      // 1  -"-
        new ArrayList(),    // 2
        new LinkedList(),     // 3
        new String(),
        new String(L"x"),
        new StringBuffer(),
        new StringBuffer(L"x"),
        new Double(153.),
        new Double(334.),
        new Int(153),
        new Int(334),
        new Long(153),
        new Long(334),
        new Color(1,2,3),
        new Point(123, 321),
        new Rect(1,2,3,4)
    };
    int i;
    for (i = 0; i < countof(objects); i++) {
        s = XMLEncoder::encode(objects[i]);
        Object* o = XMLDecoder::decode(s);
        assert(o->equals(objects[i]));
        delete s;  s = null;
        delete o;  o = null;
    }

    dynamic_cast<LinkedList*>(objects[3])->add(111);
    dynamic_cast<LinkedList*>(objects[3])->add(L" aaa bbb ");
    dynamic_cast<LinkedList*>(objects[3])->add(L" 'aaa <\"> bbb& ");
    dynamic_cast<LinkedList*>(objects[3])->add(111.234);

    dynamic_cast<ArrayList*>(objects[2])->add(L"def");
    dynamic_cast<ArrayList*>(objects[2])->add(objects[3]);
    dynamic_cast<ArrayList*>(objects[2])->add(567);
    dynamic_cast<HashMap*>(objects[1])->put(L"abc", objects[2]);
    dynamic_cast<HashMap*>(objects[1])->put(123, objects[2]);
    dynamic_cast<HashSet*>(objects[0])->add(objects[1]);


    for (i = 0; i < countof(objects); i++) {
        s = XMLEncoder::encode(objects[i]);
//      traceln(s);
        Object* o = XMLDecoder::decode(s);
        assert(o->equals(objects[i]));
        delete s;  s = null;
        delete o;  o = null;
    }

    for (i = 0; i < countof(objects); i++) {
        delete objects[i];
    }
}

static void test_Image() {
    {
        Image image;
        assert(image.createRGBA(256, 256));
    }
    for (int headeronly = 0; headeronly <= 1; headeronly++) {
        int i = 0;
        uchar* jfile[] = { L"materials\\testprog.jpg", L"materials\\testimg.jpg", 
                           L"materials\\testimgp.jpg", L"materials\\testorig.jpg", 
                           L"materials\\corrupted.jpg" };
        for (i = 0; i < countof(jfile); i++) {
            Image image;
            int w = 0;
            int h = 0;
            assert((i < countof(jfile) - 1) == image.load(jfile[i], w, h, headeronly != 0));
            if (i == countof(jfile) - 1) {
                assert(image.getError()->compareTo(L"Maximum supported image dimension is 65500 pixels") == 0);
            }
        }
        // TODO: http://entropymine.com/jason/testbed/pngtrans/
        uchar* pfile[] = { L"materials\\testprog.png", L"materials\\testorig.png", L"materials\\testorig16.png", 
                           L"materials\\testorig8.png", L"materials\\testorig4.png", L"materials\\corrupted.png" };
        for (i = 0; i < countof(pfile); i++) {
            Image image;
            int w = 0;
            int h = 0;
            assert((i < countof(pfile) - 1) == image.load(pfile[i], w, h, headeronly != 0));
            if (i == countof(pfile) - 1) {
                assert(image.getError()->compareTo(L"IHDR: CRC error") == 0);
            }
        }
    }
}

extern void test_app(int argc, char ** argv);

int main(int argc, char ** argv) {
#ifdef _DEBUG
    /* will call _CrtDumpMemoryLeaks() after global dtors */
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF|_CRTDBG_LEAK_CHECK_DF);
#endif
    // HashSet working directory
    uchar fnm[1024];
    if (!GetModuleFileName(null, fnm, countof(fnm))) {
        traceError();
        return 1;
    }
    int i = ustrlen(fnm);
    while (i > 0 && fnm[i - 1] != '/' && fnm[i - 1] != '\\') i--;
    ustrcpy(fnm + i, sizeof(fnm), L"..\\src\\tests");
    if (!SetCurrentDirectory(fnm)) {
        traceError();
        return 1;
    }
    // Run tests
    test_map();
    test_array();
    test_XMLEncodeDecode();
    test_Image();
    test_app(argc, argv);
    return 0;
}
