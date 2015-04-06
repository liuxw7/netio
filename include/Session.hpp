#pragma once

/**
 * @file   Session.hpp
 * @author liuzf <liuzf@liuzf-H61M-DS2>
 * @date   Mon Apr  6 14:31:17 2015
 * 
 * @brief  
 *
 * session combind user and the connection.
 *
 * When receive a heartbeat packet, we raise a session, the session key represent remote ip:port.
 * and bind with uid. The session manager hold to map, one map with key uid, and another with remote info.
 * tcp connection is fd, and udp is ip:port 
 */


#include <atomic>
#include <stdint.h>
#include <memory>
#include <netinet/in.h>
#include <map>
#include <mutex>

#include "TcpConnection.hpp"
#include "VecBuffer.hpp"

using namespace std;

namespace netio {

class Session {
 public:
  Session(uint32_t uin, uint32_t sessKey) :
      _uin(uin),
      _sk(sessKey),
      _tsCreate(TimeUtil::timestampMS()),
      _tsUpdate(_tsCreate),
      _seq(0)
  {}
  
  Session(uint32_t uin, uint32_t sessKey, uint32_t createTime) :
      _uin(uin),
      _sk(sessKey),
      _tsCreate(createTime),
      _tsUpdate(_tsCreate),
      _seq(0)
  {}
  
  void touch(uint64_t updateTime) {
    _tsUpdate = updateTime;
  }

  void touch() {
    _tsUpdate = TimeUtil::timestampMS();
  }

  uint32_t incSeq() {
    return _seq.fetch_add(1);
  }

  uint64_t lastUpdateTime() const {
    return _tsUpdate;
  }

  uint64_t createTime() const {
    return _tsCreate;
  }

  uint32_t sessionKey() const {
    return _sk;
  }

  uint32_t uin() const {
    return _uin;
  }

  uint64_t sid() const {
    return _sid;
  }

 protected:
  uint64_t _sid; // other key generate by real session.
 private:
  uint32_t _uin;
  uint32_t _sk; // session key for one user session.
  uint64_t _tsCreate;
  uint64_t _tsUpdate;
  atomic<uint32_t> _seq; // Sequence number just for generate request number.
};

class TcpSession : public Session {
 public:
  TcpSession(uint32_t uin, uint32_t sessKey, SpTcpConnection& connection, uint64_t createTime) :
      Session(uin, sessKey, createTime),
      _conn(connection)
  {}

  TcpSession(uint32_t uin, uint32_t sessKey, SpTcpConnection& connection) :
      Session(uin, sessKey),
      _conn(connection)
  {}
  
  void send(const SpVecBuffer& buffer) {
    _conn->send(buffer);
  }

  void sendMultiple(list<SpVecBuffer>& datas) {
    _conn->sendMultiple(datas);
  }

  void getFd() const {
    return _conn->getFd();
  }

 private:
  SpTcpConnection _conn;
};

class UdpSession : public Session {
 public:
  
 private:
  uint32_t _rip;
  uint16_t _rport;
};

class SessionManager {
 private:
  uint64_t generateSessionId() {
    
  }
}

}

















