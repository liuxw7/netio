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
#include "TcpServer.hpp"
// dispatcher base on dynamic register handler.

using namespace std;

namespace netio {

template <typename MSG>
class Dispatcher : public Noncopyable {
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
      //      LOGW("%s can't find handler for the KEY : %s",__func__, _dbgKeyInfo);
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

  // static __thread char _dbgKeyInfo[50];
};


// np is abbr with netpack protocol.
// refer with FieldLenNetPack
template <typename NP>
class TcpDispatcher : public Dispatcher<decltype(NP::readMessage(*(new SpVecBuffer(nullptr))))> {
  typedef shared_ptr<LooperPool<MultiplexLooper> > SpLooperPool;
  typedef decltype(NP::readMessage(*(new SpVecBuffer(nullptr)))) SpMsgType;
 public:
  explicit TcpDispatcher(SpLooperPool& loopers, uint16_t port) :
      _server(port, loopers)
  {
    _server.setNewConnectionHandler(std::bind(&TcpDispatcher::onNewConnection, this, std::placeholders::_1, std::placeholders::_2));
  }

  void startWork() {
    _server.startWork();
  }
 private:

  // new connection handler for TcpServer.
  void onNewConnection(int connId, SpTcpConnection& connection) {
    connection->setNewMessageHandler(std::bind(&TcpDispatcher::onNewMessage, this, std::placeholders::_1, std::placeholders::_2));
    connection->attach();
  }

  void onNewMessage(SpTcpConnection connection, SpVecBuffer buffer) {
    size_t _predMsgLen = 1000;  // FIXME

    while (true) {
      SpMsgType message = NP::readMessage(buffer);

      if(nullptr != message) {
        this->dispatch(message->getKey(), message, connection);
      } else {
        if((0 == buffer->readableSize()) && (buffer->writtableSize() < _predMsgLen)) {
          buffer.reset(new VecBuffer(_predMsgLen));
        } else {
          ssize_t expect = NP::peekMessageLen(buffer);
          if(expect < 0) {
            expect = _predMsgLen;
          }
          buffer->ensure(expect);
        }
      }
      break;
    };
  }
  
  SpLooperPool _loopPool;
  TcpServer _server;
};

template <typename NP>
class UdpDispatcher : public Dispatcher<decltype(NP::readMessage(*(new SpVecBuffer(nullptr))))> {
  
};

}


















