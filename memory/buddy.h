#ifndef __MEMORY_BUDDY_H__
#define __MEMORY_BUDDY_H__

#include "memory/pub_macros.h"

BEGIN_NS(memory)

class Buddy {
public:
    Buddy(size_t pages = 1024);
    ~Buddy();
	void init(size_t pages);
	void *alloc(size_t pages);
	void free(void *addr);
	char *dump();
    char *buffer() { return buf; }

private:
    char *buf;
    char *pageAddr;
    uint8_t *tree;
    uint8_t depth;
};

END_NS

#endif
