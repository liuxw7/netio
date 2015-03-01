#pragma once

#include "Utils.hpp"
#include "PeerMessage.hpp"
#include "VecBuffer.hpp"

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

  /**
   * Peek pack len from SpVecBuffer.
   */
  static ssize_t peekPackLen(SpVecBuffer buffer) {
    if(buffer->readableSize() < sizeof(PH)) {
      return -1;
    }

    return PH::getPackLen(static_cast<const void*>(buffer->readablePtr()), sizeof(PH));
  }

  /**
   * read PeerMessage information from buffer.
   */
  static void parsePeerMessageInfo(SpVecBuffer buffer, struct PMInfo& info) {
    ASSERT(buffer->readableSize() >= sizeof(PH));
    PH::decode(info, static_cast<const void*>(buffer->readablePtr()), sizeof(PH));
    buffer->markRead(sizeof(PH));
  }

  /**
   * Read peer message from buffer.
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

  /**
   * Create netpack header SpVecBuffer. Use this function for wrap exist message buffer.
   * While sending message, it will add two buffer, header and message body splited.
   *
   * @info[in] : Peer message information to encode the header.
   * @contentLen[in] : content length specification.
   * @return : return new header buffer.
   */
  static SpVecBuffer createPendingBuffer(const struct PMInfo& info, size_t contentLen) {
    SpVecBuffer pendBuf(new VecBuffer(sizeof(PH)));
    PH::encode(info, contentLen, pendBuf->writtablePtr(), pendBuf->writtableSize());
    pendBuf->markWrite(sizeof(PH));
    return pendBuf;
  }

  /**
   * Write header to header prepended buffer.
   *
   * @info[in] : meesage information to encode to the header.
   * @buffer[in] : buffer that have prepended message buffer, this buffer may created by createPrependVecBuffer function.
   */
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

















