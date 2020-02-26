#include "util/buffer_queue.h"
#include "util/string.h"
#include <sys/uio.h>

namespace util {

bool BufferQueue::enqueue(const void *buf, size_t size) {
    if (size + 1 > _capacity - length()) {
        return false;
    }
    
    size_t size1 = _capacity - _writePos;
    if (_writePos < _readPos || size <= size1) {
        memcpy(_buffer + _writePos, buf, size);
        _writePos += size;
    } else {
        size_t size2 = size - size1;
        memcpy(_buffer + _writePos, buf, size1);
        memcpy(_buffer, (char*)buf + size1, size2);
        _writePos = size2;
    }

    return true;
}

size_t BufferQueue::dequeue(void *buf, size_t size) {
    if (empty()) {
        return 0;
    }
    
    if (_readPos < _writePos) {
        size_t n = MIN(_writePos - _readPos, size);
        memcpy(buf, _buffer + _readPos, n);
        _readPos += n;
        return n;
    }
    
    size_t size1 = _capacity - _readPos;
    if (size <= size1) {
        memcpy(buf, _buffer + _readPos, size);
        _readPos += size;
        return size;
    }
    
    size_t size2 = MIN(size - size1, _writePos);
    memcpy(buf, _buffer + _readPos, size1);
    memcpy((char*)buf + size1, _buffer, size2);
    _readPos = size2;
    
    return size1 + size2;
}

bool BufferQueue::dequeueAll(String &buf) {
    buf.resize(length());
    return dequeue((char*)buf.data(), length());
}

bool BufferQueue::dequeueUntil(String &buf, const char *pattern, bool flush) {
    static uint16_t _move[128];
    size_t plen = strlen(pattern);
    for (int i = 0; i < 128; ++i) {
        _move[i] = plen + 1;
    }
    for (int i = 0; i < plen; ++i) {
        _move[int(pattern[i])] = plen - i;
    }
    size_t tlen = length();
    int s = 0, j;
    while (s <= tlen - plen) {
        j = 0;
        size_t i = (_readPos + s) % _capacity;
        while (_buffer[(i + j) % _capacity] == pattern[j]) {
            ++j;
            if (j >= plen) {
                buf.append(pattern);
                setReadPos(s + plen);
                return true;
            }
        }
        int mlen = _move[int(_buffer[i + plen])];
        buf.append(&_buffer[i], mlen);
        s += mlen;
    }
    if (flush) {
        setReadPos(tlen);
    } else {
        buf.clear();
    }
    
    return false;
}

int BufferQueue::getWriteIov(struct iovec iov[2]) {
    if (full()) {
        return 0;
    }
    
    if (_writePos <= _readPos) {
        iov[0].iov_base = _buffer + _writePos;
        iov[0].iov_len = (_writePos ? _readPos - _writePos : _capacity) - 1;
        return 1;
    }
    
    int n = _readPos ? 2 : 1;
    iov[0].iov_base = _buffer + _writePos;
    iov[0].iov_len = _capacity - _writePos - (n == 1 ? 1 : 0);
    iov[1].iov_base = _buffer;
    iov[1].iov_len = _readPos - (n == 2 ? 1 : 0);
    
    return n;
}

void BufferQueue::setWritePos(size_t n) {
    if (_writePos <= _readPos) {
        _writePos += n;
    } else {
        size_t size1 = _capacity - _writePos;
        if (n < size1) {
            _writePos += n;
        } else {
            _writePos = n - size1;
        }
    }
}

int BufferQueue::getReadIov(struct iovec iov[2]) {
    if (empty()) {
        return 0;
    }
    
    if (_readPos < _writePos) {
        iov[0].iov_base = _buffer + _readPos;
        iov[0].iov_len = _writePos - _readPos;
        return 1;
    }
    
    iov[0].iov_base = _buffer + _readPos;
    iov[0].iov_len = _capacity - _readPos;
    iov[1].iov_base = _buffer;
    iov[1].iov_len = _writePos;
    
    return 2;
}

void BufferQueue::setReadPos(size_t n) {
    if (_readPos < _writePos) {
        _readPos += n;
    } else {
        size_t size1 = _capacity - _readPos;
        if (n < size1) {
            _readPos += n;
        } else {
            _readPos = n - size1;
        }
    }
}

}
