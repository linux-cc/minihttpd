#include "util/string.h"

namespace util {
    
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

String &String::replace(int pos, int length, const String &str, int subpos, int sublen) {
    int strlen = str.length();
    if (!str.empty() && subpos < strlen && sublen) {
        int subend = subpos + sublen;
        if (sublen == npos || sublen > strlen) {
            subend = strlen;
        }
        replace(pos, length, &str[subpos], subend - subpos);
    }
    
    return *this;
}

String &String::replace(int pos, int length, const char *str, int strlen) {
    if (str && *str && strlen > 0 && appendEnabled(str, strlen)) {
        int end = pos + length;
        int oldLength = _ptr->_length;
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

int String::find(const char *str, int pos) const {
    int index = npos;
    if (!empty() && pos < length() && str && *str) {
        const char *src = data();
        const char *p = strstr(src + pos, str);
        if (p) index = int(p - src);
    }
    
    return index;
}

int String::find(const char *str, int pos, int n) const {
    int index = npos;
    int strn = (int)strlen(str);
    if (!empty() && str && *str && pos < n && n < strn) {
        const char *src = data();
        char *pn = (char*)str + n;
        char cn = *pn;
        *pn = 0;
        const char *p = strstr(src + pos, str);
        *pn = cn;
        if (p) index = int(p - src);
    }
    
    return index;
}

int String::find(char c, int pos) const {
    int index = npos;
    if (!empty() && pos < length()) {
        const char *src = data();
        const char *p = strchr(src + pos, c);
        if (p) index = int(p - src);
    }
    
    return index;
}

String &String::append(const char *str, int length) {
    if (appendEnabled(str, length)) {
        int oldLength = _ptr->_length;
        _ptr->resize(oldLength + length);
        memcpy(_ptr->_data + oldLength, str, length);
    }
    
    return *this;
}

bool String::appendEnabled(const char *str, int length) {
    if (empty()) {
        _ptr = memory::Allocater<Value>::New(str, length);
        return false;
    } else {
        if (_ptr->hasRef()) {
            _ptr = memory::Allocater<Value>::New(_ptr->_data, _ptr->_length);
        }
        return true;
    }
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

String::CharProxy &String::CharProxy::operator=(char c) {
    _str._ptr = memory::Allocater<Value>::New(_str.data(), _str.length());
    _str._ptr->_data[_index] = c;
    return *this;
}

} /* namespace util */
