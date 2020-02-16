#ifndef __UTIL_STRING_H__
#define __UTIL_STRING_H__

#include "util/scoped_ref.h"

namespace util {

class String {
public:
    enum { npos = -1, };
    
    class CharProxy {
    public:
        CharProxy(String &str, int index): _str(str), _index(index) {}
        CharProxy &operator=(const CharProxy &proxy) { return this->operator=((char)proxy); }
        CharProxy &operator=(char c);
        operator char () const { return _str._ptr->_data[_index]; }
        
    private:
        String &_str;
        int _index;
    };
    
    String(const char *str = NULL) { *this = str; }
    String(const char *str, int length);
    String(const String &str, int pos, int length = npos);
    String &operator=(const char *str);
    
    void resize(int length) { _ptr->resize(length); }
    
    bool empty() const { return !_ptr; }
    
    const char *data() const { return empty() ? NULL : _ptr->_data; }
    
    int length() const { return empty() ? 0 : _ptr->_length; }
    
    int capacity() const { return empty() ? 0 : _ptr->_capacity; }
    
    CharProxy operator[](int index) const { return CharProxy(*(String*)this, index); }
    CharProxy operator[](int index) { return CharProxy(*this, index); }
    
    String &operator+=(const String &str) { return append(str); }
    String &operator+=(const char *str) { return append(str); }
    String &operator+=(char c) { return append(c); }
    
    String &append(const String &str) { return str.empty() ? *this : append(str.data(), str.length()); }
    String &append(const char *str);
    String &append(char c);
    
    String &erase(int pos = 0, int length = npos);
    void clear() { erase(); }
    
    String &replace(int pos, int length, const String &str);
    String &replace(int pos, int length, const String &str, int subpos, int sublen);
    String &replace(int pos, int length, const char *str);
    String &replace(int pos, int length, const char *str, int n);
    
    int find(const String &str, int pos = 0) const;
    int find(const char *str, int pos = 0) const;
    int find(const char *str, int pos, int n) const;
    int find(char c, int pos = 0) const;
    
    String substr(int pos = 0, int length = npos);
    
    int refCount() const { return empty() ? 0 : _ptr->refConut(); }
    
private:
    String &append(const char *str, int length);
    
    class Value : public RefCounted<Value> {
    public:
        Value(const char *data, int length);
        void resize(int length);
        void append(const char *data, int length);
        
        char *_data;
        int _length;
        int _capacity;
    private:
        void init(const char *data, int length);
        ~Value();
        friend class memory::Allocater<Value>;
    };
    
    ScopedRef<Value> _ptr;
    friend class CharProxy;
};

} /* namespace util */

#endif /* __UTIL_STRING_H__ */
