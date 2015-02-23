#pragma once

#include <stdint.h>
#include <thread>
#include <vector>
#include "InetSock.hpp"

using namespace std;

namespace netio {
/**
 * TcpPump.
 *
 * Contain one send thread, one receive thread, and some receive process threads.
 *
 */
class TcpPump {
 public:
  // we can explicitely specify process threads count by threads parameter.
  explicit TcpPump(uint16_t port, size_t threads = 5);
  explicit TcpPump(int fd, size_t threads = 5);
  TcpPump(const struct sockaddr_in& sockaddr, size_t threads = 5);

  ~TcpPump();

  void startWork();
  
 private:
  void receiveLooper();
  void sendLooper();

  // be careful the order of member variable.
  ServerSocket _srvSock;

  shared_ptr<thread> _spSender;
  shared_ptr<thread> _spReceiver;
  
  // thread _sender;
  // thread _receiver;
  // vector<thread> _processors;

  int _epollFd;
};

}

















