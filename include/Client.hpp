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
  typedef NetPackDispatcher<FLNPack, TcpConnection> TcpDispatcher;
 public:
  TcpClient(const char* rip, uint16_t rport) :
      _thread(nullptr),
      _looper(),
      _connector(&_looper, 0, InetAddr(rip, rport)),
      _dispatcher()
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
  
 private:
  void onNewConnection(int fd, const InetAddr& addr)  {
    COGFUNC();
  }
  
  unique_ptr<thread> _thread;
  MultiplexLooper _looper;
  TcpConnector _connector;
  TcpDispatcher _dispatcher;
};



















