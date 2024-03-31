
#include <FreeImagePlus.h>

#include <ludo/api.h>
#include <ludo/assimp/api.h>
#include <ludo/opengl/util.h>

int main()
{
  auto inst = ludo::instance();

  // SETUP

  auto max_instance_count = 10;
  auto box_counts = ludo::box_counts();
  auto minifig_counts = ludo::import_counts("assets/models/minifig.dae");

  ludo::allocate<ludo::rendering_context>(inst, 1);
  ludo::allocate<ludo::windowing_context>(inst, 1);

  ludo::allocate<ludo::animation>(inst, max_instance_count);
  ludo::allocate<ludo::armature>(inst, max_instance_count);
  ludo::allocate<ludo::armature_instance>(inst, max_instance_count);
  ludo::allocate<ludo::mesh>(inst, max_instance_count);
  ludo::allocate<ludo::render_program>(inst, max_instance_count);
  ludo::allocate<ludo::script>(inst, 5);
  ludo::allocate<ludo::shader>(inst, max_instance_count * 2);
  ludo::allocate<ludo::texture>(inst, max_instance_count);
  ludo::allocate<ludo::window>(inst, 1);

  ludo::allocate<std::vector<ludo::vec3>>(inst, 1); // TODO WTF??? This is for importing rigid bodies?

  ludo::add(inst, ludo::windowing_context());
  ludo::add(inst, ludo::window { .title = "spinny cube!", .width = 1920, .height = 1080, .v_sync = false });

  auto rendering_context = ludo::add(inst, ludo::rendering_context(), 1);

  ludo::allocate_vram<ludo::draw_command>(inst, max_instance_count);
  ludo::allocate_vram<ludo::instance_t>(inst, max_instance_count * (sizeof(ludo::mat4) + sizeof(uint64_t) + ludo::max_bones_per_armature * sizeof(ludo::mat4)));
  ludo::allocate_heap_vram<ludo::index_t>(inst, box_counts.first * max_instance_count + minifig_counts.first);
  ludo::allocate_heap_vram<ludo::vertex_t>(inst, box_counts.second * max_instance_count * ludo::vertex_format_pnc.size + minifig_counts.second * 80);

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

  auto render_program_p = ludo::add(inst, ludo::render_program(), {});
  auto render_program_pc = ludo::add(inst, ludo::render_program(), { .colors = true });
  auto render_program_pt = ludo::add(inst, ludo::render_program(), { .texture = true });

  // TEXTURE

  auto image = fipImage();
  image.load("assets/effects/atmosphere.tiff");

  auto texture = ludo::add(inst, ludo::texture { .datatype = ludo::pixel_datatype::FLOAT32, .width = image.getWidth(), .height = image.getHeight() });
  ludo::write(*texture, reinterpret_cast<std::byte*>(image.accessPixels()));

  // CUBY

  auto cuby = ludo::add(
    inst,
    ludo::mesh { .render_program_id = render_program_p->id },
    box_counts.first,
    box_counts.second,
    render_program_p->format.size
  );

  auto index_index = uint32_t(0);
  auto vertex_index = uint32_t(0);
  ludo::box(*cuby, render_program_p->format, index_index, vertex_index);

  // RUBY

  auto ruby_cuby = ludo::add(
    inst,
    ludo::mesh { .render_program_id = render_program_pc->id },
    box_counts.first,
    box_counts.second,
    render_program_pc->format.size
  );

  index_index = uint32_t(0);
  vertex_index = uint32_t(0);
  ludo::box(*ruby_cuby, render_program_pc->format, index_index, vertex_index);
  ludo::colorize(*ruby_cuby, render_program_pc->format, 0, box_counts.second, ludo::vec4(1.0f, 0.0f, 0.0f, 1.0f));

  // TUBY

  auto tuby_cuby = ludo::add(
    inst,
    ludo::mesh { .render_program_id = render_program_pt->id, .texture_id = texture->id },
    box_counts.first,
    box_counts.second,
    render_program_pt->format.size
  );

  index_index = uint32_t(0);
  vertex_index = uint32_t(0);
  ludo::box(*tuby_cuby, render_program_pt->format, index_index, vertex_index);

  // MINIFIG

  ludo::import(inst, "assets/models/minifig.dae");

  // SCRIPTS

  ludo::add<ludo::script>(inst, [](ludo::instance& inst)
  {
    auto animation = ludo::first<ludo::animation>(inst);
    auto armature = ludo::first<ludo::armature>(inst);
    auto armature_instance = ludo::first<ludo::armature_instance>(inst);
    auto& meshes = ludo::data<ludo::mesh>(inst);

    meshes[0].transform = ludo::mat4(ludo::vec3(-2.5f, 0.0f, -4.0f), ludo::mat3(ludo::vec3_unit_y, inst.total_time));
    meshes[1].transform = ludo::mat4(ludo::vec3(0.0f, 0.0f, -4.0f), ludo::mat3(ludo::vec3_unit_y, inst.total_time));
    meshes[2].transform = ludo::mat4(ludo::vec3(2.5f, 0.0f, -4.0f), ludo::mat3(ludo::vec3_unit_y, inst.total_time));
    meshes[3].transform = ludo::mat4(ludo::vec3(0.0f, 0.0f, -3.0f), ludo::mat3(ludo::vec3_unit_y, inst.total_time));

    ludo::interpolate(*animation, *armature, inst.total_time, armature_instance->transforms);
  });

  ludo::add<ludo::script>(inst, ludo::update_windows);

  // TODO Maybe find a better way to do this?
  ludo::add<ludo::script>(inst, [](ludo::instance& inst)
  {
    auto& vram_draw_commands = data<ludo::draw_command>(inst);
    auto& vram_instances = data<ludo::instance_t>(inst);

    ludo::clear(vram_draw_commands);
    ludo::clear(vram_instances);
  });

  ludo::add<ludo::script, ludo::render_options>(inst, ludo::render, {});
  ludo::add<ludo::script>(inst, ludo::wait_for_render);

  // PLAY

  ludo::play(inst);

  // TEARDOWN

  ludo::deallocate_vram<ludo::draw_command>(inst);
  ludo::deallocate_vram<ludo::instance_t>(inst);
  ludo::deallocate_heap_vram<ludo::index_t>(inst);
  ludo::deallocate_heap_vram<ludo::vertex_t>(inst);

  ludo::deallocate<ludo::animation>(inst);
  ludo::deallocate<ludo::armature>(inst);
  ludo::deallocate<ludo::armature_instance>(inst);
  ludo::deallocate<ludo::mesh>(inst);
  ludo::deallocate<ludo::render_program>(inst);
  ludo::deallocate<ludo::script>(inst);
  ludo::deallocate<ludo::shader>(inst);
  ludo::deallocate<ludo::texture>(inst);
  ludo::deallocate<ludo::window>(inst);

  ludo::deallocate<ludo::rendering_context>(inst);
  ludo::deallocate<ludo::windowing_context>(inst);
}
