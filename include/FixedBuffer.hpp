#pragma once

#include <memory>
#include "BitmapTree.hpp"

/**
 * Fixed size buffer pool implementation.
 *
 * we must garentee the ALLOC class will alloc N byte each time
 * we called alloc
 */

template <int C, int N>
struct FixedBufferPool {
  static void* alloc() {
    off64_t offset = _bmt.bitRequire();
    if(offset >= 0) {
      return static_cast<uint8_t*>(_poolBuf) + offset * N;
    }
    return nullptr;
  }

  static void release(void* buf) {
    if(__builtin_expect((nullptr != buf), true)) {
        off64_t offset = (static_cast<uint8_t*>(buf) - static_cast<uint8_t*>(_poolBuf)) / N;
        printf("========== %p, %p \n", _poolBuf, buf);
        _bmt.bitTurnback(offset);
    }
  }
private:
  static void* _poolBuf;
  static BitmapTree _bmt;
};

template<int C, int N> void* FixedBufferPool<C, N>::_poolBuf = malloc(C * N);
template<int C, int N> BitmapTree FixedBufferPool<C, N>::_bmt(C);

template <int C, int N, typename ALLOC=FixedBufferPool<C, N> >
class FixedBuffer {
 public:
  FixedBuffer() : _len(0) {
    _bufptr = ALLOC::alloc();
  }
  
  ~FixedBuffer() {
    ALLOC::release(_bufptr);
  }

  /* can not by copyed */
  FixedBuffer(const FixedBuffer& ) = delete;
  FixedBuffer& operator= (const FixedBuffer& ) = delete;

  inline void setLength(size_t len) {
    _len = len;
  }
  
  inline size_t getLength() const {
    return _len;
  }

  inline void* getBuffer() const {
    return _bufptr;
  }
  
  inline size_t getCapacity() const {
    return CAP;
  }
 private:
  enum {CAP=N};  // static describe capacity of th buffer
  size_t _len;   // data length
  void* _bufptr; // buffer address
};

template <class T, int N, typename ALLOC=FixedBufferPool<sizeof(T), N> >
class FixedObjCreator {
 public:
  static T* create() {
    void* buf = ALLOC::alloc();
    if(!buf) return 0;

    return new(buf) T;
  }
};




