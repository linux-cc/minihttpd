#ifndef __CONFIG_H__
#define __CONFIG_H__

#ifdef __DEBUG__
#define LOG_DEBUG(fmt, ...)
#define _LOG_(fmt, ...)         util::writeLog(__PRETTY_FUNCTION__, __LINE__, fmt, ##__VA_ARGS__)
#else
#define _LOG_(...)    
#endif

#ifdef __linux__
#include <sys/sendfile.h>
#define STAT_MTIME(st)      (st).st_mtime
#else
#define STAT_MTIME(st)      (st).st_mtimespec.tv_sec
#endif

#define CHAR_CR             '\r'
#define CHAR_LF             '\n'
#define CHAR_SP             ' '
#define CHAR_COLON          ':'
#define CHAR_QUOTE          '"'
#define STR_COLON           ":"
#define STR_SP              " "
#define ONE_CRLF            "\r\n"
#define TWO_CRLF            "\r\n\r\n"
#define TWO_CRLF_SIZE       4
#define MAX(a, b)           ((a) > (b) ? (a) : (b))
#define MIN(a, b)           ((a) > (b) ? (b) : (a))
#define MMAP_PROT           (PROT_READ|PROT_WRITE)
#define MMAP_FLAGS          (MAP_ANONYMOUS|MAP_PRIVATE)

#define MAX_WORKER          4
#define MAX_WORKER_CONN     8
#define CONN_TIMEOUT        10
#define BUFFER_SIZE         8192
#define MAX_IOV_CNT         8

#endif /* ifndef __CONFIG_H__ */
