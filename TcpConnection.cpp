
#include <strings.h>
#include <functional>

#include "InetSock.hpp"
#include "TcpConnection.hpp"
#include "VecBuffer.hpp"


using namespace netio;

thread_local int8_t TcpConnection::_rcvPendingBuffer[SIZE_K(32)];

TcpConnection::TcpConnection(MultiplexLooper* looper, int fd, const struct sockaddr_in& addr) :
    _peerAddr(addr),
    _sock(fd),
    _channel(looper, fd),
    _rcvBuf(new VecBuffer(SIZE_K(1)))
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
  ssize_t readed;
  struct iovec iovecs[2];
  xxxxxxxxxxxxx
  do {
    bzero(iovecs, sizeof(iovecs));
  } while((readed = _sock.readv(iovecs, 2)) > 0)

  iovecs[0].iov_base = _rcvBuf->writtablePtr();
  iovecs[0].iov_len = _rcvBuf->writtableSize();
  iovecs[1].iov_base = _rcvPendingBuffer;
  iovecs[1].iov_len = sizeof(_rcvPendingBuffer);

  ssize_t readed = _sock.readv(iovecs, 2);
  if(readed >= 0) {
    
  }
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

void TcpConnection::sendInternal() {
  unique_lock<mutex> lock(_sndMutex);

  size_t bufCount = _sndBufList.size();
  struct iovec* iovecs = new struct iovec[bufCount]();
    
  auto itSpBuf = _sndBufList.begin();
  for(int i = 0; i < bufCount; i++) {
    iovecs[i].iov_base = (*itSpBuf)->readablePtr();
    iovecs[i].iov_len = (*itSpBuf)->readableSize();
    itSpBuf++;
  }
    
  size_t sended = _sock.writev(iovecs, bufCount);
  
  if(LIKELY(sended > 0)) {
    markSended(sended);
  }
  delete[] iovecs;
}

void TcpConnection::markSended(size_t size) {
  size_t sended = size;
  auto itSpBuf = _sndBufList.begin();

  while(itSpBuf != _sndBufList.end()) {
    if(sended >= (*itSpBuf)->readableSize()) {
      _sndBufList.erase(itSpBuf++);
      sended -= (*itSpBuf)->readableSize();
    } else {
      (*itSpBuf)->markRead(sended);
      sended = 0;
    }

    if(0 == sended) {
      break;
    }
  }
}

