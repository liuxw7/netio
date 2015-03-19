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
#include "TcpConnection.hpp"
// dispatcher base on dynamic register handler.

using namespace std;

namespace netio {

template <typename MSG>
class Dispatcher : public Noncopyable {
  typedef shared_ptr<TcpConnection> SpTcpConnection;
  typedef function<void(MSG&, SpTcpConnection&)> Handler;
  typedef decltype(MSG(nullptr)->getKey()) KeyType;
 public:
  // handler for all events 
  void registerAnyHandler(Handler& callback) {
    _anyHandles.push_back(callback);
  }
  void registerAnyHandler(Handler&& callback) {
    _anyHandles.push_back(std::move(callback));
  }
  // register handlers
  void registerHandler(KeyType& key, Handler& callback) {
    _handleMap.insert(std::pair<KeyType, Handler>(key, callback));
  }
  void registerHandler(KeyType& key, Handler&& callback) {
    _handleMap.insert(std::pair<KeyType, Handler>(key, std::move(callback)));
  }

  // dispatch buffer
  void dispatch(const KeyType& key, MSG& buffer, SpTcpConnection& connection) {
    //    map<KeyType, Handler>::const_iterator iter = _handleMap.find(key);

    auto anyIter = _anyHandles.begin();
    while(anyIter != _anyHandles.end()) {
      (*anyIter)(buffer, connection);
      anyIter ++;
    }
    
    auto iter = _handleMap.find(key);

    if(LIKELY(iter != _handleMap.end())) {
      (*iter).second(buffer, connection);
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

  map<KeyType, Handler> _handleMap;
  list<Handler> _anyHandles;

  static __thread char _dbgKeyInfo[50];
};

}












