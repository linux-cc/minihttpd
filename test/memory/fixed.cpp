#include "memory/fixed_malloc.h"
#include "memory/buddy.h"
#include <vector>
#include <stdio.h>

USING_NS(memory);

int main(int argc, char *argv[]) {
    using std::vector;
    vector<void*> v;
    Buddy buddy(8);
    FixedMalloc fixed(buddy);
    fixed.init(16, 2048);
    for (;;) {
        int cmd, size;
        printf("enter command:");
        scanf("%d %d", &cmd, &size);
        if (cmd == 1) {
            void *p = fixed.alloc(size);
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
            fixed.free(p);
            v.erase(it + size);
            printf("free : %p\n", p);
        }
        printf("alloc list: ");
        for (size_t i = 0; i < v.size(); ++i)
            printf("%p, ", v[i]);
        printf("\nbuddy dump: %s\n", buddy.dump());
        //printf("slab dump:\n%s\n", slab.dump());
    }
    return 0;
}

