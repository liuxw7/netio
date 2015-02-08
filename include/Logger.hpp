#pragma once

#include <algorithm>
#include <stdint.h>
#include <string.h>
#include <memory>
#include <limits>
#include <iostream>

//#include "SwapCache.hpp"
#include "DailyFile.hpp"

namespace netio {

typedef enum {
  VERBOSE,
  DEBUG,
  INFO,
  WARRING,
  ERROR
} LOG_LEVEL;

/**
 * template F : specify log file class, typically is daily named cached file log.
 * tempalte CON : specify that if will print log on console. Default closed, it
 * will open usually when unit testing.
 */
template <typename F, bool CON = false>
class Logger {
  static const int MAX_NUM_LEN = 32;
  
#define LOGGER_ASSERT_STR "not enough for store numeric"
  static_assert(MAX_NUM_LEN - 10 > std::numeric_limits<double>::digits10, LOGGER_ASSERT_STR);
  static_assert(MAX_NUM_LEN - 10 > std::numeric_limits<long double>::digits10, LOGGER_ASSERT_STR);
  static_assert(MAX_NUM_LEN - 10 > std::numeric_limits<long>::digits10, LOGGER_ASSERT_STR);
  static_assert(MAX_NUM_LEN - 10 > std::numeric_limits<long long>::digits10, LOGGER_ASSERT_STR);

  typedef Logger<F, CON> self;

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

  
  template <bool WC>
  void test() {
    std::cout << "11111111111111111  " << std::endl;
  }

  template <>
  void test<true> () {
    std::cout << "222222222222222222 " << std::endl;
  }

  void tt() {
    test<CON>();
  }
  
};

}


















