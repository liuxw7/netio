#pragma once

#include <thread>
#include <memory>

#include "TcpConnection.hpp"
#include "TcpConnector.hpp"
#include "MultiplexLooper.hpp"

using namespace netio;

class TcpClient {
  typedef function<void(SpTcpConnection&)> NewConnectionHandler;
  typedef function<void(SpTcpConnection&, SpVecBuffer&)> NewMessageHandler;  
 public:
  TcpClient(const char* rip, uint16_t rport);
  ~TcpClient() {}

  /** 
   * Control the client work state
   */
  void startWork();
  void stopWork();

  /** 
   * Set callback when connection establish
   * 
   * @param handler 
   */
  void setConnectionHandler(const NewConnectionHandler& handler);
  void setConnectionHandler(NewConnectionHandler&& handler);

  /** 
   * Set callback when TcpConnection readable
   * 
   * @param handler 
   */
  void setMessageHandler(const NewMessageHandler& handler);
  void setMessageHandler(NewMessageHandler&& handler);

  SpTcpConnection getConnection() const {
    return _spConn;
  }
  
  private:
  /** 
   * default callback for new connection and readable event
   * 
   * @param conn 
   */
  void dummyConnectionHandler(SpTcpConnection& conn);
  void dummyMessageHandler(SpTcpConnection& conn, SpVecBuffer& buffer);

  void onNewConnection(int fd, const InetAddr& addr) {
    SpTcpConnection spConn = SpTcpConnection(new TcpConnection(&_looper, fd, addr.getSockAddr()));
    _spConn = spConn;
    _newConnHandler(spConn);
  }
  
  unique_ptr<thread> _thread;
  MultiplexLooper _looper;
  TcpConnector _connector;
  SpTcpConnection _spConn;
  // callbacks for client code
  NewConnectionHandler _newConnHandler;
  // callbacks for client code
  NewMessageHandler _newMsgHandler;
};

