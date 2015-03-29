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
class BitmapTree;

class BitmapLayer {
  friend class BitmapTree;
 public:
  /** 
   * Constructor
   * 
   * @param size : how many unit this layer can descript. 
   */
  BitmapLayer(size_t size);

  ~BitmapLayer();

  /* calculate how much uint64_t contain size's bits */
  size_t calculate_lwords(size_t size) {
    return (size + (sizeof(unsigned long) << 3) - 1) >> 6;
  }

  /** 
   * Debug bits used status.
   */
  void _print_dbg() const;

  /** 
   * Set specified bit to 1
   * 
   * @param bitIdx : set bit index to 1
   * 
   * @return : if original uint64_t is 0, return true, else return false
   */
  bool set(uint bitIdx) {
    if(__builtin_expect((bitIdx > _size), 0)) {
      return false;
    }
    
    off64_t lwoff = bitIdx >> 6;
    /* if original is 0, set one bit will affect up layer */
    bool upApply = !(_bits[lwoff]);
    _bits[lwoff] |= (1 << (LONG_BITS - 1 - (bitIdx & 0x3F)));
    return upApply;
  }

  /** 
   * Reset specified index bit to 0
   * 
   * @param bitIdx : the bit index witch will be reset.
   * 
   * @return : reset layer's bit offset and return it's uint64_t index
   */
  bool reset(uint bitIdx) {
    if(__builtin_expect((bitIdx > _size), 0)) {
      return false;
    }

    off64_t lwoff = bitIdx >> 6;
    /* if new value is zero, affect up layer */
    return !(_bits[lwoff] &= ~(1L << (LONG_BITS - 1 - (bitIdx & 0x3F))));
  }

  /** 
   *  find first seted bit in uint64_t
   * 
   * @param lwoff : start search from the index.
   * 
   * @return 
   */
  int firstBitSet(uint lwoff) {
    if(__builtin_expect((lwoff > _lwords), 0)) {
      return -1;
    }

    if(0 == _bits[lwoff]) {
      return -1;
    }

    return (lwoff << 6) + __builtin_clzl(_bits[lwoff]);
  }
 private:
  uint64_t* _bits;
  size_t _lwords;   // indicate how much long ints.
  size_t _tailBits; // how much valid bits in tail uint64_t
  size_t _size;
};

/* 8 layer max deep, can describe 64 ^ 8 = 2 ^ 48 objs */
#define BMT_MAX_DEEP (8)

class BitmapTree {
 public:
  BitmapTree(size_t size); 
  ~BitmapTree();

  uint8_t getDeep() const {
    return _deep;
  }

  void _print_dbg() const {
    for(int i = 0; i < _deep; i++) {
      _layers[i]->_print_dbg();
      printf("\n");
    }
  }

  /** 
   * require one bit from bitmap tree
   * 
   * @return : offset of leafs.
   */
  off64_t bitRequire();

  /** 
   * Return bit to bitmap tree.
   * 
   * @param idx 
   */
  void bitTurnback(off64_t idx) {
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







