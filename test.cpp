
#include <iostream>
#include <vector>
#include <memory>
#include "InetAddr.hpp"
#include "SingleCache.hpp"
#include "Logger.hpp"


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



  string str = "abc";
  str += "m";
  int found = str.find_last_of('c');

  std::cout << "found = " << found << std::endl;

  std::cout << "xxx : " << str << std::endl;

  /*
  shared_ptr<DailyLogFile> logFile(new DailyLogFile("", "kkk_"));
  SingleCache<DailyLogFile, 20> cache(logFile);

  usleep(1000);
  
  cache.append("123456789");
  cache.append("123456");
  cache.append("xxxxxxxxxxxxxxxxxxxlllllllllllllllllllllllll");
  cache.append("mmmmmmmmmmmmmmmmmmm");
  */
  //  Logger<true> ll("", "mmm");
  //  l << "123" << std::endl;
  // l << "4567" << std::endl;

  Logger<true> lll("", "");
  lll << "123" << "\n";
  lll << "123" << "\n";
  lll << "123" << "\n";
  
  return 0;
}


