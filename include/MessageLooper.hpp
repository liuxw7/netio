#pragma once
#include <stdint.h>
#include <functional>
#include <mutex>
#include <condition_variable>
#include <list>
#include <memory>

#include "Utils.hpp"

using namespace std;

namespace netio {

/**
 * Loop message.
 *
 * what : message type for Loop handler;
 * _lparam : first parameter for Loop handler;
 * _rparam : second parameter for loop handler;
 * _functor : if what is negative, we just call _functor and we don't need LoopHandler
 */
class LoopMessage {
  typedef function<void (void) > functor;
 
 public:
  static const int32_t MSG_FUNC;
  static const int32_t MSG_TERM;
  
  LoopMessage(const functor& func) :
      _functor(func),
      _what(0)
  {
    ASSERT(_functor);
  }

  LoopMessage(functor&& func) :
      _functor(std::move(func)),
      _what(0)
  {}

  LoopMessage(int32_t what, void* lparam = nullptr, void* rparam = nullptr) :
      _what(what),
      _lparam(lparam),
      _rparam(rparam)
  {
    ASSERT(MSG_FUNC != what);
  }

  functor getFunctor() const { return _functor; }
  int32_t what() const { return _what; }
  void* lparam() const { return _lparam; }
  void* rparam() const { return _rparam; }
 private:
  functor _functor;
  int32_t _what;
  void* _lparam;
  void* _rparam;
};

class LoopHandler {
 public:
  virtual void handleMessage(LoopMessage& message) = 0;
};

class MessageLooper {
  typedef unique_ptr<LoopMessage> UpMessage;
  
 public:
  MessageLooper() :
      _handler(nullptr),
      _mutex(),
      _cond()
  {}

  void startLoop();
  void stopLoop(bool immediate = false);

  void postMessage(UpMessage message, bool first = false);

  void postFunctor(function<void(void)>&& functor, bool first = false) {
    UpMessage msg(new LoopMessage(std::forward<function<void(void)>>(functor)));
    postMessage(std::move(msg), first);
  }
  
  void postFunctor(function<void(void)>& functor, bool first = false) {
    postMessage(UpMessage(new LoopMessage(functor)), first);
  }
  
  void postMessage(int32_t what, void* lparam = nullptr, void* rparam = nullptr, bool first = false) {
    postMessage(UpMessage(new LoopMessage(what, lparam, rparam)), first);
  }
  
  void setHandler(LoopHandler* handler) { _handler = handler; }
  
 private:
  mutable mutex _mutex;
  condition_variable _cond;
  list<UpMessage> _mq;

  // deal with message that have positive _what
  LoopHandler* _handler;
};

}









