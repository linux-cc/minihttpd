#ifndef __HTTPD_GZIP_H__
#define __HTTPD_GZIP_H__

#include "config.h"
#include <string>

BEGIN_NS(httpd)

class GTree;
using std::string;

class GZip {
public:
    GZip();
    ~GZip();
    bool zip(const string &infile, const string &outfile = "");
    bool zip(const void *in, int inLen, void *out, int outLen);

private:
    bool init(const string &infile, const string &outfile);
    bool deflate();
    bool deflateFast();
    bool putLong(uint32_t ui) {
        return putShort(ui & 0xffff) && putShort(ui >> 16);
    }
    bool putShort(uint16_t us);
    bool putByte(uint8_t uc);
    void updateHash(uint8_t uc);
    unsigned insertString(unsigned pos);
    unsigned longestMatch(unsigned hashHead);
    void fillWindow();
    bool flushOutbuf();
    int readFile(void *buf, unsigned len);
    
    GTree *_gtree;
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
    uint8_t _level;
    bool _eof;

    static Config _configTable[10];
    friend class GTree;
};

END_NS
#endif /* ifndef __HTTPD_GZIP_H__ */
