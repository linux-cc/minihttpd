#include "fcgid/libfcgi.h"
#include "httpd/connection.h"
#include <unistd.h>
#include <errno.h>

namespace fcgid {

using httpd::Connection;

Manager::Manager(int children, bool isLocal):
_children(children),
_local(isLocal) {

}

bool Manager::start(const char *host, const char *service) {
    if (!_socket.create(host, service)) {
        _LOG_("socket create error:%d:%s", errno, strerror(errno));
        return false;
    }

    while (_children--) {
        pid_t pid = fork();
        if (pid < 0) {
            _LOG_("fork error:%d:%s", errno, strerror(errno));
            return false;
        } else if (pid == 0) {
            process();
            return true;
        }
    }
    //monitor();

    return true;
}

void Manager::process() {
    int fd = -1;
    while ((fd = _socket.accept()) > 0) {

    }
}

}

