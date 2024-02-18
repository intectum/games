/*
 * This file is part of ludo. See the LICENSE file for the full license governing this code.
 */

#ifndef LUDO_LOGGING_H
#define LUDO_LOGGING_H

#include <iostream>

#include "core.h"

namespace ludo
{
  ///
  /// Logs an info message using printf formatting.
  ///
  /// message is not a const reference because MSVC can't handle variadics after reference types...
  /// @param tag The tag to log the message with
  /// @param message The message to log
  /// @param ... The arguments to include in the message
  LUDO_API void log_info(const std::string& tag, std::string message, ...);

  ///
  /// Logs a debug message using printf formatting.
  ///
  /// message is not a const reference because MSVC can't handle variadics after reference types...
  /// @param tag The tag to log the message with
  /// @param message The message to log
  /// @param ... The arguments to include in the message
  LUDO_API void log_debug(const std::string& tag, std::string message, ...);

  ///
  /// Logs a warning message using printf formatting.
  ///
  /// message is not a const reference because MSVC can't handle variadics after reference types...
  /// @param tag The tag to log the message with
  /// @param message The message to log
  /// @param ... The arguments to include in the message
  LUDO_API void log_warning(const std::string& tag, std::string message, ...);

  ///
  /// Logs an error message using printf formatting.
  ///
  /// message is not a const reference because MSVC can't handle variadics after reference types...
  /// @param tag The tag to log the message with
  /// @param message The message to log
  /// @param ... The arguments to include in the message
  LUDO_API void log_error(const std::string& tag, std::string message, ...);

  ///
  /// Logs a fatal message using printf formatting.
  ///
  /// message is not a const reference because MSVC can't handle variadics after reference types...
  /// @param tag The tag to log the message with
  /// @param message The message to log
  /// @param ... The arguments to include in the message
  LUDO_API void log_fatal(const std::string& tag, std::string message, ...);

  ///
  /// Adds an output stream for a particular tag prefix. Logs will be output to the stream for the longest matching tag
  /// prefix. Setting the stream for the empty string ("") tag prefix will set the default stream which will be used
  /// for tags that do not have a stream set for a matching tag prefix.
  /// @param tag_prefix The prefix of the tags to output to the stream
  /// @param stream The output stream
  LUDO_API void add_log_stream(const std::string& tag_prefix, std::ostream& stream);
}

#endif // LUDO_LOGGING_H
