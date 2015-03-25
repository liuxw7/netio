#include <stddef.h>
#include <functional>
#include <memory>
#include <list>
#include <atomic>
#include <sys/time.h>

#include "Utils.hpp"
#include "Logger.hpp"
#include "Channel.hpp"

using namespace std;

/**
 * reference to netty HashedWheelTimer for c++ implements
 */

namespace netio {
class HashedWheelTimeout;
class HashedWheelBucket;
typedef shared_ptr<HashedWheelTimeout> SpHashedWheelTimeout;

class HashedWheelTimeout {
  constexpr static int ST_INIT = 0;
  constexpr static int ST_CANCELLED = 1;
  constexpr static int ST_EXPIRED = 2;

 public:
  HashedWheelTimeout(uint32_t rounds, uint64_t deadline, function<void()> task);
  ~HashedWheelTimeout() {
    COGD("HashedWheelTimeout destroy");

    struct timeval tv;
    uint64_t ms;

    gettimeofday(&tv, NULL);
    ms = tv.tv_sec * 1000 + tv.tv_usec / 1000;

    COGI("timeout destroy %p, ts=%llu", this, ms);
  }
  
  int state() const {
    return _state;
  }

  bool isCancelled() const {
    return ST_CANCELLED == _state;
  }

  void cancel() {
    _state = ST_CANCELLED;
  }

  bool isExpired() const {
    return ST_EXPIRED == _state;
  }

  void expire() {
    int expect = ST_INIT;
    if(_state.compare_exchange_strong(expect, ST_EXPIRED)) {
      if(_task) {
        struct timeval tv;
        uint64_t ms;

        gettimeofday(&tv, NULL);
        ms = tv.tv_sec * 1000 + tv.tv_usec / 1000;

        COGI("timeout expire %p, ts=%llu", this, ms);
        
        _task();        
      }
    }
  }

  uint32_t getRemainingRounds() const {
    return _remainingRounds;
  }

  void decreaseRounds() {
    ASSERT(_remainingRounds > 0);
    -- _remainingRounds;
    COGI("decrease rounds");
  }
  
 private:
  atomic_int _state;
  uint32_t _remainingRounds;
  uint64_t _deadline;
  function<void()> _task;
  SpHashedWheelTimeout _spNext;
  SpHashedWheelTimeout _spPrev;
};

class HashedWheelBucket {
 public:
  void addTimeout(SpHashedWheelTimeout& timeout) {
    _timeoutList.push_back(timeout);
  }
  void addTimeout(SpHashedWheelTimeout&& timeout) {
    _timeoutList.push_back(std::move(timeout));
  }
  void expireTimeouts() {
    auto iter = _timeoutList.begin();
    while(iter != _timeoutList.end()) {
      // check if task timeout occured
      bool remove = false;

      if(0 == (*iter)->getRemainingRounds()) {
        (*iter)->expire();
        remove = true;
      } else if((*iter)->isCancelled()) {
        remove = true;
      } else {
        (*iter)->decreaseRounds();
      }

      if(remove) {
        iter = _timeoutList.erase(iter);
      } else {
        ++ iter;
      }
    }
  }

  void clearTimeouts() {
    _timeoutList.clear();
  }
 private:
  list<SpHashedWheelTimeout> _timeoutList;
};

class HashedWheelTimer {
 public:
  explicit HashedWheelTimer(uint32_t msPerTick, uint32_t ticksPerWheel);

  SpHashedWheelTimeout addTimeout(function<void()>& task, uint64_t expireMs) {
    uint32_t tick = convertExpireMsToTicks(expireMs);
    uint32_t rounds = tick >> _normalizeShift;
    uint32_t index = tick & _mask;

    COGI("add timeout index = %u, rounds=%d", index, rounds);

    SpHashedWheelTimeout timeout(new HashedWheelTimeout(rounds, tick, task));
    _buckets[index].addTimeout(timeout);
    return timeout;
  }
  
  SpHashedWheelTimeout addTimeout(function<void()>&& task, uint64_t expireMs) {
    uint32_t tick = convertExpireMsToTicks(expireMs);
    uint32_t rounds = tick >> _normalizeShift;
    uint32_t index = tick & _mask;

    COGI("add timeout index = %u", index);

    SpHashedWheelTimeout timeout(new HashedWheelTimeout(rounds, tick, std::move(task)));
    _buckets[index].addTimeout(timeout);
    return timeout;
  }

  // wheel tick execution.
  void tick() {
    _buckets[_ticked & _mask].expireTimeouts();
    ++ _ticked;
  }
 private:
  uint32_t convertExpireMsToTicks(uint64_t expireMs) const {
    return (expireMs + _msPerTick - 1) / _msPerTick;
  }
  
  uint32_t calculateNormalizeShift(uint32_t ticksPerWheel) {
    uint32_t shift = 0;
    while ((1 << shift) < ticksPerWheel) {
      ++ shift;
    }
    return shift;
  }
  
  uint64_t _ticked;
  uint32_t _normalizeShift;
  uint32_t _ticksPerWheel;
  uint32_t _msPerTick;
  uint32_t _mask;
  vector<HashedWheelBucket> _buckets;
};

inline HashedWheelTimeout::HashedWheelTimeout(uint32_t rounds, uint64_t deadline, function<void()> task) :
    _state(ST_INIT),
    _remainingRounds(rounds),
    _deadline(deadline),
    _task(task)
{
  struct timeval tv;
  uint64_t ms;

  gettimeofday(&tv, NULL);
  ms = tv.tv_sec * 1000 + tv.tv_usec / 1000;

  COGI("timeout created %p, ts=%llu", this, ms);
}

inline HashedWheelTimer::HashedWheelTimer(uint32_t msPerTick, uint32_t ticksPerWheel) :
    _ticked(0L),
    _normalizeShift(calculateNormalizeShift(ticksPerWheel)),
    _ticksPerWheel(1 << _normalizeShift),
    _msPerTick(msPerTick),
    _mask(_ticksPerWheel - 1),
    _buckets(_ticksPerWheel)
{
  COGD("create hashed wheel timer, ticks per wheel = %u, shift=%u buckets=%llu", ticksPerWheel, _normalizeShift, _buckets.size());
}

}





