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
  ludo::allocate<ludo::script>(inst, 6);
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
  ludo::add<ludo::script>(inst, [&](ludo::instance& inst)
  {
    auto& mesh_instances = ludo::data<ludo::mesh_instance>(inst);

    ludo::add_draw_command(*render_program_p, mesh_instances[0]);
    ludo::add_draw_command(*render_program_pc, mesh_instances[1]);
    ludo::add_draw_command(*render_program_pt, mesh_instances[2]);
  });
  ludo::add<ludo::script, ludo::render_options>(inst, ludo::render, {});
  ludo::add<ludo::script>(inst, ludo::finalize_render);

  // PLAY

  ludo::play(inst);
}
