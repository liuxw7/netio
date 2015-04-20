
#include "TcpServer.hpp"
#include "Logger.hpp"
#include "Utils.hpp"


using namespace netio;

const char* TcpServer::LOG_TAG = "TcpServ";

TcpServer::TcpServer(uint16_t port, SpLooperPool loopPool) :
    _loopPool(loopPool),
    _mainLooper(loopPool->getLooper()),
    _acceptor(_mainLooper, port),
    _newConnHandler(std::bind(&TcpServer::dummyConnectionHandler, this, placeholders::_1)),
    _newMsgHandler(std::bind(&TcpServer::dummyMessageHandler, this, placeholders::_1, placeholders::_2)),
    _closeConnHandler(std::bind(&TcpServer::dummyResetConnectionHandler, this, placeholders::_1, placeholders::_2))
{
}

TcpServer::~TcpServer() {
  auto iter = _connSet.begin();
  while(iter != _connSet.end()) {
    (*iter)->detach();
    iter++;
  }
  _connSet.clear();
  _acceptor.detach();
}


void TcpServer::startWork() {
  //  ASSERT(_newConnHandler);
  _acceptor.setNewConnCallback(bind(&TcpServer::onNewConnection, this, placeholders::_1, placeholders::_2));
  _acceptor.attach();
}

void TcpServer::stopWork() {
  _acceptor.detach();
}

void TcpServer::onNewConnection(int fd, const InetAddr& addr) {
  SpTcpConnection spConn = SpTcpConnection(new TcpConnection(_loopPool->getLooper(), fd, addr.getSockAddr()));
  _connSet.insert(spConn);
  _newConnHandler(spConn);
  spConn->setNewMessageHandler(_newMsgHandler);
  spConn->setCloseHandler(_closeConnHandler);
}













