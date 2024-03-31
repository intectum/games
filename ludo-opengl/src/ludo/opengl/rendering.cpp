/*
 * This file is part of ludo. See the LICENSE file for the full license governing this code.
 */

#include <GL/glew.h>

#include <ludo/algorithm.h>
#include <ludo/animation.h>
#include <ludo/graphs.h>

#include "frame_buffers.h"
#include "render_programs.h"
#include "rendering_contexts.h"
#include "textures.h"
#include "util.h"

namespace ludo
{
  const uint64_t sync_timeout = 1000000000; // One second.

  auto draw_modes = std::unordered_map<mesh_primitive, GLenum>
  {
    { mesh_primitive::POINT_LIST, GL_POINTS },
    { mesh_primitive::LINE_LIST, GL_LINES },
    { mesh_primitive::LINE_STRIP, GL_LINE_STRIP },
    { mesh_primitive::TRIANGLE_LIST, GL_TRIANGLES },
    { mesh_primitive::TRIANGLE_STRIP, GL_TRIANGLE_STRIP }
  };

  std::unordered_map<uint64_t, std::vector<mesh>> find_meshes(const instance& instance, const rendering_context& rendering_context, const render_options& options);
  uint64_t get_render_program_id(const render_options& options, const mesh& mesh);
  std::array<vec4, 6> frustum_planes(const camera& camera);
  int32_t frustum_test(const std::array<vec4, 6>& planes, const aabb& bounds);

