/*
 * This file is part of ludo. See the LICENSE file for the full license governing this code.
 */

#include <GL/glew.h>

#include <ludo/algorithm.h>
#include <ludo/graphs.h>

#include "frame_buffers.h"
#include "meshes.h"
#include "render_programs.h"
#include "rendering_contexts.h"
#include "util.h"

namespace ludo
{
  const uint64_t sync_timeout = 1000000000; // One second.

  auto mesh_primitives = std::unordered_map<mesh_primitive, GLenum>
  {
    { mesh_primitive::POINT_LIST, GL_POINTS },
    { mesh_primitive::LINE_LIST, GL_LINES },
    { mesh_primitive::LINE_STRIP, GL_LINE_STRIP },
    { mesh_primitive::TRIANGLE_LIST, GL_TRIANGLES },
    { mesh_primitive::TRIANGLE_STRIP, GL_TRIANGLE_STRIP }
  };

  std::array<vec4, 6> frustum_planes(const camera& camera);
  int32_t frustum_test(const std::array<vec4, 6>& planes, const aabb& bounds);

  void render(instance& instance, const render_options& options)
  {
    // TODO crashes without linear octrees!
    auto& linear_octrees = data<linear_octree>(instance);
    auto& meshes = data<mesh>(instance);
    auto& mesh_buffers = data<mesh_buffer>(instance);
    auto& render_programs = data<render_program>(instance);
    auto rendering_context = first<ludo::rendering_context>(instance);
    assert(rendering_context && "rendering context not found");

    bind(*rendering_context);

    if (options.frame_buffer_id)
    {
      auto frame_buffer = get<ludo::frame_buffer>(instance, options.frame_buffer_id);
      assert(frame_buffer && "frame buffer not found");

      bind(*frame_buffer);
    }
    else
    {
      auto window = options.window_id ? get<ludo::window>(instance, options.window_id) : first<ludo::window>(instance);
      assert(window && "window not found");

      bind(frame_buffer { .width = window->width, .height = window->height });
    }

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); check_opengl_error();

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, options.data_buffer.id); check_opengl_error();

    auto meshes_to_render = std::set<mesh>();
    if (!options.mesh_ids.empty())
    {
      for (auto& mesh : meshes)
      {
        if (std::find(options.mesh_ids.begin(), options.mesh_ids.end(), mesh.id) != options.mesh_ids.end())
        {
          meshes_to_render.insert(mesh);

          if (meshes_to_render.size() == options.mesh_ids.size())
          {
            break;
          }
        }
      }
    }
    else if (linear_octrees.array_size)
    {
      auto planes = frustum_planes(get_camera(*rendering_context));

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

        auto results = find_parallel(linear_octree,
          [&planes, &octant_size](const aabb& bounds)
          {
            return frustum_test(planes,
            // Include neighbouring octants to ensure the meshes that overlap from them into this octant are included.
            {
              .min = bounds.min - octant_size,
              .max = bounds.max + octant_size
            });
          }
        );

        meshes_to_render.insert(results.begin(), results.end());
      }
    }
    else
    {
      meshes_to_render.insert(meshes.begin(), meshes.end());
    }

    auto command_counts = std::vector<uint32_t>(mesh_buffers.array_size, 0);
    auto mesh_buffer = static_cast<ludo::mesh_buffer*>(nullptr);
    auto mesh_buffer_index = uint64_t(0);

    for (auto& mesh : meshes_to_render)
    {
      if (!mesh_buffer || mesh.mesh_buffer_id != mesh_buffer->id)
      {
        mesh_buffer = find_by_id(mesh_buffers.begin(), mesh_buffers.end(), mesh.mesh_buffer_id);
        assert(mesh_buffer != mesh_buffers.end() && "mesh buffer not found");

        mesh_buffer_index = mesh_buffer - mesh_buffers.begin();
      }

      write(mesh_buffer->command_buffer, command_counts[mesh_buffer_index]++ * sizeof(draw_command), draw_command
      {
        .index_count = static_cast<GLuint>(mesh.index_buffer.size / sizeof(uint32_t)),
        .instance_count = mesh.instance_count,
        .index_start = static_cast<GLuint>((mesh.index_buffer.data - mesh_buffer->index_buffer.data) / sizeof(uint32_t)),
        .vertex_start = static_cast<GLuint>((mesh.vertex_buffer.data - mesh_buffer->vertex_buffer.data) / mesh_buffer->format.size),
        .instance_start = mesh.instance_start
      });
    }

    auto render_program_id = uint32_t(0);
    for (auto index = 0; index <  mesh_buffers.array_size; index++)
    {
      auto command_count = command_counts[index];
      if (command_count == 0)
      {
        continue;
      }

      auto& mesh_buffer_to_render = mesh_buffers[index];

      auto render_program_to_execute_id = options.render_program_id ? options.render_program_id : mesh_buffer_to_render.render_program_id;
      assert(render_program_to_execute_id && "render program not specified");

      if (render_program_to_execute_id != render_program_id)
      {
        auto render_program = find_by_id(render_programs.begin(), render_programs.end(), render_program_to_execute_id);
        assert(render_program != render_programs.end() && "render program not found");

        render_program_id = render_program_to_execute_id;
        bind(*render_program);
      }

      bind(mesh_buffer_to_render);

      glMultiDrawElementsIndirect(
        mesh_primitives[mesh_buffer_to_render.primitive],
        GL_UNSIGNED_INT,
        nullptr,
        static_cast<GLsizei>(command_count),
        sizeof(draw_command)
      ); check_opengl_error();
    }
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
