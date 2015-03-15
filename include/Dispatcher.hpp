#pragma once

#include <map>
#include <functional>
#include <mutex>
#include <strings.h>
#include <string>
#include <stdio.h>

#include "Noncopyable.hpp"
#include "VecBuffer.hpp"
#include "PeerMessage.hpp"
#include "Logger.hpp"
// dispatcher base on dynamic register handler.

using namespace std;

namespace netio {

template <typename KEYTYPE>
class Dispatcher : public Noncopyable {
  typedef function<void(SpVecBuffer&, PMAddr&)> Handler;
 public:
  // register handlers
  void registerHandler(KEYTYPE& key, Handler& callback) {
    unique_lock<mutex> lock(_mutex);
    _handleMap.insert(std::pair<KEYTYPE, Handler>(key, callback));
  }
  void registerHandler(KEYTYPE& key, Handler&& callback) {
    unique_lock<mutex> lock(_mutex);
    _handleMap.insert(std::pair<KEYTYPE, Handler>(key, std::move(callback)));
  }

  // dispatch buffer
  void dispatch(const KEYTYPE& key, SpVecBuffer& buffer, PMAddr& peerInfo) {
    //    map<KEYTYPE, Handler>::const_iterator iter = _handleMap.find(key);
    auto iter = _handleMap.find(key);

    if(LIKELY(iter != _handleMap.end())) {
      (*iter).second(buffer, peerInfo);
    } else {
      //      printKeyInfo(key);
      LOGW("%s can't find handler for the KEY : %s",__func__, _dbgKeyInfo);
    }
  }

 private:
  // template <class KT>
  // void printKeyInfo(KT key) {
  //   static string unknownStr("Unknown type");
  //   snprintf(_dbgKeyInfo, sizeof(_dbgKeyInfo) - 1, unknownStr.c_str());
  //   _dbgKeyInfo[unknownStr.length()] = 0;
  // }

  // template <>
  // void printKeyInfo<int>(int key) {
  //   int len = snprintf(_dbgKeyInfo, sizeof(_dbgKeyInfo) - 1, "%d", key);
  //   if(len > 0 && (len <= (sizeof(_dbgKeyInfo) - 1))) {
  //     _dbgKeyInfo[len] = 0;
  //   }
  // }

  map<KEYTYPE, Handler> _handleMap;
  mutable mutex _mutex;

  static __thread char _dbgKeyInfo[50];
};

}


