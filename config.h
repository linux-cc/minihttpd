#ifndef __CONFIG_H__
#define __CONFIG_H__

#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#define BEGIN_NS(name)      namespace myframe { namespace name {
#define END_NS              }}
#define USING_NS(name)      using namespace myframe::name
#define MAX(a, b)           ((a) > (b) ? (a) : (b))
#define MIN(a, b)           ((a) > (b) ? (b) : (a))

#endif /* ifndef __CONFIG_H__ */
