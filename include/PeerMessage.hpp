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

typedef enum {
  CONTYPE_TCP,
  CONTYPE_UDP,
  CONTYPE_PIPE
} ConnectType;

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
 * Describe peer message data that unpacked.
 */
typedef struct iovec PMData;

/**
 * Describe remote address, if peer has fd, then _fd is natural number.
 */
typedef struct {
  int _fd;
  struct sockaddr_in _addr;
} PMAddr;

/**
 * Describe peer message for send and receive.
 */
class PeerMessage {
 public:
  PMInfo _info;
  SpVecBuffer _buffer;
};

typedef shared_ptr<PMAddr> SpPMAddr;
typedef shared_ptr<PeerMessage> SpPeerMessage;

}
















