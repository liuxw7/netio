#pragma once

#include "Connection.hpp"
#include "Channel.hpp"

namespace netio {

/**
 *
 */

class TcpConn : public Connection {
 public:
 private:
  Channel _channel;
};

}

















