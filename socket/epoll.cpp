#include "socket/epoll.h"
#include <unistd.h>
#include <errno.h>

BEGIN_NS(socket)

#ifdef __linux__
inline void epoll_set_event(int fd, int events, epoll_event &ev) {
    ev.events = events;
    ev.data.fd = fd;
}
inline int epoll_get_events(const epoll_event &ev) {
    return ev.events;
}
inline int epoll_get_fd(const epoll_event &ev) {
    return ev.data.fd;
}
#else /* mac osx */
inline int epoll_create(int) {
    return kqueue();
}
inline int epoll_wait(int fd, epoll_event *events, int size, int timeout) {
    timespec ts;
    ts.tv_sec = timeout / 1000;
    ts.tv_nsec = (timeout % 1000) * 1000000;
    return kevent(fd, NULL, 0, events, size, &ts);
}
inline int epoll_ctl(int efd, int action, int fd, epoll_event *ev) {
    timespec ts = { 0, 0 };
    EV_SET(ev, fd, ev->filter, action, 0, 0, NULL);
    return kevent(efd, ev, 1, NULL, 0, &ts);
}
inline void epoll_set_event(int fd, int events, epoll_event &ev) {
    ev.filter = events;
    ev.ident = fd;
}
inline int epoll_get_events(const epoll_event &ev) {
    return ev.filter;
}
inline int epoll_get_fd(const epoll_event &ev) {
    return ev.ident;
}
#endif /* __linux__ */

inline void EPollEvent::init(int fd, int events) {
    epoll_set_event(fd, events, _event);
}

int EPollEvent::events() const {
    return epoll_get_events(_event);
}

int EPollEvent::fd() const {
    return epoll_get_fd(_event);
}

EPoller::~EPoller() {
    if (_events) {
        delete []_events;
    }
    close(_fd);
}

bool EPoller::create(int size) {
    _fd = epoll_create(1024);
    if (_fd == -1) {
        return false;
    }
    _size = size;
    _events = new EPollEvent[_size];

    return _events;
}

int EPoller::add(int fd) {
    return ctl(fd, EPOLL_CTL_ADD, EPOLLIN | EPOLLONESHOT | EPOLLET);
}

int EPoller::mod(int fd) {
    return ctl(fd, EPOLL_CTL_MOD, EPOLLIN | EPOLLONESHOT | EPOLLET);
}

int EPoller::del(int fd) {
    return ctl(fd, EPOLL_CTL_DEL, 0);
}

EPollResult EPoller::wait(int timeout) {
    int nfds = epoll_wait(_fd, _events[0], _size, timeout);
    if (nfds < 0 && errno == EINTR) {
        nfds = 0;
    }

    return EPollResult(_events, nfds);
}

int EPoller::ctl(int fd, int action, int events) {
    EPollEvent ev;
    ev.init(fd, events);
    return epoll_ctl(_fd, action, fd, ev);
}

EPollResult &EPollResult::operator=(const EPollResult &other) {
    _events = other._events;
    _size = other._size;
    return *this;
}

END_NS
