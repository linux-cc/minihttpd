#ifndef __UTIL_STRING_H__
#define __UTIL_STRING_H__

#include "util/scoped_ref.h"
#include "memory/simple_alloc.h"
#include <string.h>

namespace util {

class String {
public:
    static const size_t npos;
    
    class CharProxy {
    public:
        CharProxy(String &str, size_t index): _str(str), _index(index) {}
        CharProxy &operator=(const CharProxy &proxy) { return this->operator=((char)proxy); }
        CharProxy &operator=(char c);
        operator char () const { return _str._ptr->_data[_index]; }
        const char *operator&() const { return &_str._ptr->_data[_index]; }
        
    private:
        String &_str;
        size_t _index;
    };
    
    String(const char *str = NULL) { *this = str; }
    String(const char *str, size_t length) { if (str && length > 0) _ptr = memory::SimpleAlloc<Value>::New(str, length); }
    String(const String &str, size_t pos, size_t length = npos);
    String &operator=(const char *str) { if (str && *str) _ptr = memory::SimpleAlloc<Value>::New(str, strlen(str)); return *this; }
    
    void resize(size_t length) { _ptr->resize(length); }
    
    bool empty() const { return !_ptr || _ptr->_length == 0; }
    
    const char *data() const { return empty() ? NULL : _ptr->_data; }
    
    size_t length() const { return empty() ? 0 : _ptr->_length; }
    
    size_t capacity() const { return empty() ? 0 : _ptr->_capacity; }
    
    CharProxy operator[](size_t index) const { return CharProxy(*(String*)this, index); }
    CharProxy operator[](size_t index) { return CharProxy(*this, index); }
    
    String &operator+=(const String &str) { return append(str); }
    String &operator+=(const char *str) { return append(str); }
    String &operator+=(char c) { return append(c); }
    
    String &append(const String &str) { return str.empty() ? *this : append(str.data(), str.length()); }
    String &append(const char *str) { return append(str, strlen(str)); return *this; }
    String &append(const char *str, size_t length);
    String &append(char c) { char data[2] = { c, 0 }; return append(data, 1); }
    
    String &erase(size_t pos = 0, size_t length = npos);
    void clear() { erase(); }
    
    String &replace(size_t pos, size_t length, const String &str) { return replace(pos, length, str, 0, npos); }
    String &replace(size_t pos, size_t length, const String &str, size_t subpos, size_t sublen);
    String &replace(size_t pos, size_t length, const char *str) { if (str && *str) replace(pos, length, str, strlen(str)); return *this; }
    String &replace(size_t pos, size_t length, const char *str, size_t strlen);
    
    size_t find(const String &str, size_t pos = 0) const { return find(str.data(), pos); }
    size_t find(const char *str, size_t pos = 0) const;
    size_t find(const char *str, size_t pos, size_t n) const;
    size_t find(char c, size_t pos = 0) const;
    
    String substr(size_t pos = 0, size_t length = npos) { return String(*this, pos, length); }
    
    int refCount() const { return empty() ? 0 : _ptr->refConut(); }
    
private:
    bool appendEnabled(const char *str, size_t length);
    
    class Value : public RefCounted<Value> {
    public:
        Value(const char *data, size_t length);
        void resize(size_t length);
        
        char *_data;
        size_t _length;
        size_t _capacity;
    private:
        ~Value() { memory::SimpleAlloc<char[]>::Delete(_data, _capacity); }
        friend class memory::SimpleAlloc<Value>;
    };
    
    ScopedRef<Value> _ptr;
    friend class CharProxy;
};

} /* namespace util */

#endif /* __UTIL_STRING_H__ */