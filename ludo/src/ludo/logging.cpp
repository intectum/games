/*
 * This file is part of ludo. See the LICENSE file for the full license governing this code.
 */

#include <cassert>
#include <cstdarg>

#include "logging.h"

const uint32_t MAX_ARG_BUFFER_SIZE = 1024;

namespace ludo
{
  auto streams = std::unordered_map<std::string, std::ostream*>();

  std::ostream& get_stream(const std::string& tag)
  {
    auto selected_stream = &std::cout;

    auto matching_prefix_length = 0ul;
    for (auto& candidate_stream : streams)
    {
      auto prefix_length = candidate_stream.first.size();
      if (prefix_length <= matching_prefix_length)
      {
        continue;
      }

      if (prefix_length <= tag.size() && candidate_stream.first == tag.substr(0, prefix_length))
      {
        selected_stream = candidate_stream.second;
        matching_prefix_length = prefix_length;
      }
    }

    return *selected_stream;
  }

  void log(const std::string& severity, const std::string& tag, std::string message, va_list args)
  {
    auto formattedMessage = std::string(message.size() + MAX_ARG_BUFFER_SIZE, 'x');
    vsnprintf(&formattedMessage[0], formattedMessage.size(), message.c_str(), args);
    formattedMessage = formattedMessage.c_str(); // Cut off the trailing 'x's

    assert(severity != "E" && severity != "F" && "logged error");

    auto& stream = get_stream(tag);

    stream << severity << " || " << tag << " || " << formattedMessage << std::endl;
  }

  void log_info(const std::string& tag, std::string message, ...)
  {
    va_list args;
    va_start(args, message);
    log("I", tag, message, args);
    va_end(args);
  }

  void log_debug(const std::string& tag, std::string message, ...)
  {
    va_list args;
    va_start(args, message);
    log("D", tag, message, args);
    va_end(args);
  }

  void log_warning(const std::string& tag, std::string message, ...)
  {
    va_list args;
    va_start(args, message);
    log("W", tag, message, args);
    va_end(args);
  }

  void log_error(const std::string& tag, std::string message, ...)
  {
    va_list args;
    va_start(args, message);
    log("E", tag, message, args);
    va_end(args);
  }

  void log_fatal(const std::string& tag, std::string message, ...)
  {
    va_list args;
    va_start(args, message);
    log("F", tag, message, args);
    va_end(args);
  }

  void add_log_stream(const std::string& tag_prefix, std::ostream& stream)
  {
    streams[tag_prefix] = &stream;
  }
}
