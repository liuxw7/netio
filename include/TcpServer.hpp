#pragma once

#include <stdint.h>
#include <map>
#include <memory>

#include "PeerMessage.hpp"
#include "TcpConnection.hpp"
#include "TcpAcceptor.hpp"
#include "LooperPool.hpp"

using namespace std;

namespace netio {


typedef shared_ptr<TcpAcceptor> SpTcpAcceptor;
typedef shared_ptr<TcpConnection> SpTcpConnection;

class TcpServer {
 public:
  // port to listen
  TcpServer(uint16_t port, int threads);

 private:
  void OnNewConnection(int fd, const InetAddr& addr);

  LooperPool _loopPool;
};

}

















