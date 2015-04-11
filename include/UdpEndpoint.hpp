#pragma once

#include <functional>
#include <unistd.h>
#include <list>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>


#include "InetSock.hpp"
#include "Channel.hpp"
#include "LooperPool.hpp"
#include "MultiplexLooper.hpp"
#include "VecBuffer.hpp"
#include "FixedBuffer.hpp"
#include "Endian.hpp"
#include "Logger.hpp"
#include "Utils.hpp"

namespace netio {

typedef struct CachedBuffer {
  CachedBuffer(SpVecBuffer buffer, uint32_t rip, uint16_t rport) :
      _buffer(buffer),
      _rip(rip),
      _rport(rport)
  {}
  
  SpVecBuffer _buffer;
  uint32_t _rip;
  uint16_t _rport;
} CachedBuffer;

// LCC: fixed buffer cache pool count
// LCN: fixed buffer size in each buffer.
class UdpEndpoint {
  typedef function<void(SpVecBuffer&, uint32_t rip, uint16_t rport)> OnNewMessage;
  typedef shared_ptr<CachedBuffer> SpCacheBuffer;
 public:
  
  UdpEndpoint(MultiplexLooper* looper, uint16_t port, size_t mtu, int rcvBufSize = 0, int sndBufSize = 0) :
      _mtu(mtu),
      _sock(port),
      _channel(looper, _sock.getFd()),
      _rcvBuf(new VecBuffer(_mtu)),
      _newMessageHandler(std::bind(&UdpEndpoint::dummyNewMessageHandler, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3))
  {
    ASSERT(_sock.getFd() >= 0);
    _sock.setNonblocking(true);

    // set buffer size for socket
    if(rcvBufSize != 0) {
      _sock.setRecvBufSize(rcvBufSize);      
    }
    if(sndBufSize != 0) {
      _sock.setSendBufSize(sndBufSize);      
    }

    _channel.setReadCallback(std::bind(&UdpEndpoint::handleRead, this));
    _channel.setWriteCallback(std::bind(&UdpEndpoint::handleWrite, this));
    _channel.setCloseHandler(std::bind(&UdpEndpoint::handleClose, this));
    _channel.enableRead(true);
  }

  // attach and detach channel
  void attach() {
    ASSERT(_newMessageHandler);
    _channel.attach();
  }
  void detach() { _channel.detach(); }


  void setNewMessageHandler(const OnNewMessage& handler) {
    if(handler) {
      _newMessageHandler = handler;
    } else {
      _newMessageHandler = std::bind(&UdpEndpoint::dummyNewMessageHandler, this,
                                     std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
    }
  }
  void setNewMessageHandler(OnNewMessage&& handler) {
    if(handler) {
      _newMessageHandler = std::move(handler);       
    } else {
      _newMessageHandler = std::bind(&UdpEndpoint::dummyNewMessageHandler, this,
                                     std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
    }
  }

  void send(const SpVecBuffer& buffer, uint32_t rip, uint16_t rport) {
    auto runnable = [=] () {
      _sndList.push_back(SpCacheBuffer(new CachedBuffer(buffer, rip, rport)));
      sendInternal();
    };

    _channel.getLooper()->postRunnable(runnable);
  }
  
  void handleRead() {
    struct sockaddr_in raddr;
    while(true) {
      ssize_t readed = _sock.recvfrom(_rcvBuf->writtablePtr(), _rcvBuf->writtableSize(), raddr, 0);
      if(readed > 0) {
        _rcvBuf->markWrite(readed);
        _newMessageHandler(_rcvBuf, Endian::ntoh32(raddr.sin_addr.s_addr), Endian::ntoh16(raddr.sin_port));
        _rcvBuf.reset(new VecBuffer(_mtu));
      } else if(readed < 0) {
        // nothing readed
        if(EAGAIN != errno && EINTR != errno) {
          COGE("Udp connection got error, msg=%s", strerror(errno));
          detach();
          _sock.close();
          errno = 0;
        }
        break;
      } else {
        COGE("Udp connection close by peer ??? ");
        detach();
        _sock.close();
        break;
      }
    }
  }
  void handleWrite() {
    sendInternal();
  }
  void handleClose() {
    FOGI("Udp socket closed");
  }
 private:
  void dummyNewMessageHandler(SpVecBuffer& buffer, uint32_t rip, uint16_t rport) {
    COGW("udp port %d recv from ["IPQUAD_FMT":%u], size=%lu",
         _sock.getLocalAddr().port(),
         HIP_QUAD(rip),
         rport,
         buffer->readableSize());
  }

  void sendInternal() {
    while(!_sndList.empty()) {
      SpCacheBuffer& cached = _sndList.front();
      ssize_t sended = _sock.sendto(cached->_buffer->writtablePtr(), cached->_buffer->writtableSize(), 0, cached->_rip, cached->_rport);
      if(sended == cached->_buffer->writtableSize()) {
        _sndList.pop_front();
      } else if(sended > 0) {
        COGE("Udp sended size > 0 but not equal to buffer size");
        break;
      } else if(-1 == sended) {
        if(EAGAIN == errno || EINTR == errno) {
          _channel.enableWrite(true, true);
        } else {
          // error occur
          COGE("TcpConnection error occur when write fd=%d errno=%d", _sock.getFd(), errno);
          _sock.close();
        }
      }
    }
  }
  
  size_t _mtu;
  DGramSocket _sock;
  Channel _channel;
  SpVecBuffer _rcvBuf;
  list<shared_ptr<CachedBuffer> > _sndList;
  OnNewMessage _newMessageHandler;
};

}
