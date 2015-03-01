
#include <strings.h>
#include <functional>

#include "InetSock.hpp"
#include "TcpConnection.hpp"
#include "VecBuffer.hpp"


using namespace netio;

thread_local int8_t TcpConnection::_rcvPendingBuffer[SIZE_K(32)];
