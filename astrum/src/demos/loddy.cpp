#include <cmath>
#include <iomanip>
#include <iostream>

#include <ludo/api.h>
#include <ludo/opengl/util.h>

#include "../constants.h"
#include "../meshes/lod_shaders.h"

int main()
{
  // SETUP

  auto timer = ludo::timer();

  auto inst = ludo::instance();
  ludo::thread_pool_start();

  auto window = ludo::window { .title = "loddy!", .width = 1920, .height = 1080, .v_sync = false };
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
    .range = 100.0f
  });

  // RENDER PROGRAMS

  auto lod_format = ludo::vertex_format_pnc;
  lod_format.components.insert(lod_format.components.end(), ludo::vertex_format_pnc.components.begin(), ludo::vertex_format_pnc.components.end());
  lod_format.size *= 2;

  auto render_program = ludo::render_program { .format = lod_format };
  auto vertex_shader_code = astrum::lod_vertex_shader_code(ludo::vertex_format_pnc, false);
  auto fragment_shader_code = astrum::lod_fragment_shader_code(ludo::vertex_format_pnc, false);
  ludo::init(render_program, vertex_shader_code, fragment_shader_code);

  // TREE

  auto tree_import = ludo::import("assets/models/oak-tree.dae", { .merge_meshes = true });
  auto lod_meshes = astrum::build_lod_meshes(tree_import.meshes[0], ludo::vertex_format_pnc, { 200, 50 });

  // TRANSFORMS

  auto transform_arena = ludo::allocate_arena_vram(2 * sizeof(ludo::mat4));
  auto transforms = (ludo::mat4*) ludo::allocate(transform_arena, sizeof(ludo::mat4));

  // RENDER COMMANDS

  auto render_command_arena = ludo::allocate_arena_vram(2 * sizeof(ludo::render_command));

  *((ludo::render_command*) ludo::allocate(render_command_arena, sizeof(ludo::render_command))) = ludo::render_command
  {
    .index_count = static_cast<uint32_t>((lod_meshes[0].indices.end - lod_meshes[0].indices.start) / sizeof(uint32_t))
  };

  *((ludo::render_command*) ludo::allocate(render_command_arena, sizeof(ludo::render_command))) = ludo::render_command
  {
    .index_count = static_cast<uint32_t>((lod_meshes[1].indices.end - lod_meshes[1].indices.start) / sizeof(uint32_t)),
    .instance_start = 1
  };

  // LODS

  auto lod_arena = ludo::allocate_arena_vram(2 * 2 * sizeof(float));

  // LOD INDICES

  auto lod_index_arena = ludo::allocate_arena_vram(2 * sizeof(uint32_t));
  *((uint32_t*) ludo::allocate(lod_index_arena, sizeof(uint32_t))) = 0;
  *((uint32_t*) ludo::allocate(lod_index_arena, sizeof(uint32_t))) = 1;

  std::cout << std::fixed << std::setprecision(4) << "load time (seconds): " << ludo::elapsed(timer) << std::endl;

  // PLAY

  ludo::fence fence;
  auto mouse_movement_accumulator = new std::array<int32_t, 2>();

  ludo::play(inst, [&]
  {
    ludo::receive_input(window);

    if (window.active_window_frame_button_states[ludo::window_frame_button::CLOSE] == ludo::button_state::UP)
    {
      ludo::stop(inst);
    }

    if (window.active_mouse_button_states[ludo::mouse_button::LEFT] == ludo::button_state::HOLD)
    {
      (*mouse_movement_accumulator)[0] += window.mouse_movement[0];
      (*mouse_movement_accumulator)[1] += window.mouse_movement[1];
    }

    auto rotation = ludo::quat(float((*mouse_movement_accumulator)[1]) / 250.0f, float((*mouse_movement_accumulator)[0]) / 250.0f, 0.0f);
    transforms[0] = ludo::mat4(ludo::vec3(-6.0f, -4.0f, -16.0f), ludo::mat3(rotation));
    transforms[1] = ludo::mat4(ludo::vec3(6.0f, -4.0f, -16.0f), ludo::mat3(rotation));

    auto time = std::sin(inst.total_time / 2.0f) * 0.5f + 0.5f;
    auto stream = ludo::stream(lod_arena);
    ludo::write(stream, 16.0f + 32.0f * time);
    ludo::write(stream, -16.0f + 32.0f * time);
    ludo::write(stream, 16.0f + 32.0f * time);
    ludo::write(stream, -16.0f + 32.0f * time);

    if (fence.id) ludo::wait(fence);
    ludo::swap_buffers(window);
    ludo::use_and_clear({ .width = window.width, .height = window.height });

    ludo::bind_data(0, camera_arena.id);
    ludo::bind_data(1, light_arena.id);
    ludo::bind_data(2, transform_arena.id);
    ludo::bind_data(3, lod_arena.id);
    ludo::bind_data(4, lod_index_arena.id);
    ludo::bind_commands(render_command_arena.id);

    ludo::bind_indices(lod_meshes[0].indices.id);
    ludo::bind_vertices(lod_meshes[0].vertices.id);
    ludo::render(render_program, 0, 1);

    ludo::bind_indices(lod_meshes[1].indices.id);
    ludo::bind_vertices(lod_meshes[1].vertices.id);
    ludo::render(render_program, 1, 1);

    ludo::init(fence);
  });

  ludo::thread_pool_stop();
}
