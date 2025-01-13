#include <random>

#include <ludo/api.h>
#include <ludo/opengl/util.h>

int main()
{
  // SETUP

  auto inst = ludo::instance();
  ludo::thread_pool_start();

  auto window = ludo::window { .title = "noisy!", .width = 1920, .height = 1080, .v_sync = false };
  ludo::init(window);
  ludo::init_rendering();

  auto max_instance_count = uint32_t(1920 * 1080);
  auto circle_counts = ludo::circle_counts(ludo::vertex_format_pc, { .divisions = 20 });

  // CAMERA

  auto camera_arena = ludo::allocate_arena_vram(256 /* TODO */);
  auto camera_bytes = ludo::allocate(camera_arena, 256 /* TODO */);

  // RENDER PROGRAMS

  auto render_program = ludo::render_program();
  ludo::init(render_program, ludo::vertex_format_pc);

  // MESHES

  auto exclusion_zone_mesh = ludo::mesh();
  ludo::mesh_alloc(exclusion_zone_mesh, circle_counts.first, circle_counts.second, render_program.format.size);

  auto sample_mesh = ludo::mesh();
  ludo::mesh_alloc(sample_mesh, circle_counts.first, circle_counts.second, render_program.format.size);

  // CONTAINER

  auto container = ludo::container
  {
    .components =
    {
      { .name = "transform", .size = sizeof(ludo::mat4), .vram = true }
    },
    .archetypes =
    {
      {
        .name = "exclusion_zone",
        .component_names =
        {
          "transform"
        },
        .capacity = max_instance_count
      },
      {
        .name = "sample",
        .component_names =
        {
          "transform"
        },
        .capacity = max_instance_count
      }
    }
  };
  ludo::init(container);

  // POISSON DISC
  // Based on https://www.cs.ubc.ca/~rbridson/docs/bridson-siggraph07-poissondisk.pdf
  auto random = std::mt19937(123456);
  std::uniform_real_distribution<float> distribution(0.0f, 1.0f);
  std::uniform_real_distribution<float> distribution2(-1.0f, 1.0f);
  auto minimum_distance = 10.0f / 180.0f * ludo::pi;
  auto maximum_attempts = 30;

  ludo::circle(exclusion_zone_mesh, render_program.format, 0, 0, { .dimensions = { minimum_distance, 0.0f, 0.0f }, .divisions = 20, .color = { 0.0f, 0.0f, 1.0f, 1.0f } });
  ludo::circle(sample_mesh, render_program.format, 0, 0, { .dimensions = { minimum_distance * 0.1f, 0.0f, 0.0f }, .divisions = 20, .color = { 0.0f, 0.0f, 0.5f, 1.0f } });

  auto samples = std::vector<ludo::vec3> { { 0.0f, 1.0f, 0.0f } };
  auto active_samples = std::vector<ludo::vec3> { { 0.0f, 1.0f, 0.0f } }; // TODO make this a random point

  while (!active_samples.empty())
  {
    auto active_index = uint32_t(distribution(random) * float(active_samples.size()));
    auto& active_sample = active_samples[active_index];

    auto sample_added = false;
    for (auto attempt = 0; attempt < maximum_attempts; attempt++)
    {
      auto random_unit_vector = ludo::vec3 { distribution2(random), distribution2(random), distribution2(random) };
      ludo::normalize(random_unit_vector);
      auto axis = ludo::cross(active_sample, random_unit_vector);
      ludo::normalize(axis);
      auto angle = minimum_distance + distribution(random) * minimum_distance;

      auto sample = active_sample;
      ludo::rotate(sample, axis, angle);

      bool rejected = false;
      for (auto& existing_sample : samples)
      {
        if (ludo::angle_between(sample, existing_sample) < minimum_distance)
        {
          rejected = true;
          break;
        }
      }

      if (!rejected)
      {
        samples.push_back(sample);
        active_samples.push_back(sample);
        sample_added = true;
        break;
      }
    }

    if (!sample_added)
    {
      active_samples.erase(active_samples.begin() + active_index);
    }
  }

  for (auto& sample : samples)
  {
    auto rotation = ludo::mat3(ludo::quat(ludo::vec3_unit_z, sample));

    ludo::add(
      container,
      "exclusion_zone",
      ludo::mat4(sample, rotation)
      );

    ludo::add(
      container,
      "sample",
      ludo::mat4(sample * 1.0001f, rotation)
    );
  }

  // RENDER COMMANDS

  auto render_command_arena = ludo::allocate_arena_vram(2 * sizeof(ludo::render_command));

  *((ludo::render_command*) ludo::allocate(render_command_arena, sizeof(ludo::render_command))) = ludo::render_command
  {
    .index_count = circle_counts.first,
    .instance_count = (uint32_t) samples.size()
  };

  *((ludo::render_command*) ludo::allocate(render_command_arena, sizeof(ludo::render_command))) = ludo::render_command
  {
    .index_count = circle_counts.first,
    .instance_count = (uint32_t) samples.size()
  };

  // VRAM ARENAS

  auto& exclusion_zone_transform_arena = container.archetypes[0].component_data[0];
  auto& sample_transform_arena = container.archetypes[1].component_data[0];

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

    auto view = ludo::mat4(ludo::vec3_zero, ludo::mat3(ludo::quat(float((*mouse_movement_accumulator)[0]) / 500.0f, float((*mouse_movement_accumulator)[1]) / 500.0f, 0.0f)));
    ludo::translate(view, { 0.0f, 0.0f, 3.0f });
    ludo::write_camera_aligned(camera_bytes, ludo::camera
    {
      .view = view,
      .projection = ludo::perspective(60.0f, 16.0f / 9.0f, 0.1f, 1000.0f)
    });

    if (fence.id) ludo::wait(fence);
    ludo::swap_buffers(window);
    ludo::use_and_clear({ .width = window.width, .height = window.height });

    ludo::bind_data(0, camera_arena.id);
    ludo::bind_commands(render_command_arena.id);

    ludo::bind_indices(exclusion_zone_mesh.indices.id);
    ludo::bind_vertices(exclusion_zone_mesh.vertices.id);
    ludo::bind_data(2, exclusion_zone_transform_arena.id);
    ludo::render(render_program, 0, 1);

    ludo::bind_indices(sample_mesh.indices.id);
    ludo::bind_vertices(sample_mesh.vertices.id);
    ludo::bind_data(2, sample_transform_arena.id);
    ludo::render(render_program, 1, 1);

    ludo::init(fence);
  });

  ludo::thread_pool_stop();
}
