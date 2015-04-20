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
#include "TimeUtil.hpp"
#include "UdpEndpoint.hpp"
#include "HashedWheelTimer.hpp"
#include "TimerWrap.hpp"
#include "MultiplexLooper.hpp"

using namespace std;

namespace netio {

template <class SRCType>
class Session {
  typedef weak_ptr<HashedWheelTimeout> WpWheelTimeout;
 public:
  static uint64_t genConnectId(const SRCType& src) {
    uint64_t cid = 0L;
    cid = src.getPeerIp();
    cid = (cid << 32) | src.getPeerPort();
    return cid;
  }
  
  Session(uint32_t uin, uint32_t sessKey, const SRCType& src) :
      _cid(genConnectId(src)),
      _uin(uin),
      _sk(sessKey),
      _tsCreate(TimeUtil::timestampMS()),
      _tsUpdate(_tsCreate),
      _seq(0),
      _timeout(),
      _source(src)
  {}

  Session(uint32_t uin, uint32_t sessKey, SRCType&& src) :
      _cid(genConnectId(src)),
      _uin(uin),
      _sk(sessKey),
      _tsCreate(TimeUtil::timestampMS()),
      _tsUpdate(_tsCreate),
      _seq(0),
      _timeout(),
      _source(std::move(src))
  {}
  
  Session(uint32_t uin, uint32_t sessKey, uint32_t createTime, const SRCType& src) :
      _cid(genConnectId(src)),
      _uin(uin),
      _sk(sessKey),
      _tsCreate(createTime),
      _tsUpdate(_tsCreate),
      _seq(0),
      _timeout(),
      _source(src)
  {}

  Session(uint32_t uin, uint32_t sessKey, uint32_t createTime, SRCType&& src) :
      _cid(genConnectId(src)),
      _uin(uin),
      _sk(sessKey),
      _tsCreate(createTime),
      _tsUpdate(_tsCreate),
      _seq(0),
      _timeout(),
      _source(std::move(src))
  {}

  ~Session() {
    //    LOGI("Session","session deconstruct use count=%d", _source._conn.use_count());
  }
  
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

  void resetTimeout(const shared_ptr<HashedWheelTimeout>& timeout) {
    if(shared_ptr<HashedWheelTimeout> spTimeout = _timeout.lock()) {
      spTimeout->cancel();
    }
    _timeout = timeout;
  }

  void cancelTimeout() {
    if(auto spTimeout = _timeout.lock()) {
      spTimeout->cancel();
    }
  }

  const SRCType& getSource() const {
    return _source;
  }

  const char* getPeerInfo() {
    return _source.getPeerInfo();
  }

 protected:
  uint64_t addrToCid(int localFd, uint32_t rip, uint16_t rport) {
    return (static_cast<uint64_t>(rip) << 32) | (rport << 16) | (localFd & 0xFF);
  }

 private:
  uint64_t _cid; // connection key witch specify  
  uint32_t _uin;
  uint32_t _sk; // session key for one user session.
  uint64_t _tsCreate;
  uint64_t _tsUpdate;
  atomic<uint32_t> _seq; // Sequence number just for generate request number.
  WpWheelTimeout _timeout; // session timeout pointer, use for cancle timeout.
  SRCType _source; // TcpSource or UdpSource
};

/*
 * SRCType -> source type : TcpSource, UdpSource
 *
 * This will hold all session in server point. We can find session by uin or cid.
 * 
 */
template <typename SRCType>
class SessionManager {
  enum { TimerInterval = 100 };
  
  typedef Session<SRCType> SessionType;
  typedef shared_ptr<SessionType> SpSession;
  typedef function<void(const SpSession&)> PreSessionRemove;
  typedef shared_ptr<HashedWheelTimeout> SpWheelTimeout;
  
  //  typedef shared_ptr<TcpSession> SpSession;
 public:
  SessionManager(MultiplexLooper* looper, uint32_t expireMS) :
      _uinMap(),
      _cidMap(),
      _idleExpireMs(expireMS),
      _timer(looper, TimerInterval, expireMS / TimerInterval)
  {}

