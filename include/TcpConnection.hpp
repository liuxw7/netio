#pragma once

#include <netinet/in.h>
#include <mutex>
#include <list>
#include <thread>

#include "PeerMessage.hpp"
#include "MultiplexLooper.hpp"
#include "Channel.hpp"
#include "InetAddr.hpp"
#include "InetSock.hpp"
#include "Utils.hpp"


namespace netio {

class TcpConnection : public enable_shared_from_this<TcpConnection> {

 public:
  TcpConnection(MultiplexLooper* looper, int fd, const struct sockaddr_in& addr);
  ~TcpConnection();

  // callbacks for read/write error and close event.
  void handleRead();
  void handleWrite();
  void handleError();
  void handleClose();

  // attach and detach channel
  void attach();
  void detach();

  // get remote address information
  InetAddr getPeerAddr() const { return _sock.getPeerAddr(); }

  // for send message
  void send(const SpVecBuffer& data) {
    unique_lock<mutex> lck(_sndMutex);
    _sndBufList.push_back(data);
  }

  void send(SpVecBuffer&& data) {
    unique_lock<mutex> lck(_sndMutex);
    _sndBufList.push_back(data);
  }

 private:
    // this function is called by looper when writtable event happened.
  void sendInternal();
  void markSended(size_t size); // mark size of bytes sended, not thread safe.
  
 // send buffer
  mutable mutex _sndMutex;
  list<SpVecBuffer> _sndBufList;

  // receive buffer
  SpVecBuffer _rcvBuf;

  // connection.
  InetAddr _peerAddr;
  StreamSocket _sock;
  Channel _channel;

  // use this buffer if there is much buffer to read, reduce calling recv system call.
  static thread_local int8_t _rcvPendingBuffer[SIZE_K(32)];
};

}



