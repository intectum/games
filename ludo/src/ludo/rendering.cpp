/*
 * This file is part of ludo. See the LICENSE file for the full license governing this code.
 */

#include "animation.h"
#include "rendering.h"

namespace ludo
{
  uint8_t pixel_depth(const texture& texture)
  {
    auto component_count = uint8_t(0);
    if (texture.components == pixel_components::BGR || texture.components == pixel_components::RGB)
    {
      component_count = 3;
    }
    else if (texture.components == pixel_components::BGRA || texture.components == pixel_components::RGBA)
    {
      component_count = 4;
    }
    else if (texture.components == pixel_components::DEPTH)
    {
      component_count = 1;
    }
    else
    {
      assert(false && "unsupported components");
    }

    if (texture.datatype == pixel_datatype::UINT8)
    {
      return component_count;
    }
    else if (texture.datatype == pixel_datatype::FLOAT16)
    {
      return component_count * 2;
    }
    else if (texture.datatype == pixel_datatype::FLOAT32)
    {
      return component_count * 4;
    }
    else
    {
      assert(false && "unsupported datatype");
    }

    return 3;
  }

  void init(render_mesh& render_mesh)
  {
    render_mesh.id = next_id++;
  }

  void init(render_mesh& render_mesh, render_program& render_program, const mesh& mesh, const heap& indices, const heap& vertices, uint32_t instance_capacity)
  {
    render_mesh.id = next_id++;
    ludo::connect(render_mesh, render_program, instance_capacity);
    ludo::connect(render_mesh, mesh, indices, vertices);
    ludo::init_instances(render_mesh, mesh, instance_capacity);
  }

  void de_init(render_mesh& render_mesh)
  {
    render_mesh.id = 0;
  }

  void connect(render_mesh& render_mesh, render_program& render_program, uint32_t instance_capacity)
  {
    render_mesh.render_program_id = render_program.id;
    render_mesh.instance_buffer = allocate(render_program.instance_buffer_back, render_program.instance_size * instance_capacity);
    render_mesh.instance_size = render_program.instance_size;
  }

  void disconnect(render_mesh& render_mesh, render_program& render_program)
  {
    render_mesh.render_program_id = 0;
    if (render_mesh.instance_buffer.data)
    {
      deallocate(render_program.instance_buffer_back, render_mesh.instance_buffer);
    }
    render_mesh.instance_size = 0;
  }

  void connect(render_mesh& render_mesh, const mesh& mesh, const heap& indices, const heap& vertices)
  {
    render_mesh.indices.start = (mesh.index_buffer.data - indices.data) / sizeof(uint32_t);
    render_mesh.indices.count = mesh.index_buffer.size / sizeof(uint32_t);

    render_mesh.vertices.start = (mesh.vertex_buffer.data - vertices.data) / mesh.vertex_size;
    render_mesh.vertices.count = mesh.vertex_buffer.size / mesh.vertex_size;
  }

  void init_instances(render_mesh& render_mesh, const mesh& mesh, uint32_t instance_count)
  {
    for (auto instance_index = uint32_t(0); instance_index < instance_count; instance_index++)
    {
      instance_transform(render_mesh, instance_index) = mat4_identity;

      if (mesh.texture_id)
      {
        set_instance_texture(render_mesh, texture { .id = mesh.texture_id }, instance_index);
      }

      if (mesh.armature_id)
      {
        auto bone_transform = instance_bone_transforms(render_mesh, instance_index);
        for (auto bone_transform_index = uint32_t(0); bone_transform_index < max_bones_per_armature; bone_transform_index++)
        {
          *(bone_transform + bone_transform_index) = mat4_identity;
        }
      }
    }
  }

  mat4& instance_transform(render_mesh& render_mesh, uint32_t instance_index)
  {
    return cast<mat4>(render_mesh.instance_buffer, instance_index * render_mesh.instance_size);
  }

  const mat4& instance_transform(const render_mesh& render_mesh, uint32_t instance_index)
  {
    return cast<const mat4>(render_mesh.instance_buffer, instance_index * render_mesh.instance_size);
  }

  ludo::mat4* instance_bone_transforms(render_mesh& render_mesh, uint32_t instance_index)
  {
    return &cast<ludo::mat4>(render_mesh.instance_buffer, instance_index * render_mesh.instance_size + sizeof(ludo::mat4) + 16);
  }

  const ludo::mat4* instance_bone_transforms(const render_mesh& render_mesh, uint32_t instance_index)
  {
    return &cast<const ludo::mat4>(render_mesh.instance_buffer, instance_index * render_mesh.instance_size + sizeof(ludo::mat4) + 16);
  }
}
