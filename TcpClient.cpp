#include "TcpClient.hpp"

using namespace netio;


TcpClient::TcpClient(const char* rip, uint16_t rport) :
    _thread(nullptr),
    _looper(),
    _connector(&_looper, 0, InetAddr(rip, rport)),
    _spConn(nullptr),
    _newConnHandler(std::bind(&TcpClient::dummyConnectionHandler, this, std::placeholders::_1)),
    _newMsgHandler(std::bind(&TcpClient::dummyMessageHandler, this, std::placeholders::_1, std::placeholders::_2))
{}

void TcpClient::startWork() {
  _thread = unique_ptr<thread>(new thread(std::bind(&MultiplexLooper::startLoop, &_looper)));
  
  _connector.setNewConnCallback(std::bind(&TcpClient::onNewConnection, this, std::placeholders::_1, std::placeholders::_2));
  _connector.attach();
  _connector.connect();
}
  
void TcpClient::stopWork()  {
  _connector.detach();
  _connector.disconnect();
  _looper.stopLoop();
  _thread->join();
}

void TcpClient::setConnectionHandler(const NewConnectionHandler& handler) {
  if(handler) {
    _newConnHandler = handler;      
  } else {
    _newConnHandler = std::bind(&TcpClient::dummyConnectionHandler, this, placeholders::_1);
  }
}

void TcpClient::setConnectionHandler(NewConnectionHandler&& handler) {
  if(handler) {
    _newConnHandler = std::move(handler);      
  } else {
    _newConnHandler = std::bind(&TcpClient::dummyConnectionHandler, this, placeholders::_1);      
  }
}

void TcpClient::setMessageHandler(const NewMessageHandler& handler) {
  if(handler) {
    _newMsgHandler = handler;
  } else {
    _newMsgHandler = std::bind(&TcpClient::dummyMessageHandler, this, placeholders::_1, placeholders::_2);
  }
}

void TcpClient::setMessageHandler(NewMessageHandler&& handler) {
  if(handler) {
    _newMsgHandler = std::move(handler);
  } else {
    _newMsgHandler = std::bind(&TcpClient::dummyMessageHandler, this, placeholders::_1, placeholders::_2);
  }
}

void TcpClient::dummyConnectionHandler(SpTcpConnection& conn) {
  LOGW("tci", "%s new connection, dummy", conn->strInfo());
}
void TcpClient::dummyMessageHandler(SpTcpConnection& conn, SpVecBuffer& buffer) {
  LOGW("tci", "%s recv, size=%d, dummy", conn->strInfo(), buffer->readableSize());
}
