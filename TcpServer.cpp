
#include "TcpServer.hpp"
#include "Logger.hpp"
#include "Utils.hpp"


using namespace netio;

const char* TcpServer::LOG_TAG = "TcpServ";

TcpServer::TcpServer(uint16_t port, int threads) :
    _loopPool(threads),
    _acceptor(_loopPool.getLooper(), port)
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
  LOGI(LOG_TAG ,"%s get new connection fd=%d peer=%s", __func__, fd, addr.strIpPort().c_str());
  SpTcpConnection spConn = SpTcpConnection(new TcpConnection<GenFieldLenPack>(_loopPool.getLooper(), fd, addr.getSockAddr()));
  _newConnHandler(spConn);
}












