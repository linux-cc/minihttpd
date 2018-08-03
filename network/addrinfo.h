#ifndef __NETWORK_ADDRINFO_H__
#define __NETWORK_ADDRINFO_H__ 

#include <netdb.h>
#include <string.h>

namespace myframe {
namespace network {

class Addrinfo {
public:
    class Iterator {
    public:
        Iterator &operator++() {
            _cur = _cur->ai_next;
            return *this;
        }
        const Iterator operator++(int) {
            Iterator old = *this;
            ++(*this);
            return old;
        }
        bool operator!=(const Iterator &other) {
            return _cur != other._cur;
        }
        const addrinfo *operator->() {
            return _cur;
        }

    private:
        Iterator(const addrinfo *result): _cur(result) {}
        
        const addrinfo *_cur;
        friend class Addrinfo;
    };     
public:
    Addrinfo(int family, int socktype, int protocol): _result(NULL) {
        memset(&_hints, 0, sizeof(_hints));
        _hints.ai_family = family;
        _hints.ai_socktype = socktype;
        _hints.ai_protocol = protocol;
    }
    ~Addrinfo() {
        if (_result) freeaddrinfo(_result);
    }
    int getAddrinfo(const char *host, const char *service) {
        return getaddrinfo(host, service, &_hints, &_result);
    }
    void family(int family) {
        _hints.ai_family = family;
    }
    void socktype(int socktype) {
        _hints.ai_socktype = socktype;
    }
    void protocol(int protocol) {
        _hints.ai_protocol = protocol;
    }
    void flags(int flags) {
        _hints.ai_flags = flags;
    }
    bool isLocal() const {
        return _hints.ai_family == PF_LOCAL;
    }
    Iterator begin() const {
        return Iterator(_result);
    }
    Iterator end() const {
        return Iterator(NULL);
    }
    const char *errinfo(int errno) const {
        return gai_strerror(errno);
    }

private:
    Addrinfo(const Addrinfo &);
    Addrinfo &operator=(const Addrinfo &);

    addrinfo *_result;
    addrinfo _hints;
};

} /* namespace network */
} /* namespace myframe */

#endif /* ifndef __NETWORK_ADDRINFO_H__ */
