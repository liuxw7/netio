#pragma once

#include <sys/epoll.h>
#include <unistd.h>
#include <map>
#include <mutex>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>

#include "Utils.hpp"
#include "Channel.hpp"
#include "Logger.hpp"

using namespace std;

namespace netio {

class ManageChannel {
 public:
  ManageChannel(MultiplexLooper* looper) : 
      _channel(looper, setupPair())
  {
    _channel.setReadCallback(std::bind(&ManageChannel::handleRead, this));
    _channel.enableRead(false);
    _channel.attach();
  }
  
  ~ManageChannel() {
    _channel.detach();
    ::close(_fds[0]);
    ::close(_fds[1]);
  }
  
  void wakeup() {
    CHKRET(::write(_fds[0], "w", 1));
  }

  void handleRead() {
    char temp[10] = {0};
    CHKRET(::read(_fds[1], temp, sizeof(temp)));
    COGI("wakeup channel, readed=%s", temp);
  }

 private:
  int setupPair() {
    CHKRET(::socketpair(AF_LOCAL, SOCK_STREAM, 0, _fds));
    return _fds[1];
  }
  // use socketpair, fd[0] for write, fd[1] for read.
  int _fds[2];
  Channel _channel;
};

/**
 * Multiplex looper.
 *
 * Use epoll to manage fds's read and write events.
 */
class MultiplexLooper {
  //  friend class ManageChannel;
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
    _mgrChan->wakeup();
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

  // Use for manage the looper, use internal.
  ManageChannel* _mgrChan;

  volatile bool _looping;
};

}

