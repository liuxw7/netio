#pragma once

#include <mutex>

#include "PeerMessage.hpp"
#include "ChannelBuffer.hpp"
//#include "Netpack.hpp"

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
  
  static SpVecBuffer createPrependVecBuffer(size_t size) {
    return NP::createPrependVecBuffer(size);
  }

  /**
   * Get buffer for receving.
   */
  SpVecBuffer getRecvBuffer() {
    return _recvBuf;
  }
  
  /**
   * Do send PeerMessage operator.
   *
   * Append PeerMessage's ChannelBuffer to list.
   * Notify send thread if nessesary.
   */
  void doSend(PeerMessage& pm);
  
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
  SpPeerMessage doRecvProcess();

 private:
  mutable mutex _sndMutex;
  list<SpVecBuffer> _sendList;
  SpVecBuffer _recvBuf;
  size_t _initRecvSize;
};

}


















