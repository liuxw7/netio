
#include <strings.h>
#include <functional>

#include "InetSock.hpp"
#include "TcpConnection.hpp"
#include "VecBuffer.hpp"


using namespace netio;

__thread int8_t TcpConnection::_rcvPendingBuffer[SIZE_K(32)];
__thread char TcpConnection::_strInfo[100];
const size_t TcpConnection::_predMsgLen = SIZE_K(1);


void TcpConnection::handleRead() {
  while(true) {
    struct iovec iovecs[2];
    ssize_t readed;
    size_t readCap;

    // if _rcvBuf has not readable data and more than half size of _rcvBuf was used,
    // reset it, we will create new buffer for receive.
    if(0 == _rcvBuf->readableSize() && ((_rcvBuf->writtableSize() << 1) < _predMsgLen)) {
      _rcvBuf.reset(new VecBuffer(_predMsgLen));
    }

    readCap = _rcvBuf->writtableSize() + sizeof(_rcvPendingBuffer);


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
      if(LIKELY(_newMessageHandler)) {
        _newMessageHandler(this->shared_from_this(), _rcvBuf);
      } else {
        COGW("tcpconnection callback, but no message handler");
      }

        
      if(LIKELY(static_cast<size_t>(readed) < readCap)) {
        // we have read all data out
        break;
      }
    } else if(readed < 0) {
      // nothing readed
      if(EAGAIN != errno && EINTR != errno) {
        COGI("TcpConnection [%s] close by peer", getPeerInfo().c_str());
        detach();
        _sock.close();
        if(LIKELY(nullptr != _closedHandler)) {
          _closedHandler(this->shared_from_this(), errno);
        } else {
          COGW("connection closed, but we don't have close handler");
        }

        errno = 0;
      }
      break;
    } else { // we got eof
      detach();
      _sock.close();
      if(LIKELY(nullptr != _closedHandler)) {
        _closedHandler(this->shared_from_this(), 0);
      } else {
        COGW("connection closed, but we don't have close handler");
      }
      break;
    }
  }
}

void TcpConnection::sendInternal() {
  constexpr int vecMax = 50;

  COGD("%s buffe",__func__);

  // break on these cases:
  // 1. nothing to be send
  // 2. send EAGAIN
  // 3. error occur
  while(true) {
    struct iovec iovecs[vecMax] = {0};
    int vecCount = 0;
    {
      unique_lock<mutex> lock(_sndMutex);

      if(!_sndBufList.empty()) {
        vecCount = std::min(_sndBufList.size(), ARRAY_SIZE(iovecs));

        auto itSpBuf = _sndBufList.begin();
        for(int i = 0; i < vecCount; i++) {
          iovecs[i].iov_base = (*itSpBuf)->readablePtr();
          iovecs[i].iov_len = (*itSpBuf)->readableSize();
          COGI("iov_len = %lu", iovecs[i].iov_len);
          itSpBuf++;
        }
      } else {
        break;
      }
    }
      
    // vecCount will be positive
    if(vecCount > 0) {
      size_t sended = _sock.writev(iovecs, vecCount);
      COGD("%s has sended %d" , __func__, sended);

      if(LIKELY(sended > 0)) {
        unique_lock<mutex> lock(_sndMutex);
        markSended(sended);
      } else {
        if(EAGAIN == errno || EINTR == errno) {
          _channel.enableWrite(true, true);
        } else {
          // error occur
          COGE("TcpConnection error occur when write fd=%d errno=%d", _sock.getFd(), errno);
          _sock.close();
          handleClose();
        }
        break;
      }
    } else {
      break;
    }
  }
}



