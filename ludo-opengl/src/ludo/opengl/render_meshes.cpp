/*
 * This file is part of ludo. See the LICENSE file for the full license governing this code.
 */

#include <ludo/meshes.h>

#include "./textures.h"

namespace ludo
{
  void set_instance_texture(render_mesh &render_mesh, const texture& texture, uint32_t instance_index)
  {
    cast<uint64_t>(render_mesh.instance_buffer, instance_index * render_mesh.instance_size + sizeof(mat4)) = handle(texture);
  }
}
