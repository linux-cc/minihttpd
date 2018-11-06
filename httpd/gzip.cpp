#include "httpd/gzip.h"
#include "httpd/gtree.h"
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>

#define H_SHIFT             5
#define TOO_FAR             4096
#define GZIP_MAGIC          "\037\213"
#define HALF_WSIZE          (WSIZE >> 1)
#define TWO_WSIZE           (WSIZE << 1)
#define WMASK               (WSIZE-1)
#define _head               (_prev+WSIZE)
#define MIN_LOOKAHEAD       (MAX_MATCH+MIN_MATCH+1)
#define MAX_DIST            (WSIZE-MIN_LOOKAHEAD)

namespace httpd {

GZip::Config GZip::_configTable[] = {
    /*      good lazy nice chain */
    /* 0 */ {0,    0,  0,    0},  /* store only */
    /* 1 */ {4,    4,  8,    4},  /* maximum speed, no lazy matches */
    /* 2 */ {4,    5, 16,    8},
    /* 3 */ {4,    6, 32,   32},

    /* 4 */ {4,    4, 16,   16},  /* lazy matches */
    /* 5 */ {8,   16, 32,   32},
    /* 6 */ {8,   16, 128, 128},
    /* 7 */ {8,   32, 128, 256},
    /* 8 */ {32, 128, 258, 1024},
    /* 9 */ {32, 258, 258, 4096}  /* maximum compression */
};

inline void GZip::updateHash(uint8_t uc) {
    _insH = ((_insH << H_SHIFT) ^ (uc)) & WMASK;
}

inline unsigned GZip::insertString(unsigned pos) {
    _insH = ((_insH << H_SHIFT) ^ (_window[pos + MIN_MATCH - 1])) & WMASK,
    _prev[pos & WMASK] = _head[_insH], _head[_insH] = pos;

    return _prev[pos & WMASK];
}

GZip::GZip(GCallback *callback):
_outcnt(0),
_bytesIn(0),
_strStart(0),
_blkStart(0),
_insH(0),
_infd(-1),
_outfd(-1),
_crc(0),
_level(6),
_eof(0) {
    _gtree = new GTree(*this);
    _callback = callback ? callback : this;
    _window = new uint8_t[TWO_WSIZE];
    _lbuf = new uint8_t[WSIZE];
    _dbuf = new uint16_t[WSIZE];
    _prev = new uint16_t[TWO_WSIZE];
    _outbuf = new uint8_t[HALF_WSIZE];
}

GZip::~GZip() {
    delete _gtree;
    delete[] _window;
    delete[] _lbuf;
    delete[] _dbuf;
    delete[] _prev;
    delete[] _outbuf;
}

bool GZip::init() {
    updateCrc(NULL, 0);
    memset((char*)_head, 0, WSIZE * sizeof(*_head));
    _lookAhead = fill(_window, TWO_WSIZE);
    if (_lookAhead <= 0) {
        return false;
    }
    for (int i = 0; i < MIN_MATCH - 1; i++) {
        updateHash(_window[i]);
    }
    _gtree->init();

    return true;
}

bool GZip::init(const char *infile, const char *outfile) {
    struct stat sbuf;
    if (stat(infile, &sbuf) || !S_ISREG(sbuf.st_mode)) {
        return false;
    }
    _infd = open(infile, O_RDONLY);
    if (_infd < 0) {
        return false;
    }
    char _outfile[256];
    if (!outfile) {
        snprintf(_outfile, sizeof(_outfile), "%s.gz", infile);
    }
    _outfd = open(_outfile, O_WRONLY|O_TRUNC|O_CREAT, sbuf.st_mode);
    if (_outfd < 0) {
        return false;
    }

    return init();
}

void GZip::zip() {
    _config = _configTable[_level];
    putByte(GZIP_MAGIC[0]);
    putByte(GZIP_MAGIC[1]);
    putByte(8); /* compression method */
    putByte(0); /* general flags */
    putLong(0); /* no timestamp */
    putByte(0); /* extra flags */
    putByte(3); /* os code assume Unix */
    _level < 4 ? deflateFast() : deflate();
    putLong(_crc);
    putLong(_bytesIn);
    _callback->gzflush(_outbuf, _outcnt, true);
}

void GZip::deflate() {
    unsigned prevMatch;
    unsigned matchLength = MIN_MATCH - 1;
    bool matchAvailable = false;
    
    while (_lookAhead) {
        unsigned hashHead = insertString(_strStart);
        _prevLength = matchLength, prevMatch = _matchStart;
        if (hashHead && _prevLength < _config.maxLazy && _strStart - hashHead <= MAX_DIST) {
            matchLength = longestMatch(hashHead);
            if (matchLength > _lookAhead) {
                matchLength = _lookAhead;
            }
            if (matchLength == MIN_MATCH && _strStart - _matchStart > TOO_FAR) {
                matchLength--;
            }
        }
        if (_prevLength >= MIN_MATCH && matchLength <= _prevLength) {
            bool flush = _gtree->tally(_strStart - 1 - prevMatch, _prevLength - MIN_MATCH);
            _lookAhead -= _prevLength - 1;
            _prevLength -= 2;
            while (_prevLength--) {
                hashHead = insertString(++_strStart);
            }
            matchAvailable = false;
            matchLength = MIN_MATCH - 1;
            _strStart++;
            if (flush) {
                _gtree->flushBlock();
                _blkStart = _strStart;
            }
        } else if (matchAvailable) {
            if (_gtree->tally(0, _window[_strStart - 1])) {
                _gtree->flushBlock();
                _blkStart = _strStart;
            }
            _strStart++;
            _lookAhead--;
        } else {
            matchAvailable = true;
            _strStart++;
            _lookAhead--;
        }
        if (_lookAhead < MIN_LOOKAHEAD && !_eof) {
            fillWindow();
        }
    }
    if (matchAvailable) {
        _gtree->tally(0, _window[_strStart-1]);
    }

    _gtree->flushBlock(1);
}

void GZip::deflateFast() {
    unsigned matchLength = 0;
    _prevLength = MIN_MATCH - 1;
    
    while (_lookAhead) {
        bool flush;
        unsigned hashHead = insertString(_strStart);
        if (hashHead && _strStart - hashHead <= MAX_DIST) {
            matchLength = longestMatch(hashHead);
            if (matchLength > _lookAhead) {
                matchLength = _lookAhead;
            }
        }
        if (matchLength >= MIN_MATCH) {
            flush = _gtree->tally(_strStart - _matchStart, matchLength - MIN_MATCH);
            _lookAhead -= matchLength;
            if (matchLength <= _config.maxLazy) {
                matchLength--;
                do {
                    hashHead = insertString(++_strStart);
                } while(--matchLength);
                _strStart++;
            } else {
                _strStart += matchLength;
                matchLength = 0;
                _insH = _window[_strStart];
                updateHash(_window[_strStart + 1]);
            }
        } else {
            flush = _gtree->tally(0, _window[_strStart]);
            _lookAhead--;
            _strStart++;
        }
        if (flush) {
            _gtree->flushBlock();
            _blkStart = _strStart;
        }
        if (_lookAhead < MIN_LOOKAHEAD && !_eof) {
            fillWindow();
        }
    }

    _gtree->flushBlock(1);
}

unsigned GZip::longestMatch(unsigned hashHead) {
    unsigned bestLen = _prevLength;
    unsigned chainLength = _config.maxChain;
    unsigned limit = _strStart > MAX_DIST ? _strStart - MAX_DIST : 0;

    if (bestLen >= _config.goodLength) {
        chainLength >>= 2;
    }
    do {
        uint8_t *scan = _window + _strStart;
        uint8_t *scanEnd = scan + MAX_MATCH;
        uint8_t *match = _window + hashHead;

        if (match[bestLen] != scan[bestLen] ||
            match[bestLen-1] != scan[bestLen-1] ||
            match[0] != scan[0] ||
            match[1] != scan[1]) {
            continue;
        }
        scan += 2, match += 2;
        while (*++scan == *++match && *++scan == *++match &&
               *++scan == *++match && *++scan == *++match &&
               *++scan == *++match && *++scan == *++match &&
               *++scan == *++match && *++scan == *++match &&
               scan < scanEnd);
        unsigned len = MAX_MATCH - (scanEnd - scan);
        if (len > bestLen) {
            _matchStart = hashHead;
            bestLen = len;
            if (bestLen >= _config.niceLength) {
                break;
            }
        }
    } while ((hashHead = _prev[hashHead & WMASK]) > limit && --chainLength != 0);

    return bestLen;
}

void GZip::fillWindow() {
    unsigned more = TWO_WSIZE - _lookAhead - _strStart;

    if (_strStart >= WSIZE + MAX_DIST) {
        memcpy(_window, _window + WSIZE, WSIZE);
        _matchStart -= WSIZE;
        _strStart -= WSIZE;
        _blkStart -= WSIZE;

        for (int i = 0; i < WSIZE; i++) {
            unsigned n = _head[i];
            _head[i] = (n >= WSIZE ? n - WSIZE : 0);
        }
        for (int i = 0; i < WSIZE; i++) {
            unsigned n = _prev[i];
            _prev[i] = (n >= WSIZE ? n - WSIZE : 0);
        }
        more += WSIZE;
    }

    if (!_eof) {
        int n = fill(_window + _strStart + _lookAhead, more);
        if (n <= 0) {
            _eof = true;
        } else {
            _lookAhead += n;
        }
    }
}

int GZip::fill(void *buf, unsigned len) {
    int n = _callback->gzfill(buf, len);
    if (n > 0) {
        updateCrc((uint8_t*)buf, n);
        _bytesIn += n;
    }

    return n;
}

void GZip::putShort(uint16_t us) {
    /* Output a 16 bit value, lsb first */
    if (_outcnt < HALF_WSIZE - 2) {
        _outbuf[_outcnt++] = us & 0xff;
        _outbuf[_outcnt++] = us >> 8;
    } else {
        putByte(us & 0xff);
        putByte(us >> 8);
    }
}

void GZip::putByte(uint8_t uc) {
    _outbuf[_outcnt++] = uc;
    if (_outcnt == HALF_WSIZE) {
        if (!_callback->gzflush(_outbuf, _outcnt, false)) {
            abort();
        }
        _outcnt = 0;
    }
}

bool GZip::gzflush(const void *buf, int len, bool eof) {
    eof = false;
    const char *pos = (const char*)buf;
    while (len) {
        int n = write(_outfd, pos, len);
        if (n < 0) {
            return false;
        }
        len -= n;
        pos += n;
    }

    return true;
}

void GZip::updateCrc(uint8_t *in, uint32_t len) {
    uint32_t mask = (uint32_t)-1;
    static uint32_t crc = mask;
    uint32_t c = mask;

    if (in) {
        c = crc;
        while (len--) {
            c = _crcTable[((int)c ^ (*in++)) & 0xff] ^ (c >> 8);
        }
    }
    crc = c;
    _crc = c ^ mask;
}

uint32_t GZip::_crcTable[] = {
  0x00000000L, 0x77073096L, 0xee0e612cL, 0x990951baL, 0x076dc419L, 0x706af48fL, 0xe963a535L, 0x9e6495a3L,
  0x0edb8832L, 0x79dcb8a4L, 0xe0d5e91eL, 0x97d2d988L, 0x09b64c2bL, 0x7eb17cbdL, 0xe7b82d07L, 0x90bf1d91L,
  0x1db71064L, 0x6ab020f2L, 0xf3b97148L, 0x84be41deL, 0x1adad47dL, 0x6ddde4ebL, 0xf4d4b551L, 0x83d385c7L,
  0x136c9856L, 0x646ba8c0L, 0xfd62f97aL, 0x8a65c9ecL, 0x14015c4fL, 0x63066cd9L, 0xfa0f3d63L, 0x8d080df5L,
  0x3b6e20c8L, 0x4c69105eL, 0xd56041e4L, 0xa2677172L, 0x3c03e4d1L, 0x4b04d447L, 0xd20d85fdL, 0xa50ab56bL,
  0x35b5a8faL, 0x42b2986cL, 0xdbbbc9d6L, 0xacbcf940L, 0x32d86ce3L, 0x45df5c75L, 0xdcd60dcfL, 0xabd13d59L,
  0x26d930acL, 0x51de003aL, 0xc8d75180L, 0xbfd06116L, 0x21b4f4b5L, 0x56b3c423L, 0xcfba9599L, 0xb8bda50fL,
  0x2802b89eL, 0x5f058808L, 0xc60cd9b2L, 0xb10be924L, 0x2f6f7c87L, 0x58684c11L, 0xc1611dabL, 0xb6662d3dL,
  0x76dc4190L, 0x01db7106L, 0x98d220bcL, 0xefd5102aL, 0x71b18589L, 0x06b6b51fL, 0x9fbfe4a5L, 0xe8b8d433L,
  0x7807c9a2L, 0x0f00f934L, 0x9609a88eL, 0xe10e9818L, 0x7f6a0dbbL, 0x086d3d2dL, 0x91646c97L, 0xe6635c01L,
  0x6b6b51f4L, 0x1c6c6162L, 0x856530d8L, 0xf262004eL, 0x6c0695edL, 0x1b01a57bL, 0x8208f4c1L, 0xf50fc457L,
  0x65b0d9c6L, 0x12b7e950L, 0x8bbeb8eaL, 0xfcb9887cL, 0x62dd1ddfL, 0x15da2d49L, 0x8cd37cf3L, 0xfbd44c65L,
  0x4db26158L, 0x3ab551ceL, 0xa3bc0074L, 0xd4bb30e2L, 0x4adfa541L, 0x3dd895d7L, 0xa4d1c46dL, 0xd3d6f4fbL,
  0x4369e96aL, 0x346ed9fcL, 0xad678846L, 0xda60b8d0L, 0x44042d73L, 0x33031de5L, 0xaa0a4c5fL, 0xdd0d7cc9L,
  0x5005713cL, 0x270241aaL, 0xbe0b1010L, 0xc90c2086L, 0x5768b525L, 0x206f85b3L, 0xb966d409L, 0xce61e49fL,
  0x5edef90eL, 0x29d9c998L, 0xb0d09822L, 0xc7d7a8b4L, 0x59b33d17L, 0x2eb40d81L, 0xb7bd5c3bL, 0xc0ba6cadL,
  0xedb88320L, 0x9abfb3b6L, 0x03b6e20cL, 0x74b1d29aL, 0xead54739L, 0x9dd277afL, 0x04db2615L, 0x73dc1683L,
  0xe3630b12L, 0x94643b84L, 0x0d6d6a3eL, 0x7a6a5aa8L, 0xe40ecf0bL, 0x9309ff9dL, 0x0a00ae27L, 0x7d079eb1L,
  0xf00f9344L, 0x8708a3d2L, 0x1e01f268L, 0x6906c2feL, 0xf762575dL, 0x806567cbL, 0x196c3671L, 0x6e6b06e7L,
  0xfed41b76L, 0x89d32be0L, 0x10da7a5aL, 0x67dd4accL, 0xf9b9df6fL, 0x8ebeeff9L, 0x17b7be43L, 0x60b08ed5L,
  0xd6d6a3e8L, 0xa1d1937eL, 0x38d8c2c4L, 0x4fdff252L, 0xd1bb67f1L, 0xa6bc5767L, 0x3fb506ddL, 0x48b2364bL,
  0xd80d2bdaL, 0xaf0a1b4cL, 0x36034af6L, 0x41047a60L, 0xdf60efc3L, 0xa867df55L, 0x316e8eefL, 0x4669be79L,
  0xcb61b38cL, 0xbc66831aL, 0x256fd2a0L, 0x5268e236L, 0xcc0c7795L, 0xbb0b4703L, 0x220216b9L, 0x5505262fL,
  0xc5ba3bbeL, 0xb2bd0b28L, 0x2bb45a92L, 0x5cb36a04L, 0xc2d7ffa7L, 0xb5d0cf31L, 0x2cd99e8bL, 0x5bdeae1dL,
  0x9b64c2b0L, 0xec63f226L, 0x756aa39cL, 0x026d930aL, 0x9c0906a9L, 0xeb0e363fL, 0x72076785L, 0x05005713L,
  0x95bf4a82L, 0xe2b87a14L, 0x7bb12baeL, 0x0cb61b38L, 0x92d28e9bL, 0xe5d5be0dL, 0x7cdcefb7L, 0x0bdbdf21L,
  0x86d3d2d4L, 0xf1d4e242L, 0x68ddb3f8L, 0x1fda836eL, 0x81be16cdL, 0xf6b9265bL, 0x6fb077e1L, 0x18b74777L,
  0x88085ae6L, 0xff0f6a70L, 0x66063bcaL, 0x11010b5cL, 0x8f659effL, 0xf862ae69L, 0x616bffd3L, 0x166ccf45L,
  0xa00ae278L, 0xd70dd2eeL, 0x4e048354L, 0x3903b3c2L, 0xa7672661L, 0xd06016f7L, 0x4969474dL, 0x3e6e77dbL,
  0xaed16a4aL, 0xd9d65adcL, 0x40df0b66L, 0x37d83bf0L, 0xa9bcae53L, 0xdebb9ec5L, 0x47b2cf7fL, 0x30b5ffe9L,
  0xbdbdf21cL, 0xcabac28aL, 0x53b39330L, 0x24b4a3a6L, 0xbad03605L, 0xcdd70693L, 0x54de5729L, 0x23d967bfL,
  0xb3667a2eL, 0xc4614ab8L, 0x5d681b02L, 0x2a6f2b94L, 0xb40bbe37L, 0xc30c8ea1L, 0x5a05df1bL, 0x2d02ef8dL
};

} /* namespace httpd */

