#include "network/addrinfo.h"
#include <sys/un.h>
#include <stdio.h>

BEGIN_NS(network)

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
    int n = snprintf(sun->sun_path, sizeof(sun->sun_path), "/tmp/sock_local_%s_%s",
            host ? host : "null", service ? service : "null");
    _result->ai_addrlen = ((intptr_t)&((sockaddr_un*)0)->sun_path) + n;

    return 0;
}

END_NS
