/*
 * This file is part of ludo. See the LICENSE file for the full license governing this code.
 */

#include <iostream>

#include "debug.h"
#include "math.h"

namespace ludo
{
  debug_drawer::debug_drawer() :
    mesh(nullptr),
    indices(),
    vertices(),
    debug_mode(btIDebugDraw::DBG_DrawWireframe)
  {
  }

  void debug_drawer::drawLine(const btVector3& from, const btVector3& to, const btVector3& color)
  {
    cast<uint32_t>(indices, mesh->indices.count * sizeof(uint32_t)) = mesh->indices.count;
    mesh->indices.count++;
    cast<uint32_t>(indices, mesh->indices.count * sizeof(uint32_t)) = mesh->indices.count;
    mesh->indices.count++;

    cast<vec3>(vertices, mesh->vertices.count * mesh->vertex_size) = to_vec3(from);
    cast<vec4>(vertices, mesh->vertices.count * mesh->vertex_size + sizeof(vec3)) = vec4(to_vec3(color));
    mesh->vertices.count++;
    cast<vec3>(vertices, mesh->vertices.count * mesh->vertex_size) = to_vec3(to);
    cast<vec4>(vertices, mesh->vertices.count * mesh->vertex_size + sizeof(vec3)) = vec4(to_vec3(color));
    mesh->vertices.count++;
  }

  void debug_drawer::clearLines()
  {
    mesh->indices.count = 0;
    mesh->vertices.count = 0;
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
