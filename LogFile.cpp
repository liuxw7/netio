
#include "LogFile.hpp"
#include "TimeUtil.hpp"

using namespace std;

namespace netio
{
DailyLogFile::~DailyLogFile() {
  unique_lock<mutex> lck(_mutex);
  closeFile();
  _logging = false;
  printf("aaaaaaa \n");
  _cond.notify_all();
  printf("4444444 \n");
  //  _thread.join();
  _thread.detach();
  printf("9999 \n");
}

void DailyLogFile::flush(SpCache& spVec) {
  unique_lock<mutex> lck(_mutex);
  printf("222222222 \n");
  _cacheQueue.push(spVec);
  _cond.notify_all();
};

void DailyLogFile::looper() {
  // consumer looper to peek SpCache and write to specified file
  while(_logging) {
    unique_lock<mutex> lck(_mutex);
    while(_logging && (_cacheQueue.empty() || !_ready)) {
      _cond.wait(lck);
      printf("11111111 \n");
    }

    printf("3333333 \n");

    // Get log peice from _cacheQueue at front.
    SpCache cache(_cacheQueue.front());
    if(cache->size() > 0) {
      // write to file
      size_t size = fwrite(cache->data(), cache->size(), 1, _file);
      if(size != 1) {
        fprintf(stderr, "write log failed");
      }
      printf("666666666 \n");
    }
    
    _cacheQueue.pop();
  }
}

void DailyLogFile::createNewFile() {
  char* logPath = getFileName();
  // create file, we don not detect if base path is valid directory.
  _file = fopen(logPath, "a");
  if(nullptr != _file) {
    _ready = true;
  } else {
    fprintf(stderr, "%s create log file failed, %s", __FILE__, logPath);
    _ready = false;
  }
  
}

void DailyLogFile::closeFile() {
  if(nullptr != _file) {
    fflush(_file);
    fclose(_file);
    _file = nullptr;
    _ready = false;
  }
}

void DailyLogFile::recreateNewFile() {
  unique_lock<mutex> lck(_mutex);
  closeFile();
  createNewFile();
  _cond.notify_all();
}

char* DailyLogFile::getFileName() const {
  static char logPath[256] = {0};
  snprintf(logPath, sizeof(logPath), "%s%s_%s.log",_basePath.c_str(), _prefix.c_str(), TimeUtil::fmt_yymmdd());
  return logPath;
}

}


















