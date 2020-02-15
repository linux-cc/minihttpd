#include "network/addrinfo.h"
#include <unistd.h>
#include <stdio.h>

namespace network { 

Peername::Peername(const Sockaddr &addr) {
    const void *inaddr;
    int family = addr.family();
    if (family == PF_INET) {
        const sockaddr_in *si = addr;
        inaddr = &si->sin_addr;
        _port = ntohs(si->sin_port);
    } else if (family == PF_INET6) {
        const sockaddr_in6 *si6 = addr;
        inaddr = &si6->sin6_addr;
        _port = ntohs(si6->sin6_port);
    } else {
        if (family == PF_LOCAL) {
            const sockaddr_un *un = addr;
            sscanf(un->sun_path, "%*[^_]_%[^_]_%d", _name, &_port);
        }
        return;
    }

    inet_ntop(family, inaddr, _name, sizeof(_name));
}
    
Peername& Peername::operator=(const Peername &other) {
    if (this != &other) {
        strcpy(_name, other._name);
        _port = other._port;
    }
    return *this;
}

Addrinfo::Addrinfo(int family, int socktype, int protocol, int flags): _result(NULL) {
    memset(&_hints, 0, sizeof(_hints));
    _hints.ai_family = family;
    _hints.ai_socktype = socktype;
    _hints.ai_flags = flags;
    _hints.ai_protocol = protocol;
}

Addrinfo::~Addrinfo() {
    if (_result) {
        if (_hints.ai_family != PF_LOCAL) {
            freeaddrinfo(_result);
        } else {
            free(_result);
        }
    }
}

int Addrinfo::getaddrinfo(const char *host, const char *service) {
    if (_hints.ai_family != PF_LOCAL) {
        return ::getaddrinfo(host, service, &_hints, &_result);
    }
    _result = (addrinfo*)malloc(sizeof(addrinfo) + sizeof(sockaddr_un));
    if (!_result) {
        return -1;
    }
    _result->ai_next = NULL;
    _result->ai_addr = (sockaddr*)(_result + 1);
    _result->ai_family = _hints.ai_family;
    _result->ai_socktype = _hints.ai_socktype;
    _result->ai_protocol = _hints.ai_protocol;
    _result->ai_flags = _hints.ai_flags;
    sockaddr_un *sun = (sockaddr_un*)_result->ai_addr;
    sun->sun_family = _hints.ai_family;
    int n = snprintf(sun->sun_path, sizeof(sun->sun_path), "/tmp/localsock_%s_%s",
            host ? host : "null", service ? service : "null");
    _result->ai_addrlen = ((intptr_t)&((sockaddr_un*)0)->sun_path) + n;
    if (_hints.ai_flags & AI_PASSIVE) {
        unlink(sun->sun_path);
    }

    return 0;
}

} /* namespace network */
