
#include <iostream>
#include "InetAddr.hpp"
#include "Logger.hpp"

using namespace netio;

int main(int argc, char *argv[])
{
  InetAddr addr(16);
  std::cout << "addr : " << addr.strIpPort() << std::endl;

  InetAddr addr2(0);

  bool resolved = InetAddr::resolve("www.baidu.com", addr2);

  std::cout << "baidu resolved " << resolved << ", addr= " << addr2.strIp() << std::endl;

  

  return 0;
}

