  void render(instance& instance, const render_options& options)
  {
    // TODO crashes without armature instances!
    auto& armature_instances = data<armature_instance>(instance);
    auto& render_programs = data<render_program>(instance);
    auto rendering_context = first<ludo::rendering_context>(instance);
    auto& textures = data<texture>(instance);

    auto& vram_draw_commands = data<draw_command>(instance);
    auto& vram_instances = data<instance_t>(instance);
    auto& vram_indices = data_heap<index_t>(instance);
    auto& vram_vertices = data_heap<vertex_t>(instance);

    assert(rendering_context && "rendering context not found");
    bind(*rendering_context);

    glBindBuffer(GL_DRAW_INDIRECT_BUFFER, vram_draw_commands.id); check_opengl_error();
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vram_indices.id); check_opengl_error();
    glBindBuffer(GL_ARRAY_BUFFER, vram_vertices.id); check_opengl_error();
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, options.shader_buffer.id); check_opengl_error();

    if (options.frame_buffer_id)
    {
      auto frame_buffer = get<ludo::frame_buffer>(instance, options.frame_buffer_id);
      assert(frame_buffer && "frame buffer not found");

      bind(*frame_buffer);
    }
    else
    {
      auto window = first<ludo::window>(instance);

      bind(frame_buffer { .width = window->width, .height = window->height });
    }

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); check_opengl_error();

    auto grouped_meshes = find_meshes(instance, *rendering_context, options);
    for (auto& mesh_group : grouped_meshes)
    {
      auto render_program = find_by_id(render_programs.begin(), render_programs.end(), mesh_group.first);
      assert(render_program != render_programs.end() && "render program not found");

      auto draw_command_start = uint32_t(vram_draw_commands.array_size);
      auto instance_byte_start = uint64_t(vram_instances.array_size);
      auto instance_byte_index = instance_byte_start;
      auto instance_byte_size = mesh_group.second.size() * (sizeof(mat4) + (count(render_program->format, 't') ? sizeof(uint64_t) : 0));
      auto animation_byte_start = instance_byte_start + instance_byte_size;
      auto animation_byte_index = animation_byte_start;
      auto animation_byte_size = mesh_group.second.size() * (count(render_program->format, 'f') ? max_bones_per_armature * sizeof(mat4) : 0);

      for (auto& mesh : mesh_group.second)
      {
        write(vram_instances, instance_byte_index, mesh.transform);
        instance_byte_index += sizeof(mat4);

        if (count(render_program->format, 't'))
        {
          if (mesh.texture_id)
          {
            auto texture = find_by_id(textures.begin(), textures.end(), mesh.texture_id);
            assert(texture != textures.end() && "texture not found");

            write(vram_instances, instance_byte_index, handle(*texture));
            instance_byte_index += sizeof(uint64_t);
          }
          else
          {
            write(vram_instances, instance_byte_index, uint64_t(0));
            instance_byte_index += sizeof(uint64_t);
          }
        }

        // TODO 'f' is a pretty general component, maybe add a 'b' component specifically for bones?
        if (count(render_program->format, 'f'))
        {
          auto armature_instance = find_by_id(armature_instances.begin(), armature_instances.end(), mesh.armature_instance_id);
          assert(armature_instance != armature_instances.end() && "armature instance not found");

          for (auto& transform : armature_instance->transforms)
          {
            write(vram_instances, animation_byte_index, transform);
            animation_byte_index += sizeof(mat4);
          }
        }

        // TODO combine instance commands where possible?
        add(vram_draw_commands, draw_command
        {
          .index_count = static_cast<GLuint>(mesh.index_buffer.size / sizeof(uint32_t)),
          .index_start = static_cast<GLuint>((mesh.index_buffer.data - vram_indices.data) / sizeof(uint32_t)),
          .vertex_start = static_cast<GLuint>((mesh.vertex_buffer.data - vram_vertices.data) / render_program->format.size),
          .instance_start = static_cast<GLuint>(vram_draw_commands.array_size)
        });
      }

      bind(*render_program);

      glBindBufferRange(
        GL_SHADER_STORAGE_BUFFER,
        3,
        vram_instances.id,
        static_cast<GLintptr>(instance_byte_start),
        static_cast<GLintptr>(instance_byte_size)
      ); check_opengl_error();

      // The bone transforms are bound separately because including an array of mat4 within the shader's instance_t struct did not work...
      if (animation_byte_size)
      {
        glBindBufferRange(
          GL_SHADER_STORAGE_BUFFER,
          4,
          vram_instances.id,
          static_cast<GLintptr>(animation_byte_start),
          static_cast<GLintptr>(animation_byte_size)
        ); check_opengl_error();
      }

      glMultiDrawElementsIndirect(
        draw_modes[render_program->primitive],
        GL_UNSIGNED_INT,
        reinterpret_cast<void*>(draw_command_start * sizeof(draw_command)),
        static_cast<GLsizei>(vram_draw_commands.array_size - draw_command_start),
        sizeof(draw_command)
      ); check_opengl_error();

      vram_instances.array_size += instance_byte_size + animation_byte_size;
    }
  }

  std::unordered_map<uint64_t, std::vector<mesh>> find_meshes(const instance& instance, const rendering_context& rendering_context, const render_options& options)
  {
    auto grouped_meshes = std::unordered_map<uint64_t, std::vector<mesh>>();

    if (!options.mesh_ids.empty())
    {
      auto& meshes = data<mesh>(instance);
      for (auto mesh_id : options.mesh_ids)
      {
        auto mesh = find_by_id(meshes.begin(), meshes.end(), mesh_id);
        assert(mesh && "mesh not found");

        grouped_meshes[get_render_program_id(options, *mesh)].push_back(*mesh);
      }
    }
    else if (exists<linear_octree>(instance))
    {
      auto& linear_octrees = data<linear_octree>(instance);
      auto planes = frustum_planes(get_camera(rendering_context));

      for (auto& linear_octree : linear_octrees)
      {
        if (!options.linear_octree_ids.empty())
        {
          auto linear_octree_iter = find_by_id(linear_octrees.begin(), linear_octrees.end(), linear_octree.id);
          if (linear_octree_iter == linear_octrees.end())
          {
            continue;
          }
        }

        auto octant_size = ludo::octant_size(linear_octree);

        auto results = find_parallel(linear_octree, [&planes, &octant_size](const aabb& bounds)
        {
          return frustum_test(
            planes,
            // Include neighbouring octants to ensure the meshes that overlap from them into this octant are included.
            {
              .min = bounds.min - octant_size,
              .max = bounds.max + octant_size
            }
          );
        });

        for (auto& mesh : results)
        {
          grouped_meshes[get_render_program_id(options, mesh)].push_back(mesh);
        }
      }
    }
    else if (exists<mesh>(instance))
    {
      auto& meshes = data<mesh>(instance);
      for (auto& mesh : meshes)
      {
        grouped_meshes[get_render_program_id(options, mesh)].push_back(mesh);
      }
    }

    return grouped_meshes;
  }

  uint64_t get_render_program_id(const render_options& options, const mesh& mesh)
  {
    auto render_program_id = options.render_program_id ? options.render_program_id : mesh.render_program_id;
    assert(render_program_id && "render program not specified");

    return render_program_id;
  }

  void wait_for_render()
  {
    auto sync = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0); check_opengl_error();

    // First do a quick check.
    auto result = glClientWaitSync(sync, 0, 0); check_opengl_error();
    while (result == GL_TIMEOUT_EXPIRED)
    {
      // Now flush and wait...
      result = glClientWaitSync(sync, GL_SYNC_FLUSH_COMMANDS_BIT, sync_timeout); check_opengl_error();
    }

    glDeleteSync(sync); check_opengl_error();
  }

  // Generate the planes of the view frustum.
  // Planes have their normals pointing into the view frustum.
  // Based on: http://www.cs.otago.ac.nz/postgrads/alexis/planeExtraction.pdf
  std::array<vec4, 6> frustum_planes(const camera& camera)
  {
    auto view_inverse = camera.view;
    invert(view_inverse);
    auto view_projection = camera.projection * view_inverse;

    auto rows = std::array<vec4, 4>
    {
      vec4 { view_projection[0], view_projection[4], view_projection[8], view_projection[12] },
      vec4 { view_projection[1], view_projection[5], view_projection[9], view_projection[13] },
      vec4 { view_projection[2], view_projection[6], view_projection[10], view_projection[14] },
      vec4 { view_projection[3], view_projection[7], view_projection[11], view_projection[15] },
    };

    return std::array<vec4, 6>
    {
      rows[3] + rows[0], // Left
      rows[3] - rows[0], // Right
      rows[3] + rows[1], // Bottom
      rows[3] - rows[1], // Top
      rows[3] + rows[2], // Near
      rows[3] - rows[2] // Far
    };
  }

  // Based on: https://old.cescg.org/CESCG-2002/DSykoraJJelinek/index.html
  int32_t frustum_test(const std::array<vec4, 6>& planes, const aabb& bounds)
  {
    for (auto& plane : planes)
    {
      // This is the vertex that would be closest to the plane if the AABB is fully within the negative halfspace (the p-vertex).
      // If this vertex is indeed in the negative halfspace, all other vertices of the AABB must also be in the negative halfspace.
      auto closest_negative = vec4
      {
        plane[0] > 0.0f ? bounds.max[0] : bounds.min[0],
        plane[1] > 0.0f ? bounds.max[1] : bounds.min[1],
        plane[2] > 0.0f ? bounds.max[2] : bounds.min[2],
        1.0f
      };

      // Check if the p-vertex is within the negative halfspace.
      if (dot(plane, closest_negative) < 0.0f)
      {
        return -1;
      }

      // This is the vertex that would be closest to the plane if the AABB is fully within the positive halfspace (the n-vertex).
      // If this vertex is actually in the negative halfspace, the AABB intersects the plane (since we already showed that at-least one vertex is in the positive halfspace).
      auto closest_positive = vec4
      {
        plane[0] > 0.0f ? bounds.min[0] : bounds.max[0],
        plane[1] > 0.0f ? bounds.min[1] : bounds.max[1],
        plane[2] > 0.0f ? bounds.min[2] : bounds.max[2],
        1.0f
      };

      // Check if the n-vertex is within the negative halfspace.
      if (dot(plane, closest_positive) < 0.0f)
      {
        return 0;
      }
    }

    return 1;
  }
}
