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
  Session(int localFd, uint32_t rip, uint16_t rport, uint32_t uin, uint32_t sessKey) :
      _cid(addrToCid(localFd, rip, rport)),
      _uin(uin),
      _sk(sessKey),
      _tsCreate(TimeUtil::timestampMS()),
      _tsUpdate(_tsCreate),
      _seq(0)
  {}
  
  Session(int localFd, uint32_t rip, uint16_t rport, uint32_t uin, uint32_t sessKey, uint32_t createTime) :
      _cid(addrToCid(localFd, rip, rport)),      
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

  uint64_t cid() const {
    return _cid;
  }

 protected:
  uint64_t addrToCid(int localFd, uint32_t rip, uint16_t rport) {
    return (static_cast<uint64_t>(rip) << 32) | (rport << 16) | (localFd & 0xFF);
  }
  
  uint64_t _cid; // other key generate by real session.
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
      Session(0, connection->getPeerIp(), connection->getPeerPort(), uin, sessKey, createTime),
      _conn(connection)
  {}

  TcpSession(uint32_t uin, uint32_t sessKey, SpTcpConnection& connection) :
      Session(0, connection->getPeerIp(), connection->getPeerPort(), uin, sessKey),
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
  typedef shared_ptr<TcpSession> SpSession;
 public:
  void addSession(const SpSession& session) {
    unique_lock<mutex> lock(_mutex);
    _cidMap.insert(std::pair<uint64_t, SpSession>(session->cid(), session));
    _uinMap.insert(std::pair<uint32_t, SpSession>(session->uin(), session));
  }

  void removeSession(SpSession& session) {
    unique_lock<mutex> lock(_mutex);
    _cidMap.erase(session->cid());
    _uinMap.erase(session->uin());
  }

  SpSession findSessionByUin(uint32_t uin) {
    auto iter = _uinMap.find(uin);
    if(iter != _uinMap.end()) {
      return (*iter).second;
    }
    return nullptr;
  }

  SpSession findSessionByCid(uint64_t cid) {
    auto iter = _cidMap.find(cid);
    if(iter != _cidMap.end()) {
      return (*iter).second;
    }
    return nullptr;
  }


  void touchSessionByUin(uint32_t uin) {
    unique_lock<mutex> lock(_mutex);
    
    auto iter = _uinMap.find(uin);
    if(iter != _uinMap.end()) {
      (*iter).second->touch();
    }
  }

  void touchSessionByUin(uint32_t uin, uint64_t ts) {
    unique_lock<mutex> lock(_mutex);
    
    auto iter = _uinMap.find(uin);
    if(iter != _uinMap.end()) {
      (*iter).second->touch(ts);
    }
  }

  void touchSessionByCid(uint64_t cid) {
    unique_lock<mutex> lock(_mutex);

    auto iter = _cidMap.find(cid);
    if(iter != _cidMap.end()) {
      (*iter).second->touch();
    }
  }

  void touchSessionByCid(uint64_t cid, uint64_t ts) {
    unique_lock<mutex> lock(_mutex);
    auto iter = _cidMap.find(cid);
    if(iter != _cidMap.end()) {
      (*iter).second->touch(ts);
    }
  }

  void sendToUin(uint32_t uin, const SpVecBuffer& buffer) {
    unique_lock<mutex> lock(_mutex);
    
    auto iter = _uinMap.find(uin);
    if(iter != _uinMap.end()) {
      (*iter).second->send(buffer);
    }        
  }

  void sendMultipleToUin(uint32_t uin, list<SpVecBuffer>& datas) {
    unique_lock<mutex> lock(_mutex);
    
    auto iter = _uinMap.find(uin);
    if(iter != _uinMap.end()) {
      (*iter).second->sendMultiple(datas);
    }        
  }

  void sendToCid(uint64_t cid, const SpVecBuffer& buffer) {
    unique_lock<mutex> lock(_mutex);

    auto iter = _cidMap.find(cid);
    if(iter != _cidMap.end()) {
      (*iter).second->send(buffer);
    }
  }

  void sendMultipleToCid(uint64_t cid, list<SpVecBuffer>& datas) {
    unique_lock<mutex> lock(_mutex);

    auto iter = _cidMap.find(cid);
    if(iter != _cidMap.end()) {
      (*iter).second->sendMultiple(datas);
    }
  }
  
 private:
  map<uint64_t, SpSession> _cidMap;
  map<uint32_t, SpSession> _uinMap;

  mutable mutex _mutex;
}

}

