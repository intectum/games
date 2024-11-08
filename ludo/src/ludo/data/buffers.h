/*
 * This file is part of ludo. See the LICENSE file for the full license governing this code.
 */

#pragma once

#include <vector>

#include "../core.h"

namespace ludo
{
  ///
  /// A buffer
  struct buffer
  {
    uint64_t id = 0; ///< The ID of the buffer (buffers allocated in VRAM may have overlapping IDs with buffers allocated in RAM).
    std::byte* data = nullptr; ///< The data.
    uint64_t size = 0; ///< The size (in bytes).
  };

  ///
  /// A double buffer
  struct double_buffer
  {
    buffer front; ///< The front buffer.
    buffer back; ///< The back buffer.
  };

  ///
  /// A stream
  struct stream
  {
    std::byte* data = nullptr; ///< The data.
    uint64_t size = 0; ///< The size (in bytes).
    mutable uint64_t position = 0; ///< The current position in the stream.

    ///
    /// \param buffer The buffer to create the stream from.
    /// \param position The initial position.
    explicit stream(const buffer& buffer, uint32_t position = 0);
  };

  ///
  /// The type of access provided by a VRAM buffer.
  enum class vram_buffer_access_hint
  {
    READ,
    WRITE,
    READ_WRITE
  };

  ///
  /// Allocates a buffer.
  /// \param size The size (in bytes).
  /// \return The buffer.
  buffer allocate(uint64_t size);

  ///
  /// Allocates a buffer in VRAM.
  /// \param size The size (in bytes).
  /// \param access_hint The type of access desired.
  /// \return The buffer.
  buffer allocate_vram(uint64_t size, vram_buffer_access_hint access_hint = vram_buffer_access_hint::WRITE);

  ///
  /// Deallocates a buffer.
  /// \param buffer The buffer to deallocate.
  void deallocate(buffer& buffer);

  ///
  /// Deallocates a buffer from VRAM.
  /// \param buffer The buffer to deallocate.
  void deallocate_vram(buffer& buffer);

  ///
  /// Casts data at a position within a buffer.
  /// \param buffer The buffer to cast data within.
  /// \param position The position to cast data at.
  /// \return The data.
  template<typename T>
  T& cast(buffer& buffer, uint64_t position);
  template<typename T>
  const T& cast(const buffer& buffer, uint64_t position);

  ///
  /// Allocates a 'dual residency' double buffer i.e with the front buffer in VRAM and the back buffer in RAM.
  /// \param size The size (in bytes).
  /// \param access_hint The type of access desired.
  /// \return The double buffer.
  double_buffer allocate_dual(uint64_t size, vram_buffer_access_hint access_hint = vram_buffer_access_hint::WRITE);

  ///
  /// Deallocates 'dual residency' double buffer.
  /// \param buffer The double buffer to deallocate.
  void deallocate_dual(double_buffer& buffer);

  ///
  /// Pushes data from the back buffer to the front buffer.
  /// \param buffer The double buffer to push.
  void commit(double_buffer& buffer);

  ///
  /// Reads data from a stream (does not change the position).
  /// \param stream The stream to read from.
  /// \return The data.
  template<typename T>
  T& peek(stream& stream);
  template<typename T>
  const T& peek(const stream& stream);

  ///
  /// Reads data from a stream (and advances the position by sizeof(T)).
  /// \param stream The stream to read from.
  /// \return The data.
  template<typename T>
  T& read(stream& stream);
  template<typename T>
  const T& read(const stream& stream);

  ///
  /// Writes data to a stream (and advances the position by sizeof(T)).
  /// \param stream The stream to write to.
  /// \param value The data to write.
  template<typename T>
  void write(stream& stream, const T& value);

  ///
  /// Determines if the stream has reached the end of the data.
  /// \param stream The stream to check.
  /// \return True the stream has reached the end of the data, false otherwise.
  bool ended(stream& stream);
}

#include "buffers.hpp"
