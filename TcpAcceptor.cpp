
#include "Utils.hpp"
#include "TcpAcceptor.hpp"
#include "TcpConnection.hpp"
#include "Utils.hpp"
#include "Logger.hpp"


using namespace netio;

TcpAcceptor::TcpAcceptor(MultiplexLooper* looper, uint16_t port) :
    _sock(port),
    _channel(looper, _sock.getFd())
{
  ASSERT(_sock.getFd());
  CHKRET(_sock.listen());

  COGI("TcpAcceptor listen addr=%s", _sock.getLocalAddr().strIpPort().c_str());
  
  _channel.setReadCallback(bind(&TcpAcceptor::handleRead, this));
  _channel.enableRead(false);
}

TcpAcceptor::~TcpAcceptor() {
  ASSERT(!_channel.isAttached());
}

void TcpAcceptor::handleRead() {
  struct sockaddr_in clientaddr;
  bzero(&clientaddr, sizeof(struct sockaddr_in));
  int fd = _sock.accept(clientaddr);

  if(LIKELY(fd >= 0)) {
    if(LIKELY(_onNewConn)) {
      _onNewConn(fd, clientaddr);
    } else {
      ::close(fd);
    }
  } else {
    LOGSYSERR();
  }

  COGI("Tcp Acceptor get new connection : fd = %d", fd);
}

void TcpAcceptor::attach() {
  _channel.attach();
}

void TcpAcceptor::detach() {
  _channel.detach();
}










