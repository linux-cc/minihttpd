#ifndef __THREAD_RUNNABLE_H__
#define __THREAD_RUNNABLE_H__ 

namespace thread {

class Runnable
{
public:
    virtual void run() = 0;
    virtual ~Runnable() {}
};

} /* namespace thread */
#endif /* ifndef __THREAD_RUNNABLE_H__ */
