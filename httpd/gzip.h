#ifndef __HTTPD_GZIP_H__
#define __HTTPD_GZIP_H__

#include "config.h"
#include <unistd.h>

BEGIN_NS(httpd)

class GTree;

class GCallback {
public:
    virtual ~GCallback() {}
    virtual int gzfill(void *buf, int len) = 0;
    virtual bool gzflush(const void *buf, int len, bool eof) = 0;
};

class GZip : public GCallback {
public:
    GZip(GCallback *callback = NULL);
    ~GZip();
    void setLevel(int level) {
        _level = level;
    }
    bool init(const char *infile, const char *outfile = NULL);
    bool init();
    void zip();

private:
    void deflate();
    void deflateFast();
    void putLong(uint32_t ui) {
        putShort(ui & 0xffff), putShort(ui >> 16);
    }
    void putShort(uint16_t us);
    void putByte(uint8_t uc);
    void updateHash(uint8_t uc);
    unsigned insertString(unsigned pos);
    unsigned longestMatch(unsigned hashHead);
    void fillWindow();
    int fill(void *buf, unsigned len);
    void updateCrc(uint8_t *in, uint32_t len);
    int gzfill(void *buf, int len) {
        return read(_infd, buf, len);
    }
    bool gzflush(const void *buf, int len, bool eof);
    
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
    int _infd;
    int _outfd;
    uint32_t _crc;
    uint8_t _level: 4;
    uint8_t _eof: 1;
    uint8_t _reserve: 3;

    static Config _configTable[];
    static uint32_t _crcTable[];
    friend class GTree;
};

END_NS
#endif /* ifndef __HTTPD_GZIP_H__ */
