#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <vector>
#include "dl_malloc.cc"

int main() {
    std::vector<void*> v;
    for (;;) {
        int cmd, size;
        printf("enter command:");
        scanf("%d %d", &cmd, &size);
        if (cmd == 1) {
            void *p = dlmalloc(size);
            if (p) {
                v.push_back(p);
                printf("alloc: %p, %d\n", p, size);
            }
        }
        if (cmd == 0) {
            if (v.empty())
                continue;
            std::vector<void*>::iterator it = v.begin();
            void *p = *(it + size);
            dlfree(p);
            v.erase(it + size);
            printf("free : %p\n", p);
        }
        printf("alloc list: ");
        for (size_t i = 0; i < v.size(); ++i)
            printf("%p, ", v[i]);
        printf("\n");
    }

    return 0;
}
