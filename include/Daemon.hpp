#pragma once

#include <signal.h>

#include "MessageLooper.hpp"

namespace netio {

class Daemon {
 public:
  typedef  void (*OnTerminate)(int);
  
  void init(OnTerminate cb);
  void startWork();
  void stopWork();
 private:
  void setupSignal(OnTerminate cb);
  void setupRLimit();

  MessageLooper _msgLooper;
};

}

