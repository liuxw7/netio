
#include <sys/types.h>
#include <sys/socket.h>
#include <vector>

#include "Utils.hpp"
#include "MultiplexLooper.hpp"
#include "Logger.hpp"


using namespace std;

namespace netio {

MultiplexLooper::MultiplexLooper() : _pollFd(epoll_create1(EPOLL_CLOEXEC)) {
  _wakeupChan = new EventChannel(this);
  _runnableChan = new EventChannel(this);

  _runnableChan->setEventHandler(std::bind(&MultiplexLooper::executeRunnables, this));
  
  CHKRET(_pollFd);
  ASSERT(nullptr != _wakeupChan);
  ASSERT(nullptr != _runnableChan);
  _looping = true;
}

MultiplexLooper::~MultiplexLooper() {
  delete _wakeupChan;
  if(_pollFd >= 0) {
    close(_pollFd);
  }
}

void MultiplexLooper::startLoop() {
  _threadId = this_thread::get_id();
  vector<struct epoll_event> events(20);
  
  COGFUNC();
  
  while(_looping) {
    int evCount = epoll_wait(_pollFd, &*events.begin(), events.size(), -1);
    
    if(UNLIKELY(evCount < 0)) {
      LOGE("Looper", "epoll_wait");
    }

    for(int i = 0; i < evCount; i++) {
      Channel* channel = static_cast<Channel*>(events[i].data.ptr);
      if(UNLIKELY(events[i].events & EPOLLHUP)) {
        channel->handleClosed();
      }

      if(events[i].events & EPOLLERR) {
        channel->handleError();
      }
      
      if(events[i].events & Channel::EVENT_READ) {
        channel->handleRead();
      }

      if(events[i].events & Channel::EVENT_WRITE) {
        channel->handleWrite();
      }
    }
    
    if(UNLIKELY(evCount == events.size())) {
      events.resize(events.size() * 2);
    }
  }

  COGI("loop done");
}

void MultiplexLooper::stopLoop() {
  _looping = false;
  wakeup();
}

}


