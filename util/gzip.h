#ifndef __UTIL_GZIP_H__
#define __UTIL_GZIP_H__

#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>

namespace util {

class GTree;

class GCallback {
public:
    virtual ~GCallback() {}
    virtual ssize_t gzflush(const void *buf, size_t len, bool eof) = 0;
};

class GZip : public GCallback {
public:
    GZip();
    ~GZip();
    void setLevel(int level) { _level = level; }
    void zip(const char *infile, const char *outfile = NULL);
    void zip(int fileFd, GCallback *callback);
    int error() const { return _errno; }

private:
    void clear();
    void init(int fileFd);
    void deflate(int fileFd);
    void deflateFast(int fileFd);
    void putLong(uint32_t ui) { putShort(ui & 0xffff); putShort(ui >> 16); }
    void putShort(uint16_t us);
    void putByte(uint8_t uc);
    void updateHash(uint8_t uc);
    unsigned insertString(unsigned pos);
    unsigned longestMatch(unsigned hashHead);
    void fillWindow(int fileFd);
    void updateCrc(uint8_t *in, uint32_t len);
    ssize_t gzflush(const void *buf, size_t len, bool eof) { return write(_outfd, buf, len); }
    
    GTree *_gtree;
    GCallback *_callback;
    struct Config {
        uint16_t goodLength;    /* reduce lazy search above this match length */
        uint16_t maxLazy;       /* do not perform lazy search above this match length */
        uint16_t niceLength;    /* quit search above this match length */
        uint16_t maxChain;
    }_config;
    uint8_t *_window;
    uint8_t *_outbuf;
    uint8_t *_lbuf;
    uint16_t *_dbuf;
    uint16_t *_prev;
    unsigned _outcnt;
    unsigned _bytesIn;
    unsigned _strStart;
    unsigned _blkStart;
    unsigned _lookAhead;
    unsigned _insH;
    unsigned _prevLength;
    unsigned _matchStart;
    int _outfd;
    int _errno;
    uint32_t _crc;
    uint8_t _level: 4;
    uint8_t _eof: 1;
    uint8_t _reserve: 3;

    static Config _configTable[];
    static uint32_t _crcTable[];
    friend class GTree;
};

} /* namespace util */
#endif /* ifndef __UTIL_GZIP_H__ */
