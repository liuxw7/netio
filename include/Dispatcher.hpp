#pragma once

#include <memory>
#include <functional>
#include <map>
#include <list>

#include "Noncopyable.hpp"
#include "VecBuffer.hpp"
#include "Utils.hpp"

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

template <typename MsgType, typename SrcType>
class Dispatcher : public Noncopyable {
  typedef shared_ptr<MsgType> SpMsgType;
  typedef shared_ptr<SrcType> SpSrcType;
  typedef typename MsgType::CmdType CmdType;
  typedef function<void(SpMsgType&, SpSrcType&)> Handler;
 public:
  /** 
   * register handler for all messages.
   * 
   * @param callback 
   */
  void registerAnyHandler(const Handler& callback) {
    _anyHandler.push_back(callback);
  }
  void registerAnyHandler(Handler&& callback) {
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
  void dispatch(const CmdType& cmd, SpMsgType& msg, SpSrcType& src) {
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

