#include "memory/slab_malloc.h"
#include <vector>

USING_NS(memory);

int main(int argc, char *argv[]) {
    using std::vector;
    vector<void*> v;
    Buddy buddy(8);
    SlabMalloc slab(&buddy);
    for (;;) {
        int cmd, size;
        printf("enter command:");
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
        for (int i = 0; i < v.size(); ++i)
            printf("%p, ", v[i]);
        printf("\nbuddy dump: %s\n", buddy.dump());
        printf("slab dump:\n%s\n", slab.dump());
    }
    return 0;
}

