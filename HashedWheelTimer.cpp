#include "HashedWheelTimer.hpp"

using namespace std;
using namespace netio;

void HashedWheelBucket::expireTimeouts() {
  int i = 0;

  auto iter = _timeoutList.begin();
  while(iter != _timeoutList.end()) {
    // check if task timeout occured
    bool remove = false;
    i ++;

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

HashedWheelTimer::HashedWheelTimer(uint32_t msPerTick, uint32_t ticksPerWheel) :
    _ticked(0L),
    _normalizeShift(calculateNormalizeShift(ticksPerWheel)),
    _ticksPerWheel(1 << _normalizeShift),
    _msPerTick(msPerTick),
    _mask(_ticksPerWheel - 1),
    _buckets(_ticksPerWheel)
{
  LOGD(LOG_NETIO_TAG, "HashedWheelTimer, msPerTick=%u, ticksPerWheel=%u, shift=%u buckets=%llu", msPerTick, ticksPerWheel, _normalizeShift, _buckets.size());
}

uint32_t HashedWheelTimer::calculateNormalizeShift(uint32_t ticksPerWheel) {
  uint32_t shift = 0;
  while (static_cast<uint32_t>(1 << shift) < ticksPerWheel) {
    ++ shift;
  }
  return shift;
}



