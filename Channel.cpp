
#include "PeerMessage.hpp"
#include "Channel.hpp"


namespace netio {

template <class NP>
void Channel<NP>::doSend(PeerMessage& pm) {
  SpVecBuffer data = pm._buffer;
  if(data->getOffset() > 0) {
    NP::writePendingInfo(pm._info, data);
    // append buffer to send list
    {
      unique_lock<mutex> lock(_sndMutex);
      _sendList.push_back(data);
    }
  } else {
    SpVecBuffer headBuf(nullptr);
    NP::createPrependVecBuffer(pm._info, data, headBuf);
    // append buffer to send list
    {
      unique_lock<mutex> lock(_sndMutex);
      _sendList.push_back(headBuf);
      _sendList.push_back(data);
    }
  }
}

template <class NP>
SpPeerMessage Channel<NP>::doRecvProcess() {
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

}

















