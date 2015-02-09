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
 * Single cache is used for caching buffer that not affect immdiatly.
 * When cache if full, it will call flush operator to make a copy with shared_ptr of vector<int_8>
 * to buffer receiver. The output stream receive must not blocking the cache thread.
 *
 * OPS is output stream, such as file output, it must provide member function:
 * void flush(shared_ptr<vector<int8_t>>&);
 * NOTE: flush must asynchronize process.
 */

template <typename OPS, int N = SIZE_K(50)>
class SingleCache {
 public:
  typedef SingleCache<OPS, N> self;
  SingleCache(shared_ptr<OPS>& outputStream) : _cache(N), _mutex(), _len(0), _wpOps(outputStream) {}
  ~SingleCache() {}

  /**
   * Flush cache to output stream thread safely.
   */
  void flushSafely() {
    lock_guard<mutex> lock(_mutex);
    flushNoLock();
  }

  /*
   * Append buffer to cache. If not enough cache currently, swap cache out and reset the cache.
   *
   * @buf[in] : buffer that will append to cache.
   * @len[in] : buffer len.
   */
  inline void append(const void* buf, size_t len) {
    ASSERT(nullptr != buf);
    
    int8_t* dstptr = markUsed(len);
    memcpy(dstptr, buf, len);
  }

  /**
   * Append string to cache. Reference void append(const void* buf, size_t len)
   *
   * @str[in] : string that will append to cache.
   */
  inline void append(const char* str) {
    ASSERT(nullptr != str);
    
    append(str, strlen(str));
  }
 private:
  /**
   * Mark certain count of cache used.
   *
   * @expect[in] : how much bytes expected, expect len must less than cache size.
   * @return : dest cache ptr that to write at. It will always return valid pointer
   *           if expect len is valid.
   */
  inline int8_t* markUsed(size_t expect) {
    if(LIKELY(expect <= N)) {
      lock_guard<mutex> lock(_mutex);
      // if there is not enough space for store expect size buffer, flush fisrt
      if(UNLIKELY((expect + _len) > N)) {
        flushNoLock();
      }
      int8_t* bufptr= _cache.data() + _len;
      // increase used size
      _len = _len + expect;
      return bufptr;
    } else {
      lock_guard<mutex> lock(_mutex);
      flushNoLock();
      _cache.resize(expect);
      _len = expect;
      return _cache.data();
    }
  }
  
  /** 
   * Notify OPS that cache will swap out.
   */
  inline void flushNoLock() {
    if(_len > 0) {
      shared_ptr<vector<int8_t>> spVec(new vector<int8_t>(_len));
      memcpy(spVec->data(), _cache.data(), _len);
      
      auto spOps = _wpOps.lock();
      if(nullptr != spOps) {
        spOps->flush(spVec);
      }
      _len = 0;
    }
  }
  
  vector<int8_t> _cache;
  size_t _len;
  mutable mutex _mutex;
  weak_ptr<OPS> _wpOps;
};


}













