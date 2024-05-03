/*
 * This file is part of ludo. See the LICENSE file for the full license governing this code.
 */

#include <sstream>

#include <ludo/graphs.h>

namespace ludo
{
  compute_program* add_linear_octree_compute_program(instance& instance, const linear_octree& octree, uint32_t max_render_programs)
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

struct draw_command_t
{
  uint index_count;
  uint instance_count;
  uint index_start;
  uint vertex_start;
  uint instance_start;
};

struct mesh_instance_t
{
  uint64_t id;
  uint64_t render_program_id;
  uint instance_index;
  uint index_start;
  uint index_count;
  uint vertex_start;
};

struct octant_t
{
  uint count;
)--";

    code << "  mesh_instance_t mesh_instances[" << octree.octant_capacity << "];" << std::endl;

    code <<
R"--(
};

struct aabb_t
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

layout(std430, binding = 2) buffer linear_octree_layout
{
  aabb_t bounds;
  vec3 octant_dimensions;
  octant_t octants[];
};

layout(std430, binding = 3) buffer command_layout
{
  draw_command_t commands[];
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

// Based on: https://old.cescg.org/CESCG-2002/DSykoraJJelinek/index.html
int frustum_test(aabb_t bounds)
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
  uint octant_count_1d = gl_NumWorkGroups.x * gl_WorkGroupSize.x;
  uint octant_index =
    gl_GlobalInvocationID.x * octant_count_1d * octant_count_1d +
    gl_GlobalInvocationID.y * octant_count_1d +
    gl_GlobalInvocationID.z;

  vec3 octant_min = bounds.min + gl_GlobalInvocationID * octant_dimensions;

  // Include neighbouring octants to ensure the mesh instances that overlap from them into this octant are included.
  aabb_t test_bounds = aabb_t(octant_min - octant_dimensions, octant_min + octant_dimensions * 2);
  if (frustum_test(test_bounds)  != -1)
  {
    for (uint mesh_instance_index = 0; mesh_instance_index < octants[octant_index].count; mesh_instance_index++)
    {
      mesh_instance_t mesh_instance = octants[octant_index].mesh_instances[mesh_instance_index];

      int render_program_index = get_render_program_index(mesh_instance.render_program_id);
      if (render_program_index == -1)
      {
        continue;
      }

      uint command_index = render_programs[render_program_index].active_command_start + atomicAdd(render_programs[render_program_index].active_command_count, 1);

      commands[command_index].index_count = mesh_instance.index_count;
      commands[command_index].instance_count = 1;
      commands[command_index].index_start = mesh_instance.index_start;
      commands[command_index].vertex_start = mesh_instance.vertex_start;
      commands[command_index].instance_start = mesh_instance.instance_index;
    }
  }
}
)--";

    return add(instance, ludo::compute_program(), code);
  }
}
