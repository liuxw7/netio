#pragma once

#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <mutex>
#include <list>
#include <thread>
#include <errno.h>
#include <string>

#include "MultiplexLooper.hpp"
#include "Channel.hpp"
#include "InetAddr.hpp"
#include "InetSock.hpp"
#include "Utils.hpp"
#include "VecBuffer.hpp"


namespace netio {

/**
 * Cause connction is real data read/write channel, we have codec for data to pack to structured.
 * template parameter @NP specify the netpack codec.
 * 
 * Codec must have static method :
 *   // read the first PeerMessage from vector buffer if the buffer contain one or more message.
 *   // otherwise return nullptr
 *   SpPeerMessage readPeerMessage(SpVecBuffer& buffer)
 *   // From current buffer, we forecast how much bytes the buffer must have to contain current message.
 *   // if there is not enough buffer to predict, return negative number.
 *   ssize_t getExpectLen(const SpVecBuffer& buffer)
 */
class TcpConnection : public enable_shared_from_this<TcpConnection> {
  typedef shared_ptr<TcpConnection> SpTcpConnection; 
  typedef function<void(SpTcpConnection, SpVecBuffer&)> OnNewMessage;
  typedef function<void(SpTcpConnection, int)> OnConnClose;
 public:
  TcpConnection(MultiplexLooper* looper, int fd, const struct sockaddr_in& addr) :
      _rcvBuf(new VecBuffer(_predMsgLen)),
      _peerAddr(addr),
      _sock(fd),
      _channel(looper, fd)
  {
    ASSERT(fd >= 0);
    // set non blocking mode for TcpConnection
    _sock.setNonblocking(true);

    // setup events callback.
    _channel.setReadCallback(std::bind(&TcpConnection::handleRead, this));
    _channel.setWriteCallback(std::bind(&TcpConnection::handleWrite, this));
    _channel.setCloseHandler(std::bind(&TcpConnection::handleClose, this));
  
    // enable readble event by default.
    //    _channel.enableAll(true);
    _channel.enableRead(true);

    // for debug
    bzero(_strInfo, sizeof(_strInfo));
    snprintf(_strInfo, sizeof(_strInfo), "[%d,%s:%u]\t", _sock.getFd(), _peerAddr.strIp().c_str(), _peerAddr.port());
    LOGI("tc", "%s created", _strInfo);
  }
  
  ~TcpConnection() {
    ASSERT(!_channel.isAttached());
    LOGI("tc", "%s destroy", _strInfo);
    _sock.close();
  }

  int getFd() const {
    return _sock.getFd();
  }

  // callbacks for read/write error and close event from looper thread.
  void handleRead();
  
  void handleWrite() {
    sendInternal();
  }
  
  void handleClose() {
    if(_closedHandler) {
      _closedHandler(this->shared_from_this(), errno);
    }
  }

  // attach and detach channel
  void attach() { _channel.attach(); }
  void detach() { _channel.detach(); }

  void setNewMessageHandler(const OnNewMessage& handler) { _newMessageHandler = handler; }
  void setNewMessageHandler(OnNewMessage&& handler) { _newMessageHandler = std::move(handler); }

  void setCloseHandler(const OnConnClose& handler) { _closedHandler = handler; }
  void setCloseHandler(OnConnClose&& handler) { _closedHandler = std::move(handler); }

  // get remote address information
  InetAddr getPeerAddr() const { return _peerAddr; }
  uint32_t getPeerIp() const { return _peerAddr.ip(); }
  uint16_t getPeerPort() const { return _peerAddr.port(); }

  void sendMultiple(list<SpVecBuffer>& datas) {
    auto runnable = [&, datas] () {
      _sndBufList.splice(_sndBufList.end(), static_cast<list<SpVecBuffer> >(datas));
      sendInternal();
    };

    _channel.getLooper()->postRunnable(runnable);
  }
  
  void send(const SpVecBuffer& data) {
    auto runnable = [&, data] () {
      _sndBufList.push_back(data);
      sendInternal();
    };

    _channel.getLooper()->postRunnable(runnable);
  }
  

  const char* strInfo() const { 
    return _strInfo;
  }

  string getPeerInfo() const {
    return _peerAddr.strIpPort();
  }

  MultiplexLooper* getLooper() const {
    return _channel.getLooper();
  }

 private:
  // this function is called by looper when writtable event happened.
  void sendInternal();

  // mark size of bytes sended, not thread safe.
  void markSended(size_t size) { 
    size_t sended = size;
    auto itSpBuf = _sndBufList.begin();

    while(itSpBuf != _sndBufList.end()) {
      if(sended >= (*itSpBuf)->readableSize()) {
        _sndBufList.erase(itSpBuf++);
        sended -= (*itSpBuf)->readableSize();
      } else {
        (*itSpBuf)->markRead(sended);
        sended = 0;
      }

      if(0 == sended) {
        break;
      }
    }
  }

  void sendInLoopThread() {
    _channel.getLooper()->postRunnable(std::bind(&TcpConnection::sendInternal, this));
  }

  void dummyNewMessageHandler(SpTcpConnection conn, SpVecBuffer& buffer) {
    LOGW("tc", "%s receive buffer size=%d, dummy", conn->strInfo());
  }
  
  // send buffer
  mutable mutex _sndMutex;
  list<SpVecBuffer> _sndBufList;

  // receive buffer
  SpVecBuffer _rcvBuf;

  // connection.
  InetAddr _peerAddr;
  StreamSocket _sock;
  Channel _channel;

  // callbacks
  OnNewMessage _newMessageHandler;
  OnConnClose _closedHandler; // The handler will be called after fd closed.

  static const size_t _predMsgLen;
  
  // use this buffer if there is much buffer to read, reduce calling recv system call.
  static __thread int8_t _rcvPendingBuffer[SIZE_K(32)];
  char _strInfo[40]; // fd(10) + ,(1) + ip:port(21) + [](2) = 34
};

typedef shared_ptr<TcpConnection> SpTcpConnection;

}




