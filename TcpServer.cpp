
#include "TcpServer.hpp"
#include "Logger.hpp"
#include "Utils.hpp"


using namespace netio;

const char* TcpServer::LOG_TAG = "TcpServ";

TcpServer::TcpServer(uint16_t port, SpLooperPool loopPool) :
    _loopPool(loopPool),
    _mainLooper(loopPool->getLooper()),
    _acceptor(_mainLooper, port)
{
}

TcpServer::~TcpServer() {
}


void TcpServer::startWork() {
  ASSERT(_newConnHandler);
  _acceptor.setNewConnCallback(bind(&TcpServer::OnNewConnection, this, placeholders::_1, placeholders::_2));
  _acceptor.attach();
}

void TcpServer::stopWork() {
  _acceptor.detach();
}

void TcpServer::OnNewConnection(int fd, const InetAddr& addr) {
  SpTcpConnection spConn = SpTcpConnection(new TcpConnection(_loopPool->getLooper(), fd, addr.getSockAddr()));
  LOGI(LOG_TAG ,"%s get new connection[%s]", __func__, spConn->strInfo());
  
  int hash = TcpServer::connectionHashCode(spConn);
  
  _connMap[hash] = spConn; 
  _newConnHandler(hash, spConn);
}
