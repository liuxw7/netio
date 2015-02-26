

#include "TcpServer.hpp"


using namespace netio;

TcpServer::TcpServer(uint16_t port, int threads) :
    _loopPool(threads)
{
  
}

void TcpServer::OnNewConnection(int fd, const InetAddr& addr) {
  SpPMAddr spAddr = SpPMAddr(new struct PMAddr);
  spAddr->_fd = fd;
  spAddr->_addr = addr.getSockAddr();
  //  SpTcpConnection spConn = SpTcpConnection(new TcpConnection)
}



















