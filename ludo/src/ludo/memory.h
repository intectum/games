/*
 * This file is part of ludo. See the LICENSE file for the full license governing this code.
 */

#pragma once

#include <cstdint>
#include <utility>

namespace ludo
{
  ///
  /// An identifiable buffer
  struct buffer
  {
    uint32_t id = 0;
    std::byte* data = nullptr;
    uint64_t size = 0;

    std::byte* operator[](uint32_t index);
    const std::byte* operator[](uint32_t index) const;
  };

  ///
  /// An arena allocator
  struct arena : buffer
  {
    std::byte* next = nullptr;
  };

  ///
  /// A pool allocator
  struct pool : buffer
  {
    uint32_t block_count = 0;
    uint32_t block_size = 0;
    bool* blocks = nullptr;
  };

  ///
  /// A range
  struct range
  {
    uint32_t id = 0;
    uint32_t start = 0;
    uint32_t count = 0;
  };

  ///
  /// A stream
  struct stream: buffer
  {
    mutable uint64_t position = 0; ///< The current position in the stream.

    ///
    /// \param buffer The buffer to create the stream from.
    /// \param position The initial position.
    explicit stream(const buffer& buffer, uint32_t position = 0);
  };

  ///
  /// The type of access to optimise for.
  enum class access_hint
  {
    READ,
    WRITE,
    READ_WRITE
  };

  buffer allocate_buffer(uint64_t size, access_hint access_hint = access_hint::WRITE);
  buffer allocate_buffer_vram(uint64_t size, access_hint access_hint = access_hint::WRITE);

  void free_buffer(buffer& buffer);
  void free_buffer_vram(buffer& buffer);

  template<typename T>
  T& cast(buffer& buffer, uint64_t position);
  template<typename T>
  const T& cast(const buffer& buffer, uint64_t position);

  arena allocate_arena(uint64_t size, access_hint access_hint = access_hint::WRITE);
  arena allocate_arena_vram(uint64_t size, access_hint access_hint = access_hint::WRITE);

  void free_arena(arena& arena);
  void free_arena_vram(arena& arena);

  std::byte* allocate(arena& arena, uint64_t size);
  void reset(arena& arena);

  pool allocate_pool(uint32_t block_count, uint32_t block_size, access_hint access_hint = access_hint::WRITE);
  pool allocate_pool_vram(uint32_t block_count, uint32_t block_size, access_hint access_hint = access_hint::WRITE);

  void free_pool(pool& pool);
  void free_pool_vram(pool& pool);

  std::byte* allocate(pool& pool);
  void free(pool& pool, std::byte* data);

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

#include "memory.hpp"
