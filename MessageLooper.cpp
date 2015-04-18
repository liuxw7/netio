
#include "MessageLooper.hpp"
#include "Utils.hpp"

#include "Logger.hpp"

using namespace netio;

const int32_t LoopMessage::MSG_TERM = -1;  // use message with negative _what to terminate the looper
const int32_t LoopMessage::MSG_FUNC = 0;   // use message with zero _what to indicate the message with ower functor to execute


void MessageLooper::postMessage(UpMessage message, bool first) {
  unique_lock<mutex> lck(_mutex);
  if(first) {
    _mq.push_front(std::move(message));
  } else {
    _mq.push_back(std::move(message));
  }
  _cond.notify_all();
}

void MessageLooper::startLoop() {
  LOGD(LOG_NETIO_TAG, "MessageLooper start loop, thread id=0x%X", this_thread::get_id());
  while(true) {
    unique_lock<mutex> lck(_mutex);
    while(_mq.empty()) {
      _cond.wait(lck);
    }
    
    UpMessage upMsg = std::move(_mq.front());
    _mq.pop_front();
    
    if(upMsg->what() > 0) {
      ASSERT(_handler);
      _handler->handleMessage(*upMsg);
    } else if(upMsg->what() == LoopMessage::MSG_FUNC) {
      upMsg->getFunctor()();
    } else {
      // message with negative _what, stop loop
      break;
    }
  }
}

void MessageLooper::stopLoop(bool immediate) {
  postMessage(LoopMessage::MSG_TERM, nullptr, nullptr, immediate);
}
