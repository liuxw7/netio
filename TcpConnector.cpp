
#include "Utils.hpp"
#include "TcpConnector.hpp"

using namespace netio;

TcpConnector::TcpConnector(MultiplexLooper* looper, uint16_t localPort, const InetAddr& remoteAddr) :
    _sock(localPort),
    _remoteAddr(remoteAddr),
    _channel(looper, _sock.getFd())
{
  ASSERT(_sock.getFd());

  COGI("TcpConnector connection establish, localaddr=%s, remoteaddr=%s ", _sock.getLocalAddr().strIpPort().c_str(), remoteAddr.strIpPort().c_str());
  
  _channel.setReadCallback(bind(&TcpConnector::handleRead, this));
  _channel.enableRead(true);
}

TcpConnector::~TcpConnector() {
  ASSERT(!_channel.isAttached());
}

void TcpConnector::handleRead() {
  COGFUNC();
}

void TcpConnector::connectInternal() {
  int ret = _sock.connect(_remoteAddr.getSockAddr());
  if(0 == ret) {
    COGI("TcpConnector connection establish, localaddr=%s, remoteaddr=%s ", _sock.getLocalAddr().strIpPort().c_str(), _remoteAddr.strIpPort().c_str());
    _onNewConn(_sock.getFd(), _remoteAddr);
  } else {
    COGE("TcpConnection connect error");
  }
}


void TcpConnector::attach() {
  _channel.attach();
}

void TcpConnector::detach() {
  _channel.detach();
}
