#ifndef MEMORY_BUDDY_H_
#define MEMORY_BUDDY_H_

#include "pub_macros.h"

namespace memory {

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

}

#endif /* UTILS_BUDDY_H_ */
