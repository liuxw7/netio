
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
  // set non blocking mode for TcpConnection
  _sock.setNonblocking(true);

  // setup events callback.
  _channel.setReadCallback(std::bind(&TcpConnection::handleRead, this));
  _channel.setWriteCallback(std::bind(&TcpConnection::handleWrite, this));
  _channel.setErrorHandler(std::bind(&TcpConnection::handleError, this));
  _channel.setCloseHandler(std::bind(&TcpConnection::handleClose, this));
  
  // enable readble event by default.
  _channel.enableRead(true);
}

TcpConnection::~TcpConnection() {
  ASSERT(!_channel.isAttached());
}

void TcpConnection::handleRead() {
  
}

void TcpConnection::handleWrite() {
  
}

void TcpConnection::handleError() {
  
}

void TcpConnection::handleClose() {
  
}

void TcpConnection::attach() {
  _channel.attach();
}

void TcpConnection::detach(){
  _channel.detach();
}



















