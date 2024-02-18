/*
 * This file is part of ludo. See the LICENSE file for the full license governing this code.
 */

#include "logging.h"
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
    log_info("ludo testing", "%i/%i tests passed.", test_count - test_failed_count, test_count);
    if (!test_failed_logs.empty())
    {
      log_info("ludo testing", "Failed tests:");
      for (auto& test_failed_log : test_failed_logs)
      {
        log_info("ludo testing", "  %s", test_failed_log.c_str());
      }
    }

    return test_failed_logs.empty() ? 0 : 1;
  }
}
