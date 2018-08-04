#include "memory/buddy.h"

#define LCHILD(i)		(((i) << 1) + 1)
#define PARENT(i)		((i - 1) >> 1)
#define PAGES(n)        (1 << (n - 1))

BEGIN_NS(memory)

Buddy::Buddy(size_t pages):
buf(NULL),
tree(NULL),
depth(0) {
    init(pages);
}

Buddy::~Buddy() {
    delete[] buf;
    delete[] tree;
    buf = NULL;
    tree = NULL;
}

void Buddy::init(size_t pages) {
    if (buf) {
        return;
    }
    depth = LOG2PLUS1(pages);
    pages = 1 << depth;
    size_t nodes = (pages << 1) - 1;
    pageAddr = buf = new char[(pages + 1) * PAGE_SIZE];
    intptr_t addr = ALIGN_PAGE((intptr_t)pageAddr);
    if (addr != (intptr_t)pageAddr) {
        pageAddr = (char*)addr;
    }
    tree = new uint8_t[nodes];
    tree[0] = depth;
    for (size_t i = 1; i < nodes; ++i) {
        tree[i] = tree[PARENT(i)] - 1;
    }
}

void *Buddy::alloc(size_t pages) {
    uint8_t n1 = LOG2PLUS1(pages);
    size_t idx = 0;
	if (pages == 0 || tree[idx] < n1) {
	    return NULL;
	}
    uint8_t n2 = depth + 1;
    while (n1 != --n2) {
        idx = LCHILD(idx);
        if (tree[idx] < n1) {
            ++idx;
        }
	}
    tree[idx] = 0;
	size_t offset = (idx + 1) * PAGES(n2) - PAGES(depth);
    while (idx) {
        idx = PARENT(idx);
        size_t child = LCHILD(idx);
        tree[idx] = MAX(tree[child], tree[child + 1]);
    }

    return pageAddr + offset * PAGE_SIZE;
}

void Buddy::free(void *addr) {
    char *paddr = (char*)addr;
    size_t pages = PAGES(depth);
    char *end = pageAddr + pages * PAGE_SIZE;
    if (paddr < pageAddr || paddr > end) {
        return;
    }

    size_t offset = (paddr - pageAddr) / PAGE_SIZE;
    size_t idx = offset + pages - 1;
    uint8_t n = 1;
    for (; tree[idx]; idx = PARENT(idx)) {
        ++n;
        if (idx == 0)
            return;
    }
    tree[idx] = n;

    while(idx) {
        idx = PARENT(idx);
        ++n;
        uint8_t left = tree[LCHILD(idx)];
        uint8_t right = tree[LCHILD(idx) + 1];
        if (left == n - 1 && right == n - 1)
            tree[idx] = n;
        else
            tree[idx] = MAX(left, right);
    }
}

char *Buddy::dump() {
    size_t pages = PAGES(depth);
    size_t nodes = (pages << 1) - 1;
    size_t n = LOG2PLUS1(pages) + 1;
    char *buf = new char[pages + 1];
    
    for (size_t i = 0; i < pages; ++i)
        buf[i] = '_';
    for (size_t i = 0; i < nodes; ++i) {
        if (IS_POW2(i+1))
            --n;
        if (tree[i]) 
            continue;
        if (i >= pages - 1) {
            buf[i - pages + 1] = '*';
        } else if (tree[LCHILD(i)] && tree[LCHILD(i) + 1]) {
            size_t nsize = PAGES(n);
            size_t offset = (i + 1) * nsize - pages;
            for (size_t j = offset; j < offset + nsize; ++j)
                buf[j] = '*';
        }
    }
    buf[pages] = '\0';
    return buf;
}

END_NS
