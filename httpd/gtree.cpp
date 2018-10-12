#include "httpd/gtree.h"
#include "httpd/gzip.h"

#define _freq           _fc.freq
#define _code           _fc.code
#define _dad            _dl.dad
#define _len            _dl.len

#define _dLTree         _lDesc._dynamic
#define _dDTree         _dDesc._dynamic
#define _sLTree         _lDesc._static
#define _sDTree         _dDesc._static
#define _blTree         _blDesc._dynamic

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

#define STORED_BLOCK    0
#define STATIC_TREES    1
#define DYN_TREES       2

BEGIN_NS(httpd)

static uint8_t blOrder[BL_CODES] = {16,17,18,0,8,7,9,6,10,5,11,4,12,3,13,2,14,1,15};
static uint8_t extraLBits[LENGTH_CODES] = {0,0,0,0,0,0,0,0,1,1,1,1,2,2,2,2,3,3,3,3,4,4,4,4,5,5,5,5,0};
static uint8_t extraDBits[D_CODES] = {0,0,0,0,1,1,2,2,3,3,4,4,5,5,6,6,7,7,8,8,9,9,10,10,11,11,12,12,13,13};
static uint8_t extraBLBits[BL_CODES] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,3,7};
static unsigned optimalLen, staticLen;

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
    _gbit = new GBit;
}

GTree::~GTree() {
    delete[] _dLTree;
    delete[] _sLTree;
    delete[] _dDTree;
    delete[] _sDTree;
    delete[] _blTree;
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
    _dLTree = new Tree[2*L_CODES+1];
    _sLTree = new Tree[L_CODES+2];
    _lDesc._extraBits = extraLBits;
    _lDesc._extraBase = LITERALS+1;
    _lDesc._elems = LENGTH_CODES;
    _lDesc._maxLength = MAX_BITS;
    _lDesc._maxCode = 0;

    _dDTree = new Tree[2*D_CODES+1];
    _sDTree = new Tree[D_CODES];
    _dDesc._extraBits = extraDBits;
    _dDesc._extraBase = 0;
    _dDesc._elems = D_CODES;
    _dDesc._maxLength = MAX_BITS;
    _dDesc._maxCode = 0;

    _blTree = new Tree[2*BL_CODES+1];
    _blDesc._static = NULL;
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

    initStatic();
    genCodes(_sLTree, L_CODES + 1);
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

void GTree::initStatic() {
    int n = 0;
    while (n <= 143) {
        _sLTree[n++]._len = 8;
        _blCount[8]++;
    }
    while (n <= 255) {
        _sLTree[n++]._len = 9;
        _blCount[9]++;
    }
    while (n <= 279) {
        _sLTree[n++]._len = 7;
        _blCount[7]++;
    }
    while (n <= 287) {
        _sLTree[n++]._len = 8;
        _blCount[8]++;
    }

    for (n = 0; n < D_CODES; n++) {
        _sDTree[n]._len = 5;
        _sDTree[n]._code = _gbit->reverseBits(n, 5);
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
        _dLTree[n]._freq = 0;
    }
    for (n = 0; n < D_CODES; n++) {
        _dDTree[n]._freq = 0;
    }
    for (n = 0; n < BL_CODES; n++) {
        _blTree[n]._freq = 0;
    }

    _dLTree[END_BLOCK]._freq = 1;
    optimalLen = staticLen = 0;
    _lastL = _lastD = _lastF = 0;
    _flags = 0;
    _flagBit = 1;
}

bool GTree::tally(unsigned dist, unsigned lc) {
    _gzip._lbuf[_lastL++] = lc;
    
    if (dist == 0) {
        _dLTree[lc]._freq++;
    } else {
        dist--;
        _dLTree[_lcode[lc]+LITERALS+1]._freq++;
        _dDTree[D_CODE(dist)]._freq++;
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
            outLength += _dDTree[code]._freq * (5 + extraDBits[code]);
        }
        outLength >>= 3;
        if (_lastD < _lastL/2 && outLength < inLength/2) {
            return true;
        }
    }
    return (_lastL == WSIZE - 1 || _lastD == WSIZE);
}

bool GTree::flushBlock(int eof) {
    _flagBuf[_lastF] = _flags;
    
    build(_lDesc);
    build(_dDesc);

    int maxBlIndex = buildBl();
    int optimalLenB = (optimalLen+3+7)>>3;
    int staticLenB = (staticLen+3+7)>>3;
    if (staticLenB <= optimalLenB) {
        _gbit->sendBits((STATIC_TREES<<1)+eof, 3);
        compressBlock(_sLTree, _sDTree);
    } else {
        _gbit->sendBits((DYN_TREES<<1)+eof, 3);
        sendAllTrees(_lDesc._maxCode + 1, _dDesc._maxCode + 1, maxBlIndex + 1);
        compressBlock(_dLTree, _dDTree);
    }

    initBlock();
    if (eof) {
        return _gbit->winDup(_gzip);
    }
    return true;
}

void GTree::build(TreeDesc &desc) {
    Tree *dt = desc._dynamic;
    Tree *st = desc._static;
    desc._maxCode = buildHeap(dt, st, desc._elems);

    do {
        int n = headPop(dt);
        int m = _heap[SMALLEST];
        int node = desc._elems;

        _heap[--_heapMax] = n;
        _heap[--_heapMax] = m;
        dt[node]._freq = dt[n]._freq + dt[m]._freq;
        _depth[node] = MAX(_depth[n], _depth[m]) + 1;
        dt[n]._dad = dt[m]._dad = node;
        _heap[SMALLEST] = node++;
        heapDown(dt, SMALLEST);
    } while (_heapLen > SMALLEST);
    _heap[--_heapMax] = _heap[SMALLEST];
    genBitLen(desc);
    genCodes(dt, desc._maxCode);
}

