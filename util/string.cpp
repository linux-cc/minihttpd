#include "util/string.h"
#include "util/algorithm.h"

namespace util {

const size_t String::npos = -1;

String::String(const String &str, size_t pos, size_t length) {
    if (!str.empty() && pos < str.length() && length) {
        const char *data = str.data() + pos;
        size_t rest = str.length() - pos;
        if (length == npos || length > rest) {
            length = rest;
        }
        _ptr = memory::SimpleAlloc<Value>::New(data, length);
    }
}

String &String::erase(size_t pos, size_t length) {
    if (!empty()) {
        if (_ptr->hasRef()) {
            _ptr = memory::SimpleAlloc<Value>::New(_ptr->_data, _ptr->_length);
        }
        size_t end = pos + length;
        if (length == npos || end >= _ptr->_length) {
            _ptr->_length = pos;
            _ptr->_data[pos] = 0;
        } else {
            memmove(_ptr->_data + pos, _ptr->_data + end, _ptr->_length - end + 1);
            _ptr->_length -= length;
        }
    }
    
    return *this;
}

String &String::replace(size_t pos, size_t length, const String &str, size_t subpos, size_t sublen) {
    size_t strlen = str.length();
    if (!str.empty() && subpos < strlen && sublen) {
        size_t subend = subpos + sublen;
        if (sublen == npos || sublen > strlen) {
            subend = strlen;
        }
        replace(pos, length, &str[subpos], subend - subpos);
    }
    
    return *this;
}

String &String::replace(size_t pos, size_t length, const char *str, size_t strlen) {
    if (str && *str && strlen > 0 && appendEnabled(str, strlen)) {
        size_t end = pos + length;
        size_t oldLength = _ptr->_length;
        if (length == npos || end > oldLength) {
            end = oldLength;
        }
        _ptr->resize(oldLength - (end - pos) + strlen);
        if (end < oldLength) {
            memmove(_ptr->_data + pos + strlen, _ptr->_data + end, oldLength - end);
        }
        memcpy(_ptr->_data + pos, str, strlen);
    }
    
    return *this;
}

size_t String::find(const char *str, size_t pos) const {
    size_t index = npos;
    if (!empty() && pos < length() && str && *str) {
        const char *src = data();
        index = sundaySearch(src + pos, str);
    }
    
    return index;
}

size_t String::find(const char *str, size_t pos, size_t n) const {
    size_t index = npos;
    size_t strn = strlen(str);
    if (!empty() && str && *str && pos < n && n < strn) {
        const char *src = data();
        index = sundaySearch(src + pos, str, n);
    }
    
    return index;
}

size_t String::find(char c, size_t pos) const {
    size_t index = npos;
    if (!empty() && pos < length()) {
        const char *src = data();
        const char *p = strchr(src + pos, c);
        if (p) index = p - src;
    }
    
    return index;
}

String &String::append(const char *str, size_t length) {
    if (appendEnabled(str, length)) {
        size_t oldLength = _ptr->_length;
        _ptr->resize(oldLength + length);
        memcpy(_ptr->_data + oldLength, str, length);
    }
    
    return *this;
}

bool String::appendEnabled(const char *str, size_t length) {
    if (empty()) {
        _ptr = memory::SimpleAlloc<Value>::New(str, length);
        return false;
    } else {
        if (_ptr->hasRef()) {
            _ptr = memory::SimpleAlloc<Value>::New(_ptr->_data, _ptr->_length);
        }
        return true;
    }
}

String::Value::Value(const char *data, size_t length) {
    _capacity = (_length = length) << 1;
    _data = memory::SimpleAlloc<char[]>::New(_capacity);
    memcpy(_data, data, _length);
    _data[_length] = 0;
}

void String::Value::resize(size_t length) {
    if (length >= _capacity) {
        size_t newCapacity = length << 1;
        char *newData = memory::SimpleAlloc<char[]>::New(newCapacity);
        memcpy(newData, _data, _length);
        memory::SimpleAlloc<char[]>::Delete(_data, _capacity);
        _data = newData;
        _capacity = newCapacity;
    }
    _length = length;
    _data[_length] = 0;
}

String::CharProxy &String::CharProxy::operator=(char c) {
    _str._ptr = memory::SimpleAlloc<Value>::New(_str.data(), _str.length());
    _str._ptr->_data[_index] = c;
    return *this;
}

} /* namespace util */
