#include "httpd/gtree.h"
#include "httpd/gzip.h"

#define _freq           _fc.freq
#define _code           _fc.code
#define _dad            _dl.dad
#define _len            _dl.len

#define CHAR_BITS       __CHAR_BIT__
#define MAX_BITS        15
#define MAX_BL_BITS     7
#define LENGTH_CODES    29
#define LITERALS        256
#define END_BLOCK       256
#define D_CODES         30
#define BL_CODES        19
#define REP_3_6         16
#define REPZ_3_10       17
#define REPZ_11_138     18
#define L_CODES         (LITERALS + LENGTH_CODES + 1)
#define D_CODE(d)       ((d) < LITERALS ? _dcode[d] : _dcode[LITERALS+((d)>>7)])
#define HEAP_SIZE       (2 * L_CODES + 1)
#define SMALLEST        1
#define DYN_TREES       2

BEGIN_NS(httpd)

static uint8_t blOrder[BL_CODES] = {16,17,18,0,8,7,9,6,10,5,11,4,12,3,13,2,14,1,15};
static uint8_t extraLBits[LENGTH_CODES] = {0,0,0,0,0,0,0,0,1,1,1,1,2,2,2,2,3,3,3,3,4,4,4,4,5,5,5,5,0};
static uint8_t extraDBits[D_CODES] = {0,0,0,0,1,1,2,2,3,3,4,4,5,5,6,6,7,7,8,8,9,9,10,10,11,11,12,12,13,13};
static uint8_t extraBLBits[BL_CODES] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,3,7};

GTree::GTree(GZip &gzip):
_gzip(gzip) {
    initTreeDesc();
    _heap = new int[HEAP_SIZE];
    _depth = new uint8_t[HEAP_SIZE];
    _blCount = new uint16_t[MAX_BITS + 1];
    _lcode = new uint8_t[MAX_MATCH-MIN_MATCH+1];
    _dcode = new uint8_t[2 * LITERALS];
    _baseL = new int[LENGTH_CODES];
    _baseD = new int[D_CODES];
    _flagBuf = new uint8_t[WSIZE/CHAR_BITS];
    _gbit = new GBit(_gzip);
}

GTree::~GTree() {
    delete[] _lDesc._tree;
    delete[] _dDesc._tree;
    delete[] _blDesc._tree;
    delete[] _heap;
    delete[] _depth;
    delete[] _blCount;
    delete[] _lcode;
    delete[] _dcode;
    delete[] _baseL;
    delete[] _baseD;
    delete[] _flagBuf;
    delete _gbit;
}

void GTree::initTreeDesc() {
    _lDesc._tree = new Tree[2*L_CODES+1];
    _lDesc._extraBits = extraLBits;
    _lDesc._extraBase = LITERALS+1;
    _lDesc._elems = L_CODES;
    _lDesc._maxLength = MAX_BITS;
    _lDesc._maxCode = 0;

    _dDesc._tree = new Tree[2*D_CODES+1];
    _dDesc._extraBits = extraDBits;
    _dDesc._extraBase = 0;
    _dDesc._elems = D_CODES;
    _dDesc._maxLength = MAX_BITS;
    _dDesc._maxCode = 0;

    _blDesc._tree = new Tree[2*BL_CODES+1];
    _blDesc._extraBits = extraBLBits;
    _blDesc._extraBase = 0;
    _blDesc._elems = BL_CODES;
    _blDesc._maxLength = MAX_BL_BITS;
    _blDesc._maxCode = 0;
}

void GTree::init() {
    initLengthCode();
    initDistCode();

    for (int bits = 0; bits <= MAX_BITS; bits++) {
        _blCount[bits] = 0;
    }

    initBlock();
}

void GTree::initLengthCode() {
    int length = 0, code;
    for (code = 0; code < LENGTH_CODES-1; code++) {
        _baseL[code] = length;
        for (int n = 0; n < (1<<extraLBits[code]); n++) {
            _lcode[length++] = code;
        }
    }
    _lcode[length-1] = code;
}

void GTree::initDistCode() {
    int dist = 0, code;
    for (code = 0 ; code < 16; code++) {
        _baseD[code] = dist;
        for (int n = 0; n < (1<<extraDBits[code]); n++) {
            _dcode[dist++] = code;
        }
    }
    dist >>= 7;
    for ( ; code < D_CODES; code++) {
        _baseD[code] = dist << 7;
        for (int n = 0; n < (1<<(extraDBits[code]-7)); n++) {
            _dcode[LITERALS + dist++] = code;
        }
    }
}