int GTree::buildHeap(Tree *dt, Tree *st, int elems) {
    _heapLen = 0;
    _heapMax = HEAP_SIZE;
    int maxCode = -1;

    for (int n = 0; n < elems; n++) {
        if (dt[n]._freq) {
            _heap[++_heapLen] = maxCode = n;
            _depth[n] = 0;
        } else {
            dt[n]._len = 0;
        }
    }

    while (_heapLen < 2) {
        int n = _heap[++_heapLen] = maxCode < 2 ? ++maxCode : 0;
        dt[n]._freq = 1;
        _depth[n] = 0;
        optimalLen--;
        if (st) {
            staticLen -= st[n]._len;
        }
    }

    for (int n = _heapLen/2; n >= 1; n--) {
        heapDown(dt, n);
    }

    return maxCode;
}

void GTree::heapDown(Tree *t, int dad) {
#define smaller(t, n, m)    (t[n]._freq < t[m]._freq || (t[n]._freq == t[m]._freq && _depth[n] <= _depth[m]))
    
    int dv = _heap[dad];
    int son = dad << 1;

    while (son <= _heapLen) {
        if (son < _heapLen && smaller(t, _heap[son+1], _heap[son])) {
            son++;
        }
        if (smaller(t, dv, _heap[son])) {
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
    Tree *dt = desc._dynamic;
    Tree *st = desc._static;
    int maxLength = desc._maxLength;
    int maxCode = desc._maxCode;
    int base = desc._extraBase;

    dt[_heap[_heapMax]]._len = 0;
    for (h = _heapMax + 1; h < HEAP_SIZE; h++) {
        int n = _heap[h];
        int bits = dt[dt[n]._dad]._len + 1;
        if (bits > maxLength) {
            bits = maxLength;
            overflow++;
        }
        dt[n]._len = bits;
        if (n > maxCode) {
            continue;
        }
        _blCount[bits]++;
        uint8_t xbits = n >= base ? desc._extraBits[n-base] : 0;
        uint16_t f = dt[n]._freq;
        optimalLen += f * (bits + xbits);
        if (st) {
            staticLen += f * (st[n]._len + xbits);
        }
    }

    if (overflow == 0) {
        return;
    }
    ajustBitLen(desc, overflow, h);
}

void GTree::ajustBitLen(TreeDesc &desc, int overflow, int h) {
    int maxLength = desc._maxLength;
    int maxCode = desc._maxCode;
    Tree *dt = desc._dynamic;

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
            if (dt[m]._len != (unsigned) bits) {
                optimalLen += (bits-dt[m]._len) * dt[m]._freq;
                dt[m]._len = bits;
            }
            n--;
        }
    }
}

int GTree::buildBl() {
    scanTree(_dLTree, _lDesc._maxCode);
    scanTree(_dDTree, _dDesc._maxCode);
    build(_blDesc);

    int maxBlIndex;
    for (maxBlIndex = BL_CODES-1; maxBlIndex >= 3; maxBlIndex--) {
        if (_blTree[blOrder[maxBlIndex]]._len != 0) {
            break;
        }
    }
    optimalLen += 3*(maxBlIndex+1) + 5 + 5 + 4;

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
            _blTree[curLen]._freq += count;
        } else if (curLen != 0) {
            if (curLen != prevLen) {
                _blTree[curLen]._freq++;
            }
            _blTree[REP_3_6]._freq++;
        } else if (count <= 10) {
            _blTree[REPZ_3_10]._freq++;
        } else {
            _blTree[REPZ_11_138]._freq++;
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
        _gbit->sendBits(_blTree[blOrder[rank]]._len, 3);
    }
    sendTree(_dLTree, lcodes - 1); /* send the literal tree */
    sendTree(_dDTree, dcodes - 1); /* send the distance tree */
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
                _gbit->sendCode(curLen, _blTree);
            } while (--count);
        } else if (curLen) {
            if (curLen != prevLen) {
                _gbit->sendCode(curLen, _blTree);
                count--;
            }
            _gbit->sendCode(REP_3_6, _blTree);
            _gbit->sendBits(count - 3, 2);
        } else if (count <= 10) {
            _gbit->sendCode(REPZ_3_10, _blTree);
            _gbit->sendBits(count - 3, 3);
        } else {
            _gbit->sendCode(REPZ_11_138, _blTree);
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

GTree::GBit::GBit():
_buf(0),
_size(CHAR_BITS * sizeof(uint16_t)),
_valid(0) {

}

uint16_t GTree::GBit::sendCode(int idx, Tree *tree) {
    return sendBits(tree[idx]._code, tree[idx]._len);
}

uint16_t GTree::GBit::sendBits(int value, int length) {
    uint16_t result = 0;

    _buf |= value << _valid;
    if (_valid + length > _size) {
        result = _buf;
        _buf = (uint16_t)value >> (_size - _valid);
        length -= _size;
    }
    _valid += length;

    return result;
}

unsigned GTree::GBit::reverseBits(unsigned value, int length) {
    unsigned result = 0;

    while (length--) {
        result |= value & 1;
        value >>= 1;
        result <<= 1;
    }

    return result;
}

bool GTree::GBit::winDup(GZip &gzip) {
    bool result = true;

    if (_valid > CHAR_BITS) {
        result = gzip.putShort(_buf);
    } else if (_valid > 0) {
        result = gzip.putByte(_buf);
    }
    _buf = 0;
    _valid = 0;

    return result;
}

END_NS
