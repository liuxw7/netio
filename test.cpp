
#include <iostream>
#include <vector>
#include <memory>
#include "InetAddr.hpp"
#include "SingleCache.hpp"
#include "Logger.hpp"
#include "ChannelBuffer.hpp"

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
  ChannelBuffer chan;
  chan.writeInt16(123);
  std::cout << chan.readInt16() << std::endl;
}

int main(int argc, char *argv[])
{
  test_logger(50);
  test_inetaddr("www.baidu.com");

  test_channelbuffer();
  
  return 0;
}







