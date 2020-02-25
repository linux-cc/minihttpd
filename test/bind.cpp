#include "closure/bind.h"
#include "util/scoped_ptr.h"
#include "util/scoped_ref.h"
#include "util/weak_ptr.h"
#include <stdio.h>
#include <string>

void test(char a1, short a2, int a3, long a4, const char *a5, const std::string &a6, int a7) {
    printf("%s void(char:%d, short:%d, int:%d, long:%ld, char*:%s, string:%s, int:%d)\n",
        __FUNCTION__, a1, a2, a3, a4, a5, a6.c_str(), a7);
}

struct Base {
    void test(char a1, short a2, int a3, long a4, const char *a5, const std::string &a6) {
        printf("%s void(char:%d, short:%d, int:%d, long:%ld, char*:%s, string:%s)\n",
                __FUNCTION__, a1, a2, a3, a4, a5, a6.c_str());
    }
    virtual ~Base() {
        printf("~Base\n");
    }
};

struct Derive: public Base {
    ~Derive () {
        printf("~Derive\n");
    }
};

struct Other {};

struct RBase : public util::RefCounted<RBase> {
    virtual ~RBase() {
        printf("~RBase\n");
    }
};

struct RDerive: public RBase {
    ~RDerive () {
        printf("~RDerive\n");
    }
};
void test1() {
    closure::Callback<void()> cb0 = closure::bind(test, 1, 2, 3, 4, "5hello", "6hello", 7);
    cb0.run();
    closure::Callback<void(int)> cb1 = closure::bind(test, 1, 2, 3, 4, "5hello", "6hello");
    cb1.run(7);
    closure::Callback<void(const std::string&,int)> cb2 = closure::bind(test, 1, 2, 3, 4, "5hello");
    cb2.run("6hello", 7);
    closure::Callback<void(const char*, const std::string&, int)> cb3 = closure::bind(test, 1, 2, 3, 4);
    cb3.run("5hello", "6hello", 7);
    closure::Callback<void(long, const char*, const std::string&, int)> cb4 = closure::bind(test, 1, 2, 3);
    cb4.run(4, "5hello", "6hello", 7);
    closure::Callback<void(int, long, const char*, const std::string&, int)> cb5 = closure::bind(test, 1, 2);
    cb5.run(3, 4, "5hello", "6hello", 7);
    closure::Callback<void(short, int, long, const char*, const std::string&, int)> cb6 = closure::bind(test, 1);
    cb6.run(2, 3, 4, "5hello", "6hello", 7);
    closure::Callback<void(char, short, int, long, const char*, const std::string&, int)> cb7 = closure::bind(test);
    cb7.run(1, 2, 3, 4, "5hello", "6hello", 7);
}

void test2() {
    Base *t = new Base;
    closure::Callback<void()> cb0 = closure::bind(&Base::test, t, 1, 2, 3, 4, "5hello", "6hello");
    cb0.run();
    closure::Callback<void(const std::string&)> cb1 = closure::bind(&Base::test, t, 1, 2, 3, 4, "5hello");
    cb1.run("6hello");
    closure::Callback<void(const char*, const std::string&)> cb2 = closure::bind(&Base::test, t, 1, 2, 3, 4);
    cb2.run("5hello", "6hello");
    closure::Callback<void(long, const char*, const std::string&)> cb3 = closure::bind(&Base::test, t, 1, 2, 3);
    cb3.run(4, "5hello", "6hello");
    closure::Callback<void(int, long, const char*, const std::string&)> cb4 = closure::bind(&Base::test, t, 1, 2);
    cb4.run(3, 4, "5hello", "6hello");
    closure::Callback<void(short, int, long, const char*, const std::string&)> cb5 = closure::bind(&Base::test, t, 1);
    cb5.run(2, 3, 4, "5hello", "6hello");
    closure::Callback<void(char, short, int, long, const char*, const std::string&)> cb6 = closure::bind(&Base::test, t);
    cb6.run(1, 2, 3, 4, "5hello", "6hello");
}

void testRefptr() {
    printf("testRefptr start\n");
    util::ScopedRef<RBase> r1 = new RBase;
    util::ScopedRef<RBase> r2 = new RDerive;
    util::ScopedRef<RBase> r3 = r1;
    util::ScopedRef<RBase> r4 = r2;

    r1 = new RBase;
    r2 = new RDerive;
    r3 = r1;
    r4 = r2;
    printf("testRefptr end\n");
}

void testPtr() {
    printf("testPtr start\n");
    util::ScopedPtr<Base> p1(new Base);
    util::ScopedPtr<Base> p2(new Derive);
    util::ScopedPtr<Derive> p3(new Derive);
    printf("=====p1: %p=====\n", &p1);
    util::ScopedPtr<Base> p4 = p1.pass();
    printf("=====2=====\n");
    util::ScopedPtr<Base> p5 = p3.passAs<Base>();
    printf("=====3=====\n");
    util::ScopedPtr<Base> p6(p4.pass());
    printf("=====4=====\n");
    p4 = p2.pass();
    printf("testPtr p4 assign\n");
    p5 = p3.pass();
    printf("testPtr p5 assign\n");
    printf("testPtr end\n");
}

class WeakTest : public util::SupportsWeakPtr<WeakTest> {
public:
    void test() {
        printf("hello\n");
    }
};

void testWeakPtr() {
    WeakTest *pw = new WeakTest;
    util::WeakPtr<WeakTest> wp = pw->asWeakPtr();
    if (wp) {
        wp->test();
    }
    delete pw;
    if (wp) {
        wp->test();
    }
}

int testBind() {
    test1();
    test2();
    testRefptr();
    testPtr();
    printf("========\n");
    testWeakPtr();

    return 0;
}
