#include <ludo/api.h>
#include <ludo/opengl/textures.h>
#include <ludo/opengl/util.h>

int main()
{
  // SETUP

  auto inst = ludo::instance();

  auto window = ludo::window { .title = "importy!", .width = 1920, .height = 1080, .v_sync = false };
  ludo::init(window);
  ludo::init_rendering();

  // CAMERA

  auto camera_arena = ludo::allocate_arena_vram(256 /* TODO */);
  ludo::write_camera_aligned(ludo::allocate(camera_arena, 256 /* TODO */), ludo::camera
  {
    .view = ludo::mat4_identity,
    .projection = ludo::perspective(60.0f, 16.0f / 9.0f, 0.1f, 1000.0f)
  });

  // LIGHTS

  auto light_arena = ludo::allocate_arena_vram(256 /* TODO */);
  ludo::write_light_aligned(ludo::allocate(light_arena, 256 /* TODO */), ludo::light
  {
    .ambient = { 0.01f, 0.01f, 0.01f, 1.0f },
    .diffuse = { 0.7f, 0.7f, 0.7f, 1.0f },
    .specular = { 0.01f, 0.01f, 0.01f, 1.0f },
    .position = { 0.0f, 4.0f, 0.0f },
    .attenuation = { 1.0f, 0.0f, 0.0f },
    .strength = 1.0f,
    .range = 10.0f
  });

  // RENDER PROGRAMS

  auto render_program = ludo::render_program();
  ludo::init(render_program, ludo::format(true, true, true, true));

  // MINIFIG

  auto minifig = ludo::import(ludo::asset_folder + "/models/minifig.dae");

  // TRANSFORMS

  auto transform_arena = ludo::allocate_arena_vram(sizeof(ludo::mat4));
  auto transform = (ludo::mat4*) ludo::allocate(transform_arena, sizeof(ludo::mat4));

  // TEXTURES

  auto texture_arena = ludo::allocate_arena_vram(sizeof(uint64_t));
  *((uint64_t*) ludo::allocate(texture_arena, sizeof(uint64_t))) = ludo::handle(minifig.textures[0]);

  // BONE TRANSFORMS

  auto bone_transform_arena = ludo::allocate_arena_vram(ludo::max_bones_per_armature * sizeof(ludo::mat4));
  auto bone_transforms = (ludo::mat4*) ludo::allocate(bone_transform_arena, ludo::max_bones_per_armature * sizeof(ludo::mat4));
  for (auto index = 0; index < ludo::max_bones_per_armature; index++)
  {
    bone_transforms[index] = ludo::mat4_identity;
  }

  // RENDER COMMANDS

  auto render_command_arena = ludo::allocate_arena_vram(sizeof(ludo::render_command));
  *((ludo::render_command*) ludo::allocate(render_command_arena, sizeof(ludo::render_command))) = ludo::render_command
  {
    .index_count = static_cast<uint32_t>((minifig.meshes[0].indices.end - minifig.meshes[0].indices.start) / sizeof(uint32_t))
  };

  // PLAY

  ludo::fence fence;

  ludo::play(inst, [&]
  {
    ludo::receive_input(window);

    if (window.active_window_frame_button_states[ludo::window_frame_button::CLOSE] == ludo::button_state::UP)
    {
      ludo::stop(inst);
    }

    *transform = ludo::mat4(ludo::vec3(0.0f, 0.0f, -3.0f), ludo::mat3(ludo::vec3_unit_y, inst.total_time));
    ludo::interpolate(minifig.animations[0], minifig.armatures[0], inst.total_time, bone_transforms);

    if (fence.id) ludo::wait(fence);
    ludo::swap_buffers(window);
    ludo::use_and_clear({ .width = window.width, .height = window.height });

    ludo::bind_indices(minifig.meshes[0].indices.id);
    ludo::bind_vertices(minifig.meshes[0].vertices.id);
    ludo::bind_data(0, camera_arena.id);
    ludo::bind_data(1, light_arena.id);
    ludo::bind_data(2, transform_arena.id);
    ludo::bind_data(3, texture_arena.id);
    ludo::bind_data(4, bone_transform_arena.id);
    ludo::bind_commands(render_command_arena.id);

    ludo::render(render_program, 0, 1);

    ludo::init(fence);
  });
}
