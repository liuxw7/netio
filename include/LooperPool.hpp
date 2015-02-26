#pragma once

#include <functional>
#include <thread>
#include <map>

#include "MultiplexLooper.hpp"

using namespace std;

namespace netio {

class LooperPool {
 public:
  template <typename ... Types>
  LooperPool(function<void(Types...)> loopFunc, size_t threadCount) {
    for(int i = 0; i < threadCount; i++) {
      thread newThread(loopFunc);
    }
  }

  MultiplexLooper* getLooper() const;
  
 private:
  
};

}













