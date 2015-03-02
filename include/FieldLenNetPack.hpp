#pragma once

#include "PeerMessage.hpp"
#include "Logger.hpp"

namespace netio {

/**
 * generic length field based net pack
 */
struct GenericLenFieldHeader {
  uint8_t _proto;
  uint8_t _ver;      // which protocol and version to decode the packet content.
  uint16_t _len;     // total len, including header.  
  uint16_t _seq;     // sequence number of the packet
  uint16_t _reserv;  // reservb 
  uint32_t _cmd;     // command id

  /**
   * Get packet total len from network endian plat header data.
   *
   * @return : bytes of total packet, including header.
   */
  static ssize_t getPackLen(const void* buf, size_t len) {
    ASSERT(nullptr != buf && len >= sizeof(GenericLenFieldHeader));
    const GenericLenFieldHeader* header = static_cast<const GenericLenFieldHeader*>(buf);

    return static_cast<ssize_t>(Endian::ntoh16(static_cast<uint16_t>(header->_len)));
  }

  /**
   * Encode PMInfo struct to network endian plat header.
   *
   * @info[in] : peer message infomation.
   * @conLen[in] : content len, not including header len.
   * @buf[in|out] : buffer to encode to.
   * @bufLen[in] : buf size.
   */
  static void encode(const struct PMInfo& info, size_t conLen, void* buf, size_t bufLen) {
    ASSERT(nullptr != buf && bufLen >= sizeof(GenericLenFieldHeader));
    GenericLenFieldHeader* header = static_cast<GenericLenFieldHeader*>(buf);

    header->_proto = static_cast<uint8_t>(info._proto);
    header->_ver = static_cast<uint8_t>(info._version);
    header->_len = static_cast<uint16_t>(Endian::hton16(static_cast<uint16_t>(conLen + sizeof(GenericLenFieldHeader))));
    header->_seq = static_cast<uint16_t>(Endian::hton16(static_cast<uint16_t>(info._seq)));
    header->_cmd = static_cast<uint32_t>(Endian::hton32(static_cast<uint32_t>(info._cmd)));
  }

  /**
   * Decode network endian binary plat header to PMInfo.
   *
   * @info[in|out] : PMInfo to be construct.
   * @buf[in] : plat buffer will decode from
   * @bufLen[in] : plat buffer len
   */
  static void decode(struct PMInfo& info, const void* buf, size_t bufLen) {
    ASSERT(nullptr != buf && bufLen >= sizeof(GenericLenFieldHeader));
    const GenericLenFieldHeader* header = static_cast<const GenericLenFieldHeader*>(buf);

    info._proto = static_cast<PMProto>(header->_proto);
    info._version = static_cast<uint32_t>(header->_ver);
    info._seq = static_cast<uint32_t>(Endian::ntoh16(header->_seq));
    info._cmd = static_cast<uint32_t>(Endian::ntoh32(header->_cmd));
  }
};

static_assert(sizeof(GenericLenFieldHeader) == 12, "sizeof GenericLenFieldHeader NOT expect length");

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
  static ssize_t peekMessageLen(SpVecBuffer buffer) {
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
  static SpPeerMessage readMessage(SpVecBuffer& buffer) {
    ssize_t packSize = peekMessageLen(buffer);
    
    SpPeerMessage spPeerMsg(nullptr);
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

typedef FieldLenNetpack<GenericLenFieldHeader> GenFieldLenPack;


}
















