#include <fstream>

#include "constants.h"
#include "paths.h"
#include "physics/gravity.h"
#include "physics/point_masses.h"

namespace astrum
{
  void add_prediction_paths(ludo::instance& inst, const std::vector<ludo::vec4>& colors)
  {
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
      auto mesh = ludo::mesh();
      ludo::mesh_alloc(mesh, path_steps, path_steps, ludo::vertex_format_p.size);

      auto index_stream = ludo::stream(mesh.indices);
      for (auto step_index = uint32_t(0); step_index < path_steps; step_index++)
      {
        ludo::write(index_stream, step_index);
      }
    }
  }

  void update_prediction_paths(ludo::container& container)
  {
    // TODO a proper deep copy
    auto prediction_container = container;

    ludo::instance prediction_inst;

    auto physics_context = ludo::physics_context { .gravity = ludo::vec3_zero };

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

      for (auto& prediction_point_mass : prediction_point_masses)
      {
        ludo::cast<ludo::vec3>(meshes[path_index].vertices, byte_index) = prediction_point_mass.transform.position - relative_position;
        path_index++;
        byte_index += path_steps * ludo::vertex_format_p.size;
      }

      prediction_inst.delta_time = path_delta_time;
      prediction_inst.total_time += path_delta_time;
      ludo::frame(prediction_inst, [&]
      {
        simulate_gravity(prediction_inst, prediction_container);
        ludo::simulate(physics_context, prediction_inst.delta_time);
        simulate_point_mass_physics(prediction_inst, prediction_container, { "person", "spaceship" });
      });
    }
  }
}