void GTree::genCodes(Tree *tree, int maxCode) {
    uint16_t nextCode[MAX_BITS+1];
    uint16_t code = 0;

    for (int bits = 1; bits <= MAX_BITS; bits++) {
        nextCode[bits] = code = (code + _blCount[bits-1]) << 1;
    }

    for (int n = 0;  n <= maxCode; n++) {
        int len = tree[n]._len;
        if (len == 0) continue;
        tree[n]._code = _gbit->reverseBits(nextCode[len]++, len);
    }
}

void GTree::initBlock() {
    int n;

    for (n = 0; n < L_CODES; n++) {
        _lDesc._tree[n]._freq = 0;
    }
    for (n = 0; n < D_CODES; n++) {
        _dDesc._tree[n]._freq = 0;
    }
    for (n = 0; n < BL_CODES; n++) {
        _blDesc._tree[n]._freq = 0;
    }

    _lDesc._tree[END_BLOCK]._freq = 1;
    _lastL = _lastD = _lastF = 0;
    _flags = 0;
    _flagBit = 1;
}

bool GTree::tally(unsigned dist, unsigned lc) {
    _gzip._lbuf[_lastL++] = lc;
    
    if (dist == 0) {
        _lDesc._tree[lc]._freq++;
    } else {
        dist--;
        _lDesc._tree[_lcode[lc]+LITERALS+1]._freq++;
        _dDesc._tree[D_CODE(dist)]._freq++;
        _gzip._dbuf[_lastD++] = dist;
        _flags |= _flagBit;
    }
    _flagBit <<= 1;

    if ((_lastL & 7) == 0) {
        _flagBuf[_lastF++] = _flags;
        _flags = 0, _flagBit = 1;
    }
    if (_gzip._level > 2 && (_lastL & 0xfff) == 0) {
        unsigned outLength = _lastL * CHAR_BITS;
        unsigned inLength = _gzip._strStart - _gzip._blkStart;
        for (int code = 0; code < D_CODES; code++) {
            outLength += _dDesc._tree[code]._freq * (5 + extraDBits[code]);
        }
        outLength >>= 3;
        if (_lastD < _lastL/2 && outLength < inLength/2) {
            return true;
        }
    }
    return (_lastL == WSIZE || _lastD == WSIZE);
}

void GTree::flushBlock(int eof) {
    _flagBuf[_lastF] = _flags;
    
    build(_lDesc);
    build(_dDesc);

    int maxBlIndex = buildBl();
    _gbit->sendBits((DYN_TREES<<1)+eof, 3);
    sendAllTrees(_lDesc._maxCode + 1, _dDesc._maxCode + 1, maxBlIndex + 1);
    compressBlock(_lDesc._tree, _dDesc._tree);

    initBlock();
    if (eof) {
        _gbit->winDup();
    }
}

void GTree::build(TreeDesc &desc) {
    Tree *tree = desc._tree;
    int node = desc._elems;
    desc._maxCode = buildHeap(tree, desc._elems);

    do {
        int n = headPop(tree);
        int m = _heap[SMALLEST];

        _heap[--_heapMax] = n;
        _heap[--_heapMax] = m;
        tree[node]._freq = tree[n]._freq + tree[m]._freq;
        _depth[node] = MAX(_depth[n], _depth[m]) + 1;
        tree[n]._dad = tree[m]._dad = node;
        _heap[SMALLEST] = node++;
        heapDown(tree, SMALLEST);
    } while (_heapLen > SMALLEST);
    _heap[--_heapMax] = _heap[SMALLEST];
    genBitLen(desc);
    genCodes(tree, desc._maxCode);
}

int GTree::buildHeap(Tree *tree, int elems) {
    _heapLen = 0;
    _heapMax = HEAP_SIZE;
    int maxCode = -1;

    for (int n = 0; n < elems; n++) {
        if (tree[n]._freq) {
            _heap[++_heapLen] = maxCode = n;
            _depth[n] = 0;
        } else {
            tree[n]._len = 0;
        }
    }

    while (_heapLen < 2) {
        int n = _heap[++_heapLen] = maxCode < 2 ? ++maxCode : 0;
        tree[n]._freq = 1;
        _depth[n] = 0;
    }

    for (int n = _heapLen/2; n >= 1; n--) {
        heapDown(tree, n);
    }

    return maxCode;
}

