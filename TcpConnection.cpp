
#include <functional>

#include "TcpConnection.hpp"
#include "VecBuffer.hpp"

using namespace netio;

TcpConnection::TcpConnection(MultiplexLooper* looper, int fd, const struct sockaddr_in& addr) :
    _peerAddr(addr),
    _sock(fd),
    _channel(looper, fd)
{
  ASSERT(fd >= 0);

  
  _channel.setReadCallback(std::bind(&TcpConnection::handleRead, this));
  _channel.enableRead(true);
}

TcpConnection::~TcpConnection() {
  ASSERT(!_channel.isAttached());
}

void TcpConnection::handleRead() {
  
}

void TcpConnection::handleWrite() {
  
}

void TcpConnection::attach() {
  _channel.attach();
}

void TcpConnection::detach(){
  _channel.detach();
}



















