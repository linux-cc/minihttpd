#include "util/string.h"
#include "util/buffer_queue.h"
#include "memory/simple_alloc.h"
#include "httpd/server.h"
#include <stdio.h>
#include <string.h>

//#define P_STR(desc, str)    printf("%s: {%d|%d|%p|%s}\n", desc, str.refCount(), str.length(), str.data(), str.data())
#define P_STR(fmt, str, ...)    printf(fmt ": {%d|%lu|%p|%s}\n", ##__VA_ARGS__, str.refCount(), str.length(), str.data(), str.data())

void testString() {
    const char *data = "hello world";
    int length = (int)strlen(data);
    util::String str1 = data;
    str1.erase(0, 0);
    P_STR("str1 = data", str1);
    util::String str2 = str1;
    P_STR("str2 = str1", str2);
    P_STR("str1 = data", str1);
    util::String str3 = str2;
    P_STR("str3 = str2", str3);
    P_STR("str1 = data", str1);
    P_STR("str2 = str1", str2);
    str2[0] = str1[1];
    printf("after str2[0] = str1[1]\n");
    P_STR("str1 = data", str1);
    P_STR("str2 = str1", str2);
    P_STR("str3 = str2", str3);
    util::String str4(data, 5);
    P_STR("str4(data, 5)", str4);
    for (int i = 0; i < length; i++) {
        util::String str5(str1, i, util::String::npos);
        P_STR("str5(str1, %d, %ld)", str5, i, util::String::npos);
        util::String str6(str1, i, length - i - 1);
        P_STR("str6(str1, %d, %d)", str6, i, length - i - 1);
    }
    util::String str7;
    str7.append(str1).append(data).append(data[0]);
    P_STR("str7.append(%s).append(%s).append(%c)", str7, str1.data(), data, data[0]);
    for (int i = 0; i < str7.length(); i++) {
        util::String str8 = str7;
        P_STR("str8 = str7", str7);
        util::String str9 = str7;
        P_STR("str9 = str8", str7);
        str8.erase(i, i);
        P_STR("str8.erase(%d, %d)", str8, i, i);
        P_STR("str7", str7);
        str9.erase(i, str7.length() - i);
        P_STR("str9.erase(%d, %lu)", str9, i, str7.length() - i);
        P_STR("str7", str7);
    }
    for (int i = 0; i < length; i++) {
        util::String str10 = str1;
        str10.replace(i, length - i - 1, str1);
        P_STR("str10.replace(%d, %d, %s)", str10, i, length - i - 1, str1.data());
        util::String str11 = str1;
        str11.replace(i, util::String::npos, str1);
        P_STR("str11.replace(%d, %ld, %s)", str11, i, util::String::npos, str1.data());
    }
    for (int i = 0; i < str7.length(); i++) {
        printf("str7(%s).find(str1(%s), %d): %ld\n", str7.data(), str1.data(), i, str7.find(str1, i));
        printf("str7(%s).find(%s, %d, %ld): %ld\n", str7.data(), str1.data(), i, str7.length() - i, str7.find(str1.data(), i, str7.length() - i));
        printf("str7(%s).find(hello, %d): %ld\n", str7.data(), i, str7.find("hello", i));
        printf("str7(%s).find(l, %d): %ld\n", str7.data(), i, str7.find('l', i));
        printf("str7(%s).substr(%d, %d): %s\n", str7.data(), i, i + 1, str7.substr(i, i + 1).data());
    }
}

void testBufferQueue() {
    util::BufferQueue bq(128);
    char buf[32];
    for (int i = 0; i < 10; ++i) {
        int n = sprintf(buf, "hello,world[%02d],", i);
        bool ret = bq.enqueue(buf, n);
        printf("enqueue ret: %d\n", ret);
    }
    int length = 0;
    for (int i = 0; i < 5; i++) {
        length += (i + 1) * 5;
        bool ret = bq.dequeue(buf, (i + 1) * 5);
        printf("dequeue ret: %d\n", ret);
    }
    for (int i = 10; i < 13; ++i) {
        int n = sprintf(buf, "hello,world[%02d],", i);
        bool ret = bq.enqueue(buf, n);
        printf("enqueue ret: %d\n", ret);
    }
    util::String sbuf;
    bool ret = bq.dequeueUntil(sbuf, "world[10],hello", false);
    if (ret) {
        printf("dequeueUntil: %s\n", sbuf.data());
    }
}

#ifdef __TEST__
int main(int argc, char *argv[]) {
    testString();
    testBufferQueue();
    
    return 0;
}
#endif
