#include "memory/todo/dl_malloc.h"
#include "memory/buddy.h"
#include <vector>
#include <stdio.h>

#define TREEBIN_SHIFT       8
#define SIZE_T_SIZE         (sizeof(size_t))
#define SIZE_T_ONE          ((size_t)1)
#define CHUNK_OVERHEAD      (SIZE_T_SIZE)
#define MALLOC_ALIGNMENT    ((size_t)(2 * sizeof(void *)))
#define CHUNK_ALIGN_MASK    (MALLOC_ALIGNMENT - SIZE_T_ONE)
#define pad_request(req)    (((req) + CHUNK_OVERHEAD + CHUNK_ALIGN_MASK) & ~CHUNK_ALIGN_MASK)

size_t computeFreeIndex(size_t nb) {
    uint32_t x = nb >> TREEBIN_SHIFT;
    if (x == 0)
        return 0;
    uint32_t k = sizeof(x) * __CHAR_BIT__ - 1 - __builtin_clz(x);
    return (k << 1) + (nb >> (k + TREEBIN_SHIFT - 1) & 1);
}

using namespace memory;

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("usage %s length\n", argv[0]);
        return -1;
    }
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