  void handlerHeartbeatReq(const SRCType& src, uint32_t uin, uint32_t sessKey, uint32_t seq) {
    uint64_t cid = SessionType::genConnectId(src);
    SpSession spSession = findSessionByCid(cid);

    if(nullptr != spSession) {
      touchSession(spSession);
    } else {
      spSession = SpSession(new SessionType(uin, sessKey, src));
      addSession(spSession);
      touchSession(spSession);
    }
  }
    
  void addSession(const SpSession& spSession) {
    LOGI("tsm", "%s add session uin=%d, sk=%d", spSession->getPeerInfo(), spSession->uin(), spSession->sessionKey());
    {
      unique_lock<mutex> lck1(_uinMutex1, defer_lock);
      unique_lock<mutex> lck2(_cidMutex2, defer_lock);
      lock(lck1, lck2);
    
      _cidMap.insert(std::pair<uint64_t, SpSession>(spSession->cid(), spSession));
      _uinMap.insert(std::pair<uint32_t, SpSession>(spSession->uin(), spSession));
    }
  }

  void removeSession(const SpSession& spSession) {
    LOGI("tsm", "%s remove session uin=%d", spSession->getPeerInfo(), spSession->uin());
    _sessionRemoveHandler(spSession);

    {
      unique_lock<mutex> lck1(_uinMutex1, defer_lock);
      unique_lock<mutex> lck2(_cidMutex2, defer_lock);
      lock(lck1, lck2);
    
      _cidMap.erase(spSession->cid());
      _uinMap.erase(spSession->uin());      
    }
  }

  SpSession findSessionByCid(uint64_t cid) {
    unique_lock<mutex> lck(_cidMutex2);
    auto iter = _cidMap.find(cid);
    if(iter != _cidMap.end()) {
      return (*iter).second;
    }

    return nullptr;
  }

  void touchSession(uint64_t cid) {
    SpSession spSession = findSessionByCid(cid);
    touchSession(spSession);
  }

  void touchSession(const SpSession& spSession) {
    auto rmfunc = std::bind(&SessionManager<SRCType>::removeSession, this, spSession);
    SpWheelTimeout timeoutPtr = _timer.addTimeout(rmfunc, _idleExpireMs);
    spSession->resetTimeout(timeoutPtr);
    spSession->touch();
  }

  void sendToUin(uint32_t uin, const SpVecBuffer& buffer) {
    list<SpSession> sessionList;
    {
      unique_lock<mutex> lck(_uinMutex1);
      auto iterRange = _uinMap.equal_range(uin);
      for(auto iter = iterRange.first; iter != iterRange.second; ++ iter) {
        sessionList.push_back((*iter).second);
      }
    }
    auto iter = sessionList.begin();
    while(iter != sessionList.end()) {
      (*iter)->send(buffer);
      ++ iter;
    }
  }

  void sendMultipleToUin(uint32_t uin, list<SpVecBuffer>& datas) {
    list<SpSession> sessionList;
    {
      unique_lock<mutex> lck(_uinMutex1);
      auto iterRange = _uinMap.equal_range(uin);
      for(auto iter = iterRange.first; iter != iterRange.second; ++ iter) {
        sessionList.push_back((*iter).second);
      }
    }
    auto iter = sessionList.begin();
    while(iter != sessionList.end()) {
      (*iter)->sendMultiple(datas);
      ++ iter;
    }
  }

  void setSessionRemoveHandler(const PreSessionRemove& handler) {
    _sessionRemoveHandler = handler;
  }

  void setSessionRemoveHandler(PreSessionRemove&& handler) {
    _sessionRemoveHandler = std::move(handler);
  }

  void startWork() {
    _timer.attach();
  }

  void stopWork() {
    _timer.detach();
  }

 private:
  // suport multi login future
  mutable mutex _uinMutex1;
  multimap<uint32_t, SpSession> _uinMap;
  // connection map
  mutable mutex _cidMutex2;
  map<uint64_t, SpSession> _cidMap;
  // idle session kick timer
  uint32_t _idleExpireMs;
  TimerWrap<HashedWheelTimer> _timer;
  
  PreSessionRemove _sessionRemoveHandler;  
};

}