void GTree::heapDown(Tree *tree, int dad) {
#define smaller(t, n, m)    (t[n]._freq < t[m]._freq || (t[n]._freq == t[m]._freq && _depth[n] <= _depth[m]))
    
    int dv = _heap[dad];
    int son = dad << 1;

    while (son <= _heapLen) {
        if (son < _heapLen && smaller(tree, _heap[son+1], _heap[son])) {
            son++;
        }
        if (smaller(tree, dv, _heap[son])) {
            break;
        }
        _heap[dad] = _heap[son];
        dad = son;
        son <<= 1;
    }

    _heap[dad] = dv;

#undef smaller
}

int GTree::headPop(Tree *tree) {
    int result = _heap[SMALLEST];
    _heap[SMALLEST] = _heap[_heapLen--];
    heapDown(tree, SMALLEST);

    return result;
}

void GTree::genBitLen(TreeDesc &desc) {
    for (int bits = 0; bits <= MAX_BITS; bits++) {
        _blCount[bits] = 0;
    }

    int overflow = 0, h;
    Tree *tree = desc._tree;
    int maxLength = desc._maxLength;
    int maxCode = desc._maxCode;

    tree[_heap[_heapMax]]._len = 0;
    for (h = _heapMax + 1; h < HEAP_SIZE; h++) {
        int n = _heap[h];
        int bits = tree[tree[n]._dad]._len + 1;
        if (bits > maxLength) {
            bits = maxLength;
            overflow++;
        }
        tree[n]._len = bits;
        if (n > maxCode) {
            continue;
        }
        _blCount[bits]++;
    }

    if (overflow == 0) {
        return;
    }
    ajustBitLen(desc, overflow, h);
}

void GTree::ajustBitLen(TreeDesc &desc, int overflow, int h) {
    int maxLength = desc._maxLength;
    int maxCode = desc._maxCode;
    Tree *tree = desc._tree;

    do {
        int bits = maxLength - 1;
        while (_blCount[bits] == 0) bits--;
        _blCount[bits]--;
        _blCount[bits+1] += 2;
        _blCount[maxLength]--;
        overflow -= 2;
    } while (overflow > 0);

    for (int bits = maxLength; bits != 0; bits--) {
        int n = _blCount[bits];
        while (n != 0) {
            int m = _heap[--h];
            if (m > maxCode) continue;
            if (tree[m]._len != (unsigned) bits) {
                tree[m]._len = bits;
            }
            n--;
        }
    }
}

int GTree::buildBl() {
    scanTree(_lDesc._tree, _lDesc._maxCode);
    scanTree(_dDesc._tree, _dDesc._maxCode);
    build(_blDesc);

    int maxBlIndex;
    for (maxBlIndex = BL_CODES-1; maxBlIndex >= 3; maxBlIndex--) {
        if (_blDesc._tree[blOrder[maxBlIndex]]._len != 0) {
            break;
        }
    }

    return maxBlIndex;
}

void GTree::scanTree(Tree *tree, int maxCode) {
    int prevLen = -1, curLen, nextLen = tree[0]._len;
    int count = 0, minCount = 4, maxCount = 7;

    if (nextLen == 0) {
        minCount = 3, maxCount = 138;
    }
    tree[maxCode+1]._len = 0xffff;

    for (int n = 0; n <= maxCode; n++) {
        curLen = nextLen;
        nextLen = tree[n+1]._len;
        if (++count < maxCount && curLen == nextLen) {
            continue;
        } else if (count < minCount) {
            _blDesc._tree[curLen]._freq += count;
        } else if (curLen != 0) {
            if (curLen != prevLen) {
                _blDesc._tree[curLen]._freq++;
            }
            _blDesc._tree[REP_3_6]._freq++;
        } else if (count <= 10) {
            _blDesc._tree[REPZ_3_10]._freq++;
        } else {
            _blDesc._tree[REPZ_11_138]._freq++;
        }
        count = 0, prevLen = curLen;
        if (nextLen == 0) {
            minCount = 3, maxCount = 138;
        } else if (curLen == nextLen) {
            minCount = 3, maxCount = 6;
        } else {
            minCount = 3, maxCount = 7;
        }
    }
}

