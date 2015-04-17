#pragma once

#include <memory>
#include <functional>
#include <map>
#include <list>

#include "Noncopyable.hpp"
#include "VecBuffer.hpp"
#include "Utils.hpp"
#include "TcpConnection.hpp"
#include "UdpEndpoint.hpp"

using namespace std;

namespace netio {

template <typename CT>
struct Message {
  typedef CT CmdType;

  Message(CmdType cmd, SpVecBuffer& buffer) :
      _cmd(cmd),
      _buffer(buffer)
  {}

  Message() :
      _buffer(nullptr)
  {}

  const CmdType& getCmd() {
    return _cmd;
  }
  
  CmdType getCmd() const {
    return _cmd;
  }
  
  CmdType _cmd;
  SpVecBuffer _buffer;
};


class TcpSource {
 public:
  TcpSource(const SpTcpConnection& connection) :
      _conn(connection)
  {}

  TcpSource(SpTcpConnection&& connection) :
      _conn(std::move(connection))
  {}
  
  void send(const SpVecBuffer& buffer) {
    _conn->send(buffer);
  }

  void sendMultiple(list<SpVecBuffer>& datas) {
    _conn->sendMultiple(datas);
  }

  uint32_t getPeerIp() const {
    return _conn->getPeerIp();
  }

  uint16_t getPeerPort() const {
    return _conn->getPeerPort();
  }
 private:
  SpTcpConnection _conn;
};

class UdpSource {
 public:
  UdpSource(const SpUdpEndpoint& endpoint, uint32_t rip, uint16_t rport) :
      _ept(endpoint),
      _raddr(rip, rport)
  {}

  UdpSource(SpUdpEndpoint&& endpoint, uint32_t rip, uint16_t rport) :
      _ept(std::move(endpoint)),
      _raddr(rip, rport)
  {}

  void send(const SpVecBuffer& buffer) {
    _ept->send(buffer, _raddr);
  }

  void sendMultiple(list<SpVecBuffer>& datas) {

  }

  uint32_t getPeerIp() const {
    return _raddr.ip();
  }

  uint16_t getPeerPort() const {
    return _raddr.port();
  }
 private:
  SpUdpEndpoint _ept;
  InetAddr _raddr;
};

template <typename MsgType, typename SrcType>
class Dispatcher : public Noncopyable {
  typedef shared_ptr<MsgType> SpMsgType;
  typedef typename MsgType::CmdType CmdType;
 public:
  typedef function<void(SpMsgType&, SrcType&)> Handler;
  /** 
   * register handler for all messages.
   * 
   * @param callback 
   */
  void registerHandler(const Handler& callback) {
    _anyHandler.push_back(callback);
  }
  void registerHandler(Handler&& callback) {
    _anyHandler.push_back(std::move(callback));
  }

  /** 
   * register handler for specific message.
   * 
   * @param cmd 
   * @param callback 
   */
  void registerHandler(CmdType& cmd, const Handler& callback) {
    _handlerMap.insert(std::pair<CmdType, Handler>(cmd, callback));
  }
  void registerHandler(CmdType& cmd, Handler&& callback) {
    _handlerMap.insert(std::pair<CmdType, Handler>(cmd, std::move(callback)));
  }


  /** 
   * Dispatch message
   * 
   * @param cmd 
   * @param msg 
   * @param src buffer generate source.
   */
  void dispatch(const CmdType& cmd, SpMsgType& msg, SrcType& src) {
    auto anyIter = _anyHandler.begin();
    while(anyIter != _anyHandler.end()) {
      (*anyIter)(msg, src);
      anyIter ++;
    }

    auto specIter = _handlerMap.find(cmd);
    if(LIKELY(specIter != _handlerMap.end())) {
      (*specIter).second(msg, src);
    }
  }
 private:
  map<CmdType, Handler> _handlerMap;
  list<Handler> _anyHandler;
};


}











