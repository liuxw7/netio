#pragma once

#include <map>
#include <time.h>
#include <memory>
#include <functional>

#include "TcpServer.hpp"
#include "TcpConnection.hpp"
#include "Dispatcher.hpp"

#include "Logger.hpp"

namespace netio {

/***
 * We assume that SpMsgType must be type wrap with shared_ptr. Message must have function
 * getKey().
 * NP, is abbreviate for Netpack class. It must have functions readMessage,
 * peekMessageLen.
 */

class UserSession {
  typedef enum {
    STATE_NEW, // new session, not verified
    STATE_CHECKED, // session checked
  } SessionState;
  
 public:
  UserSession(int id, time_t createTime, SpTcpConnection connection) :
      _id(id),
      _createTime(createTime),
      _updateTime(createTime),
      _connection(connection),
      _seq(0),
      _state(STATE_NEW)
  {}

  void touch(time_t updateTime) { _updateTime = updateTime; }

  uint32_t getId() const { return _id; }

  time_t createTime() const { return _createTime; }

  time_t updateTime() const { return _updateTime; }

  
 private:
  uint32_t _id; // session id is unique global for distribute system.
  time_t _createTime;
  time_t _updateTime;
  uint32_t _seq; // sequence number for generate push message sequence number.
  SessionState _state; 
  SpTcpConnection _connection;
};

template <typename SpMsgType>
class UserSessionMgr {
  typedef unique_ptr<UserSession> UpUserSession;
 public:
  void process(SpMsgType& msg, SpTcpConnection& connection) {
    
    
    COGFUNC();
    if(nullptr != msg) {
      COGI("msg's key = %d", msg->getKey());
      //      msg->getkey();
      COGI("%s receive content=%s", __func__, msg->_buffer->readablePtr());
    }
  }
 private:
  map<int, UpUserSession> _usMap;
};

/**
 * 
 */
template <class NP>
class TcpProxy {
  typedef shared_ptr<LooperPool<MultiplexLooper> > SpLooperPool;
  typedef decltype(NP::readMessage(*(new SpVecBuffer(nullptr)))) SpMsgType;
  typedef decltype(SpMsgType(nullptr)->getKey()) MsgKeyType;
  typedef UserSessionMgr<SpMsgType> USMgr;
  typedef Dispatcher<SpMsgType> DispatcherType;
 public:
  /**
   * @loopsers hold threads for process events.
   * @servPort for client connection.
   * @pushPort for inner module transport that aim for send data to client
   */
  explicit TcpProxy(SpLooperPool& loopers, uint16_t servPort, uint16_t pushPort) :
      _server(servPort, loopers)
  {
    _dispatcher.registerAnyHandler(std::bind(&USMgr::process, &_usMgr, std::placeholders::_1, std::placeholders::_2));
    _server.setNewConnectionHandler(std::bind(&TcpProxy::onNewConnection, this, std::placeholders::_1, std::placeholders::_2));
    _server.startWork();
  }

  
  DispatcherType& getDispatcher() { return _dispatcher; }

   
 private:
  // new connection handler for TcpServer.
  void onNewConnection(int connId, SpTcpConnection& connection) {
    connection->setNewMessageHandler(std::bind(&TcpProxy::onNewMessage, this, std::placeholders::_1, std::placeholders::_2));
    connection->attach();
  }

  void onNewMessage(SpTcpConnection connection, SpVecBuffer buffer) {
    SpMsgType message(nullptr); // netpack message read operation must return shared_ptr
    // read all complete message.
    size_t _predMsgLen = 1000;  // FIXME

    COGI("on new message fd = %d", connection->getFd());

    while (true) {
      SpMsgType message = NP::readMessage(buffer);

      if(nullptr != message) {
        _dispatcher.dispatch(message->getKey(), message, connection);
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
  USMgr _usMgr;
  TcpServer _server;
  DispatcherType _dispatcher;
};

}

