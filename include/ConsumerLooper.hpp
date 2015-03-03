#pragma once

#include <condition_variable>
#include <functional>
#include <mutex>
#include <queue>
#include <memory>

#include "Utils.hpp"

using namespace std;

namespace netio {

// this is a comsumer looper which perform produce and consume.
// template T is the type data payload, it must provide consume function
// int T::consume();

template <class T>
class ConsumerLooper {
  typedef function<void(shared_ptr<T>&, int)> ResultHandler;
 public:
  ConsumerLooper() : _looping(true) {}
  ~ConsumerLooper() { ASSERT(false == _looping); }
  
  void setResultHandler(ResultHandler& handler) { _resultHandler = handler; }

  void produce(shared_ptr<T>& t) {
    unique_lock<mutex> lck(_mutex);
    _srcQueue.push(t);
    _cond.notify_one();
  }

  void startLoop() {
    ASSERT(nullptr != _resultHandler);
    
    while(_looping) {
      unique_lock<mutex> lck(_mutex);
      while(_srcQueue.empty()) {
        _cond.wait(lck); 
      }
      weak_ptr<T>& wpt = _srcQueue.top();
      _srcQueue.pop();
      shared_ptr<T> spt = wpt.lock();
      if(nullptr != spt) {
        int result = spt->consume();
        _resultHandler(spt, result);
      }
    }
  }

  void stopLoop() {
    unique_lock<mutex> lck(_mutex);
    _looping = false;
    _cond.notify_one();
  }
  
 private:
  ResultHandler _resultHandler;
  mutable mutex _mutex;
  condition_variable _cond;
  queue<weak_ptr<T> > _srcQueue;
  bool _looping;
};

}












