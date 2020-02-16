#include "util/string.h"
#include "memory/allocater.h"
#include <string.h>

namespace util {
    
String::String(const char *str, int length) {
    if (str && length > 0) {
        _ptr = memory::Allocater<Value>::New(str, length);
    }
}
    
String::String(const String &str, int pos, int length) {
    if (!str.empty() && pos < str.length() && length) {
        const char *data = str.data() + pos;
        int rest = str.length() - pos;
        if (length == npos || length > rest) {
            length = rest;
        }
        _ptr = memory::Allocater<Value>::New(data, length);
    }
}

String &String::operator=(const char *str) {
    if (str && *str) {
        _ptr = memory::Allocater<Value>::New(str, (int)strlen(str));
    }
    
    return *this;
}

String &String::append(const char *str) {
    if (str && *str) {
        return append(str, (int)strlen(str));
    }
    
    return *this;
}

String &String::append(char c) {
    char data[2] = { c, 0 };
    
    return append(data, 1);
}

String &String::append(const char *str, int length) {
    if (empty()) {
        _ptr = memory::Allocater<Value>::New(str, length);
    } else {
        if (_ptr->hasRef()) {
            _ptr = memory::Allocater<Value>::New(_ptr->_data, _ptr->_length);
        }
        int oldLength = _ptr->_length;
        _ptr->resize(oldLength + length);
        memcpy(_ptr->_data + oldLength, str, length);
    }
    
    return *this;
}

String &String::erase(int pos, int length) {
    if (!empty()) {
        if (_ptr->hasRef()) {
            _ptr = memory::Allocater<Value>::New(_ptr->_data, _ptr->_length);
        }
        int end = pos + length;
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

String &String::replace(int pos, int length, const String &str) {
    if (!str.empty()) {
        if (empty()) {
            _ptr = memory::Allocater<Value>::New(str.data(), str.length());
        } else {
            if (_ptr->hasRef()) {
                _ptr = memory::Allocater<Value>::New(_ptr->_data, _ptr->_length);
            }
            int end = pos + length;
            int oldLength = _ptr->_length;
            if (length == npos || end > oldLength) {
                end = oldLength;
            }
            _ptr->resize(oldLength - (end - pos) + str.length());
            if (end < oldLength) {
                memmove(_ptr->_data + pos + str.length(), _ptr->_data + end, oldLength - end);
            }
            memcpy(_ptr->_data + pos, str.data(), str.length());
        }
    }
    
    return *this;
}

String::Value::Value(const char *data, int length) {
    _capacity = (_length = length) << 1;
    _data = memory::Allocater<char[]>::New(_capacity);
    memcpy(_data, data, _length);
    _data[_length] = 0;
}

void String::Value::resize(int length) {
    if (length >= _capacity) {
        int newCapacity = length << 1;
        char *newData = memory::Allocater<char[]>::New(newCapacity);
        memcpy(newData, _data, _length);
        memory::Allocater<char[]>::Delete(_data, _capacity);
        _data = newData;
        _capacity = newCapacity;
    }
    _length = length;
    _data[_length] = 0;
}

String::Value::~Value() {
    memory::Allocater<char[]>::Delete(_data, _capacity);
}

String::CharProxy &String::CharProxy::operator=(char c) {
    _str._ptr = memory::Allocater<Value>::New(_str.data(), _str.length());
    _str._ptr->_data[_index] = c;
    return *this;
}

} /* namespace util */
