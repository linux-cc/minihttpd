#include "thread.h"
#include <sys/time.h>

namespace thread {

Thread::Thread(Runnable *r, DestroyNotify notify):
_pid(0),
_run(r),
_notify(notify) {
    if (!_run) {
        _run = this;
    }
}

Thread::~Thread() {
    if (_run && _notify) {
        _notify(_run);
    }
}

bool Thread::start() {
    pthread_attr_t attr;

    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
    pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, NULL);
    int ret = pthread_create(&_pid, &attr, threadFunc, this);
    pthread_attr_destroy(&attr);

    return ret == 0;
}

void *Thread::threadFunc(void *arg) {
    Thread *thr = (Thread *)arg;
    //	pthread_cleanup_push(cleanup, thr);
    if (!thr->onInit())
        return NULL;
    thr->_run->run();
    thr->onCancel();
    //	pthread_cleanup_pop(0);

    return NULL;
}

void Thread::cleanup(void *arg) {
    Thread *run = (Thread *)arg;
    run->onCancel();
}

Mutex::Mutex(int type) {
    if (type == PTHREAD_MUTEX_DEFAULT) {
        pthread_mutex_init(&_mutex, NULL);
        return;
    }
    pthread_mutexattr_t attr;
    pthread_mutexattr_settype(&attr, type);
    pthread_mutex_init(&_mutex, &attr);
}

int Cond::wait(int milliSeconds) {
    struct timespec ts;
    struct timeval now;
    
    gettimeofday(&now, NULL);
    ts.tv_sec = now.tv_sec + milliSeconds / 1000;
    ts.tv_nsec = now.tv_usec * 1000 + (milliSeconds % 1000) * 1000000;

    return pthread_cond_timedwait(&_cond, _mutex, &ts);
}

} /* namespace thread */
