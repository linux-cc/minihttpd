#include "closure/bind.h"
#include "util/scoped_ptr.h"
#include <stdio.h>
#include <string>

void test(char a1, short a2, int a3, long a4, const char *a5, const std::string &a6, int a7) {
    printf("%s void(char:%d, short:%d, int:%d, long:%ld, char*:%s, string:%s, int:%d)\n",
        __FUNCTION__, a1, a2, a3, a4, a5, a6.c_str(), a7);
}

struct Test {
    void test(char a1, short a2, int a3, long a4, const char *a5, const std::string &a6) {
        printf("%s void(char:%d, short:%d, int:%d, long:%ld, char*:%s, string:%s)\n",
                __FUNCTION__, a1, a2, a3, a4, a5, a6.c_str());
    }
    ~Test() {
        printf("~Test\n");
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
    Test *t = new Test;
    closure::Callback<void()> cb0 = closure::bind(&Test::test, t, 1, 2, 3, 4, "5hello", "6hello");
    cb0.run();
    closure::Callback<void(const std::string&)> cb1 = closure::bind(&Test::test, t, 1, 2, 3, 4, "5hello");
    cb1.run("6hello");
    closure::Callback<void(const char*, const std::string&)> cb2 = closure::bind(&Test::test, t, 1, 2, 3, 4);
    cb2.run("5hello", "6hello");
    closure::Callback<void(long, const char*, const std::string&)> cb3 = closure::bind(&Test::test, t, 1, 2, 3);
    cb3.run(4, "5hello", "6hello");
    closure::Callback<void(int, long, const char*, const std::string&)> cb4 = closure::bind(&Test::test, t, 1, 2);
    cb4.run(3, 4, "5hello", "6hello");
    closure::Callback<void(short, int, long, const char*, const std::string&)> cb5 = closure::bind(&Test::test, t, 1);
    cb5.run(2, 3, 4, "5hello", "6hello");
    closure::Callback<void(char, short, int, long, const char*, const std::string&)> cb6 = closure::bind(&Test::test, t);
    cb6.run(1, 2, 3, 4, "5hello", "6hello");
}

int main() {
    test1();
    test2();
    util::ScopedPtr<Test> ptr1(new Test);
    util::ScopedPtr<Test> ptr2 = ptr1.pass();
    util::ScopedPtr<Test> ptr3;
    ptr3 = ptr2.pass();

    return 0;
}