void GTree::compressBlock(Tree *ltree, Tree *dtree) {
    unsigned li = 0, di = 0, fi = 0;
    uint8_t flag = 0;

    if (_lastL) do {
        if ((li & 7) == 0) {
            flag = _flagBuf[fi++];
        }
        int lc = _gzip._lbuf[li++];
        if ((flag & 1) == 0) {/* send a literal byte */
            _gbit->sendCode(lc, ltree);
        } else {/* Here, lc is the match length - MIN_MATCH */
            unsigned code = _lcode[lc];
            _gbit->sendCode(code+LITERALS+1, ltree); /* send the length code */
            int extra = extraLBits[code];
            if (extra) {
                lc -= _baseL[code];
                _gbit->sendBits(lc, extra); /* send the extra length bits */
            }
            unsigned dist = _gzip._dbuf[di++];
            code = D_CODE(dist); /* Here, dist is the match distance - 1 */
            _gbit->sendCode(code, dtree); /* send the distance code */
            extra = extraDBits[code];
            if (extra) {
                dist -= _baseD[code];
                _gbit->sendBits(dist, extra); /* send the extra distance bits */
            }
        }
        flag >>= 1;
    } while (li < _lastL);
    _gbit->sendCode(END_BLOCK, ltree);
}

void GTree::sendAllTrees(int lcodes, int dcodes, int blcodes) {
    _gbit->sendBits(lcodes - 257, 5);
    _gbit->sendBits(dcodes - 1, 5);
    _gbit->sendBits(blcodes - 4, 4);

    for (int rank = 0; rank < blcodes; rank++) {
        _gbit->sendBits(_blDesc._tree[blOrder[rank]]._len, 3);
    }
    sendTree(_lDesc._tree, lcodes - 1); /* send the literal tree */
    sendTree(_dDesc._tree, dcodes - 1); /* send the distance tree */
}

void GTree::sendTree(Tree *tree, int maxCode) {
    int prevLen = -1, curLen = 0, nextLen = tree[0]._len;
    int count = 0, minCount = 4, maxCount = 7;

    if (nextLen == 0) {
        minCount = 3, maxCount = 138;
    }
    for (int n = 0; n <= maxCode; n++) {
        curLen = nextLen;
        nextLen = tree[n+1]._len;
        if (++count < maxCount && curLen == nextLen) {
            continue;
        } else if (count < minCount) {
            do {
                _gbit->sendCode(curLen, _blDesc._tree);
            } while (--count);
        } else if (curLen) {
            if (curLen != prevLen) {
                _gbit->sendCode(curLen, _blDesc._tree);
                count--;
            }
            _gbit->sendCode(REP_3_6, _blDesc._tree);
            _gbit->sendBits(count - 3, 2);
        } else if (count <= 10) {
            _gbit->sendCode(REPZ_3_10, _blDesc._tree);
            _gbit->sendBits(count - 3, 3);
        } else {
            _gbit->sendCode(REPZ_11_138, _blDesc._tree);
            _gbit->sendBits(count - 11, 7);
        }
        count = 0, prevLen = curLen;
        if (nextLen == 0) {
            minCount = 3, maxCount = 138;
        } else if (curLen == nextLen) {
            minCount = 3, maxCount = 6;
        } else {
            minCount = 4, maxCount = 7;
        }
    }
}

GTree::GBit::GBit(GZip &gzip):
_gzip(gzip),
_buf(0),
_size(CHAR_BITS * sizeof(uint16_t)),
_valid(0) {
}

void GTree::GBit::sendCode(int idx, Tree *tree) {
    sendBits(tree[idx]._code, tree[idx]._len);
}

void GTree::GBit::sendBits(int value, int length) {
    _buf |= value << _valid;
    if (_valid + length > _size) {
        _gzip.putShort(_buf);
        _buf = (uint16_t)value >> (_size - _valid);
        length -= _size;
    }
    _valid += length;
}

unsigned GTree::GBit::reverseBits(unsigned value, int length) {
    unsigned result = 0;
    while (length--) {
        result |= value & 1;
        value >>= 1;
        result <<= 1;
    }
    return result >> 1;
}

void GTree::GBit::winDup() {
    if (_valid > CHAR_BITS) {
        _gzip.putShort(_buf);
    } else if (_valid > 0) {
        _gzip.putByte(_buf);
    }
    _buf = 0;
    _valid = 0;
}

END_NS
