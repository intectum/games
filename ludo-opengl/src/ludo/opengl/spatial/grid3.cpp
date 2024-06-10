/*
 * This file is part of ludo. See the LICENSE file for the full license governing this code.
 */

#include <sstream>

#include <ludo/spatial/grid3.h>

#include "../util.h"

namespace ludo
{
  compute_program build_compute_program(const grid3& grid)
  {
    auto code = std::stringstream();

    code <<
R"--(
#version 460 core
#extension GL_ARB_gpu_shader_int64 : require

layout(local_size_x = 8, local_size_y = 4, local_size_z = 1) in;

// Types

struct render_program_t
{
  uint64_t id;
  uint active_command_start;
  uint active_command_count;
};

struct render_command_t
{
  uint index_count;
  uint instance_count;
  uint index_start;
  uint vertex_start;
  uint instance_start;
};

struct render_mesh_t
{
  uint64_t id;
  uint64_t render_program_id;
  uint instance_start;
  uint instance_count;
  uint index_start;
  uint index_count;
  uint vertex_start;
  uint vertex_count;
};

struct cell_t
{
  uint count;
)--";

    code << "  render_mesh_t render_meshes[" << grid.cell_capacity << "];" << std::endl;

    code <<
R"--(
};

struct aabb3_t
{
  vec3 min;
  vec3 max;
};

// Buffers

layout(std430, binding = 1) buffer context_layout
{
  vec4 planes[6];
  uint render_program_count;
  render_program_t render_programs[];
};

layout(std430, binding = 2) buffer grid_layout
{
  aabb3_t bounds;
  vec3 cell_dimensions;
  cell_t cells[];
};

layout(std430, binding = 3) buffer command_layout
{
  render_command_t commands[];
};

int get_render_program_index(uint64_t id)
{
  for (int index = 0; index < render_program_count; index++)
  {
    if (render_programs[index].id == id)
    {
      return index;
    }
  }

  return -1;
}

// Based on https://old.cescg.org/CESCG-2002/DSykoraJJelinek/index.html
int frustum_test(aabb3_t bounds)
{
  for (uint index = 0; index < 6; index++)
  {
    vec4 plane = planes[index];

    // This is the vertex that would be closest to the plane if the AABB is fully within the negative halfspace (the p-vertex).
    // If this vertex is indeed in the negative halfspace, all other vertices of the AABB must also be in the negative halfspace.
    vec4 closest_negative =
    vec4(
      plane[0] > 0.0 ? bounds.max[0] : bounds.min[0],
      plane[1] > 0.0 ? bounds.max[1] : bounds.min[1],
      plane[2] > 0.0 ? bounds.max[2] : bounds.min[2],
      1.0
    );

    // Check if the p-vertex is within the negative halfspace.
    if (dot(plane, closest_negative) < 0.0)
    {
      return -1;
    }

    // This is the vertex that would be closest to the plane if the AABB is fully within the positive halfspace (the n-vertex).
    // If this vertex is actually in the negative halfspace, the AABB intersects the plane (since we already showed that at-least one vertex is in the positive halfspace).
    vec4 closest_positive =
    vec4(
      plane[0] > 0.0 ? bounds.min[0] : bounds.max[0],
      plane[1] > 0.0 ? bounds.min[1] : bounds.max[1],
      plane[2] > 0.0 ? bounds.min[2] : bounds.max[2],
      1.0
    );

    // Check if the n-vertex is within the negative halfspace.
    if (dot(plane, closest_positive) < 0.0)
    {
      return 0;
    }
  }

  return 1;
}

void main()
{
  uint cell_count_1d = gl_NumWorkGroups.x * gl_WorkGroupSize.x;
  uint cell_index =
    gl_GlobalInvocationID.x * cell_count_1d * cell_count_1d +
    gl_GlobalInvocationID.y * cell_count_1d +
    gl_GlobalInvocationID.z;

  vec3 cell_min = bounds.min + gl_GlobalInvocationID * cell_dimensions;

  // Include neighbouring cells to ensure the render meshes that overlap from them into this cell are included.
  aabb3_t test_bounds = aabb3_t(cell_min - cell_dimensions, cell_min + cell_dimensions * 2);
  if (frustum_test(test_bounds)  != -1)
  {
    for (uint render_mesh_index = 0; render_mesh_index < cells[cell_index].count; render_mesh_index++)
    {
      render_mesh_t render_mesh = cells[cell_index].render_meshes[render_mesh_index];

      int render_program_index = get_render_program_index(render_mesh.render_program_id);
      if (render_program_index == -1)
      {
        continue;
      }

      uint command_index = render_programs[render_program_index].active_command_start + atomicAdd(render_programs[render_program_index].active_command_count, 1);

      commands[command_index].index_count = render_mesh.index_count;
      commands[command_index].instance_count = render_mesh.instance_count;
      commands[command_index].index_start = render_mesh.index_start;
      commands[command_index].vertex_start = render_mesh.vertex_start;
      commands[command_index].instance_start = render_mesh.instance_start;
    }
  }
}
)--";

    auto program = compute_program();
    init(program, code);

    return program;
  }

  void add_render_commands(array<grid3>& grids, array<compute_program>& compute_programs, array<render_program>& render_programs, const heap& render_commands, const camera& camera)
  {
    auto planes = frustum_planes(camera);

    // TODO take this allocation out of here?
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
      write(stream, static_cast<uint32_t>((render_program.command_buffer.data - render_commands.data) / sizeof(render_command) + render_program.active_commands.start));
      write(stream, render_program.active_commands.count);
    }

    for (auto& grid : grids)
    {
      auto compute_program = find_by_id(compute_programs.begin(), compute_programs.end(), grid.compute_program_id);
      assert(compute_program != compute_programs.end() && "compute program not found");

      glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, context_buffer.id); check_opengl_error();
      glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, grid.buffer.front.id); check_opengl_error();
      glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, render_commands.id); check_opengl_error();

      ludo::execute(*compute_program, grid.cell_count_1d / 8, grid.cell_count_1d / 4, grid.cell_count_1d); check_opengl_error();
    }

    auto fence = ludo::fence();
    init(fence);
    wait(fence);

    for (auto index = 0; index < render_programs.length; index++)
    {
      auto offset = 6 * 16 + 8 + index * (sizeof(uint64_t) + 2 * sizeof(uint32_t));
      render_programs[index].active_commands.count = cast<uint32_t>(context_buffer, offset + sizeof(uint64_t) + sizeof(uint32_t));
    }

    deallocate_vram(context_buffer);
  }
}
