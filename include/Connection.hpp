#pragma once

#include "PeerMessage.hpp"
#include "Channel.hpp"
#include <functional>
#include <memory>


using namespace std;

namespace netio {

class Connection;
typedef shared_ptr<Connection> SpConnection;
typedef shared_ptr<PMRecv> SpPMRecv;

typedef function<void (const SpConnection&, const SpPMRecv&)> MessageCallback;
typedef function<void (const SpConnection&)> WrittableCallback;
typedef function<void (const SpConnection&)> ConnectedCallback;
typedef function<void (const SpConnection&)> CloseCallback;
typedef function<void (const SpConnection&)> ErrorCallback;

class Connection {
 public:
  /**
   * Attach callback function when complete message comes
   */
  void setMessageCallback(const MessageCallback& cb) {
    _msgCb = cb;
  }

  /**
   * Attach callback when connection writtable.
   */
  void setWriteCallback(const WrittableCallback& cb) {
    _writeCb = cb;
  }

  /**
   * Attach callback when connection establish.
   */
  void setConnectedCallback(const ConnectedCallback& cb) {
    _connCb = cb;
  }

  /**
   * Attach callback when connection closed.
   */
  void setCloseCallback(const CloseCallback& cb) {
    _closeCb = cb;
  }

  /**
   * Attach callback when error occur on the connection.
   */
  void setErrorCallback(const ErrorCallback& cb) {
    _errCb = cb;
  }

 protected:
  void onEstablish() const {
    _connCb(*this);
  }

  void onClosed() const {
    _closeCb(*this);
  }
  
  void onNewMessage(const SpPMRecv& msg) const {
    _msgCb(*this, msg);
  }

  void onWrittable() const {
    _writeCb(*this);
  }

  void onError() const {
    _errCb(*this);
  }

 private:
  MessageCallback _msgCb;
  WrittableCallback _writeCb;
  ConnectedCallback _connCb;
  CloseCallback _closeCb;
  ErrorCallback _errCb;
};

}










