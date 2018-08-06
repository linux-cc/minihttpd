#include "memory/dl_malloc.h"
#include <vector>
#include <stdio.h>

USING_NS(memory);

int main(int argc, char *argv[]) {
    using std::vector;
    vector<void*> v;
    Buddy buddy(32);
    DlMalloc dl(buddy);
    for (;;) {
        int cmd, size;
        printf("enter command:");
        scanf("%d %d", &cmd, &size);
        if (cmd == 1) {
            void *p = dl.alloc(size);
            if (p) {
                v.push_back(p);
                printf("alloc: %p, %d\n", p, size);
            }
        }
        if (cmd == 0) {
            if (v.empty())
                continue;
            vector<void*>::iterator it = v.begin();
            void *p = *(it + size);
            dl.free(p);
            v.erase(it + size);
            printf("free : %p\n", p);
        }
        printf("alloc list: ");
        for (size_t i = 0; i < v.size(); ++i)
            printf("%p, ", v[i]);
        printf("\nbuddy dump: %p: %s\n", buddy.buffer(), buddy.dump());
        printf("dl dump:\n%s\n", dl.dump());
    }
    return 0;
}

