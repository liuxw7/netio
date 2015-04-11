
#include "TcpServer.hpp"
#include "Logger.hpp"
#include "Utils.hpp"


using namespace netio;

const char* TcpServer::LOG_TAG = "TcpServ";

TcpServer::TcpServer(uint16_t port, SpLooperPool loopPool) :
    _loopPool(loopPool),
    _mainLooper(loopPool->getLooper()),
    _acceptor(_mainLooper, port),
    _newConnHandler(std::bind(&TcpServer::dummyConnectionHandler, this, placeholders::_1))
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
  _acceptor.setNewConnCallback(bind(&TcpServer::OnNewConnection, this, placeholders::_1, placeholders::_2));
  _acceptor.attach();
}

void TcpServer::stopWork() {
  _acceptor.detach();
}

void TcpServer::OnNewConnection(int fd, const InetAddr& addr) {
  SpTcpConnection spConn = SpTcpConnection(new TcpConnection(_loopPool->getLooper(), fd, addr.getSockAddr()));
  LOGI(LOG_TAG ,"%s get new connection[%s]", __func__, spConn->strInfo());
  _connSet.insert(spConn);
  _newConnHandler(spConn);
}











