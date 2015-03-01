#pragma once

#include <netinet/in.h>
#include <mutex>
#include <list>
#include <thread>
#include <errno.h>

#include "PeerMessage.hpp"
#include "MultiplexLooper.hpp"
#include "Channel.hpp"
#include "InetAddr.hpp"
#include "InetSock.hpp"
#include "Utils.hpp"


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
template <class NP>
class TcpConnection : public enable_shared_from_this<TcpConnection<NP> > {
  typedef shared_ptr<TcpConnection<NP> > SpTcpConnection; 
  typedef decltype(NP::readMessage(*(new SpVecBuffer(nullptr)))) MsgType;
  typedef function<void(SpTcpConnection, MsgType&)> OnNewMessage;
  typedef function<void(SpTcpConnection, int)> OnConnClose;
 public:
  TcpConnection(MultiplexLooper* looper, int fd, const struct sockaddr_in& addr) :
      _peerAddr(addr),
      _sock(fd),
      _channel(looper, fd),
      _rcvBuf(new VecBuffer(_predMsgLen))
  {
    ASSERT(fd >= 0);
    // set non blocking mode for TcpConnection
    _sock.setNonblocking(true);

    // setup events callback.
    _channel.setReadCallback(std::bind(&TcpConnection::handleRead, this));
    _channel.setWriteCallback(std::bind(&TcpConnection::handleWrite, this));
    _channel.setErrorHandler(std::bind(&TcpConnection::handleError, this));
    _channel.setCloseHandler(std::bind(&TcpConnection::handleClose, this));
  
    // enable readble event by default.
    _channel.enableRead(true);
  }
  
  ~TcpConnection() {
    ASSERT(!_channel.isAttached());
  }


  // callbacks for read/write error and close event from looper thread.
  void handleRead() {
    while(true) {
      struct iovec iovecs[2];
      ssize_t readed;
      size_t readCap = _rcvBuf->writtableSize() + sizeof(_rcvPendingBuffer);

      iovecs[0].iov_base = _rcvBuf->writtablePtr();
      iovecs[0].iov_len = _rcvBuf->writtableSize();
      iovecs[1].iov_base = _rcvPendingBuffer;
      iovecs[1].iov_len = sizeof(_rcvPendingBuffer);

      readed = _sock.readv(iovecs, ARRAY_SIZE(iovecs));

      // we have read some buffer
      if(readed > 0) {
        ssize_t pending = (readed - _rcvBuf->writtableSize());

        if(UNLIKELY(pending > 0)) {
          // we read some data to pending buffer, merge them first.
          _rcvBuf->markWrite(_rcvBuf->writtableSize());
          _rcvBuf->enlarge(pending);
          memcpy(_rcvBuf->writtablePtr(), _rcvPendingBuffer, pending);
        } else {
          _rcvBuf->markWrite(readed);
        }

        
        // _rcvBuf currentlly store all buffer we have read this time
        MsgType msg;
        while(true) {
          msg = procRecvBuffer();
          if(msg != nullptr) {
            break;
          }
          
          // we have readed one message.
          _newMessageHandler(this->shared_from_this(), msg);
        };

        
        if(LIKELY(readed < readCap)) {
          // we have read all data out
          break;
        }
      } else if(readed < 0) {
        // nothing readed
        if(EAGAIN != errno && EINTR != errno) {
          _closeHandler(this->shared_from_this(), errno);
          errno = 0;
        }
        break;
      } else { // we got eof
        _closeHandler(this->shared_from_this(), 0);
        break;
      }
    }
  }
  
  void handleWrite() {
    
  }
  
  void handleError() {
    
  }
  
  void handleClose() {
    
  }

  // attach and detach channel
  void attach() { _channel.attach(); }
  void detach() { _channel.detach(); }

  // get remote address information
  InetAddr getPeerAddr() const { return _sock.getPeerAddr(); }

  // for send packed message
  void send(const SpVecBuffer& data) {
    unique_lock<mutex> lck(_sndMutex);
    _sndBufList.push_back(data);
  }
  
  void send(SpVecBuffer&& data) {
    unique_lock<mutex> lck(_sndMutex);
    _sndBufList.push_back(std::move(data));
  }
 private:
  MsgType procRecvBuffer() {
    MsgType message = NP::readMessage(_rcvBuf);

    // _rcvBuf not contain complete message
    if(nullptr == message) {
      if((0 == _rcvBuf->readableSize()) && (_rcvBuf->writtableSize() < _predMsgLen)) {
        _rcvBuf.reset(new VecBuffer(_predMsgLen));
      } else {
        ssize_t expect = NP::peekMessageLen(_rcvBuf);
        if(expect < 0) {
          expect = _predMsgLen;
        }

        _rcvBuf->ensure(expect);
      }
    }

    return message;
  }
  
  // this function is called by looper when writtable event happened.
  void sendInternal() {
    unique_lock<mutex> lock(_sndMutex);

    size_t bufCount = _sndBufList.size();
    struct iovec* iovecs = new struct iovec[bufCount]();
    
    auto itSpBuf = _sndBufList.begin();
    for(int i = 0; i < bufCount; i++) {
      iovecs[i].iov_base = (*itSpBuf)->readablePtr();
      iovecs[i].iov_len = (*itSpBuf)->readableSize();
      itSpBuf++;
    }
    
    size_t sended = _sock.writev(iovecs, bufCount);
  
    if(LIKELY(sended > 0)) {
      markSended(sended);
    }
    delete[] iovecs;
  }

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
  OnConnClose _closeHandler;

  static const size_t _predMsgLen;
  
  // use this buffer if there is much buffer to read, reduce calling recv system call.
  static __thread int8_t _rcvPendingBuffer[SIZE_K(32)];
};

template <typename NP>
__thread int8_t TcpConnection<NP>::_rcvPendingBuffer[SIZE_K(32)];

template <typename NP>
const size_t TcpConnection<NP>::_predMsgLen = SIZE_K(1);


}



















