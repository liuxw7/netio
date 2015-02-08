#pragma once

#include <algorithm>
#include <stdint.h>
#include <string.h>
#include <mutex>
#include <memory>
#include <limits>

#include "Utils.hpp"

namespace netio {

using namespace std;

/**
 * use double cache to avoid block when flushing.
 *
 * OPS is output stream, such as file output, it must provide member function:
 * void flush(const char*, size_t len);
 */

template <typename OPS, int N = SIZE_K(50)>
class SwapCache {
 public:


  
  typedef SwapCache<OPS, N> self;

  
  SwapCache(unique_ptr<OPS>& outputStream) : _mutex(), _len(0), _opsptr(std::move(outputStream)) {
  }
  ~SwapCache() {}

 

  inline self& operator<<(bool value) {
    append(value ? "1" : "0", 1);
  }
  
  inline self& operator<<(short value) {
    appendNumeric(value);
    return *this;
  }
  
  inline self& operator<<(unsigned short value) {
    appendNumeric(value);
    return *this;
  }
  
  inline self& operator<<(int value) {
    appendNumeric(value);
    return *this;
  }
  
  inline self& operator<<(unsigned int value) {
    appendNumeric(value);
    return *this;
  }
  
  inline self& operator<<(long value) {
    appendNumeric(value);
    return *this;
  }
  
  inline self& operator<<(unsigned long value) {
    appendNumeric(value);
    return *this;
  }
  
  inline self& operator<<(long long  value) {
    appendNumeric(value);
    return *this;
  }
  
  inline self& operator<<(unsigned long long value) {
    appendNumeric(value);
    return *this;
  }
  
  inline self& operator<<(const void* value) {
    char temp[MAX_NUM_LEN] = {0};
    uintptr_t vptr = reinterpret_cast<uintptr_t>(value);
    temp[0] = '0';
    temp[1] = 'x';
    size_t len = hexStringFromNumeric(temp + 2, vptr);
    append(temp, len);
    return *this;
  }
  
  inline self& operator<<(float value) {
    char temp[MAX_NUM_LEN] = {0};
    int len = snprintf(temp, sizeof(temp), "%.12g", value);
    append(temp, len);
    return *this;
  }
  
  inline self& operator<<(double value) {
    char temp[MAX_NUM_LEN] = {0};
    int len = snprintf(temp, sizeof(temp), "%.12g", value);
    append(temp, len);
    return *this;
  }
  
  // self& operator<<(long double);
  inline self& operator<<(char value) {
    append(&value, 1);
    return *this;
  }
  
  // self& operator<<(signed char);
  // self& operator<<(unsigned char);
  inline self& operator<<(const char* str) {
    append(str);
    return *this;
  }
  
  inline self& operator<<(const unsigned char* str) {
    append(str);
    return *this;
  }
  
  inline self& operator<<(const string& value) {
    append(value.c_str(), value.length());
    return *this;
  }
  
  void flushSafely() {
    lock_guard<mutex> lock(_mutex);
    flushNoLock();
  }

 private:
  template <typename T>
  inline void appendNumeric(T value) {
    char temp[MAX_NUM_LEN] = {0};
    size_t len = stringFromNumeric(temp, value);
    append(temp, len);
  }
  
  inline void append(const char* str, size_t len) {
    char* bufptr = markUsed(len);
    memcpy(bufptr, str, len);
  }

  inline void append(const char* str) {
    append(str, strlen(str));
    //printf("%s", str);
  }
  
  /**
   * Normally used for mark certain count of cache used.
   * @return 
   */
  inline char* markUsed(size_t expect) {
    lock_guard<mutex> lock(_mutex);


    /* if there is not enough space for store expect size buffer, flush fisrt */
    if(UNLIKELY((expect + _len) > N)) {
      flushNoLock();
    }
    char* bufptr= _cache + _len;
    /* increase used size */
    _len = _len + expect;

    return bufptr;
  }
  
  /** 
   * flush cache without lock.
   */
  inline void flushNoLock() {
    _opsptr->flush(_cache, _len);
    _len = 0;
  }
  
  char _cache[N];
  size_t _len;
  mutable mutex _mutex;
  unique_ptr<OPS> _opsptr;
};


}
