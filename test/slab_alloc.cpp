#include "memory/slab_alloc.h"
#include "memory/buddy_alloc.h"
#include <vector>
#include <stdio.h>

using namespace memory;
using namespace std;

int main(int argc, char *argv[]) {
    vector<std::pair<void*, int> > v;
    BuddyAlloc buddy(8, 4096);
    SlabAlloc slab(buddy);
    for (;;) {
        int cmd, size;
        printf("enter command(1/0 size):");
        scanf("%d %d", &cmd, &size);
        if (cmd == 1) {
            void *p = slab.alloc(size);
            if (p) {
                v.push_back(make_pair(p, size));
                printf("alloc: %p[%d]\n", p, size);
            }
        }
        if (cmd == 0) {
            if (v.empty())
                continue;
            vector<pair<void*, int> >::iterator it = v.begin();
            pair<void*, int> p = *(it + size);
            slab.free(p.first, p.second);
            v.erase(it + size);
            printf("free : %p[%d]\n", p.first, p.second);
        }
        printf("alloc list: ");
        for (size_t i = 0; i < v.size(); ++i)
            printf("%p[%d], ", v[i].first, v[i].second);
        printf("\nbuddy dump: %s\n", buddy.dump());
        printf("slab dump:\n%s\n", slab.dump());
    }
    return 0;
}

