#ifndef __THREAD_THREAD_H__
#define __THREAD_THREAD_H__

#include "runnable.h"
#include <pthread.h>
#include <signal.h>

typedef void (*DestroyNotify)(void *);

BEGIN_NS(thread)

class Thread : public Runnable {
public:
	Thread(Runnable *runnable = NULL, DestroyNotify notify = NULL);
	~Thread();
	
    virtual bool onInit() {
        return true;
    }
    virtual void onCancel() {

    };
    virtual void run() {

    }

	bool start();
    void cancel() {
	    pthread_cancel(_pid);
    }
    void join() {
	    pthread_join(_pid, NULL);
    }
    bool isAlive() {
	    return pthread_kill(_pid, 0) == 0;
    }
	void testCancel() {
        pthread_testcancel();
    }
	pthread_t pid() {
        return _pid;
    }
	Runnable *runnable() {
        return _run;
    }

private:
	static void *threadFunc(void *arg);
	static void cleanup(void *arg);

private:
	pthread_t _pid;
	Runnable *_run;
	DestroyNotify _notify;
};

class Mutex {
public:
	Mutex(int type = PTHREAD_MUTEX_DEFAULT);
    ~Mutex() {
	    pthread_mutex_destroy(&_mutex);
    }
    bool lock() {
	    return (pthread_mutex_lock(&_mutex) == 0);
    }
    bool trylock() {
	    return (pthread_mutex_trylock(&_mutex) == 0);
    }
    bool unlock() {
	    return (pthread_mutex_unlock(&_mutex) == 0);
    }
    operator pthread_mutex_t* () {
	    return &_mutex;
    }

protected:
	pthread_mutex_t _mutex;
};

class Cond {
public:
    Cond(Mutex &mutex): _mutex(mutex) {
	    pthread_cond_init(&_cond, NULL); 
    }
    ~Cond() {
	    pthread_cond_destroy(&_cond);
    }
    int broadcast() {
	    return pthread_cond_broadcast(&_cond);
    }
    int signal() {
	    return pthread_cond_signal(&_cond);
    }
    int wait() {
	    return pthread_cond_wait(&_cond, _mutex);
    }
	int timedwait(int milliSeconds);

private:
	pthread_cond_t _cond;
	Mutex &_mutex;
};

class AutoMutex {
public:
	AutoMutex(Mutex &metux):_mutex(metux), _locked(false) {
        _locked = _mutex.lock();
    }
	~AutoMutex() {
        if(_locked) _mutex.unlock();
    }
private:
	Mutex &_mutex;
	bool _locked;
};

END_NS
#endif /* ifndef __THREAD_THREAD_H__ */
