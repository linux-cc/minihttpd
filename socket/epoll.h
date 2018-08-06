#ifndef __SOCKET_EPOLL_H__
#define __SOCKET_EPOLL_H__

#include "config.h"
#include <unistd.h>
#ifdef __linux__
#include <sys/epoll.h>
#else /* mac osx */
#include <sys/event.h>
#include <sys/time.h>
#define EPOLL_CTL_ADD               (EV_ADD | EV_ONESHOT)
#define EPOLL_CTL_DEL               (EV_DELETE | EV_DISABLE)
#define EPOLL_CTL_MOD               (EV_ADD | EV_ONESHOT)
#define EPOLLIN                     EVFILT_READ
#define EPOLLOUT                    EVFILT_WRITE
#define EPOLLONESHOT                0
#define EPOLLET                     0
typedef struct kevent epoll_event;
#endif /* __linux__ */

BEGIN_NS(socket)

class EPollResult;

class EPollEvent {
public:
    operator epoll_event *() {
        return &_event;
    }
    operator const epoll_event *() const {
        return &_event;
    }
    int events() const;
    int fd() const;

private:
    void init(int fd, int events);

    epoll_event _event;
    friend class EPoller;
};

class EPoller {
public:
    EPoller(): _fd(-1), _size(0), _events(NULL) {}
    ~EPoller();

    bool create(int size);
    int add(int fd);
    int mod(int fd);
    int del(int fd);
    EPollResult wait(int timeout);

private:
    int ctl(int fd, int action, int events);

    int _fd;
    int _size;
    EPollEvent *_events;
};

class EPollResult {
public:
    class Iterator {
    public:
        Iterator(const Iterator& other): _index(other._index), _result(other._result) {}
        Iterator &operator++() {
            _index++;
            return *this;
        }
        const Iterator operator++(int) {
            Iterator old = *this;
            _index++;
            return old;
        }
        bool operator==(const Iterator &other) {
            return (_index == other._index && _result == other._result);
        }
        bool operator!=(const Iterator &other) {
            return !operator==(other);
        }
        EPollEvent &operator*() {
            return _result._events[_index];
        }
        EPollEvent *operator->() {
            return &operator*();
        }

    private:
        Iterator(int index, EPollResult &result): _index(index), _result(result) {}
        int _index;
        EPollResult &_result;
        friend class EPollResult;
    };

    EPollResult(const EPollResult& other): _events(other._events), _size(other._size) {}
    EPollResult &operator=(const EPollResult &other);

    Iterator begin() {
        return Iterator(_size > 0 ? 0 : _size, *this);
    }
    Iterator end() {
        return Iterator(_size, *this);
    }

private:
    EPollResult(EPollEvent *events, int size): _events(events), _size(size) {}
    bool operator==(const EPollResult &other) {
        return (_events == other._events && _size == other._size);
    }

    EPollEvent *_events;
    int _size;

    friend class EPoller;
    friend class EPollResult::Iterator;
};

END_NS
#endif /* __HTTPD_EPOLL_H__ */
