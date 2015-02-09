#include <time.h>
#include <sys/time.h>
#include "TimeUtil.hpp"

namespace netio {

#define MAX_TIME_STR (64)

const char* TimeUtil::fmt_yymmdd() {
  return format("%y-%m-%d");
}

const char* TimeUtil::fmt_yymmddhhmm() {
  return format("%y-%m-%d_%H:%M");
}

const char* TimeUtil::fmt_yymmddhhmmss() {
  return format("%y-%m-%d_%H:%M:%S");
}

const char* TimeUtil::format(const char* fmt) {
  time_t _ts;
  time(&_ts);
  
  return format(fmt, _ts);
}

const char* TimeUtil::format(const char* fmt, time_t _ts) {
  static __thread char str[MAX_TIME_STR] = {0};
  
  struct tm * timeinfo;
  timeinfo = localtime(&_ts);

  size_t len = strftime(str, sizeof(str) - 1, fmt, timeinfo);
  str[len] = 0;
  return str;
}

uint32_t TimeUtil::timestampSec() {
  time_t _ts;
  time(&_ts);
  return static_cast<uint32_t>(_ts);
}

uint64_t TimeUtil::timestampMS() {
  struct timeval _tv;
  
  gettimeofday(&_tv, NULL);
  return _tv.tv_sec * 1000 + _tv.tv_usec / 1000;
}

}

