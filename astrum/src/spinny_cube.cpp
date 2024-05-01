#include <FreeImagePlus.h>

#include <ludo/api.h>
#include <ludo/opengl/util.h>

int main()
{
  auto inst = ludo::instance();

  // SETUP

  auto instance_count = 4;
  auto box_counts = ludo::box_counts();
  auto minifig_counts = ludo::import_counts("assets/models/minifig.dae");

  ludo::allocate<ludo::rendering_context>(inst, 1);
  ludo::allocate<ludo::windowing_context>(inst, 1);

  ludo::allocate<ludo::animation>(inst, 1);
  ludo::allocate<ludo::armature>(inst, 1);
  ludo::allocate<ludo::body_shape>(inst, 1);
  ludo::allocate<ludo::mesh>(inst, 4);
  ludo::allocate<ludo::mesh_instance>(inst, instance_count);
  ludo::allocate<ludo::render_program>(inst, instance_count);
  ludo::allocate<ludo::script>(inst, 5);
  ludo::allocate<ludo::shader>(inst, instance_count * 2);
  ludo::allocate<ludo::texture>(inst, instance_count);
  ludo::allocate<ludo::window>(inst, 1);

  ludo::add(inst, ludo::windowing_context());
  ludo::add(inst, ludo::window { .title = "spinny cube!", .width = 1920, .height = 1080, .v_sync = false });

  auto rendering_context = ludo::add(inst, ludo::rendering_context(), 1);

  ludo::allocate_heap_vram<ludo::draw_command>(inst, instance_count * sizeof(ludo::draw_command));
  ludo::allocate_heap_vram<ludo::index_t>(inst, box_counts.first * instance_count + minifig_counts.first);
  ludo::allocate_heap_vram<ludo::vertex_t>(inst, box_counts.second * instance_count * ludo::vertex_format_pnc.size + minifig_counts.second * 80);

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

  auto render_program_p = ludo::add(inst, ludo::render_program(), ludo::format(), 1);
  auto render_program_pc = ludo::add(inst, ludo::render_program(), ludo::format(false, true), 1);
  auto render_program_pt = ludo::add(inst, ludo::render_program(), ludo::format(false, false, true), 1);

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

  ludo::add(inst, ludo::mesh_instance(), *cuby);

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

  ludo::add(inst, ludo::mesh_instance(), *ruby_cuby);

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

  ludo::add(inst, ludo::mesh_instance(), *tuby_cuby);

  // MINIFIG

  auto minifig_meshes = ludo::import(inst, "assets/models/minifig.dae");

  ludo::add(inst, ludo::mesh_instance(), minifig_meshes[0]);

  // SCRIPTS

  ludo::add<ludo::script>(inst, [](ludo::instance& inst)
  {
    auto animation = ludo::first<ludo::animation>(inst);
    auto armature = ludo::first<ludo::armature>(inst);
    auto& mesh_instances = ludo::data<ludo::mesh_instance>(inst);

    ludo::set_transform(mesh_instances[0], ludo::mat4(ludo::vec3(-2.5f, 0.0f, -4.0f), ludo::mat3(ludo::vec3_unit_y, inst.total_time)));
    ludo::set_transform(mesh_instances[1], ludo::mat4(ludo::vec3(0.0f, 0.0f, -4.0f), ludo::mat3(ludo::vec3_unit_y, inst.total_time)));
    ludo::set_transform(mesh_instances[2], ludo::mat4(ludo::vec3(2.5f, 0.0f, -4.0f), ludo::mat3(ludo::vec3_unit_y, inst.total_time)));
    ludo::set_transform(mesh_instances[3], ludo::mat4(ludo::vec3(0.0f, 0.0f, -3.0f), ludo::mat3(ludo::vec3_unit_y, inst.total_time)));

    auto bone_transforms = ludo::get_bone_transforms(mesh_instances[3]);
    ludo::interpolate(*animation, *armature, inst.total_time, bone_transforms.data());
    ludo::set_bone_transforms(mesh_instances[3], bone_transforms);
  });

  ludo::add<ludo::script>(inst, ludo::update_windows);

  // TODO Maybe find a better way to do this?
  ludo::add<ludo::script>(inst, [](ludo::instance& inst)
  {
    auto& render_programs = data<ludo::render_program>(inst);
    for (auto& render_program : render_programs)
    {
      render_program.active_commands.start = 0;
    }
  });

  ludo::add<ludo::script, ludo::render_options>(inst, ludo::render, {});
  ludo::add<ludo::script>(inst, ludo::wait_for_render);

  // PLAY

  ludo::play(inst);

  // TEARDOWN

  ludo::deallocate_heap_vram<ludo::draw_command>(inst);
  ludo::deallocate_heap_vram<ludo::index_t>(inst);
  ludo::deallocate_heap_vram<ludo::vertex_t>(inst);

  ludo::deallocate<ludo::animation>(inst);
  ludo::deallocate<ludo::armature>(inst);
  ludo::deallocate<ludo::body_shape>(inst);
  ludo::deallocate<ludo::mesh>(inst);
  ludo::deallocate<ludo::mesh_instance>(inst);
  ludo::deallocate<ludo::render_program>(inst);
  ludo::deallocate<ludo::script>(inst);
  ludo::deallocate<ludo::shader>(inst);
  ludo::deallocate<ludo::texture>(inst);
  ludo::deallocate<ludo::window>(inst);

  ludo::deallocate<ludo::rendering_context>(inst);
  ludo::deallocate<ludo::windowing_context>(inst);
}
