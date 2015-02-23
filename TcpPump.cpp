
#include <unistd.h>
#include <functional>
#include <sys/epoll.h>

#include "TcpPump.hpp"
#include "Logger.hpp"
#include "Utils.hpp"

#define CHKRET(x)  (ASSERT((x) >= 0))

using namespace std;

namespace netio {

TcpPump::TcpPump(uint16_t port, size_t threads) :
    _epollFd(-1),
    _srvSock(port),
    _spSender(nullptr),
    _spReceiver(nullptr)
{
  ASSERT(_srvSock.getFd() >= 0);
}

TcpPump::TcpPump(int fd, size_t threads) :
    _epollFd(-1),    
    _srvSock(fd),
    _spSender(nullptr),
    _spReceiver(nullptr)
{
  ASSERT(_srvSock.getFd() >= 0);
}

TcpPump::TcpPump(const struct sockaddr_in& sockaddr, size_t threads) :
    _epollFd(-1),    
    _srvSock(sockaddr),
    _spSender(nullptr),
    _spReceiver(nullptr)
{
  ASSERT(_srvSock.getFd() >= 0);
}

TcpPump::~TcpPump() {
  ::close(_epollFd);
}

void TcpPump::sendLooper() {
  
}

void TcpPump::receiveLooper() {
  
}

void TcpPump::startWork() {
  CHKRET(_srvSock.listen());
  CHKRET(_epollFd = ::epoll_create1(EPOLL_CLOEXEC));

  /*
  struct epoll_event event;
  bzero(&event, sizeof event);
  event.events = channel->events();
  event.data.ptr = channel;
  int fd = channel->fd();
  */
  
  _spSender.reset(new thread(std::bind(&TcpPump::sendLooper, this)));
  _spReceiver.reset(new thread(std::bind(&TcpPump::receiveLooper, this)));

  _spSender->detach();
  _spReceiver->detach();
}

}



















