#ifndef __MEMORY_SIMPLE_ALLOC_H__
#define __MEMORY_SIMPLE_ALLOC_H__

#include <new>
#include "util/template_util.h"
#include "memory/buddy_alloc.h"
#include "memory/slab_alloc.h"
#include "thread/thread.h"

namespace memory {

class Allocater {
public:
    Allocater(int blocks, int blockSize): _buddy(blocks, blockSize), _slab(_buddy) {}
    
    void *alloc(size_t size) { return _slab.alloc(size); }
    void free(const void *addr, size_t size) { return _slab.free(addr, size); }
    
    void *mutexAlloc(size_t size) { thread::AutoMutex m(_mutex); return alloc(size); }
    void mutexFree(const void *addr, size_t size) { thread::AutoMutex m(_mutex); return free(addr, size); }
    
    static pthread_key_t &getLocalKey() { return _localKey; }
    static void createLocalKey() { pthread_key_create(&_localKey, NULL); }
    static void deleteLocalKey() { pthread_key_delete(_localKey); }
    
private:
    BuddyAlloc _buddy;
    SlabAlloc _slab;
    thread::Mutex _mutex;
    static pthread_key_t _localKey;
};

void *allocate(size_t size);
void deallocate(const void *addr, size_t size);

template <typename T>
class SimpleAlloc {
public:
    static T *New() {
        void *p = allocate(sizeof(T));
        return construct(util::IsTrivial<T>(), p);
    }
    
    template <typename P1>
    static T *New(const P1& p1) {
        void *p = allocate(sizeof(T));
        return construct(util::IsTrivial<T>(), p, p1);
    }
    
    template <typename P1>
    static T *New(P1& p1) {
        void *p = allocate(sizeof(T));
        return construct(util::IsTrivial<T>(), p, p1);
    }
    
    template <typename P1, typename P2>
    static T *New(const P1 &p1, const P2 &p2) {
        void *p = allocate(sizeof(T));
        return construct(util::IsTrivial<T>(), p, p1, p2);
    }
    
    template <typename P1, typename P2, typename P3>
    static T *New(const P1 &p1, const P2 &p2, const P3 &p3) {
        void *p = allocate(sizeof(T));
        return construct(util::IsTrivial<T>(), p, p1, p2, p3);
    }
    
    template <typename P1, typename P2, typename P3, typename P4>
    static T *New(const P1 &p1, const P2 &p2, const P3 &p3, const P4 &p4) {
        void *p = allocate(sizeof(T));
        return construct(util::IsTrivial<T>(), p, p1, p2, p3, p4);
    }
    
    static void Delete(const T *addr) {
        destruct(util::IsTrivial<T>(), addr);
        deallocate(addr, sizeof(T));
    }
    
private:
    static T *construct(util::TrueType, void *p) { return (T*)p; }
    static T *construct(util::FalseType, void *p) { return new (p) T(); }
    template <typename P1>
    static T *construct(util::TrueType, void *p, const P1 &p1) { return *(T*)p = p1; }
    template <typename P1>
    static T *construct(util::FalseType, void *p, const P1 &p1) { return new (p) T(p1); }
    template <typename P1>
    static T *construct(util::FalseType, void *p, P1 &p1) { return new (p) T(p1); }
    template <typename P1, typename P2>
    static T *construct(util::FalseType, void *p, const P1 &p1, const P2 &p2) { return new (p) T(p1, p2); }
    template <typename P1, typename P2, typename P3>
    static T *construct(util::FalseType, void *p, const P1 &p1, const P2 &p2, const P3 &p3) { return new (p) T(p1, p2, p3); }
    template <typename P1, typename P2, typename P3, typename P4>
    static T *construct(util::FalseType, void *p, const P1 &p1, const P2 &p2, const P3 &p3, const P4 &p4) { return new (p) T(p1, p2, p3, p4); }
    static void destruct(util::TrueType, const T* p) { }
    static void destruct(util::FalseType, const T* p) { p->~T(); }
};

template <typename T>
class SimpleAlloc<T[]> {
public:
    static T *New(size_t size) {
        void *p = allocate(sizeof(T) * size);
        return construct(util::IsTrivial<T>(), p, size);
    }
    
    static void Delete(const T *addr, size_t size) {
        destruct(util::IsTrivial<T>(), addr, size);
        deallocate(addr, sizeof(T) * size);
    }
    
private:
    static T *construct(util::TrueType, void *p, size_t size) { return (T*)p; }
    static T *construct(util::FalseType, void *p, size_t size) { return new (p) T[size]; }
    static void destruct(util::TrueType, const T* p, size_t size) { }
    static void destruct(util::FalseType, const T* p, size_t size) {
        for (size_t i = 0; i < size; i++) {
            const T *pi = p + i;
            pi->~T();
        }
    }
};

} /* namespace memory */

#endif /* __MEMORY_SIMPLE_ALLOC_H__ */

