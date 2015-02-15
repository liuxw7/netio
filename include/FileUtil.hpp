#pragma once

#include <string>

using namespace std;
namespace netio {

class FileUtil {
 public:
  static bool dirReadable(const string& path);
  static bool dirWrittable(const string& path);
};

}

















