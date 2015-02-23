#pragma once

#include "PeerMessage.hpp"
#include "Netpack.hpp"
#include "Connection.hpp"
#include "Channel.hpp"
#include "InetSock.hpp"
#include "InetAddr.hpp"
#include "string.h"

using namespace std;

namespace netio {

/**
 * 
 */
template <typename NP=FieldLenNetpack<GenericLenFieldHeader> >
class TcpConnection : public Connection {
 public:
  /**
   * Constructor usually for accepted connection.
   */
  explicit TcpConnection(int fd) :
      _sock(fd)
  {}

  /**
   * Constructor usually for client connection.
   */
  TcpConnection(uint16_t port) :
      _sock(port)
  {}

  /**
   * Get socket fd that the TcpConnection hold.
   */
  int getFd() const {
    return _sock.getFd();
  }

  /**
   * Create buffer that reserved Netpack layout buffer memory.
   */
  static SpVecBuffer createPackLayoutBuffer(size_t size) {
    return NP::createPrependVecBuffer(size);
  }

  /**
   * Send buffer that packed before.
   *
   * This function just append buffet to channel to be send.
   */
  void send(const SpVecBuffer& packed) {
    _channel.sendPackedBuffer(packed);
  }

  /**
   * Send, copy when nessesary.
   */
  void send(PeerMessage& pm) {
    _channel.sendPeerMessage(pm);
  }

  /**
   * Send, copy when nessesary.
   */
  void send(PMProto proto, uint32_t version, uint32_t cmd, uint32_t seq,
            SpVecBuffer& buffer) {
    struct PMInfo info(proto, version, cmd, seq);
    _channel.sendPeerMessage(info, buffer);
  }

  /**
   * Send, copy when nessesary.
   */
  void send(const struct PMInfo& info, SpVecBuffer& buffer) {
    _channel.sendPeerMessage(info, buffer);
  }

  /**
   * Copy send
   */
  void send(PMProto proto, uint32_t version, uint32_t cmd, uint32_t seq,
            const void* data, size_t len) {
    struct PMInfo info(proto, version, cmd, seq);
    send(info, data, len);
  }

  /**
   * Copy send.
   */
  void send(const struct PMInfo& info, void* data, size_t size) {
    SpVecBuffer spBuf = _channel.createPrependVecBuffer(size);
    ASSERT(nullptr != spBuf);
    ::memcpy(spBuf->writtablePtr(), data, size);
    spBuf->markWrite(size);
    NP::writePendingInfo(info, spBuf);
  }

  /**
   * Get socket local address information.
   */
  InetAddr getLocalAddr() const {
    struct sockaddr_in addr;
    bzero(&addr, sizeof(addr));
  
    socklen_t addrlen = static_cast<socklen_t>(sizeof(addr));
    int ret = ::getsockname(_sock.getFd(), SOCKADDR_CAST(&addr), &addrlen);
    return InetAddr(addr);
  }

  /**
   * Get socket peer address information.
   */
  InetAddr getPeerAddr() const {
    struct sockaddr_in addr;
    bzero(&addr, sizeof(addr));
  
    socklen_t addrlen = static_cast<socklen_t>(sizeof(addr));
    int ret = ::getpeername(_sock.getFd(), SOCKADDR_CAST(&addr), &addrlen);
    return InetAddr(addr);
  }

  /**
   * Do real send task.
   * This function must run in just one thread.
   *
   * @return : return of writev
   */
  ssize_t sendInternal() {
    function<size_t(const iovec*, int)> functor(bind(&StreamSocket::writev, &_sock, placeholders::_1, placeholders::_2));
    return _channel.doRealSend(functor);
  }

  /**
   * Do real receive task.
   * This function must run just in one thread.
   */
  

 private:
  Channel<NP> _channel;
  StreamSocket _sock;
};

}

