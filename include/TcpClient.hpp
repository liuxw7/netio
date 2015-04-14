#pragma once

#include <thread>
#include <memory>

#include "TcpConnection.hpp"
#include "TcpConnector.hpp"
#include "MultiplexLooper.hpp"
#include "Dispatcher.hpp"
#include "NetPackDispatcher.hpp"
#include "FieldLenNetPack.hpp"

using namespace netio;

class TcpClient {
  typedef function<void(SpTcpConnection&)> NewConnectionHandler;
  typedef function<void(SpTcpConnection&, SpVecBuffer&)> NewMessageHandler;  
 public:
  TcpClient(const char* rip, uint16_t rport) :
      _thread(nullptr),
      _looper(),
      _connector(&_looper, 0, InetAddr(rip, rport)),
      _spConn(nullptr),
      _newConnHandler(std::bind(&TcpClient::dummyConnectionHandler, this, std::placeholders::_1)),
      _newMsgHandler(std::bind(&TcpClient::dummyMessageHandler, this, std::placeholders::_1, std::placeholders::_2))
  {}
  
  void startWork() {
    _thread = unique_ptr<thread>(new thread(std::bind(&MultiplexLooper::startLoop, &_looper)));
  
    _connector.setNewConnCallback(std::bind(&TcpClient::onNewConnection, this, std::placeholders::_1, std::placeholders::_2));
    _connector.attach();
    _connector.connect();
  }
  
  void stopWork()  {
    _connector.detach();
    _connector.disconnect();
    _looper.stopLoop();
    _thread->join();
  }

  void setConnectionHandler(const NewConnectionHandler& handler) {
    if(handler) {
      _newConnHandler = handler;      
    } else {
      _newConnHandler = std::bind(&TcpClient::dummyConnectionHandler, this, placeholders::_1);
    }
  }

  void setConnectionHandler(NewConnectionHandler&& handler) {
    if(handler) {
      _newConnHandler = std::move(handler);      
    } else {
      _newConnHandler = std::bind(&TcpClient::dummyConnectionHandler, this, placeholders::_1);      
    }
  }

  void setMessageHandler(const NewMessageHandler& handler) {
    if(handler) {
      _newMsgHandler = handler;
    } else {
      _newMsgHandler = std::bind(&TcpClient::dummyMessageHandler, this, placeholders::_1, placeholders::_2);
    }
  }

  void setMessageHandler(NewMessageHandler&& handler) {
    if(handler) {
      _newMsgHandler = std::move(handler);
    } else {
      _newMsgHandler = std::bind(&TcpClient::dummyMessageHandler, this, placeholders::_1, placeholders::_2);
    }
  }
  
 private:
  void dummyConnectionHandler(SpTcpConnection& conn) {
    FOGI("tcpclient receive connection : %s", conn->strInfo());
  }
  void dummyMessageHandler(SpTcpConnection& conn, SpVecBuffer& buffer) {
    FOGW("tcpclient receive msg, no message handler");
  }

  void onNewConnection(int fd, const InetAddr& addr) {
    SpTcpConnection spConn = SpTcpConnection(new TcpConnection(&_looper, fd, addr.getSockAddr()));
    _spConn = spConn;
    _newConnHandler(spConn);
  }
  
  unique_ptr<thread> _thread;
  MultiplexLooper _looper;
  TcpConnector _connector;
  SpTcpConnection _spConn;
  // callbacks for client code
  NewConnectionHandler _newConnHandler;
  // callbacks for client code
  NewMessageHandler _newMsgHandler;
};






