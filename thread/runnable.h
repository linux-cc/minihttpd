#ifndef __THREAD_RUNNABLE_H__
#define __THREAD_RUNNABLE_H__ 

#include "config.h"

BEGIN_NS(thread)

class Runnable
{
public:
	virtual void run() = 0;
    virtual ~Runnable() {}
};

END_NS
#endif /* ifndef __THREAD_RUNNABLE_H__ */
