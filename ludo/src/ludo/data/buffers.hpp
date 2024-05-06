/*
 * This file is part of ludo. See the LICENSE file for the full license governing this code.
 */

#include <cassert>

#include "buffers.h"

namespace ludo
{
  template<typename T>
  T& cast(buffer& buffer, uint64_t position)
  {
    assert(position >= 0 && position <= buffer.size - sizeof(T) && "position out of range");

    return *reinterpret_cast<T*>(buffer.data + position);
  }

  template<typename T>
  const T& cast(const buffer& buffer, uint64_t position)
  {
    assert(position >= 0 && position <= buffer.size - sizeof(T) && "position out of range");

    return *reinterpret_cast<const T*>(buffer.data + position);
  }

  template<typename T>
  T& peek(stream& stream)
  {
    assert(stream.position >= 0 && stream.position <= stream.size - sizeof(T) && "position out of range");

    return *reinterpret_cast<T*>(stream.data + stream.position);
  }

  template<typename T>
  const T& peek(const stream& stream)
  {
    assert(stream.position >= 0 && stream.position <= stream.size - sizeof(T) && "position out of range");

    return *reinterpret_cast<const T*>(stream.data + stream.position);
  }

  template<typename T>
  T& read(stream& stream)
  {
    assert(stream.position >= 0 && stream.position <= stream.size - sizeof(T) && "position out of range");

    auto data = reinterpret_cast<T*>(stream.data + stream.position);
    stream.position += sizeof(T);
    return *data;
  }

  template<typename T>
  const T& read(const stream& stream)
  {
    assert(stream.position >= 0 && stream.position <= stream.size - sizeof(T) && "position out of range");

    auto data = reinterpret_cast<const T*>(stream.data + stream.position);
    stream.position += sizeof(T);
    return *data;
  }

  template<typename T>
  void write(stream& stream, const T& value)
  {
    assert(stream.position >= 0 && stream.position <= stream.size - sizeof(T) && "position out of range");

    *reinterpret_cast<T*>(stream.data + stream.position) = value;
    stream.position += sizeof(T);
  }
}
