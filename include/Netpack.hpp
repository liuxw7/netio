#pragma once

#include "Utils.hpp"
#include "PeerMessage.hpp"
#include "ChannelBuffer.hpp"

namespace netio {

/**
 * specify pack header
 */
template <class PH>
class FieldLenNetpack {
 public:
  static SpVecBuffer createPrependVecBuffer(size_t size) {
    return SpVecBuffer(new VecBuffer(size, sizeof(PH)));
  }

  static ssize_t peekPackLen(SpVecBuffer buffer) {
    if(buffer->readableSize() < sizeof(PH)) {
      return -1;
    }

    return PH::getPackLen(static_cast<const void*>(buffer->readablePtr()), sizeof(PH));
  }

  static void parsePeerMessageInfo(SpVecBuffer buffer, struct PMInfo& info) {
    ASSERT(buffer->readableSize() >= sizeof(PH));
    PH::decode(info, static_cast<const void*>(buffer->readablePtr()), sizeof(PH));
    buffer->markRead(sizeof(PH));
  }

  /**
   * Read peer message from buffer.
   * 
   */
  static SpPeerMessage readPeerMessage(SpVecBuffer& buffer) {
    ssize_t packSize = peekPackLen(buffer);
    SpPeerMessage spPeerMsg = nullptr;
    if(LIKELY(packSize > 0)) {
      SpVecBuffer splited = buffer->split(static_cast<size_t>(packSize));
      if(nullptr != splited) {
        spPeerMsg.reset(new PeerMessage());
        parsePeerMessageInfo(splited, spPeerMsg->_info);
      }
    }

    return spPeerMsg;
  }

  static SpVecBuffer createPendingBuffer(const struct PMInfo& info, const SpVecBuffer& buffer, SpVecBuffer& pendBuf) {
    pendBuf.reset(new VecBuffer(sizeof(PH)));
    PH::encode(info, buffer->readableSize(), pendBuf->writtablePtr(), pendBuf->writtableSize());
    pendBuf->markWrite(sizeof(PH));
  }

  static void writePendingInfo(const struct PMInfo& info, SpVecBuffer& buffer) {
    ASSERT(sizeof(PH) == buffer->getOffset());
    PH::encode(info, buffer->readableSize(), buffer->bufferPtr(), sizeof(PH));
    buffer->fixPrepend();
  }
};

class DelimiterNetpack {
 public:
};

}

















