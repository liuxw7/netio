#pragma once

#include <time.h>

#include "TcpConnection.hpp"

namespace netio {

class TcpSession {
 public:
  TcpSession(time_t ts, uint32_t id, SpTcpConnection& connection);
  
  void touch(time_t ts) {
    _updateTime = ts;
  }

  uint32_t getId() const {
    return _id;
  }

  time_t lastUpdateTime() const {
    return _updateTime;
  }

  time_t createTime() const {
    return _createTime;
  }
  
 private:
  uint32_t _id; // session id is unique global for distribute system.
  time_t _createTime;
  time_t _updateTime;
  SpTcpConnection _connection;
};

class TcpSessionMgr {
 public:
 private:
};

}

