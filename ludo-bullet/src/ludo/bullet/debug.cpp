/*
 * This file is part of ludo. See the LICENSE file for the full license governing this code.
 */

#include <iostream>

#include "debug.h"
#include "math.h"

namespace ludo
{
  debug_drawer::debug_drawer(int32_t debug_mode) :
    mesh(nullptr),
    debug_mode(debug_mode),
    next_index(0)
  {
  }

  void debug_drawer::drawLine(const btVector3& from, const btVector3& to, const btVector3& color)
  {
    auto index_byte_index = next_index * sizeof(uint32_t);
    auto vertex_byte_index = next_index * vertex_format_pc.size;

    write(mesh->index_buffer, index_byte_index, next_index);
    index_byte_index += sizeof(uint32_t);
    write(mesh->index_buffer, index_byte_index, next_index + 1);

    write(mesh->vertex_buffer, vertex_byte_index, to_vec3(from));
    vertex_byte_index += sizeof(vec3);
    write(mesh->vertex_buffer, vertex_byte_index, vec4(to_vec3(color)));
    vertex_byte_index += sizeof(vec4);
    write(mesh->vertex_buffer, vertex_byte_index, to_vec3(to));
    vertex_byte_index += sizeof(vec3);
    write(mesh->vertex_buffer, vertex_byte_index, vec4(to_vec3(color)));

    next_index += 2;
  }

  void debug_drawer::clearLines()
  {
    next_index = 0;
  }

  void debug_drawer::drawContactPoint(const btVector3& PointOnB, const btVector3& normalOnB, btScalar distance, int lifeTime, const btVector3& color)
  {
  }

  void debug_drawer::draw3dText(const btVector3& location, const char* text_string)
  {
    std::cout << "bullet debug 3d text: " << text_string << std::endl;
  }

  void debug_drawer::reportErrorWarning(const char* warning_string)
  {
    std::cout << "bullet debug warning: " << warning_string << std::endl;
  }

  int debug_drawer::getDebugMode() const
  {
    return debug_mode;
  }

  void debug_drawer::setDebugMode(int debug_mode)
  {
    this->debug_mode = debug_mode;
  }
}
