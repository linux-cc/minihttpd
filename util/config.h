#ifndef __CONFIG_H__
#define __CONFIG_H__

#ifdef _DEBUG_
#include <pthread.h>
#include <stdio.h>
#define _LOG_TID_(fmt, ...)     printf("[%s:%d][%ld]" fmt "\n", , __FILE__, __LINE__, (intptr_t)pthread_self(), ##__VA_ARGS__)
#define _LOG_(fmt, ...)         printf("[%s:%d]" fmt "\n", __FILE__, __LINE__, ##__VA_ARGS__)
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
#define END_LINE            "\r\n\r\n"
#define END_LINE_LENGTH     4
#define MAX(a, b)           ((a) > (b) ? (a) : (b))
#define MIN(a, b)           ((a) > (b) ? (b) : (a))
#define MMAP_PROT           (PROT_READ|PROT_WRITE)
#define MMAP_FLAGS          (MAP_ANONYMOUS|MAP_PRIVATE)

#endif /* ifndef __CONFIG_H__ */
