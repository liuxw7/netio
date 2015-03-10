#pragma once

#include <memory>
#include <stack>
#include <string.h>
#include <mutex>

#include "Utils.hpp"

/** 
 * This must compile in gcc, 
 */

#define LONG_BITS  (64)

using namespace std;

struct BitmapLayer {
  BitmapLayer(size_t size) : _bits(nullptr), _lwords(0), _tailBits(0), _size(size) {
    if(size > 0) {
      /* allocate uint64_t array */
      _lwords = calculate_lwords(size);
      _bits = new uint64_t[_lwords];
      _tailBits = (size & 0x3F);

      /* set all bits initial */
      memset((void*)_bits, 0xFF,(_lwords << 3));

      /* fix last uint64_t */
      if(0 != _tailBits) {
        _bits[_lwords - 1] = ((_bits[_lwords - 1]) & (-1UL << ((sizeof(uint64_t) << 3) - _tailBits)));
      }

      //      printf("tail = %lu \n", _tailBits );
    }
  }

  ~BitmapLayer() {
    delete[] _bits;
    _bits = nullptr;
    _lwords = 0;
  }

  /* calculate how much uint64_t contain size's bits */
  inline size_t calculate_lwords(size_t size) {
    return (size + (sizeof(unsigned long) << 3) - 1) >> 6;
  }

  void _print_dbg() const {
    for(int i = 0; i < _lwords; i ++) {
      printf("%016lx ", _bits[i]);
    }
  }

  /** set bit index to 1
   * @return : if original uint64_t is 0, return true, else return false
   */
  inline bool set(off64_t bitIdx) {
    if(__builtin_expect((bitIdx > _size), 0)) {
      return false;
    }
    
    off64_t lwoff = bitIdx >> 6;
    /* if original is 0, set one bit will affect up layer */
    bool upApply = !(_bits[lwoff]);
    _bits[lwoff] |= (1 << (LONG_BITS - 1 - (bitIdx & 0x3F)));
    return upApply;
  }

  /* reset layer's bit offset and return it's uint64_t index */
  inline bool reset(off64_t bitIdx) {
    if(__builtin_expect((bitIdx > _size), 0)) {
      return false;
    }

    off64_t lwoff = bitIdx >> 6;
    /* if new value is zero, affect up layer */
    return !(_bits[lwoff] &= ~(1L << (LONG_BITS - 1 - (bitIdx & 0x3F))));
  }

  /* find first seted bit in uint64_t */
  inline int firstBitSet(off64_t lwoff) {
    if(__builtin_expect((lwoff > _lwords), 0)) {
      return -1;
    }

    if(0 == _bits[lwoff]) {
      return -1;
    }

    return (lwoff << 6) + __builtin_clzl(_bits[lwoff]);
  }
  
  uint64_t* _bits;
  size_t _lwords;   // indicate how much long ints.
  size_t _tailBits; // how much valid bits in tail uint64_t
  size_t _size;
};

/* 8 layer max deep, can describe 64 ^ 8 = 2 ^ 48 objs */
#define BMT_MAX_DEEP (8)

class BitmapTree {
 public:
  BitmapTree(size_t size) : _mutex() {
    static_assert((8 == sizeof(unsigned long)), "long word is not 8 bytes");
    stack<BitmapLayer*> layers;
    size_t lsize = size;

    while (lsize > 1) {
      BitmapLayer* layer = new BitmapLayer(lsize);
      layers.push(layer);
      lsize = layer->_lwords;
    }

    _deep = layers.size();

    for(int i = 0; i < ARRAY_SIZE(_layers); i++) {
      if(!layers.empty()) {
        _layers[i] = layers.top();
        layers.pop();
      } else {
        _layers[i] = nullptr;
      }
    }
  }
  ~BitmapTree() {
    _deep = 0;
    for(int i = 0; i < ARRAY_SIZE(_layers); i++) {
      if(nullptr != _layers[i]) {
        delete _layers[i];
      } else {
        break;
      }
    }
  }

  uint8_t getDeep() const {
    return _deep;
  }

  void _print_dbg() const {
    for(int i = 0; i < _deep; i++) {
      _layers[i]->_print_dbg();
      printf("\n");
    }
  }

  inline off64_t bitRequire() {
    lock_guard<mutex> lock(_mutex);
    off64_t bitsOffs[BMT_MAX_DEEP] = {0};
    off64_t lwOffs = 0;
    bool found = true;

    /* record bit offset */
    for(int i = 0; i < _deep; i ++) {
      bitsOffs[i] = _layers[i]->firstBitSet(lwOffs);
      lwOffs = bitsOffs[i];

      if(__builtin_expect((lwOffs < 0), 0)) {
        found = false;
        break;
      }
    }

    for(int i = 0; i < _deep; i++) {
      printf("bits offset = %ld \n", bitsOffs[i]);
    }

    /* if found, reset bit upward, calculate total offsets */
    if(found) {
      for(int i = (_deep -1); i >= 0; i--) {
        if(!_layers[i]->reset(bitsOffs[i])) {
          break;
        }
      }
      return lwOffs;
    }
    return -1;
  }

  inline void bitTurnback(off64_t idx) {
    lock_guard<mutex> lock(_mutex);
    off64_t tmpIdx = idx;
    for(int i = _deep - 1; i >= 0; i--) {
      if(!_layers[i]->set(tmpIdx)) {
        break;
      }
      tmpIdx = tmpIdx >> 6;
    }
  }
 private:
  mutable mutex _mutex;
  BitmapLayer* _layers[BMT_MAX_DEEP];
  uint8_t _deep;
};







