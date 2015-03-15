#include "TcpSession.hpp"

using namespace netio;

TcpSession::TcpSession(time_t ts, uint32_t id, SpTcpConnection& connection) :
    _id(id),
    _createTime(ts),
    _updateTime(ts),
    _connection(connection)
{}



