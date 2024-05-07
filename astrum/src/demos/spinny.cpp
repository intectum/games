#include <FreeImagePlus.h>

#include <ludo/api.h>
#include <ludo/opengl/util.h>

int main()
{
  auto inst = ludo::instance();

  // SETUP

  auto instance_count = 3;
  auto box_counts = ludo::box_counts();

  auto max_indices = box_counts.first * 3;
  auto max_vertices = box_counts.second * 3;

  ludo::allocate<ludo::rendering_context>(inst, 1);
  ludo::allocate<ludo::windowing_context>(inst, 1);

  ludo::allocate<ludo::mesh>(inst, instance_count);
  ludo::allocate<ludo::mesh_instance>(inst, instance_count);
  ludo::allocate<ludo::render_program>(inst, instance_count);
  ludo::allocate<ludo::script>(inst, 5);
  ludo::allocate<ludo::shader>(inst, instance_count * 2);
  ludo::allocate<ludo::texture>(inst, 1);
  ludo::allocate<ludo::window>(inst, 1);

  ludo::add(inst, ludo::windowing_context());
  ludo::add(inst, ludo::window { .title = "spinny!", .width = 1920, .height = 1080, .v_sync = false });

  auto rendering_context = ludo::add(inst, ludo::rendering_context(), 1);

  ludo::allocate_heap_vram(inst, "ludo::vram_draw_commands", instance_count * sizeof(ludo::draw_command));
  ludo::allocate_heap_vram(inst, "ludo::vram_indices", max_indices * sizeof(uint32_t));
  ludo::allocate_heap_vram(inst, "ludo::vram_vertices", max_vertices * ludo::vertex_format_pc.size);

  // LIGHTS

  ludo::set_light(*rendering_context, ludo::light
  {
    .ambient = { 0.01f, 0.01f, 0.01f, 1.0f },
    .diffuse = { 0.7f, 0.7f, 0.7f, 1.0f },
    .specular = { 0.01f, 0.01f, 0.01f, 1.0f },
    .position = ludo::vec3(0.0f, 4.0f, 0.0f),
    .attenuation = { 1.0f, 0.0f, 0.0f },
    .strength = 1.0f,
    .range = 10.0f
  }, 0);

  // RENDER PROGRAMS

  auto render_program_p = ludo::add(inst, ludo::render_program(), ludo::vertex_format_p, 1);
  auto render_program_pc = ludo::add(inst, ludo::render_program(), ludo::vertex_format_pc, 1);
  auto render_program_pt = ludo::add(inst, ludo::render_program(), ludo::vertex_format_pt, 1);

  // TEXTURE

  auto image = fipImage();
  image.load("assets/effects/atmosphere.tiff");

  auto texture = ludo::add(inst, ludo::texture { .datatype = ludo::pixel_datatype::FLOAT32, .width = image.getWidth(), .height = image.getHeight() });
  ludo::write(*texture, reinterpret_cast<std::byte*>(image.accessPixels()));

  // CUBY

  auto cuby = ludo::add(
    inst,
    ludo::mesh(),
    box_counts.first,
    box_counts.second,
    render_program_p->format.size
  );

  auto index_index = uint32_t(0);
  auto vertex_index = uint32_t(0);
  ludo::box(*cuby, render_program_p->format, index_index, vertex_index);

  ludo::add(inst, ludo::mesh_instance { .render_program_id = render_program_p->id }, *cuby);

  // RUBY

  auto ruby_cuby = ludo::add(
    inst,
    ludo::mesh(),
    box_counts.first,
    box_counts.second,
    render_program_pc->format.size
  );

  index_index = uint32_t(0);
  vertex_index = uint32_t(0);
  ludo::box(*ruby_cuby, render_program_pc->format, index_index, vertex_index);
  ludo::colorize(*ruby_cuby, render_program_pc->format, 0, box_counts.second, ludo::vec4(1.0f, 0.0f, 0.0f, 1.0f));

  ludo::add(inst, ludo::mesh_instance { .render_program_id = render_program_pc->id }, *ruby_cuby);

  // TUBY

  auto tuby_cuby = ludo::add(
    inst,
    ludo::mesh { .texture_id = texture->id },
    box_counts.first,
    box_counts.second,
    render_program_pt->format.size
  );

  index_index = uint32_t(0);
  vertex_index = uint32_t(0);
  ludo::box(*tuby_cuby, render_program_pt->format, index_index, vertex_index);

  ludo::add(inst, ludo::mesh_instance { .render_program_id = render_program_pt->id }, *tuby_cuby);

/*

  auto low_level_position_offset = ludo::offset(render_program_sphere->format, 'f');
  auto low_level_normal_offset = low_level_position_offset + sizeof(ludo::vec3);
  auto low_level_color_offset = low_level_normal_offset + sizeof(ludo::vec3);
  auto has_normals = ludo::count(render_program_sphere->format, 'n') > 0;
  auto has_colors = ludo::count(render_program_sphere->format, 'c') > 0;

  for (auto triangle_index = uint32_t(0); triangle_index < sphere_ico_counts.first / 3; triangle_index++)
  {
    auto triangles_per_low_detail = static_cast<uint32_t>(std::pow(4, 2));
    auto low_detail_triangle_index = triangle_index / triangles_per_low_detail;
    auto low_detail_base_vertex_index = low_detail_triangle_index * triangles_per_low_detail * 3;

    auto low_detail_triangle_indices = std::array<uint32_t, 3>
    {
      low_detail_base_vertex_index,
      low_detail_base_vertex_index + triangles_per_low_detail,
      low_detail_base_vertex_index + triangles_per_low_detail * 2
    };

    auto low_detail_positions = std::array<ludo::vec3, 3>
    {
      ludo::read<ludo::vec3>(sphere->vertex_buffer, low_detail_triangle_indices[0] * render_program_sphere->format.size),
      ludo::read<ludo::vec3>(sphere->vertex_buffer, low_detail_triangle_indices[1] * render_program_sphere->format.size),
      ludo::read<ludo::vec3>(sphere->vertex_buffer, low_detail_triangle_indices[2] * render_program_sphere->format.size)
    };

    auto low_detail_normal = ludo::cross(low_detail_positions[1] - low_detail_positions[0], low_detail_positions[2] - low_detail_positions[0]);
    ludo::normalize(low_detail_normal);

    for (auto triangle_vertex_index = 0; triangle_vertex_index < 3; triangle_vertex_index++)
    {
      auto position = ludo::read<ludo::vec3>(sphere->vertex_buffer, (triangle_index * 3 + triangle_vertex_index) * render_program_sphere->format.size);

      */
/*position = ludo::project_point_onto_plane(position, low_detail_positions[triangle_vertex_index], low_detail_normal);

      auto distance_0 = ludo::distance_point_to_line_segment(position, { low_detail_positions[0], low_detail_positions[1] });
      auto distance_1 = ludo::distance_point_to_line_segment(position, { low_detail_positions[1], low_detail_positions[2] });
      auto distance_2 = ludo::distance_point_to_line_segment(position, { low_detail_positions[2], low_detail_positions[0] });
      if (distance_0 < distance_1 && distance_0 < distance_2)
      {
        auto target = low_detail_positions[1] - low_detail_positions[0];
        position = ludo::project_point_onto_line(position, low_detail_positions[0], target);
      }
      else if (distance_1 < distance_2)
      {
        auto target = low_detail_positions[2] - low_detail_positions[1];
        position = ludo::project_point_onto_line(position, low_detail_positions[1], target);
      }
      else
      {
        auto target = low_detail_positions[0] - low_detail_positions[2];
        position = ludo::project_point_onto_line(position, low_detail_positions[2], target);
      }*//*


      auto distance_0 = ludo::length(low_detail_positions[0] - position);
      auto distance_1 = ludo::length(low_detail_positions[1] - position);
      auto distance_2 = ludo::length(low_detail_positions[2] - position);
      if (distance_0 < distance_1 && distance_0 < distance_2)
      {
        position = low_detail_positions[0];
      }
      else if (distance_1 < distance_2)
      {
        position = low_detail_positions[1];
      }
      else
      {
        position = low_detail_positions[2];
      }

      ludo::write(sphere->vertex_buffer, (triangle_index * 3 + triangle_vertex_index) * render_program_sphere->format.size + low_level_position_offset, position);

      if (has_normals)
      {
        ludo::write(sphere->vertex_buffer, (triangle_index * 3 + triangle_vertex_index) * render_program_sphere->format.size + low_level_normal_offset, low_detail_normal);
      }

      if (has_colors)
      {
        ludo::write(sphere->vertex_buffer, (triangle_index * 3 + triangle_vertex_index) * render_program_sphere->format.size + low_level_color_offset, ludo::vec4 {triangle_index % 3 == 0 ? 1.0f : 0.0f, triangle_index % 3 == 1 ? 1.0f : 0.0f, triangle_index % 3 == 2 ? 1.0f : 0.0f, 1.0f });
      }
    }
  }
*/

  // SCRIPTS

  ludo::add<ludo::script>(inst, [](ludo::instance& inst)
  {
    auto& mesh_instances = ludo::data<ludo::mesh_instance>(inst);

    ludo::instance_transform(mesh_instances[0]) = ludo::mat4(ludo::vec3(-2.5f, 0.0f, -4.0f), ludo::mat3(ludo::vec3_unit_y, inst.total_time));
    ludo::instance_transform(mesh_instances[1]) = ludo::mat4(ludo::vec3(0.0f, 0.0f, -4.0f), ludo::mat3(ludo::vec3_unit_y, inst.total_time));
    ludo::instance_transform(mesh_instances[2]) = ludo::mat4(ludo::vec3(2.5f, 0.0f, -4.0f), ludo::mat3(ludo::vec3_unit_y, inst.total_time));
  });

  ludo::add<ludo::script>(inst, ludo::prepare_render);
  ludo::add<ludo::script>(inst, ludo::update_windows);
  ludo::add<ludo::script, ludo::render_options>(inst, ludo::render, {});
  ludo::add<ludo::script>(inst, ludo::finalize_render);

  // PLAY

  ludo::play(inst);
}
