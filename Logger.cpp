
#include "Logger.hpp"


namespace netio {


LogLevelInfo g_loglevel_infos[LEVEL_MAX] = {
  {"F/", 2},
  {"E/", 2},
  {"W/", 2},
  {"I/", 2},
  {"D/", 2},
  {"V/", 2}
};


// define inner logger
Logger<true> g_inner_logger("", "def");
const char* g_common_tag = "common";
const char* g_syserr_tag = "syserr";
const char* LOG_NETIO_TAG = "netio";
}

