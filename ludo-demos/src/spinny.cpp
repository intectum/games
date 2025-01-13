#include <ludo/api.h>
#include <ludo/opengl/textures.h>
#include <ludo/opengl/util.h>

int main()
{
  // SETUP

  auto inst = ludo::instance();
  ludo::thread_pool_start();

  auto window = ludo::window { .title = "spinny!", .width = 1920, .height = 1080, .v_sync = false };
  ludo::init(window);
  ludo::init_rendering();

  auto instance_count = 3;
  auto box_counts = ludo::box_counts(ludo::vertex_format_pt);

  // CAMERA

  auto camera_arena = ludo::allocate_arena_vram(256 /* TODO */);
  ludo::write_camera_aligned(ludo::allocate(camera_arena, 256 /* TODO */), ludo::camera
  {
    .view = ludo::mat4_identity,
    .projection = ludo::perspective(60.0f, 16.0f / 9.0f, 0.1f, 1000.0f)
  });

  // RENDER PROGRAMS

  auto render_program_p = ludo::render_program();
  ludo::init(render_program_p, ludo::vertex_format_p);
  auto render_program_pc = ludo::render_program();
  ludo::init(render_program_pc, ludo::vertex_format_pc);
  auto render_program_pt = ludo::render_program();
  ludo::init(render_program_pt, ludo::vertex_format_pt);

  // CUBY

  auto cuby_mesh = ludo::mesh();
  ludo::mesh_alloc(cuby_mesh, box_counts.first, box_counts.second, render_program_p.format.size);
  ludo::box(cuby_mesh, render_program_p.format, 0, 0);

  // RUBY

  auto ruby_mesh = ludo::mesh();
  ludo::mesh_alloc(ruby_mesh, box_counts.first, box_counts.second, render_program_pc.format.size);
  ludo::box(ruby_mesh, render_program_pc.format, 0, 0, { .color = ludo::vec4(1.0f, 0.0f, 0.0f, 1.0f) });

  // TUBY

  auto texture = ludo::load(ludo::asset_folder + "/models/minifig-diffuse.png");

  auto tuby_mesh = ludo::mesh();
  ludo::mesh_alloc(tuby_mesh, box_counts.first, box_counts.second, render_program_pt.format.size);
  ludo::box(tuby_mesh, render_program_pt.format, 0, 0);

  // CONTAINER

  auto container = ludo::container
  {
    .components =
    {
      { .name = "position", .size = sizeof(ludo::vec3) },
      { .name = "rotation", .size = sizeof(ludo::quat) },
      { .name = "transform", .size = sizeof(ludo::mat4), .vram = true },
      { .name = "texture", .size = sizeof(uint64_t), .vram = true },
      { .name = "render_command", .size = sizeof(ludo::render_command), .vram = true }
    },
    .archetypes =
    {
      {
        .name = "cube",
        .component_names =
        {
          "position",
          "rotation",
          "transform",
          "texture",
          "render_command"
        },
        .capacity = 3
      }
    }
  };
  ludo::init(container);

  // ENTITIES

  ludo::add(
    container,
    "cube",
    ludo::vec3(-2.5f, 0.0f, -4.0f),
    ludo::quat_identity,
    ludo::mat4_identity,
    uint64_t(0),
    ludo::render_command
    {
      .index_count = box_counts.first,
    }
  );

  ludo::add(
    container,
    "cube",
    ludo::vec3(0.0f, 0.0f, -4.0f),
    ludo::quat_identity,
    ludo::mat4_identity,
    uint64_t(0),
    ludo::render_command
    {
      .index_count = box_counts.first,
      .instance_start = 1
    }
  );

  ludo::add(
    container,
    "cube",
    ludo::vec3(2.5f, 0.0f, -4.0f),
    ludo::quat_identity,
    ludo::mat4_identity,
    ludo::handle(texture),
    ludo::render_command
    {
      .index_count = box_counts.first,
      .instance_start = 2
    }
  );

  // VRAM ARENAS

  auto& transform_arena = container.archetypes[0].component_data[2];
  auto& texture_arena = container.archetypes[0].component_data[3];
  auto& render_command_arena = container.archetypes[0].component_data[4];

  // JOBS

  auto spin = ludo::job
  {
    .write_component_names = { "rotation" },
    .kernel = [&](uint32_t entity_start, uint32_t entity_count, const std::vector<ludo::arena>& read_component_data, std::vector<ludo::arena>& write_component_data)
    {
      auto rotations = reinterpret_cast<ludo::quat*>(write_component_data[0].start);

      for (auto index = 0; index < entity_count; index++)
      {
        rotations[index] = ludo::quat(ludo::vec3_unit_y, inst.total_time);
      }
    }
  };

  auto update_transforms = ludo::job
  {
    .read_component_names = { "position", "rotation" },
    .write_component_names = { "transform" },
    .kernel = [&](uint32_t entity_start, uint32_t entity_count, const std::vector<ludo::arena>& read_component_data, std::vector<ludo::arena>& write_component_data)
    {
      auto positions = reinterpret_cast<const ludo::vec3*>(read_component_data[0].start);
      auto rotations = reinterpret_cast<const ludo::quat*>(read_component_data[1].start);
      auto transforms = reinterpret_cast<ludo::mat4*>(write_component_data[0].start);

      for (auto index = 0; index < entity_count; index++)
      {
        transforms[index] = ludo::mat4(positions[index], ludo::mat3(rotations[index]));
      }
    }
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

    ludo::run(container, { spin });

    if (fence.id) ludo::wait(fence);
    ludo::swap_buffers(window);
    ludo::use_and_clear({ .width = window.width, .height = window.height });

    ludo::run(container, { update_transforms });

    ludo::bind_data(0, camera_arena.id);
    ludo::bind_data(2, transform_arena.id);
    ludo::bind_data(3, texture_arena.id);
    ludo::bind_commands(render_command_arena.id);

    ludo::bind_indices(cuby_mesh.indices.id);
    ludo::bind_vertices(cuby_mesh.vertices.id);
    ludo::render(render_program_p, 0, 1);

    ludo::bind_indices(ruby_mesh.indices.id);
    ludo::bind_vertices(ruby_mesh.vertices.id);
    ludo::render(render_program_pc, 1, 1);

    ludo::bind_indices(tuby_mesh.indices.id);
    ludo::bind_vertices(tuby_mesh.vertices.id);
    ludo::render(render_program_pt, 2, 1);

    ludo::init(fence);
  });

  ludo::thread_pool_stop();
}
