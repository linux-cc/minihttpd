#include "util/string.h"

namespace util {

const size_t String::npos = -1;

String::String(const String &str, size_t pos, size_t length) {
    const char *data = str.data() + pos;
    if (length == npos) {
        length = str.length() - pos;
    }
    _ptr = memory::SimpleAlloc<Value>::New(data, length);
}

String &String::erase(size_t pos, size_t length) {
    makeCopy();
    size_t end = pos + length;
    if (length == npos || end >= _ptr->_length) {
        _ptr->_length = pos;
    } else {
        memmove(_ptr->_data + pos, _ptr->_data + end, _ptr->_length - end);
        _ptr->_length -= length;
    }
    _ptr->_data[_ptr->_length] = 0;
    
    return *this;
}

String &String::replace(size_t pos, size_t length, const String &str, size_t subpos, size_t sublen) {
    size_t subend = subpos + sublen;
    if (sublen == npos || sublen > str.length()) {
        subend = str.length();
    }
    
    return replace(pos, length, &str[subpos], subend - subpos);
}

String &String::replace(size_t pos, size_t length, const char *str, size_t strlen) {
    makeCopy();
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
    
    return *this;
}

String &String::append(const char *str, size_t length) {
    makeCopy();
    size_t oldLength = _ptr->_length;
    _ptr->resize(oldLength + length);
    memcpy(_ptr->_data + oldLength, str, length);
    
    return *this;
}

String &String::append(char c) {
    makeCopy();
    size_t oldLength = _ptr->_length;
    _ptr->resize(oldLength + 1);
    _ptr->_data[oldLength] = c;
    
    return *this;
}

String &String::insert(size_t pos, const String &str, size_t subpos, size_t sublen) {
    size_t subend = subpos + sublen;
    if (sublen == npos || sublen > str.length()) {
        subend = str.length();
    }
    
    return insert(pos, &str[subpos], sublen);
}

String &String::insert(size_t pos, const char *str, size_t strlen) {
    makeCopy();
    size_t oldLength = _ptr->_length;
    _ptr->resize(oldLength + strlen);
    memmove(_ptr->_data + pos + strlen, _ptr->_data + pos, oldLength - pos);
    memcpy(_ptr->_data + pos, str, strlen);
    
    return *this;
}

void String::makeCopy() {
    if (_ptr->hasRef()) {
        _ptr = memory::SimpleAlloc<Value>::New(_ptr->_data, _ptr->_length);
    }
}

String::Value::Value(const char *data, size_t length) {
    _capacity = (_length = length) << 1;
    if (_capacity < 8) {
        _capacity = 32;
    }
    _data = memory::SimpleAlloc<char[]>::New(_capacity);
    memcpy(_data, data, _length);
    _data[_length] = 0;
}

void String::Value::resize(size_t length) {
    if (length >= _capacity) {
        size_t newCapacity = length <= 128 ? (length << 1) : (((length << 2) - length) >> 1);
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
