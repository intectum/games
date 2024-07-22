/*
 * This file is part of ludo. See the LICENSE file for the full license governing this code.
 */

#pragma once

#include <string>
#include <vector>

namespace ludo
{
  extern int32_t test_count;
  extern int32_t test_failed_count;
  extern std::string test_group_name;
  extern std::vector<std::string> test_failed_logs;

  void test_group(const std::string& name);

  template<typename T>
  void test_equal(const std::string& name, T actual, T expected);

  template<typename T>
  void test_not_equal(const std::string& name, T actual, T expected);

  template<typename T>
  void test_near(const std::string& name, T actual, T expected);

  int32_t test_finalize();
}

#include "testing.hpp"
