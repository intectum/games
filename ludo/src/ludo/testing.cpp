/*
 * This file is part of ludo. See the LICENSE file for the full license governing this code.
 */

#include <iostream>

#include "testing.h"

namespace ludo
{
  int32_t test_count = 0;
  int32_t test_failed_count = 0;
  std::string test_group_name;
  std::vector<std::string> test_failed_logs;

  void test_group(const std::string& name)
  {
    test_group_name = name;
  }

  int32_t test_finalize()
  {
    std::cout << test_count - test_failed_count << "/" << test_count << " tests passed" << std::endl;
    if (!test_failed_logs.empty())
    {
      std::cout << "failed tests:" << std::endl;
      for (auto& test_failed_log : test_failed_logs)
      {
        std::cout << "  " << test_failed_log << std::endl;
      }
    }

    return test_failed_logs.empty() ? 0 : 1;
  }
}
