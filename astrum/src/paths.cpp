#include <fstream>

#include "constants.h"
#include "paths.h"
#include "physics/gravity.h"
#include "physics/point_masses.h"
#include "types.h"

namespace astrum
{
  void add_prediction_paths(ludo::instance& inst, const std::vector<ludo::vec4>& colors)
  {
    auto& always_render_grid = ludo::data<ludo::grid3>(inst, "default")[1];

    auto& indices = ludo::data_heap(inst, "ludo::vram_indices");
    auto& vertices = ludo::data_heap(inst, "ludo::vram_vertices");

    auto path_count = static_cast<uint32_t>(colors.size());

    auto vertex_shader_code = std::ifstream(ludo::asset_folder + "/shaders/path.vert");
    auto fragment_shader_code = ludo::default_fragment_shader_code(ludo::vertex_format_p);
    auto render_program = ludo::add(
      inst,
      ludo::render_program
      {
        .primitive = ludo::mesh_primitive::LINE_STRIP,
        .format = ludo::vertex_format_p,
        .shader_buffer = ludo::allocate_dual(16 + path_count * sizeof(ludo::vec4))
      }
    );
    ludo::init(*render_program, vertex_shader_code, fragment_shader_code);

    auto render_program_stream = ludo::stream(render_program->shader_buffer.back);
    ludo::write(render_program_stream, path_steps);
    render_program_stream.position += 12; // align 16
    for (auto index = 0; index < path_count; index++)
    {
      ludo::write(render_program_stream, colors[index]);
    }

    for (auto path_index = 0; path_index < path_count; path_index++)
    {
      auto mesh = ludo::add(inst, ludo::mesh(), "prediction-paths");
      ludo::init(*mesh, indices, vertices, path_steps, path_steps, ludo::vertex_format_p.size);

      auto index_stream = ludo::stream(mesh->index_buffer);
      for (auto step_index = uint32_t(0); step_index < path_steps; step_index++)
      {
        ludo::write(index_stream, step_index);
      }

      auto render_mesh = ludo::add(inst, ludo::render_mesh(), "prediction-paths");
      ludo::init(*render_mesh, *render_program, *mesh, indices, vertices, 1);

      // TODO this grid is not a thing anymore...
      ludo::add(always_render_grid, *render_mesh, ludo::vec3_zero);
    }
  }

  void update_prediction_paths(ludo::instance& inst)
  {
    auto& dynamic_bodies = ludo::data<ludo::dynamic_body>(inst);
    auto& meshes = ludo::data<ludo::mesh>(inst, "prediction-paths");

    auto& point_masses = ludo::data<point_mass>(inst);
    auto& solar_system = *ludo::first<astrum::solar_system>(inst);

    ludo::instance prediction_inst;

    ludo::allocate<ludo::dynamic_body>(prediction_inst, dynamic_bodies.length);
    ludo::allocate<ludo::kinematic_body>(prediction_inst, 0); // Required by point mass physics script
    ludo::allocate<ludo::physics_context>(prediction_inst, 1);
    ludo::allocate<ludo::static_body>(prediction_inst, 0); // Required by point mass physics script

    ludo::allocate<point_mass>(prediction_inst, point_masses.length);
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

    auto& prediction_dynamic_bodies = ludo::data<ludo::dynamic_body>(prediction_inst);
    auto& prediction_point_masses = ludo::data<point_mass>(prediction_inst);
    auto prediction_central_start_position = ludo::vec3_zero;
    if (path_central_index != -1)
    {
      prediction_central_start_position = prediction_point_masses[path_central_index].transform.position;
    }

    for (auto step = uint32_t(0); step < path_steps; step++)
    {
      auto path_index = uint32_t(0);
      auto byte_index = step * ludo::vertex_format_p.size;

      auto relative_position = ludo::vec3_zero;
      if (path_central_index != -1)
      {
        relative_position = prediction_point_masses[path_central_index].transform.position - prediction_central_start_position;
      }

      for (auto& prediction_body : prediction_dynamic_bodies)
      {
        ludo::cast<ludo::vec3>(meshes[path_index].vertex_buffer, byte_index) = prediction_body.transform.position - relative_position;
        path_index++;
        byte_index += path_steps * ludo::vertex_format_p.size;
      }

      for (auto& prediction_point_mass : prediction_point_masses)
      {
        ludo::cast<ludo::vec3>(meshes[path_index].vertex_buffer, byte_index) = prediction_point_mass.transform.position - relative_position;
        path_index++;
        byte_index += path_steps * ludo::vertex_format_p.size;
      }

      prediction_inst.delta_time = path_delta_time;
      prediction_inst.total_time += path_delta_time;
      ludo::frame(prediction_inst, [](ludo::instance& prediction_inst)
      {
        auto physics_context = ludo::first<ludo::physics_context>(prediction_inst);

        simulate_gravity(prediction_inst);
        ludo::simulate(*physics_context, prediction_inst.delta_time);
        simulate_point_mass_physics(prediction_inst, {});
      });
    }

    ludo::deallocate<ludo::dynamic_body>(prediction_inst);
    ludo::deallocate<ludo::kinematic_body>(prediction_inst); // Required by point mass physics script
    ludo::deallocate<ludo::physics_context>(prediction_inst);
    ludo::deallocate<ludo::static_body>(prediction_inst); // Required by point mass physics script

    ludo::deallocate<point_mass>(prediction_inst);
    ludo::deallocate<astrum::solar_system>(prediction_inst);
  }
}
