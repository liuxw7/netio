#pragma once

#include <netinet/in.h>

#include "MultiplexLooper.hpp"
#include "Channel.hpp"
#include "InetAddr.hpp"
#include "InetSock.hpp"

namespace netio {

class TcpConnection {
 public:
  TcpConnection(MultiplexLooper* looper, int fd, const struct sockaddr_in& addr);
  ~TcpConnection();

  // callbacks for read/write error and close event.
  void handleRead();
  void handleWrite();
  void handleError();
  void handleClose();

  // attach and detach channel
  void attach();
  void detach();
  
 private:
  InetAddr _peerAddr;
  StreamSocket _sock;
  Channel _channel;
};

}

















