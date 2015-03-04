#pragma once

#include <sys/epoll.h>
#include <unistd.h>
#include <map>
#include <mutex>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/eventfd.h>

#include "Utils.hpp"
#include "Channel.hpp"
#include "Logger.hpp"

using namespace std;

namespace netio {

class EventChannel {
  typedef function<void(void)> EventHandler;
 public:
  EventChannel(MultiplexLooper* looper) : 
	_evfd(eventfd(0, EFD_CLOEXEC | EFD_NONBLOCK)), _channel(looper, _evfd)
  {
	ASSERT(_evfd >= 0);
    _channel.setReadCallback(std::bind(&EventChannel::handleRead, this));
    _channel.enableRead(true);
    _channel.attach();
  }
  
  ~EventChannel() {
    _channel.detach();
    ::close(_evfd);
  }
  
  void wakeup() {
	int64_t value = 1;
    CHKRET(::write(_evfd, &value, sizeof(value)));
  }

  void handleRead() {
	int64_t value;
    CHKRET(::read(_evfd, &value, sizeof(value)));
    COGI("wakeup channel, readed=%ld", value);
	
	if(_handleRead) {
	  _handleRead();
	}
  }

  void setEventHandler(EventHandler& eventHandler) {
	_handleRead = eventHandler;
  }

 private:
  // we use eventfd for epoll event notify.
  int _evfd;
  Channel _channel;
  EventHandler _handleRead;
};

/**
 * Multiplex looper.
 *
 * Use epoll to manage fds's read and write events.
 */
class MultiplexLooper {
  //  friend class EventChannel;
 public:
  MultiplexLooper();
  ~MultiplexLooper();

  /**
   * Add channel to be managed by the poller.
   */
  void attachChannel(Channel& channel) {
    updateChannel(EPOLL_CTL_ADD, channel);
    //    _chanMap.insert(std::pair<int, Channel*>(channel.fd(), &channel));
  }

  /**
   * remove channel.
   */
  void detachChannel(Channel& channel) {
    updateChannel(EPOLL_CTL_DEL, channel);
  }
  
  /**
   * Start loop.
   *
   * NOTE : if we have preset channel, we must install before startLoop.
   */
  void startLoop();

  /**
   * Stop loop.
   */
  void stopLoop();

  /**
   * update channel
   */
  void updateChannel(Channel& channel) {
    updateChannel(EPOLL_CTL_MOD, channel);
  }

  void wakeup() {
    _wakeupChan->wakeup();
  }
  
 private:
  void updateChannel(int operation, Channel& channel) {
    COGFUNC();
    struct epoll_event event;
    bzero(&event, sizeof(struct epoll_event));
    event.events = channel.getEvents();
    event.data.ptr = &channel;
    int ret = epoll_ctl(_pollFd, operation, channel.fd(), &event);
    ASSERT(ret >= 0);
  }

  // epoll fd;
  int _pollFd;

  // channel map <fd, chnanel> to hold chanel that attach to this object.
  // use for debug.
  map<int, Channel*> _chanMap;
  
  // function scadualer
  EventChannel* _funcChan;


  // Use for manage the looper, use internal.
  EventChannel* _wakeupChan;
  
  volatile bool _looping;
};

}











