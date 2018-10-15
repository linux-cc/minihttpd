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
    void setLevel(int level) {
        _level = level;
    }
    bool zip(const string &infile, const string &outfile = "");
    
    bool init(int infd, int outfd);
    void deflate() {
        _level < 4 ? deflateFast() : deflateHigh();
    }
    bool flushChunk();
    bool finish() {
        return finish(true);
    }

private:
    void deflateHigh();
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
    int readFile(void *buf, unsigned len);
    bool flushOutbuf();
    bool finish(bool isChunked);
    void updateCrc(uint8_t *in, uint32_t len);
    
    GTree *_gtree;
    struct Config {
        uint16_t goodLength;    /* reduce lazy search above this match length */
        uint16_t maxLazy;       /* do not perform lazy search above this match length */
        uint16_t niceLength;    /* quit search above this match length */
        uint16_t maxChain;
    }_config;
    struct Chunk {
        uint8_t *buf;
        uint8_t *pos;
        unsigned cnt;
        bool init;
        Chunk():buf(NULL), pos(NULL), cnt(0), init(false) {}
    }_chunk;
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
    unsigned _prevStart;
    unsigned _prevLength;
    unsigned _matchStart;
    unsigned _matchLength;
    int _infd;
    int _outfd;
    uint32_t _crc;
    uint8_t _level: 4;
    uint8_t _eof: 1;
    uint8_t _matchAvl: 1;
    uint8_t _reserve: 2;

    static Config _configTable[];
    static uint32_t _crcTable[];
    friend class GTree;
};

END_NS
#endif /* ifndef __HTTPD_GZIP_H__ */
