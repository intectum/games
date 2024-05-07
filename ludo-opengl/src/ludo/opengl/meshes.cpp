/*
 * This file is part of ludo. See the LICENSE file for the full license governing this code.
 */

#include <ludo/meshes.h>

#include "./textures.h"

namespace ludo
{
  void set_texture(mesh_instance &mesh_instance, const texture &texture, uint32_t instance_index)
  {
    cast<uint64_t>(mesh_instance.instance_buffer, instance_index * mesh_instance.instance_size + sizeof(mat4)) = handle(texture);
  }
}
