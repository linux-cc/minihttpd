#ifndef __HTTPD_GTREE_H__
#define __HTTPD_GTREE_H__

#include "config.h"
#include <string>

#define WSIZE           0x8000
#define MAX_MATCH       258
#define MIN_MATCH       3

BEGIN_NS(httpd)

class GZip;
using std::string;

class GTree {
public:
    GTree(GZip &gzip);
    ~GTree();
    void init();
    bool tally(unsigned dist, unsigned lc);
    void flushBlock(int eof = 0);

private:
    struct Tree;
    struct TreeDesc;
    void initTreeDesc();
    void initLengthCode();
    void initDistCode();
    void initStatic();
    void genCodes(Tree *tree, int maxCode);
    void initBlock();
    void build(TreeDesc &desc);
    int buildHeap(Tree *tree, int elems);
    void heapDown(Tree *tree, int dad);
    int headPop(Tree *tree);
    void genBitLen(TreeDesc &desc);
    void ajustBitLen(TreeDesc &desc, int overflow, int h);
    int buildBl();
    void scanTree(Tree *tree, int maxCode);
    void compressBlock(Tree *ltree, Tree *dtree);
    void sendAllTrees(int lcodes, int dcodes, int blcodes);
    void sendTree(Tree *tree, int maxCode);

    struct Tree {
        union {
            uint16_t freq;
            uint16_t code;
        }_fc;
        union {
            uint16_t dad;
            uint16_t len;
        }_dl;
    };
    struct TreeDesc {
        Tree *_tree;
        uint8_t *_extraBits;
        int _extraBase;
        int _elems;
        int _maxLength;
        int _maxCode;
    };
    TreeDesc _lDesc;
    TreeDesc _dDesc;
    TreeDesc _blDesc;
    uint16_t *_blCount;
    uint8_t *_lcode;
    uint8_t *_dcode;
    int *_baseL;
    int *_baseD;
    int *_heap;
    int _heapLen;
    int _heapMax;
    uint8_t *_depth;
    uint8_t *_flagBuf;
    unsigned _lastL;
    unsigned _lastD;
    unsigned _lastF;
    uint8_t _flags;
    uint8_t _flagBit;

    GZip &_gzip;
    class GBit;
    GBit *_gbit;
};

class GTree::GBit {
public:
    GBit(GZip &gzip);
    void sendBits(int value, int length);
    void sendCode(int idx, Tree *tree);
    unsigned reverseBits(unsigned value, int length);
    void winDup();

private:
    GZip &_gzip;
    uint16_t _buf;
    uint8_t _size;
    uint8_t _valid;
};

END_NS
#endif /* ifndef __HTTPD_GTREE_H__ */
