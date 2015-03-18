#pragma once

#include <map>
#include <list>
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
  // handler for all events 
  void registerAnyHandler(Handler& callback) {
    _anyHandles.push_back(callback);
  }
  void registerAnyHandler(Handler&& callback) {
    _anyHandles.push_back(std::move(callback));
  }
  // register handlers
  void registerHandler(KEYTYPE& key, Handler& callback) {
    _handleMap.insert(std::pair<KEYTYPE, Handler>(key, callback));
  }
  void registerHandler(KEYTYPE& key, Handler&& callback) {
    _handleMap.insert(std::pair<KEYTYPE, Handler>(key, std::move(callback)));
  }

  // dispatch buffer
  void dispatch(const KEYTYPE& key, SpVecBuffer& buffer, PMAddr& peerInfo) {
    //    map<KEYTYPE, Handler>::const_iterator iter = _handleMap.find(key);

    auto anyIter = _anyHandles.begin();
    while(anyIter != _anyHandles.end()) {
      (*anyIter)(buffer, peerInfo);
      anyIter ++;
    }
    
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
  list<Handler> _anyHandles;

  static __thread char _dbgKeyInfo[50];
};

}












