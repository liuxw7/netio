
#include <iostream>
#include <vector>
#include <memory>
#include "InetAddr.hpp"


#include "SingleCache.hpp"
using namespace std;
using namespace netio;

struct TestOps {
  void flush(shared_ptr<vector<int8_t>>& kk) {
    cout << "kk size = " << kk->size() << endl;
    char mm[1024] = {0};
    memcpy(mm, kk->data(), kk->size());
    cout << "::: " << mm << endl;
  }

  ~TestOps() {
    std::cout << "xxxxxxxx" << std::endl;
  }
};

int main(int argc, char *argv[])
{
  InetAddr addr(16);
  std::cout << "addr : " << addr.strIpPort() << std::endl;

  InetAddr addr2(0);

  bool resolved = InetAddr::resolve("www.baidu.com", addr2);

  std::cout << "baidu resolved " << resolved << ", addr= " << addr2.strIp() << std::endl;

  return 0;
}

















