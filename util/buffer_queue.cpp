#include "util/buffer_queue.h"
#include "util/string.h"
#include <sys/uio.h>

namespace util {

size_t BufferQueue::enqueue(const void *buf, size_t size) {
    if (full()) {
        return 0;
    }
    if (_writePos >= _readPos) {
        size_t size1 = _capacity - _writePos - (_readPos ? 0 : 1);
        if (size <= size1) {
            memcpy(_buffer + _writePos, buf, size);
            _writePos = index(_writePos + size);
            return size;
        }
        size_t size2 = MIN(size - size1, _readPos - (_readPos ? 1 : 0));
        memcpy(_buffer + _writePos, buf, size1);
        memcpy(_buffer, (char*)buf + size1, size2);
        _writePos = size2;
        return size1 + size2;
    }
    
    size_t size1 = MIN(_readPos - _writePos - 1, size);
    memcpy(_buffer + _writePos, buf, size1);
    _writePos = index(_writePos + size1);

    return size1;
}

size_t BufferQueue::dequeue(void *buf, size_t size) {
    if (empty()) {
        return 0;
    }
    
    if (_readPos < _writePos) {
        size_t n = MIN(_writePos - _readPos, size);
        memcpy(buf, _buffer + _readPos, n);
        _readPos = index(_readPos + n);
        return n;
    }
    
    size_t size1 = _capacity - _readPos;
    if (size <= size1) {
        memcpy(buf, _buffer + _readPos, size);
        _readPos = index(_readPos + size);
        return size;
    }
    
    size_t size2 = MIN(size - size1, _writePos);
    memcpy(buf, _buffer + _readPos, size1);
    memcpy((char*)buf + size1, _buffer, size2);
    _readPos = size2;
    
    return size1 + size2;
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
    size_t s = 0, j;
    while (s + plen <= tlen) {
        j = 0;
        size_t i = _readPos + s;
        while (_buffer[index(i + j)] == pattern[j]) {
            ++j;
            if (j >= plen) {
                buf.append(pattern);
                setReadPos(s + plen);
                return true;
            }
        }
        int mlen = _move[int(_buffer[index(i + plen)])];
        buf.append(&_buffer[index(i)], mlen);
        s += mlen;
    }
    if (flush) {
        if (s < tlen) {
            buf.append(&_buffer[index(_readPos + s)], tlen - s);
        } else if (s > tlen) {
            buf.erase(buf.length() - 1);
        }
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
    
    if (_writePos < _readPos) {
        iov[0].iov_base = _buffer + _writePos;
        iov[0].iov_len = _readPos - _writePos - 1;
        return 1;
    }
    
    iov[0].iov_base = _buffer + _writePos;
    iov[0].iov_len = _capacity - _writePos - (_readPos ? 0 : 1);
    iov[1].iov_base = _buffer;
    iov[1].iov_len = _readPos - (_readPos ? 1 : 0);
    return _readPos > 1 ? 2 : 1;
}

void BufferQueue::setWritePos(size_t n) {
    if (_writePos <= _readPos) {
        _writePos = index(_writePos + n);
    } else {
        size_t size1 = _capacity - _writePos;
        if (n < size1) {
            _writePos = index(_writePos + n);
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
    
    return _writePos ? 2 : 1;
}

void BufferQueue::setReadPos(size_t n) {
    if (_readPos < _writePos) {
        _readPos = index(_readPos + n);
    } else {
        size_t size1 = _capacity - _readPos;
        if (n < size1) {
            _readPos = index(_readPos + n);
        } else {
            _readPos = n - size1;
        }
    }
}

}
