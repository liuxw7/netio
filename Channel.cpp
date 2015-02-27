
#include <sys/epoll.h>

#include "Channel.hpp"
#include "MultiplexLooper.hpp"
#include "Logger.hpp"

using namespace netio;

const uint32_t Channel::WATCH_NONE = 0;
const uint32_t Channel::WATCH_READ = EPOLLIN;
const uint32_t Channel::WATCH_WRITE = EPOLLOUT;
const uint32_t Channel::EDGE_TRIGGER = EPOLLET;
const EventHanler Channel::DUMMY_HANDLE = []() {};

Channel::Channel(MultiplexLooper* looper, int fd) :
    _fd(fd),
    _looper(looper),
    _attached(false),
    _events(WATCH_NONE)
{
  installDummy();
}

void Channel::enableRead(bool edgeTrigger) {
  _events = WATCH_READ;
  if(edgeTrigger) {
    _events |= EDGE_TRIGGER;
  }
    
  apply();
}
  
void Channel::enableWrite(bool edgeTrigger) {
  _events = WATCH_WRITE;
  if(edgeTrigger) {
    _events |= EDGE_TRIGGER;
  }

  apply();
}

void Channel::enableAll(bool edgeTrigger) {
  _events = WATCH_READ | WATCH_WRITE;
  if(edgeTrigger) {
    _events |= EDGE_TRIGGER;
  }

  apply();
}

void Channel::attach() {
  ASSERT(_fd >= 0);
  
  if(!_attached) {
    _looper->attachChannel(*this);
    _attached = true;
  }
}

void Channel::detach() {
  if(_attached) {
    _looper->detachChannel(*this);
    _attached = false;
  }
}

void Channel::apply() {
  if(_attached) {
    _looper->updateChannel(*this);
  }
}


