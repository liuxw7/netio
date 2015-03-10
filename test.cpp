
#include <iostream>
#include <vector>
#include <memory>
#include <stdint.h>
#include "InetAddr.hpp"
#include "SingleCache.hpp"
#include "Logger.hpp"
#include "VecBuffer.hpp"
//#include "Netpack.hpp"
#include "Channel.hpp"
//#include "TcpConnection.hpp"
//#include "TcpPump.hpp"

#include "MultiplexLooper.hpp"
#include "ConsumerLooper.hpp"
#include "TcpAcceptor.hpp"
#include "MessageLooper.hpp"

#include "LooperPool.hpp"
#include "TcpServer.hpp"
#include "TcpConnector.hpp"

#include "FieldLenNetPack.hpp"

using namespace std;
using namespace netio;

void test_logger(int times) {
  Logger<true> mLogger("/home/liuzf/workspace", "123");
  mLogger.printLogLn(LEVEL_INFO, "hello", "world %llu", 0x01L);

  for (int i = 0; i < times; i ++) {
    LOGI("ttkk", "current number is %d", i);
  }
}

void test_inetaddr(const char* host) {
  InetAddr addr(16);
  std::cout << "addr : " << addr.strIpPort() << std::endl;

  InetAddr addr2(0);

  bool resolved = InetAddr::resolve(host, addr2);

  std::cout << "baidu resolved " << resolved << ", addr= " << addr2.strIp() << std::endl;
}

void test_channelbuffer() {
}

/*
template <class NP>
void test_recvToChannel(Channel<NP>& channel) {
}
*/

void test_channel() {
  /*
  PeerMessage pm;
  Channel<FieldLenNetpack<GenericLenFieldHeader> > channel;
  channel.sendPeerMessage(pm);
  channel.markSended(1000);
  */
}


void test_connection() {
  /*
  TcpConnection<> conn(INET_PORT_CAST(1234));
  std::cout << "fd = " << conn.getFd() << std::endl;
  std::cout << "peer = " << conn.getPeerAddr().strIpPort() << std::endl;
  std::cout << "local = " << conn.getLocalAddr().strIpPort() << std::endl;

  conn.sendInternal();
  */
}


void test_tcppump() {
  /*
  TcpPump pump(INET_PORT_CAST(1233));
  pump.startWork();
  */
}

void test_looperPool() {
  LooperPool<MultiplexLooper> loopers(5);

  usleep(50);
  
  loopers.getLooper();
}

class MyHandler : public LoopHandler {
  void handleMessage(LoopMessage& message) {
    COGI("message with %d %p %p", message.what(), message.lparam(), message.rparam());
  }
};

void print_hello() {
  COGFUNC();
}

void test_messageLooper() {
  MessageLooper msgLooper;
  MyHandler* handler = new MyHandler();
  msgLooper.setHandler(handler);
  thread _mythread(bind(&MessageLooper::startLoop, &msgLooper));


  for(int i = 1; i < 10; i++) {
    msgLooper.postMessage(i);
  }

  for(int i = 0; i < 5; i++) {
    msgLooper.postFunctor(function<void(void)>(print_hello));
  }

  msgLooper.stopLoop();
  _mythread.join();
}


typedef shared_ptr<TcpConnection> SpTcpConnection;

SpTcpConnection serverConn = nullptr;

void on_message(SpTcpConnection conn, SpVecBuffer buf) {
  COGFUNC();

  printf("receive buffer : %s", buf->readablePtr());
  
  SpVecBuffer buffer(new VecBuffer(20));
  sprintf((char*)buffer->writtablePtr(), "hello");
  buffer->markWrite(strlen("hello"));
  conn->send(buffer);
}

void server_on_new_conn(int hash, SpTcpConnection conn) {
  COGFUNC();
  conn->attach();

  conn->setNewMessageHandler(on_message);
  
  serverConn = conn;
  //  SpVecBuffer buf = serverConn->createPackLayoutBuffer(20);
}


void test_tcpserver() {
  SpLooperPool loopPool(new LooperPool<MultiplexLooper>(5));
  static TcpServer server(3001, loopPool);

  server.setNewConnectionHandler(server_on_new_conn);
  
  server.startWork();
  sleep(3000);
  server.stopWork();
}


MultiplexLooper clientlooper;
thread* clientThread;
SpTcpConnection clientConnection;

void client_on_new_conn(int fd, const InetAddr& addr) {
  SpTcpConnection connection(new TcpConnection(&clientlooper, fd, addr.getSockAddr()));
  clientConnection = connection;
  connection->attach();

  COGD("%s, fd=%d", __func__, fd);

  const static char* msg = "hello, i'm client";
  
  SpVecBuffer buffer(new VecBuffer(20));
  sprintf((char*)buffer->writtablePtr(), "%s", msg);
  buffer->markWrite(strlen(msg));
  connection->send(buffer);
}




void test_tcpclient() {
  clientThread = new thread(bind(&MultiplexLooper::startLoop, &clientlooper));

  TcpConnector connector(&clientlooper, 10002, InetAddr("127.0.0.1", 3001));
  connector.setNewConnCallback(client_on_new_conn);
  connector.connect();

  sleep(3000);
}

int main(int argc, char *argv[])
{
  /*
  test_logger(1);
  test_inetaddr("www.baidu.com");
  test_channelbuffer();
  test_channel();
  test_connection();

  test_tcppump();
  */

  //  test_looperPool();
  //  test_messageLooper();
  
  //test_tcpserver();
  test_tcpclient();
  
  /*
  MultiplexLooper looper;
  thread _thread(bind(&MultiplexLooper::startLoop, &looper));

  TcpAcceptor acceptor(&looper, INET_PORT_CAST(3001));
  acceptor.attach();


  sleep(1000);
  
  looper.stopLoop();
  _thread.join();

  */
  return 0;
}

