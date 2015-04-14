#pragma once
/**
 * @file   TcpProxy.hpp
 * @author liuzf <liuzf@liuzf-H61M-DS2>
 * @date   Sun Apr 12 17:46:42 2015
 * 
 * @brief  Base tcp proxy for tcp server point, hold connection by TcpServer and managed by 
 *         SessionManager. 
 */

#include <memory>

#include "TcpServer.hpp"
#include "MultiplexLooper.hpp"
#include "LooperPool.hpp"
#include "NetPackDispatcher.hpp"
#include "FieldLenNetPack.hpp"
#include "TcpConnection.hpp"
#include "Session.hpp"
#include "TimerWrap.hpp"
#include "HashedWheelTimer.hpp"
#include "Logger.hpp"

using namespace netio;
using namespace std;

class TcpProxy {
 protected:
  typedef NetPackDispatcher<FLNPack, TcpConnection> TcpDispatcher;
  typedef FLNPack::MsgType::CmdType CmdType;
  typedef shared_ptr<LooperPool<MultiplexLooper> > SpLooperPool;
  typedef shared_ptr<TcpSession> SpSession;  

 public:

  TcpProxy(const SpLooperPool& loopers, uint16_t lport, uint32_t expired) :
      _loopPool(loopers),
      _server(lport, _loopPool),
      _sm(_loopPool->getLooper(), expired),
      _dispatcher()      
  {
    _server.setConnectionHandler(std::bind(&TcpProxy::onNewConnection, this, std::placeholders::_1));
    _server.setMessageHandler(std::bind(&TcpDispatcher::dispatch, &_dispatcher, placeholders::_1, placeholders::_2));    
  }
  
  TcpProxy(size_t threadCount, uint16_t lport, uint32_t expired) :
      _loopPool(new LooperPool<MultiplexLooper>(threadCount)),
      _server(lport, _loopPool),
      _sm(_loopPool->getLooper(), expired),
      _dispatcher()      
  {
    _server.setConnectionHandler(std::bind(&TcpProxy::onNewConnection, this, std::placeholders::_1));
    _server.setMessageHandler(std::bind(&TcpDispatcher::dispatch, &_dispatcher, placeholders::_1, placeholders::_2));
  }

  void startWork() {
    _server.startWork();
    _sm.enableIdleKick();
  }
  
  void stopWork() {
    _server.stopWork();
    _sm.disableIdleKick();
  }

  void registerHandler(const TcpDispatcher::Handler& handler) {
    _dispatcher.registerHandler(handler);
  }

  void registerHandler(TcpDispatcher::Handler&& handler) {
    _dispatcher.registerHandler(std::move(handler));
  }

  void registerHandler(CmdType& cmd, const TcpDispatcher::Handler& callback) {
    _dispatcher.registerHandler(cmd, callback);
  }
  
  void registerHandler(CmdType& cmd, TcpDispatcher::Handler&& callback) {
    _dispatcher.registerHandler(cmd, std::move(callback));
  }

 private:
  /** 
   * bind message callback with dispatcher when connnection established.
   * 
   * @param connection 
   */
  void onNewConnection(SpTcpConnection& connection) {
    FOGI("TcpProxy connection establish, remoteaddr=%s ", connection->getPeerAddr().strIpPort().c_str());
    connection->attach();
  }
  
  SpLooperPool _loopPool;
  TcpServer _server;
 protected:
  // session and dispatcher is logical reference, it will real imply in subclass.
  SessionManager<TcpSession> _sm;
  TcpDispatcher _dispatcher;
};


