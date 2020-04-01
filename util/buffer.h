#ifndef __UTIL_BUFFER_H__
#define __UTIL_BUFFER_H__

#include "util/config.h"
#include "memory/simple_alloc.h"
#include <sys/types.h>

namespace util {

class String;

class IBuffer {
public:
    virtual ssize_t underflow(void *buf, size_t size) = 0;
    virtual ssize_t underflow(void *buf1, size_t size1, void *buf2, size_t size2) = 0;
    virtual ssize_t overflow(const void *buf, size_t size) = 0;
    virtual ssize_t overflow(const void *buf1, size_t size1, const void *buf2, size_t size2) = 0;
};

class Buffer {
public:
    Buffer(IBuffer *source, int capacity = BUFFER_SIZE): _source(source), _readPos(0), _writePos(0), _length(0), _capacity(capacity) {
        _buffer = memory::SimpleAlloc<char[]>::New(_capacity);
    }
    ~Buffer() { memory::SimpleAlloc<char[]>::Delete(_buffer, _capacity); }
    
    ssize_t write(const void *buf, size_t size);
    ssize_t read(void *buf, size_t size);
    ssize_t find(const char *pattern);
    ssize_t refill();
    ssize_t flush();
    
    bool empty() const { return _length == 0; }
    bool full() const { return _length == _capacity; }
    size_t length() const { return _length; }
    size_t capaticy() const { return _capacity; }
    size_t index(size_t n) const { return  n % _capacity; }
    
private:
    size_t copyFrom(const void *buf, size_t size);
    size_t copyTo(void *buf, size_t size);
    
    char *_buffer;
    IBuffer *_source;
    size_t _readPos;
    size_t _writePos;
    size_t _length;
    size_t _capacity;
};

}

#endif /* __UTIL_BUFFER_QUEUE_H__ */
