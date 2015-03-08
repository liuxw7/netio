#pragma once

#include <functional>

#include "Logger.hpp"

using namespace std;

namespace netio {

class Channel;
class MultiplexLooper;

typedef function<void()> EventHanler;

class Channel {
 public:
  Channel(MultiplexLooper*, int);
  //  Channel(MultiplexLooper*);
  int fd() const { return _fd; }

  // get channel's looper
  MultiplexLooper* getLooper() const { return _looper; }

  void setReadCallback(const EventHanler& handler) {
    _readEventHandler = handler;
  }

  void setWriteCallback(const EventHanler& handler) {
    _writeEventHandler = handler;
  }

  void setErrorHandler(const EventHanler& handler) {
    _errorHandler = handler;
  }

  void setCloseHandler(const EventHanler& handler) {
    _closedHandler = handler;
  }
  
#ifdef __GXX_EXPERIMENTAL_CXX0X__
  void setReadCallback(const EventHanler&& handler) {
    _readEventHandler = std::move(handler);
  }

  void setWriteCallback(const EventHanler&& handler) {
    _writeEventHandler = std::move(handler);
  }

  void setErrorHandler(const EventHanler&& handler) {
    _errorHandler = std::move(handler);
  }

  void setCloseHandler(const EventHanler&& handler) {
    _closedHandler = std::move(handler);
  }
#endif

  void handleRead() { _readEventHandler(); }
  void handleWrite() { _writeEventHandler(); }
  void handleError() { _errorHandler(); }
  void handleClosed() { _closedHandler(); }
  
  const static uint32_t EVENT_NONE;
  const static uint32_t EVENT_READ;
  const static uint32_t EVENT_WRITE;
  const static uint32_t EDGE_TRIGGER;
  const static uint32_t ONESHOT;
  
  // read/write event switcher.
  void setWatchEevents(uint32_t events);
  void enableRead(bool edgeTrigger);
  void enableWrite(bool edgeTrigger);
  void enableWrite(bool edgeTrigger, bool oneShot);
  void enableAll(bool edgeTrigger);

  uint32_t getEvents() const {
    return _events;
  }

  // affect the looper
  bool isAttached() const { return _attached; }
  void attach();
  void detach();
  void apply();
 private:
  int _fd;

  // events
  uint32_t _events;

  // event callbacks 
  EventHanler _readEventHandler;
  EventHanler _writeEventHandler;
  EventHanler _errorHandler;
  EventHanler _closedHandler;

  // is channel attach to owner looper
  bool _attached;

  // specify the owner looper.
  // We have raw pointer to reference owner thread.
  // So must garentee the life recycle shorter than owner thread.
  MultiplexLooper* _looper;

  static const EventHanler DUMMY_HANDLE;
  void defaultErrorHandler() {
    COGE("Channel detect error occur, fd=%d errno=%d", _fd, errno);
    _closedHandler();
  }
  
  void installDefaultHandler() {
    _readEventHandler = DUMMY_HANDLE;
    _writeEventHandler = DUMMY_HANDLE;
    _closedHandler = DUMMY_HANDLE;
    _errorHandler = std::bind(&Channel::defaultErrorHandler, this);
  }
};

}















