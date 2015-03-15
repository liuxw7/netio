#pragma once

#include "TcpServer.hpp"
#include "Dispatcher.hpp"

namespace netio {

/**
 * 
 */
template <class CMDTYPE>
class TcpProxy {
  typedef shared_ptr<LooperPool<MultiplexLooper> > SpLooperPool;
 public:
  /**
   * @loopsers hold threads for process events.
   * @servPort for client connection.
   * @pushPort for inner module transport that aim for send data to client
   */
  explicit TcpProxy(SpLooperPool& loopers, uint16_t servPort, uint16_t pushPort);

  /**
   * 
   */
  
  
  
  Dispatcher& getDispatcher() { return _dispatcher; }
 private:
  
  TcpServer _server;
  Dispatcher<CMDTYPE> _dispatcher;
};

}

