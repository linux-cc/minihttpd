#ifndef __CONFIG_H__
#define __CONFIG_H__

#ifdef _DEBUG_
#include <pthread.h>
#include <stdio.h>
#define _LOG_(fmt, ...)     printf("[%ld]"fmt, (intptr_t)pthread_self(), ##__VA_ARGS__)
//#define _LOG_(fmt, ...)     printf(fmt, ##__VA_ARGS__)
#else
#define _LOG_(...)    
#endif

#ifdef __linux__
#include <sys/sendfile.h>
#define STAT_MTIME(st)      (st).st_mtime
#else
#define STAT_MTIME(st)      (st).st_mtimespec.tv_sec
#endif

#define CR                  '\r'
#define LF                  '\n'
#define CRLF                "\r\n"
#define MAX(a, b)           ((a) > (b) ? (a) : (b))
#define MIN(a, b)           ((a) > (b) ? (b) : (a))
#define MMAP_PROT           (PROT_READ|PROT_WRITE)
#define MMAP_FLAGS          (MAP_ANONYMOUS|MAP_PRIVATE)

#endif /* ifndef __CONFIG_H__ */
