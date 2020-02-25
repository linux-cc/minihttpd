#include "util/config.h"
#include "util/buffer_queue.h"
#include "util/string.h"
#include <sys/uio.h>

namespace util {

bool BufferQueue::enqueue(const void *buf, size_t size) {
    if (_writePos >= _readPos) {
        size_t size1 = BUFFER_SIZE - _writePos;
        size_t size2 = _readPos;
        
        if (size + 1 > size1 + size2) {
            return false;
        }
        if (size <= size1) {
            memcpy(_buffer + _writePos, buf, size);
            _writePos += size;
        } else {
            memcpy(_buffer + _writePos, buf, size1);
            memcpy(_buffer, (char*)buf + size1, size - size1);
            _writePos = size - size1;
        }
    } else {
        size_t size1 = _readPos - _writePos;
        if (size + 1 > size1) {
            return false;
        }
        memcpy(_buffer + _writePos, buf, size);
        _writePos += size;
    }
    
    return true;
}

bool BufferQueue::dequeue(void *buf, size_t size) {
    if (length() < size) {
        return false;
    }
    
    size_t size1 = BUFFER_SIZE - _readPos;
    if (_readPos < _writePos || size <= size1) {
        memcpy(buf, _buffer + _readPos, size);
        _readPos += size;
    } else {
        size_t size2 = MIN(_writePos, size - size1);
        memcpy(buf, _buffer + _readPos, size1);
        memcpy((char*)buf + size1, _buffer, size2);
        _readPos = size2;
    }
    
    return true;
}

void BufferQueue::dequeueAll(String &buf) {
    buf.resize(length());
    dequeue((char*)buf.data(), length());
}

bool BufferQueue::dequeueUntil(String &buf, const char *pattern) {
    static uint16_t _move[128];
    size_t plen = strlen(pattern);
    for (int i = 0; i < 128; ++i) {
        _move[i] = plen + 1;
    }
    for (int i = 0; i < plen; ++i) {
        _move[pattern[i]] = plen - i;
    }
    size_t tlen = length();
    int s = 0, j;
    String tmp;
    while (s <= tlen - plen) {
        j = 0;
        size_t i = (_readPos + s) % BUFFER_SIZE;
        while (_buffer[i + j] == pattern[j]) {
            ++j;
            if (j >= plen) {
                buf = tmp.append(pattern);
                setReadPos(s + plen);
                return true;
            }
        }
        int mlen = _move[_buffer[i + plen]];
        tmp.append(&_buffer[s], mlen);
        s += mlen;
    }
    
    return false;
}

int BufferQueue::getWriteIov(struct iovec iov[2]) {
    if (full()) {
        return 0;
    }
    
    if (_writePos <= _readPos) {
        iov[0].iov_base = _buffer + _writePos;
        iov[0].iov_len = (_writePos ? _readPos - _writePos : BUFFER_SIZE) - 1;
        return 1;
    }
    
    int n = _readPos ? 2 : 1;
    iov[0].iov_base = _buffer + _writePos;
    iov[0].iov_len = BUFFER_SIZE - _writePos - (n == 1 ? 1 : 0);
    iov[1].iov_base = _buffer;
    iov[1].iov_len = _readPos - (n == 2 ? 1 : 0);
    
    return n;
}

void BufferQueue::setWritePos(size_t n) {
    if (_writePos <= _readPos) {
        _writePos += n;
    } else {
        size_t size1 = BUFFER_SIZE - _writePos;
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
    iov[0].iov_len = BUFFER_SIZE - _readPos;
    iov[1].iov_base = _buffer;
    iov[1].iov_len = _writePos;
    
    return 2;
}

void BufferQueue::setReadPos(size_t n) {
    if (_readPos < _writePos) {
        _readPos += n;
    } else {
        size_t size1 = BUFFER_SIZE - _readPos;
        if (n < size1) {
            _readPos += n;
        } else {
            _readPos = n - size1;
        }
    }
}

}
