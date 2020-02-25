#ifndef __UTIL_BUFFER_QUEUE_H__
#define __UTIL_BUFFER_QUEUE_H__

#include "util/config.h"
#include "memory/simple_alloc.h"

struct iovec;
namespace util {

class String;
class BufferQueue {
public:
    BufferQueue(): _readPos(0), _writePos(0) { _buffer = memory::SimpleAlloc<char[]>::New(BUFFER_SIZE); }
    ~BufferQueue() { memory::SimpleAlloc<char[]>::Delete(_buffer, BUFFER_SIZE); }
    
    bool enqueue(const void *buf, size_t size);
    
    bool dequeue(void *buf, size_t size);
    bool dequeueAll(String &buf);
    bool dequeueUntil(String &buf, const char *pattern);
    
    int getWriteIov(struct iovec iov[2]);
    void setWritePos(size_t n);
    
    int getReadIov(struct iovec iov[2]);
    void setReadPos(size_t n);
    
    bool empty() const { return _readPos == _writePos; }
    bool full() const { return (_writePos + 1) % BUFFER_SIZE == _readPos; }
    size_t length() const { return _writePos > _readPos ? _writePos - _readPos : BUFFER_SIZE - (_readPos - _writePos); }
    
private:
    char *_buffer;//空出一字节空间判断空和满
    size_t _readPos;
    size_t _writePos;
};

}

#endif /* __UTIL_BUFFER_QUEUE_H__ */
