#pragma once


#include <stdint.h>

namespace netio
{

class TimeUtil {
 public:
  /* formated time string for local */
  static const char* fmt_yymmdd();
  static const char* fmt_yymmddhhmm();
  static const char* fmt_yymmddhhmmss();
  inline static const char* format(const char* fmt);
  inline static const char* format(const char* fmt, time_t t);
  

  /* get timestamp for gm */
  static uint32_t timestampSec();
  static uint64_t timestampMS();
};

}
