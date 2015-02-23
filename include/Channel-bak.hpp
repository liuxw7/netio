#pragma once

#include <mutex>
#include <functional>

#include "PeerMessage.hpp"
#include "VecBuffer.hpp"

using namespace std;

namespace netio {

/**
 * Channel for connection orient TCP protocol.
 * It combines netpack protocol and channel buffer.
 * 
 * This class will hold channel buffer and netpack protocol.
 * when send data, this class will encode message with netpack, and depack
 * when receive data.
 *
 * template NP specify netpack protocol that apply when sending or receiving data.
 * NP must have static function:
 */
// default Channel template parameter
// template <class NP=FieldLenNetpack<GenericLenFieldHeader> >
template <typename NP>
class Channel {
 public:
  Channel(size_t initRecvSize = SIZE_K(1)) :
      _initRecvSize(initRecvSize),
      _recvBuf(new VecBuffer(initRecvSize)),
      _sndMutex(),
      _sendList()
  {}

  /**
   * Create prepend vec buffer.
   */
  static SpVecBuffer createPrependVecBuffer(size_t size) {
    return NP::createPrependVecBuffer(size);
  }

  /**
   * Append buffer to send queue.
   */
  void sendPeerMessage(PeerMessage& pm) {
    sendPeerMessage(pm._info, pm._buffer);
  }

  /**
   * Append buffer to send queue
   */
  void sendPeerMessage(const struct PMInfo& info, SpVecBuffer& data) {
    if(UNLIKELY(nullptr == data)) {
      SpVecBuffer headBuf = NP::createPendingBuffer(info, 0);
      {
        unique_lock<mutex> lock(_sndMutex);
        _sendList.push_back(headBuf);
      }
    } else if(data->getOffset() > 0) {
      NP::writePendingInfo(info, data);
      // append buffer to send list
      {
        unique_lock<mutex> lock(_sndMutex);
        _sendList.push_back(data);
      }
    } else {
      SpVecBuffer headBuf = NP::createPendingBuffer(info, data->readableSize());
      // append buffer to send list
      {
        unique_lock<mutex> lock(_sndMutex);
        _sendList.push_back(headBuf);
        _sendList.push_back(data);
      }
    }
  }

  /**
   * Append buffer to send queue that pack encoded already.
   */
  void sendPackedBuffer(SpVecBuffer& buf) {
    unique_lock<mutex> lock(_sndMutex);
    _sendList.push_back(buf);
  }

  /**
   * Process the receiving buffer.
   *
   * Generate the first complete PeerMessage if there exist, fix recv buffer. Otherwise,
   * If readable size is 0, and writtable size is less than half of _initRecvSize, create new recv buffer.
   * If the recv buffer not enough to receive current complete PeerMessage we predict base on header, ensure
   * the capacity of buffer.
   * And if threre is not enough readable buffer to predict the size of PeerMessage, we ensure the writtable size
   * with default initial recv buffer size.
   *
   * @return : if new peer message created, return true, otherwise return false.
   */
  SpPeerMessage doRecvProcess() {
    // read and parse PeerMessage, fix buffer.
    SpPeerMessage spPeerMsg = NP::readPeerMessage(_recvBuf);

    // The buffer contain not completed PeerMessage.
    if(nullptr == spPeerMsg) {
      // create new recvbuffer.
      if((0 == _recvBuf->readableSize()) && (_recvBuf->writtableSize() < _initRecvSize)) {
        _recvBuf.reset(new VecBuffer(_initRecvSize));
      } else {
        ssize_t expect = NP::peekPackLen(_recvBuf);
        if(expect < 0) {
          expect = _initRecvSize;
        }

        _recvBuf->ensure(expect);
      }
    }

    return spPeerMsg;
  }

  /**
   * Peek sending list.
   */
  const list<SpVecBuffer>& peekSendList() const {
    return _sendList;
  }

  /**
   * Real tcp connection send. we use writev because we manager send buffer
   * scatter.
   */
  size_t doRealSend(function<size_t(const iovec*, int)>& functor) {
    unique_lock<mutex> lock(_sndMutex);

    size_t bufCount = _sendList.size();
    struct iovec* iovecs = new struct iovec[bufCount]();

    auto itSpBuf = _sendList.begin();
    for(int i = 0; i < bufCount; i++) {
      iovecs[i].iov_base = (*itSpBuf)->readablePtr();
      iovecs[i].iov_len = (*itSpBuf)->readableSize();
      itSpBuf++;
    }
    
    ssize_t sended = functor(iovecs, bufCount);
    if(LIKELY(sended > 0)) {
      markSended(sended);
    }
    delete[] iovecs;
    return sended;
  }

  /**
   * Mark sended.
   */
  void markSended(size_t size) {
    size_t sended = size;
    auto itSpBuf = _sendList.begin();

    while(itSpBuf != _sendList.end()) {
      if(sended >= (*itSpBuf)->readableSize()) {
        _sendList.erase(itSpBuf++);
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
  

 private:
  mutable mutex _sndMutex;
  list<SpVecBuffer> _sendList;
  SpVecBuffer _recvBuf;
  size_t _initRecvSize;
};

}



