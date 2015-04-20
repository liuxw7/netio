
#include "Utils.hpp"
#include "TcpConnector.hpp"

using namespace netio;

TcpConnector::TcpConnector(MultiplexLooper* looper, uint16_t localPort, const InetAddr& remoteAddr) :
    _sock(localPort),
    _remoteAddr(remoteAddr),
    _channel(looper, _sock.getFd())
{
  ASSERT(_sock.getFd());

  FOGI("TcpConnector connection establish, localaddr=%s, remoteaddr=%s ", _sock.getLocalAddr().strIpPort().c_str(), remoteAddr.strIpPort().c_str());

  _channel.setReadCallback(bind(&TcpConnector::handleRead, this));
  _channel.enableRead(true);
}

TcpConnector::~TcpConnector() {
  ASSERT(!_channel.isAttached());
}

void TcpConnector::handleRead() {
  LOGW("tcc", "TcpConnector handle read hanppend by unexpected");
}

void TcpConnector::connectInternal() {
  int ret = _sock.connect(_remoteAddr.getSockAddr());
  if(0 == ret) {
    LOGD("tcc", "[%s==>%s]\tconnected ", _sock.getLocalAddr().strIpPort().c_str(), _remoteAddr.strIpPort().c_str());        
    _onNewConn(_sock.getFd(), _remoteAddr);
  } else {
    LOGI("tcc", "TcpConnector connect failed, msg=%s", strerror(errno));
  }
}

void TcpConnector::disconnectInternal() {
  _sock.close();
}


void TcpConnector::attach() {
  _channel.attach();
}

void TcpConnector::detach() {
  _channel.detach();
}










