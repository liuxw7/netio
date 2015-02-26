
#include <sys/types.h>
#include <sys/socket.h>
#include <vector>

#include "Utils.hpp"
#include "MultiplexLooper.hpp"
#include "Logger.hpp"


using namespace std;

namespace netio {

MultiplexLooper::MultiplexLooper() : _pollFd(epoll_create1(EPOLL_CLOEXEC)) {
  _mgrChan = new ManageChannel(this);
  
  CHKRET(_pollFd);
  ASSERT(nullptr != _mgrChan);
  _looping = true;
}

MultiplexLooper::~MultiplexLooper() {
  delete _mgrChan;
  if(_pollFd >= 0) {
    close(_pollFd);
  }
}

void MultiplexLooper::startLoop() {
  vector<struct epoll_event> events(20);

  COGFUNC();
  
  while(_looping) {
    int evCount = epoll_wait(_pollFd, &*events.begin(), events.size(), -1);
    
    if(UNLIKELY(evCount < 0)) {
      LOGE("Looper", "epoll_wait");
    }

    for(int i = 0; i < evCount; i++) {
      Channel* channel = static_cast<Channel*>(events[i].data.ptr);
      if(events[i].events |= EPOLLIN) {
        channel->handleRead();
      }

      if(events[i].events |= EPOLLOUT) {
        channel->handleWrite();
      }
    }
    
    if(UNLIKELY(evCount == events.size())) {
      events.resize(events.size() * 2);
    }
  }

  COGI("start loop done");
}

void MultiplexLooper::stopLoop() {
  _looping = false;
  wakeup();
}

}


