#include <stddef.h>
#include <functional>
#include <memory>
#include <list>
#include <atomic>

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
  constexpr int ST_INIT = 0;
  constexpr int ST_CANCELLED = 1;
  constexpr int ST_EXPIRED = 2;

 public:
  HashedWheelTimeout(HashedWheelTimer& timer, function<void()>& task, uint64_t expireMs);

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
    if(_state.compare_exchange_strong(ST_INIT, ST_EXPIRED)) {
      if(_task) {
        _task();        
      }
    }
  }

  uint32_t getRemainingRounds() const {
    return _remainingRounds;
  }

  void decreaseRounds() {
    ASSERT(_remainingRounds > 0);
    _remainingRounds = 0;
  }
  
 private:
  atomic_int _state;
  uint32_t _remainingRounds;
  uint64_t _deadline;
  function<void()> _task;
  SpHashedWheelTimeout _spNext;
  SpHashedWheelTimeout _spPrev;
};

typedef

class HashedWheelBucket {
 public:
  void addTimeout(SpHashedWheelTimeout& timeout) {
    _timeoutList.push_back(timeout);
  }
  void addTimeout(SpHashedWheelTimeout&& timeout) {
    _timeoutList.push_back(std::move(timeout));
  }
  void expireTimeouts(long deadline) {
    auto iter = _timeoutList.begin();
    while(iter != _timeoutList.end) {
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
  explicit HashedWheelTimer(uint32_t ticksPerWheel, uint32_t msPerTick);

  SpHashedWheelTimeout addTimeout(function<void()>& task, uint64_t expireMs) {
    
  }
  
  SpHashedWheelTimeout addTimeout(function<void()>&& task, uint64_t expireMs) {
    
  }

  // wheel tick execution.
  void tick();
 private:
  uint32_t convertExpireMsToTicks(uint64_t expireMs);
  uint32_t calculateRemainingRounds();

  uint32_t normalizeTicksPerWheel(uint32_t ticksPerWheel) {
    uint32_t normalizedTicksPerWheel = 1;
    while (normalizedTicksPerWheel < ticksPerWheel) {
      normalizedTicksPerWheel <<= 1;
    }
    return normalizedTicksPerWheel;
  }
  
  uint64_t _ticked;
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
{}

inline HashedWheelTimer::HashedWheelTimer(uint32_t ticksPerWheel, uint32_t msPerTick) :
    _ticked(0L),
    _ticksPerWheel(normalizeTicksPerWheel(icksPerWheel)),
    _msPerTick(msPerTick),
    _mask(_ticksPerWheel - 1),
    _buckets(_ticksPerWheel)
{}

}



