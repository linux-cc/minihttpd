#ifndef __UTIL_BUFFER_QUEUE_H__
#define __UTIL_BUFFER_QUEUE_H__

#include "util/config.h"
#include "memory/simple_alloc.h"

struct iovec;
namespace util {

class String;
class BufferQueue {
public:
    BufferQueue(int capacity = BUFFER_SIZE): _readPos(0), _writePos(0), _capacity(capacity) { _buffer = memory::SimpleAlloc<char[]>::New(_capacity); }
    ~BufferQueue() { memory::SimpleAlloc<char[]>::Delete(_buffer, _capacity); }
    
    size_t enqueue(const void *buf, size_t size);
    
    size_t dequeue(void *buf, size_t size);
    bool dequeueUntil(String &buf, const char *pattern, bool flush);
    
    int getWriteIov(struct iovec iov[2]);
    void setWritePos(size_t n);
    
    int getReadIov(struct iovec iov[2]);
    void setReadPos(size_t n);
    
    bool empty() const { return _readPos == _writePos; }
    bool full() const { return (_writePos + 1) % _capacity == _readPos; }
    size_t length() const { return _writePos >= _readPos ? _writePos - _readPos : _capacity - (_readPos - _writePos); }
    size_t capaticy() const { return _capacity; }
    size_t index(size_t n) const { return  n % _capacity; }
    
private:
    char *_buffer;//空出一字节空间判断空和满
    size_t _readPos;
    size_t _writePos;
    size_t _capacity;
};

}

#endif /* __UTIL_BUFFER_QUEUE_H__ */
