#include <stddef.h>
#include <functional>
#include <memory>

using namespace std;

/**
 * reference to netty HashedWheelTimer for c++ implements
 */

namespace netio {

class HashedWheelTimeout {
  typedef enum {
    ST_INIT = 0,
    ST_CANCELLED,
    ST_EXPIRED
  } STATE;
 public:
  HashedWheelTimeout(HashedWheelTimer& timer, function<void()>& task, uint64_t expireMs);
 private:
  STATE _state;
  uint32_t _remainingRounds;
  uint64_t _deadline;
  function<void()> _task;
};

class HashedWheelBucket {
  
};

class HashedWheelTimer {
 public:
  explicit HashedWheelTimer(uint32_t ticksPerWheel, uint32_t msPerTick);

  shared_ptr<HashedWheelTimeout> addTimeout(function<void()>& task, uint64_t expireMs);
  shared_ptr<HashedWheelTimeout> addTimeout(function<void()>&& task, uint64_t expireMs);
  
 private:
  
  
  uint32_t convertExpireMsToTicks(uint64_t expireMs);
  uint32_t calculateRemainingRounds();
  
  uint32_t _ticksPerWheel;
  uint32_t _msPerTick;
};

inline HashedWheelTimeout::HashedWheelTimeout(uint32_t rounds, uint64_t deadline, function<void()> task)
    : _state(ST_INIT),
      _remainingRounds(rounds),
      _deadline(deadline),
      _task(task);
{}

}



















