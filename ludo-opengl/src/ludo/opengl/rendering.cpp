/*
 * This file is part of ludo. See the LICENSE file for the full license governing this code.
 */

#include <GL/glew.h>

#include <ludo/spatial/grid3.h>
#include <ludo/timer.h>
#include <ludo/windowing.h>

#include "frame_buffers.h"
#include "render_programs.h"
#include "rendering_contexts.h"
#include "util.h"

namespace ludo
{
  auto draw_modes = std::unordered_map<mesh_primitive, GLenum>
  {
    { mesh_primitive::POINT_LIST, GL_POINTS },
    { mesh_primitive::LINE_LIST, GL_LINES },
    { mesh_primitive::LINE_STRIP, GL_LINE_STRIP },
    { mesh_primitive::TRIANGLE_LIST, GL_TRIANGLES },
    { mesh_primitive::TRIANGLE_STRIP, GL_TRIANGLE_STRIP }
  };

  void write_commands(instance& instance, const render_options& options);
  void write_command(instance& instance, const render_options& options, const mesh_instance& mesh_instance);
  uint64_t get_render_program_id(const render_options& options, const mesh_instance& mesh_instance);
  std::array<vec4, 6> frustum_planes(const camera& camera);

  void prepare_render(instance& instance)
  {
    auto& render_programs = data<render_program>(instance);
    auto rendering_context = first<ludo::rendering_context>(instance);
    assert(rendering_context && "rendering context not found");

    wait_for_fence(rendering_context->fence);

    for (auto& render_program : render_programs)
    {
      render_program.active_commands.start = 0;
    }
  }

  void render(instance& instance, const render_options& options)
  {
    auto& render_programs = data<render_program>(instance);
    auto rendering_context = first<ludo::rendering_context>(instance);
    assert(rendering_context && "rendering context not found");

    auto& draw_commands = data_heap(instance, "ludo::vram_draw_commands");
    auto& indices = data_heap(instance, "ludo::vram_indices");
    auto& vertices = data_heap(instance, "ludo::vram_vertices");

    write_commands(instance, options);

    glBindBuffer(GL_DRAW_INDIRECT_BUFFER, draw_commands.id); check_opengl_error();
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indices.id); check_opengl_error();
    glBindBuffer(GL_ARRAY_BUFFER, vertices.id); check_opengl_error();

    bind(*rendering_context);

    push(options.shader_buffer);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, options.shader_buffer.front.id); check_opengl_error();

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

    if (options.clear_frame_buffer)
    {
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); check_opengl_error();
    }

    for (auto& render_program : render_programs)
    {
      if (!render_program.active_commands.count)
      {
        continue;
      }

      bind(render_program);

      glMultiDrawElementsIndirect(
        draw_modes[render_program.primitive],
        GL_UNSIGNED_INT,
        reinterpret_cast<void*>((render_program.command_buffer.data - draw_commands.data) + render_program.active_commands.start * sizeof(draw_command)),
        static_cast<GLsizei>(render_program.active_commands.count),
        sizeof(draw_command)
      ); check_opengl_error();

      render_program.active_commands.start += render_program.active_commands.count;
      render_program.active_commands.count = 0;
    }
  }

  void finalize_render(instance& instance)
  {
    auto rendering_context = first<ludo::rendering_context>(instance);
    assert(rendering_context && "rendering context not found");

    rendering_context->fence = create_fence();
  }

  void write_commands(instance& instance, const render_options& options)
  {
    if (!options.mesh_instance_ids.empty())
    {
      for (auto mesh_instance_id : options.mesh_instance_ids)
      {
        auto mesh_instance = get<ludo::mesh_instance>(instance, mesh_instance_id);
        assert(mesh_instance && "mesh instance not found");

        write_command(instance, options, *mesh_instance);
      }
    }
    else if (!options.grid_ids.empty())
    {
      auto& render_programs = data<render_program>(instance);
      auto rendering_context = first<ludo::rendering_context>(instance);

      auto& draw_commands = data_heap(instance, "ludo::vram_draw_commands");

      auto planes = frustum_planes(get_camera(*rendering_context));

      // TODO take allocation out of frame?
      auto context_buffer = allocate_vram(6 * 16 + 8 + render_programs.length * (sizeof(uint64_t) + 2 * sizeof(uint32_t)));

      auto stream = ludo::stream(context_buffer);
      write(stream, planes[0]);
      write(stream, planes[1]);
      write(stream, planes[2]);
      write(stream, planes[3]);
      write(stream, planes[4]);
      write(stream, planes[5]);
      write(stream, static_cast<uint32_t>(render_programs.length));
      stream.position += 4; // align 8
      for (auto& render_program : render_programs)
      {
        write(stream, render_program.id);
        write(stream, static_cast<uint32_t>((render_program.command_buffer.data - draw_commands.data) / sizeof(draw_command) + render_program.active_commands.start));
        write(stream, render_program.active_commands.count);
      }

      for (auto grid_id : options.grid_ids)
      {
        auto grid = get<ludo::grid3>(instance, grid_id);
        assert(grid && "grid not found");

        auto grid_compute_program = get<ludo::compute_program>(instance, grid->compute_program_id);
        assert(grid_compute_program && "compute program not found");

        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, context_buffer.id); check_opengl_error();
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, grid->buffer.front.id); check_opengl_error();
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, draw_commands.id); check_opengl_error();

        ludo::execute(*grid_compute_program, grid->cell_count_1d / 8, grid->cell_count_1d / 4, grid->cell_count_1d); check_opengl_error();
      }

      auto fence = create_fence();
      wait_for_fence(fence);

      for (auto index = 0; index < render_programs.length; index++)
      {
        auto offset = 6 * 16 + 8 + index * (sizeof(uint64_t) + 2 * sizeof(uint32_t));
        render_programs[index].active_commands.count = cast<uint32_t>(context_buffer, offset + sizeof(uint64_t) + sizeof(uint32_t));
      }

      deallocate_vram(context_buffer);
    }
    else if (exists<mesh_instance>(instance))
    {
      auto& mesh_instances = data<mesh_instance>(instance);
      for (auto& mesh_instance : mesh_instances)
      {
        write_command(instance, options, mesh_instance);
      }
    }
  }

  void write_command(instance& instance, const render_options& options, const mesh_instance& mesh_instance)
  {
    auto render_program = get<ludo::render_program>(instance, get_render_program_id(options, mesh_instance));
    assert(render_program && "render program not found");

    auto position = (render_program->active_commands.start + render_program->active_commands.count++) * sizeof(draw_command);
    cast<draw_command>(render_program->command_buffer, position) =
    {
      .index_count = mesh_instance.indices.count,
      .instance_count = mesh_instance.instances.count,
      .index_start = mesh_instance.indices.start,
      .vertex_start = mesh_instance.vertices.start,
      .instance_start = mesh_instance.instances.start
    };
  }

  uint64_t get_render_program_id(const render_options& options, const mesh_instance& mesh_instance)
  {
    auto render_program_id = options.render_program_id ? options.render_program_id : mesh_instance.render_program_id;
    assert(render_program_id && "render program not specified");

    return render_program_id;
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
}
