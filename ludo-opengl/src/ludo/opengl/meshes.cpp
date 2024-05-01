/*
 * This file is part of ludo. See the LICENSE file for the full license governing this code.
 */

#include <ludo/meshes.h>

#include "./textures.h"

namespace ludo
{
  void set_texture(mesh_instance &mesh_instance, const texture &texture)
  {
    write<uint64_t>(mesh_instance.instance_buffer, sizeof(mat4), handle(texture));
  }
}
