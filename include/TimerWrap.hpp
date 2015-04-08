#pragma once

#include <sys/timerfd.h>
#include <memory>

#include "Utils.hpp"
#include "Channel.hpp"
#include "Logger.hpp"

using namespace std;

namespace netio {

template <class TIMER>
class TimerWrap {
  typedef typename TIMER::SpTimeout TimeoutPtr;
 public:
  template <typename ... ARGS>
  TimerWrap(MultiplexLooper* looper, uint32_t msPerTick, ARGS ... args) :
      _msPerTick(msPerTick),
      _timerfd(timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC)),
      _channel(looper, _timerfd)
  {
    ASSERT(_timerfd >= 0);
    ASSERT(_msPerTick > 0);
    _upTimer = unique_ptr<TIMER>(new TIMER(msPerTick, args...));
    _channel.setReadCallback(std::bind(&TimerWrap<TIMER>::handleRead, this));
    _channel.enableRead(true);

    startFireTick();
  }

  ~TimerWrap() {
    ASSERT(!_channel.isAttached());
    close(_timerfd);
  }

  // attach and detach channel
  void attach() { _channel.attach(); }
  void detach() { _channel.detach(); }

  TimeoutPtr addTimeout(function<void()>& task, uint64_t expireMs) {
    return _upTimer->addTimeout(task, expireMs);
  }
  
  TimeoutPtr addTimeout(function<void()>&& task, uint64_t expireMs) {
    return _upTimer->addTimeout(task, expireMs);
  }
  
 private:
  void handleRead() {
    uint64_t tocount;
    ssize_t n = ::read(_timerfd, &tocount, sizeof(tocount));

    //    COGI("TimerWrapper read %llu ", tocount);
    if(n != sizeof(tocount)) {
      COGE("TimerWrapper read %d size", n);
    }
    _upTimer->tick();
  }

  void startFireTick() {
    struct itimerspec nextTime = {0};

    nextTime.it_value.tv_sec = _msPerTick / 1000;
    nextTime.it_value.tv_nsec = (_msPerTick % 1000) * 1000000;
    nextTime.it_interval.tv_sec = nextTime.it_value.tv_sec;
    nextTime.it_interval.tv_nsec = nextTime.it_value.tv_nsec;

    int ret = ::timerfd_settime(_timerfd, 0, &nextTime, NULL);
    if(0 != ret) {
      COGE("fire next tick failed");
    }
  }

  uint32_t _msPerTick;
  int _timerfd;
  Channel _channel;
  unique_ptr<TIMER> _upTimer;
};

}

















