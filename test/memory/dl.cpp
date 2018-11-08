#include "memory/dl_malloc.h"
#include "memory/buddy.h"
#include <vector>
#include <stdio.h>
#include <unistd.h>

#define TREEBIN_SHIFT       8
#define SIZE_T_SIZE         (sizeof(size_t))
#define SIZE_T_ONE          ((size_t)1)
#define CHUNK_OVERHEAD      (SIZE_T_SIZE)
#define MALLOC_ALIGNMENT    ((size_t)(2 * sizeof(void *)))
#define CHUNK_ALIGN_MASK    (MALLOC_ALIGNMENT - SIZE_T_ONE)
#define pad_request(req)    (((req) + CHUNK_OVERHEAD + CHUNK_ALIGN_MASK) & ~CHUNK_ALIGN_MASK)

using namespace memory;

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
        printf("dlmalloc dump:\n%s\n", dl.dump());
    }
    return 0;
}

