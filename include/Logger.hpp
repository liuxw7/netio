#pragma once

#include <algorithm>
#include <stdint.h>
#include <string.h>
#include <memory>
#include <limits>
#include <iostream>

#include "LogFile.hpp"
#include "SingleCache.hpp"

namespace netio {

typedef enum {
  VERBOSE,
  DEBUG,
  INFO,
  WARRING,
  ERROR
} LOG_LEVEL;


/**
 * tempalte CON : specify that if will print log on console. Default closed, it
 * will open usually when unit testing.
 */
template <bool CON = false>
class Logger {
  static const int MAX_NUM_LEN = 32;
  
#define LOGGER_ASSERT_STR "not enough for store numeric"
  static_assert(MAX_NUM_LEN - 10 > std::numeric_limits<double>::digits10, LOGGER_ASSERT_STR);
  static_assert(MAX_NUM_LEN - 10 > std::numeric_limits<long double>::digits10, LOGGER_ASSERT_STR);
  static_assert(MAX_NUM_LEN - 10 > std::numeric_limits<long>::digits10, LOGGER_ASSERT_STR);
  static_assert(MAX_NUM_LEN - 10 > std::numeric_limits<long long>::digits10, LOGGER_ASSERT_STR);
public:
  typedef Logger<CON> self;
  template <typename T1, typename T2>
  Logger(T1&& basePath, T2&& prefix) : _cache(basePath, prefix) {}

  inline self& operator<<(bool value) {
    append(value ? "1" : "0", 1);
  }
  
  inline self& operator<<(short value) {
    appendNumeric(value);
    return *this;
  }
  
  inline self& operator<<(unsigned short value) {
    appendNumeric(value);
    return *this;
  }
  
  inline self& operator<<(int value) {
    appendNumeric(value);
    return *this;
  }
  
  inline self& operator<<(unsigned int value) {
    appendNumeric(value);
    return *this;
  }
  
  inline self& operator<<(long value) {
    appendNumeric(value);
    return *this;
  }
  
  inline self& operator<<(unsigned long value) {
    appendNumeric(value);
    return *this;
  }
  
  inline self& operator<<(long long  value) {
    appendNumeric(value);
    return *this;
  }
  
  inline self& operator<<(unsigned long long value) {
    appendNumeric(value);
    return *this;
  }
  
  inline self& operator<<(const void* value) {
    char temp[MAX_NUM_LEN] = {0};
    uintptr_t vptr = reinterpret_cast<uintptr_t>(value);
    temp[0] = '0';
    temp[1] = 'x';
    size_t len = hexStringFromNumeric(temp + 2, vptr);
    append(temp, len);
    return *this;
  }
  
  inline self& operator<<(float value) {
    char temp[MAX_NUM_LEN] = {0};
    int len = snprintf(temp, sizeof(temp), "%.12g", value);
    append(temp, len);
    return *this;
  }
  
  inline self& operator<<(double value) {
    char temp[MAX_NUM_LEN] = {0};
    int len = snprintf(temp, sizeof(temp), "%.12g", value);
    append(temp, len);
    return *this;
  }
  
  // self& operator<<(long double);
  inline self& operator<<(char value) {
    append(&value, 1);
    return *this;
  }
  
  // self& operator<<(signed char);
  // self& operator<<(unsigned char);
  inline self& operator<<(const char* str) {
    append(str);
    return *this;
  }
  
  inline self& operator<<(const unsigned char* str) {
    append(str);
    return *this;
  }
  
  inline self& operator<<(const string& value) {
    append(value.c_str(), value.length());
    return *this;
  }

 private:
  template <typename T>
  inline void appendNumeric(T value) {
    char temp[MAX_NUM_LEN] = {0};
    size_t len = stringFromNumeric(temp, value);
    append(temp, len);
  }

  inline void append(const char* str) {
    append(str, strlen(str));
  }

  /** 
   * real append 
   */
  inline void append(const char* str, size_t len) {
    _cache.append(str, len);
  }

  
  template <typename T>
  inline size_t stringFromNumeric(char buf[], T value) {
    static const char digits[] = "9876543210123456789";
    static const char* zero = digits + 9;
    static_assert(sizeof(digits) == 20, "digits string size is not 20");

    T i = value;
    char* p = buf;

    do
    {
      int lsd = static_cast<int>(i % 10);
      i /= 10;
      *p++ = zero[lsd];
    } while (i != 0);
  
    if (value < 0)
    {
      *p++ = '-';
    }
    *p = '\0';
    std::reverse(buf, p);

    return p - buf;
  }

  inline size_t hexStringFromNumeric(char buf[], uintptr_t value)
  {
    static const char digitsHex[] = "0123456789ABCDEF";
    static_assert(sizeof(digitsHex) == 17, "digits size is not 17 !!");

    uintptr_t i = value;
    char* p = buf;

    do
    {
      int lsd = static_cast<int>(i % 16);
      i /= 16;
      *p++ = digitsHex[lsd];
    } while (i != 0);

    *p = '\0';
    std::reverse(buf, p);

    return p - buf;
  }

  SingleCache<> _cache;
};

/**
 * If console option is opened, print out.
 */
template <>
void Logger<true>::append(const char* str, size_t len) {
  std::cout << str;
  _cache.append(str, len);
}

}


