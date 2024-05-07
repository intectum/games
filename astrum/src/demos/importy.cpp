#include <FreeImagePlus.h>

#include <ludo/api.h>
#include <ludo/opengl/util.h>

int main()
{
  auto inst = ludo::instance();

  // SETUP

  auto minifig_counts = ludo::import_counts("assets/models/minifig.dae");

  ludo::allocate<ludo::rendering_context>(inst, 1);
  ludo::allocate<ludo::windowing_context>(inst, 1);

  ludo::allocate<ludo::animation>(inst, 1);
  ludo::allocate<ludo::armature>(inst, 1);
  ludo::allocate<ludo::body_shape>(inst, 1);
  ludo::allocate<ludo::mesh>(inst, 1);
  ludo::allocate<ludo::mesh_instance>(inst, 1);
  ludo::allocate<ludo::render_program>(inst, 1);
  ludo::allocate<ludo::script>(inst, 5);
  ludo::allocate<ludo::shader>(inst, 2);
  ludo::allocate<ludo::texture>(inst, 1);
  ludo::allocate<ludo::window>(inst, 1);

  ludo::add(inst, ludo::windowing_context());
  ludo::add(inst, ludo::window { .title = "importy!", .width = 1920, .height = 1080, .v_sync = false });

  auto rendering_context = ludo::add(inst, ludo::rendering_context(), 1);

  ludo::allocate_heap_vram(inst, "ludo::vram_draw_commands", sizeof(ludo::draw_command));
  ludo::allocate_heap_vram(inst, "ludo::vram_indices", minifig_counts.first * sizeof(uint32_t));
  ludo::allocate_heap_vram(inst, "ludo::vram_vertices", minifig_counts.second * 80); // 80 is a rough guess to cover everything

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

  auto render_program = ludo::add(inst, ludo::render_program(), ludo::format(true, true, true, true), 1);

  // MINIFIG

  auto minifig_meshes = ludo::import(inst, "assets/models/minifig.dae");

  ludo::add(inst, ludo::mesh_instance { .render_program_id = render_program->id }, minifig_meshes[0]);

  // SCRIPTS

  ludo::add<ludo::script>(inst, [](ludo::instance& inst)
  {
    auto animation = ludo::first<ludo::animation>(inst);
    auto armature = ludo::first<ludo::armature>(inst);
    auto mesh_instance = ludo::first<ludo::mesh_instance>(inst);

    ludo::instance_transform(*mesh_instance) = ludo::mat4(ludo::vec3(0.0f, 0.0f, -3.0f), ludo::mat3(ludo::vec3_unit_y, inst.total_time));

    auto bone_transforms = ludo::get_bone_transforms(*mesh_instance);
    ludo::interpolate(*animation, *armature, inst.total_time, bone_transforms.data());
    ludo::set_bone_transforms(*mesh_instance, bone_transforms);
  });

  ludo::add<ludo::script>(inst, ludo::prepare_render);
  ludo::add<ludo::script>(inst, ludo::update_windows);
  ludo::add<ludo::script, ludo::render_options>(inst, ludo::render, {});
  ludo::add<ludo::script>(inst, ludo::finalize_render);

  // PLAY

  ludo::play(inst);
}
