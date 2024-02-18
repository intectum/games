/*
 * This file is part of ludo. See the LICENSE file for the full license governing this code.
 */

#include <sstream>

namespace ludo
{
  template<typename T>
  void test_equal(const std::string& name, T actual, T expected)
  {
    test_count++;

    if (actual != expected)
    {
      test_failed_count++;

      test_failed_logs.emplace_back(test_group_name + ": " + name);

      auto expected_stream = std::stringstream();
      expected_stream << "  Expected: " << expected;
      test_failed_logs.emplace_back(expected_stream.str());

      auto actual_stream = std::stringstream();
      actual_stream << "  Received: " << actual;
      test_failed_logs.emplace_back(actual_stream.str());
    }
  }

  template<typename T>
  void test_not_equal(const std::string& name, T actual, T expected)
  {
    test_count++;

    if (actual == expected)
    {
      test_failed_count++;

      test_failed_logs.emplace_back(test_group_name + ": " + name);

      auto expected_stream = std::stringstream();
      expected_stream << "  Expected: NOT " << expected;
      test_failed_logs.emplace_back(expected_stream.str());

      auto actual_stream = std::stringstream();
      actual_stream << "  Received: " << actual;
      test_failed_logs.emplace_back(actual_stream.str());
    }
  }

  template<typename T>
  void test_near(const std::string& name, T actual, T expected)
  {
    test_count++;

    if (!near(actual, expected))
    {
      test_failed_count++;

      test_failed_logs.emplace_back(test_group_name + ": " + name);

      auto expected_stream = std::stringstream();
      expected_stream << "  Expected: " << expected;
      test_failed_logs.emplace_back(expected_stream.str());

      auto actual_stream = std::stringstream();
      actual_stream << "  Received: " << actual;
      test_failed_logs.emplace_back(actual_stream.str());
    }
  }
}
