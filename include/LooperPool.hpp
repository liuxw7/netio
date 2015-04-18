#pragma once

#include <functional>
#include <thread>
#include <vector>

#include "Logger.hpp"

using namespace std;
namespace netio {

/**
 * Looper class must have startLoop and stopLoop member function.
 */
template <class Looper>
class LooperPool {
 public:
  LooperPool(size_t threadCount, bool attach = true) :
      _loopers(),
      _attach(attach),
      _lastLooperIdx(0)
  {
    LOGI(LOG_NETIO_TAG, "LooperPool initial thread count %d", threadCount);
    
    for(int i = 0; i < threadCount; i++) {
      Looper* looper = new Looper();
      thread* mythread = new thread(std::bind(&Looper::startLoop, looper));

      //      LOGD(LOG_NETIO_TAG, "initial loop pool, index=%d threadid=0x%X", i, mythread->get_id());

      if(!_attach) {
        mythread->detach();
      }
      
      auto loopMap = make_pair(looper, mythread);
      _loopers.push_back(loopMap);
    }
  }

  ~LooperPool() {
    for(int i = 0; i < _loopers.size(); i++) {
      _loopers[i].first->stopLoop();
      if(_attach) {
        _loopers[i].second->join();
        LOGI(LOG_NETIO_TAG, "looper thread joined");
      }

      delete _loopers[i].second;
      delete _loopers[i].first;
    }

  }
  
  Looper* getLooper() {
    Looper* looper = _loopers[_lastLooperIdx].first;
    _lastLooperIdx = ((_lastLooperIdx + 1) % _loopers.size());
    return looper;
  }

 private:
  vector<pair<Looper*, thread*> > _loopers;
  bool _attach;
  size_t _lastLooperIdx;
};

}











