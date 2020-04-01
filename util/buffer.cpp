#include "util/buffer.h"
#include "util/string.h"

namespace util {

ssize_t Buffer::write(const void *buf, size_t size) {
    size_t n1 = copyFrom(buf, size);
    _length += n1;
    _writePos = index(_writePos + n1);
    if (n1 == size) {
        return n1;
    }

    ssize_t n = flush();
    if (n < 0) {
        return n;
    }
    
    size_t n2 = copyFrom((char*)buf + n1, size - n1);
    _length += n2;
    _writePos = index(_writePos + n2);
    return n1 + n2;
}

size_t Buffer::copyFrom(const void *buf, size_t size) {
    size_t n = MIN(_capacity - _length, size);

    for (size_t i = 0; i < n; i++) {
        _buffer[index(_writePos + i)] = *((char*)buf + i);
    }

    return n;
}

ssize_t Buffer::flush() {
    ssize_t n;
    if (_readPos < _writePos || _writePos == 0) {
        n = _source->overflow(_buffer + _readPos, _length);
    } else {
        n = _source->overflow(_buffer + _readPos, _capacity - _readPos, _buffer, _writePos);
    }
    if (n > 0) {
        _readPos = index(_readPos + n);
        _length -= n;
    }
    
    return n;
}

ssize_t Buffer::read(void *buf, size_t size) {
    size_t n1 = copyTo(buf, size);
    _length -= n1;
    _readPos = index(_readPos + n1);
    if (n1 == size) {
        return n1;
    }

    ssize_t n = refill();
    if (n < 0) {
        return n;
    }
    
    size_t n2 = copyTo((char*)buf + n1, size - n1);
    _length -= n2;
    _readPos = index(_readPos + n2);
    return n1 + n2;
}

size_t Buffer::copyTo(void *buf, size_t size) {
    size_t n = MIN(_length, size);
    
    for (size_t i = 0; i < n; i++) {
        *((char*)buf + i) = _buffer[index(_readPos + i)];
    }
    
    return n;
}

ssize_t Buffer::find(const char *pattern) {
    uint16_t move[256];
    size_t plen = strlen(pattern);
    size_t s = 0, j;
    
    for (int i = 0; i < 256; ++i) {
        move[i] = plen + 1;
    }
    for (int i = 0; i < plen; ++i) {
        move[int(pattern[i])] = plen - i;
    }
    
    while (s + plen <= _length) {
        j = 0;
        while (_buffer[index(_readPos + s + j)] == pattern[j]) {
            if (++j >= plen) {
                return s;
            }
        }
        s += move[int(_buffer[index(_readPos + s + plen)])];
    }
    
    return -1;
}

ssize_t Buffer::refill() {
    ssize_t n;
    if (_writePos < _readPos || _readPos == 0) {
        n = _source->underflow(_buffer + _writePos, _capacity - _length);
    } else {
        n = _source->underflow(_buffer + _writePos, _capacity - _writePos, _buffer, _readPos);
    }
    if (n > 0) {
        _writePos = index(_writePos + n);
        _length += n;
    }
    
    return n;
}

}
