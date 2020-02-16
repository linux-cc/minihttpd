#include "memory/slab_malloc.h"
#include "memory/buddy_malloc.h"
#include <vector>
#include <stdio.h>

using namespace memory;

int main(int argc, char *argv[]) {
    using std::vector;
    vector<void*> v;
    BuddyMalloc buddy(8, 4096);
    SlabMalloc slab(buddy);
    for (;;) {
        int cmd, size;
        printf("enter command(1/0 size):");
        scanf("%d %d", &cmd, &size);
        if (cmd == 1) {
            void *p = slab.alloc(size);
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
            slab.free(p);
            v.erase(it + size);
            printf("free : %p\n", p);
        }
        printf("alloc list: ");
        for (size_t i = 0; i < v.size(); ++i)
            printf("%p, ", v[i]);
        printf("\nbuddy dump: %s\n", buddy.dump());
        printf("slab dump:\n%s\n", slab.dump());
    }
    return 0;
}

