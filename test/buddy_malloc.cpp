#include "memory/buddy_malloc.h"
#include <cstdio>
#include <cstring>
#include <vector>

using namespace memory;
using std::vector;

int main(int argc, char *argv[]) {
    vector<void*> v;
    BuddyMalloc *b = new BuddyMalloc(32, 1);
    char *buf = b->dump();
    printf("\ndump: %s\n", buf);
    delete []buf;
    printf("buffer: %p\n", b->buffer());
    for (;;) {
        int cmd, size;
        printf("enter command(1/0 size):");
        scanf("%d %d", &cmd, &size);
        if (cmd == 1) {
            void *p = b->alloc(size);
            if (p) {
                v.push_back(p);
                printf("alloc: %p, %d\n", p, size);
            }
        }
        if (cmd == 0) {
            if (v.empty() || size >= (int)v.size())
                continue;
            vector<void*>::iterator it = v.begin();
            void *p = *(it + size);
            b->free(p);
            v.erase(it + size);
            printf("free : %p\n", p);
        }
        printf("alloc list: ");
        for (size_t i = 0; i < v.size(); ++i)
            printf("%p, ", v[i]);
        buf = b->dump();
        printf("\ndump: %s\n", buf);
        delete []buf;
    }
    return 0;
}

