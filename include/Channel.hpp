#pragma once

#include "PeerMessage.hpp"
#include "ChannelBuffer.hpp"
#include "Netpack.hpp"

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

template <class NP=FieldLenNetpack<GenericLenFieldHeader> >
class Channel {
 public:
  static SpVecBuffer createPrependVecBuffer(size_t size) {
    return NP::createPrependVecBuffer(size);
  }

  void doSend(struct PeerMessage& pm);

  bool doRecv();
  
 private:  
  list<SpVecBuffer> _sendList;
  SpVecBuffer _recvBuf;
};

}

















