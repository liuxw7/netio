#pragma once

#include <netinet/in.h>
#include <sys/uio.h>
#include <assert.h>
#include <vector>

#include "Utils.hpp"
#include "Endian.hpp"
#include "VecBuffer.hpp"

namespace netio {

#define PMPROTO_BASE (0x10)

typedef enum {
  PMPROTO_PROTOBUF = PMPROTO_BASE,
  PMPROTO_JSON
} PMProto;


/**
 * Destruct inet peer.
 * if it's _fd is natural number, we can reply the peer with fd.
 * otherwise, we reply it with sockaddr_in.
 *
 * major cmd will indicate the module id that receive the message, high 16 bits
 * minor cmd is real command the module handle for, low 16 bits
 */

struct PMEmpty {
  uint8_t empty[0];
};

static_assert((sizeof(PMEmpty) == 0), "size PMEmpty != 0");

/**
 * Describe peer message information
 */
struct PMInfo {
  uint16_t majorCmd() const {
    return _cmd >> 16;
  }

  uint16_t minorCmd() const {
    return _cmd & 0xFFFF;
  }

  PMInfo(PMProto proto, uint32_t version, uint32_t cmd, uint32_t seq) :
      _proto(proto),
      _version(version),
      _cmd(cmd),
      _seq(seq)
  {}

  PMInfo(){}
  
  PMProto _proto;
  uint32_t _version;  
  uint32_t _cmd;
  uint32_t _seq;
};

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
 * Describe peer message data that unpacked.
 */
typedef struct iovec PMData;

/**
 * Describe remote address, if peer has fd, then _fd is natural number.
 */
struct PMAddr {
  int _fd;
  struct sockaddr_in _addr;
};

/**
 * Describe peer message for send and receive.
 */
class PeerMessage {
 public:
  PMInfo _info;
  SpVecBuffer _buffer;
};

typedef shared_ptr<PeerMessage> SpPeerMessage;

}


















