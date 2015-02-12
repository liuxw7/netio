
#include <unistd.h>
#include "InetSock.hpp"

namespace netio {

InetSock::InetSock(int fd) : _fd(fd) {}

InetSock::~InetSock() {
  if(_fd >= 0) {
    close(_fd);
  }
}


}
