#pragma once

#include <stdint.h>
#include <endian.h>

namespace netio {


class Endian {
 public:
  static uint16_t hton16(uint16_t x) {
    return htobe16(x);
  }

  static uint32_t hton32(uint32_t x) {
    return htobe32(x);
  }

  static uint64_t hton64(uint64_t x) {
    return htobe64(x);
  }
  
  static uint16_t ntoh16(uint16_t x) {
    return be16toh(x);
  }

  static uint32_t ntoh32(uint32_t x) {
    return be32toh(x);
  }

  static uint64_t ntoh64(uint64_t x) {
    return be64toh(x);
  }
};

}

