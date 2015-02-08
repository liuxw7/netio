#pragma once

#include <stdint.h>
#include <endian.h>

namespace netio {

class Endian {
 public:
  static inline uint16_t hton16(uint16_t x) {
    return htobe16(x);
  }

  static inline uint32_t hton32(uint32_t x) {
    return htobe32(x);
  }

  static inline uint64_t hton64(uint64_t x) {
    return htobe64(x);
  }
  
  static uint16_t ntoh16(uint16_t x) {
    return be16toh(x);
  }

  static inline uint32_t ntoh32(uint32_t x) {
    return be32toh(x);
  }

  static inline uint64_t ntoh64(uint64_t x) {
    return be64toh(x);
  }
};

}

