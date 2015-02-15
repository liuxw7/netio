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

  static ssize_t parsePackLen(SpVecBuffer buffer) {
    if(buffer->readableSize() < sizeof(PH)) {
      return -1;
    }

    return PH::getPackLen(static_cast<const void*>(buffer->readablePtr()), sizeof(PH));
  }

  static bool parsePeerMessageInfo(SpVecBuffer buffer, struct PMInfo& info) {
    if(buffer->readableSize() < sizeof(PH)) {
      return false;
    }

    PH::decode(info, static_cast<const void*>(buffer->readablePtr()), sizeof(PH));
    return true;
  }

  static SpVecBuffer createPendingBuffer(const struct PMInfo& info, const SpVecBuffer& buffer, SpVecBuffer& pendBuf) {
    pendBuf.reset(new VecBuffer(sizeof(PH)));
    PH::encode(info, buffer->readableSize(), pendBuf->writtablePtr(), pendBuf->writtableSize());
    pendBuf->markWrite(sizeof(PH));
  }

  static void writePendingInfo(const struct PMInfo& info, SpVecBuffer& buffer) {
    ASSERT(sizeof(PH) == buffer->getOffset());
    PH::encode(info, buffer->readableSize(), buffer->bufferPtr(), sizeof(PH));
    buffer->resetOffset();
  }
};

class DelimiterNetpack {
 public:
};

}

















