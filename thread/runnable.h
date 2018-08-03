#ifndef __THREAD_RUNNABLE_H__
#define __THREAD_RUNNABLE_H__ 


namespace myframe {
namespace thread {

class Runnable
{
public:
	virtual void run() = 0;
};

} /* namespace thread */
} /* namespace myframe */
#endif /* ifndef __THREAD_RUNNABLE_H__ */
