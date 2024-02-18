#include <fstream>

#include <ludo/opengl/built_ins.h>

#include "constants.h"
#include "paths.h"
#include "physics/gravity.h"
#include "physics/point_masses.h"
#include "types.h"

namespace astrum
{
  void add_prediction_paths(ludo::instance& inst, const std::vector<ludo::vec4>& colors)
  {
    auto& always_render_linear_octree = ludo::data<ludo::linear_octree>(inst, "default")[1];

    auto path_count = static_cast<uint32_t>(colors.size());

    auto mesh_buffer_options = ludo::mesh_buffer_options
    {
      .instance_count = path_count,
      .index_count = path_count * path_steps,
      .vertex_count = path_count * path_steps
    };

    auto vertex_source = std::ifstream("assets/shaders/path.vert");
    auto vertex_shader = ludo::add(inst, ludo::shader { .type = ludo::shader_type::VERTEX }, vertex_source);
    auto render_program = ludo::add(inst, ludo::render_program { .vertex_shader_id = vertex_shader->id, .fragment_shader_id = ludo::built_in_shader(inst, mesh_buffer_options, ludo::shader_type::FRAGMENT) });
    render_program->data_buffer = ludo::allocate_vram(16 + path_count * sizeof(ludo::vec4));

    auto byte_index = 0;
    write(render_program->data_buffer, byte_index, path_steps);
    byte_index += 16;
    for (auto index = 0; index < path_count; index++)
    {
      write(render_program->data_buffer, byte_index, colors[index]);
      byte_index += sizeof(ludo::vec4);
    }

    auto mesh_buffer = ludo::add(
      inst,
      ludo::mesh_buffer
      {
        .render_program_id = render_program->id,
        .primitive = ludo::mesh_primitive::LINE_STRIP
      },
      mesh_buffer_options,
      "prediction-paths"
    );

    for (auto index = 0; index < path_count; index++)
    {
      auto mesh = ludo::add(
        inst,
        ludo::mesh
          {
            .mesh_buffer_id = mesh_buffer->id,
            .index_buffer =
            {
              .data = mesh_buffer->index_buffer.data + (index * path_steps) * sizeof(uint32_t),
              .size = path_steps * sizeof(uint32_t),
            },
            .vertex_buffer =
            {
              .data = mesh_buffer->vertex_buffer.data + (index * path_steps) * mesh_buffer->format.size,
              .size = path_steps * mesh_buffer->format.size,
            }
          },
        "prediction-paths"
      );

      ludo::add(always_render_linear_octree, *mesh, ludo::vec3_zero);
    }

    for (auto index = uint32_t(0); index < path_count * path_steps; index++)
    {
      ludo::write(mesh_buffer->index_buffer, index * sizeof(uint32_t), index);
    }
  }

  void update_prediction_paths(ludo::instance& inst)
  {
    auto& dynamic_bodies = ludo::data<ludo::dynamic_body>(inst);
    auto& mesh_buffer = *ludo::first<ludo::mesh_buffer>(inst, "prediction-paths");

    auto& point_masses = ludo::data<point_mass>(inst);
    auto& solar_system = *ludo::first<astrum::solar_system>(inst);

    ludo::instance prediction_inst;

    ludo::allocate<ludo::dynamic_body>(prediction_inst, dynamic_bodies.array_size);
    ludo::allocate<ludo::kinematic_body>(prediction_inst, 0); // Required by point mass physics script
    ludo::allocate<ludo::physics_context>(prediction_inst, 1);
    ludo::allocate<ludo::script>(prediction_inst, 1);
    ludo::allocate<ludo::static_body>(prediction_inst, 0); // Required by point mass physics script

    ludo::allocate<point_mass>(prediction_inst, point_masses.array_size);
    ludo::allocate<astrum::solar_system>(prediction_inst, 1);

    ludo::add(prediction_inst, ludo::physics_context { .gravity = ludo::vec3_zero });
    ludo::add(prediction_inst, solar_system);

    for (auto& dynamic_body : dynamic_bodies)
    {
      ludo::add(prediction_inst, dynamic_body);
    }

    for (auto& point_mass : point_masses)
    {
      ludo::add(prediction_inst, point_mass);
    }

    ludo::add<ludo::script>(prediction_inst, [](ludo::instance& prediction_inst)
    {
      simulate_gravity(prediction_inst);
      ludo::simulate_physics(prediction_inst, path_delta_time, game_speed);
      simulate_point_mass_physics(prediction_inst, {});
    });

    auto& prediction_dynamic_bodies = ludo::data<ludo::dynamic_body>(prediction_inst);
    auto& prediction_point_masses = ludo::data<point_mass>(prediction_inst);
    auto prediction_central_start_position = ludo::vec3_zero;
    if (path_central_index != -1)
    {
      prediction_central_start_position = prediction_point_masses[path_central_index].transform.position;
    }

    for (auto step = uint32_t(0); step < path_steps; step++)
    {
      auto byte_index = step * mesh_buffer.format.size;

      auto relative_position = ludo::vec3_zero;
      if (path_central_index != -1)
      {
        relative_position = prediction_point_masses[path_central_index].transform.position - prediction_central_start_position;
      }

      for (auto& prediction_body : prediction_dynamic_bodies)
      {
        ludo::write(mesh_buffer.vertex_buffer, byte_index, prediction_body.transform.position - relative_position);
        byte_index += path_steps * mesh_buffer.format.size;
      }

      for (auto& prediction_point_mass : prediction_point_masses)
      {
        ludo::write(mesh_buffer.vertex_buffer, byte_index, prediction_point_mass.transform.position - relative_position);
        byte_index += path_steps * mesh_buffer.format.size;
      }

      prediction_inst.delta_time = path_delta_time;
      prediction_inst.total_time += path_delta_time;
      ludo::frame(prediction_inst);
    }

    ludo::deallocate<ludo::dynamic_body>(prediction_inst);
    ludo::deallocate<ludo::kinematic_body>(prediction_inst); // Required by point mass physics script
    ludo::deallocate<ludo::physics_context>(prediction_inst);
    ludo::deallocate<ludo::script>(prediction_inst);
    ludo::deallocate<ludo::static_body>(prediction_inst); // Required by point mass physics script

    ludo::deallocate<point_mass>(prediction_inst);
    ludo::deallocate<astrum::solar_system>(prediction_inst);
  }
}
