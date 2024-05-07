#include <random>

#include <ludo/api.h>
#include <ludo/opengl/util.h>

int main()
{
  auto inst = ludo::instance();

  // SETUP

  auto max_instance_count = uint32_t(1920 * 1080);
  auto circle_counts = ludo::circle_counts({ .divisions = 20 });

  ludo::allocate<ludo::rendering_context>(inst, 1);
  ludo::allocate<ludo::windowing_context>(inst, 1);

  ludo::allocate<ludo::mesh>(inst, 2);
  ludo::allocate<ludo::mesh_instance>(inst, 2);
  ludo::allocate<ludo::render_program>(inst, 1);
  ludo::allocate<ludo::script>(inst, 5);
  ludo::allocate<ludo::shader>(inst, 2);
  ludo::allocate<ludo::window>(inst, 1);

  ludo::add(inst, ludo::windowing_context());
  auto window = ludo::add(inst, ludo::window { .title = "noisy!", .width = 1920, .height = 1080, .v_sync = false });

  auto rendering_context = ludo::add(inst, ludo::rendering_context(), 0);
  set_camera(*rendering_context, ludo::camera
  {
    .view = ludo::mat4(ludo::vec3(0.0, 0.0, 300.0f), ludo::mat3_identity),
    .projection = ludo::perspective(60.0f, 16.0f / 9.0f, 0.1f, 1000.0f)
  });

  ludo::allocate_heap_vram(inst, "ludo::vram_draw_commands", max_instance_count * 2 * sizeof(ludo::draw_command));
  ludo::allocate_heap_vram(inst, "ludo::vram_indices", circle_counts.first * 2 * sizeof(uint32_t));
  ludo::allocate_heap_vram(inst, "ludo::vram_vertices", circle_counts.second * 2 * ludo::vertex_format_pc.size);

  // RENDER PROGRAMS

  auto render_program = ludo::add(inst, ludo::render_program(), ludo::vertex_format_pc, max_instance_count * 2);

  // MESHES

  auto exclusion_zone_mesh = ludo::add(
    inst,
    ludo::mesh(),
    circle_counts.first,
    circle_counts.second,
    render_program->format.size
  );

  ludo::colorize(*exclusion_zone_mesh, render_program->format, 0, circle_counts.second, { 0.0f, 0.0f, 0.5f, 1.0f });

  auto exclusion_zone_mesh_instances = ludo::add(
    inst,
    ludo::mesh_instance
      {
        .render_program_id = render_program->id,
        .instances = { 0, 0 }
      },
    *exclusion_zone_mesh,
    max_instance_count
  );

  auto sample_mesh = ludo::add(
    inst,
    ludo::mesh(),
    circle_counts.first,
    circle_counts.second,
    render_program->format.size
  );

  ludo::colorize(*sample_mesh, render_program->format, 0, circle_counts.second, { 0.0f, 0.0f, 1.0f, 1.0f });

  auto sample_mesh_instances = ludo::add(
    inst,
    ludo::mesh_instance
      {
        .render_program_id = render_program->id,
        .instances = { max_instance_count, 0 }
      },
    *sample_mesh,
    max_instance_count
  );

  // POISSON DISC
  // Based on https://www.cs.ubc.ca/~rbridson/docs/bridson-siggraph07-poissondisk.pdf
  auto random = std::mt19937(123456);
  std::uniform_real_distribution<float> distribution(0.0f, 1.0f);
  std::uniform_real_distribution<float> distribution2(-1.0f, 1.0f);
  auto minimum_distance = 10.0f / 180.0f * ludo::pi;
  auto maximum_attempts = 30;

  auto index_index = uint32_t(0);
  auto vertex_index = uint32_t(0);
  ludo::circle(*sample_mesh, render_program->format, index_index, vertex_index, { .dimensions = { minimum_distance * 0.1f, 0.0f, 0.0f }, .divisions = 20 });

  index_index = uint32_t(0);
  vertex_index = uint32_t(0);
  ludo::circle(*exclusion_zone_mesh, render_program->format, index_index, vertex_index, { .dimensions = { minimum_distance, 0.0f, 0.0f }, .divisions = 20 });

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

    ludo::instance_transform(*exclusion_zone_mesh_instances, exclusion_zone_mesh_instances->instances.count++) = ludo::mat4(sample, rotation);
    ludo::instance_transform(*sample_mesh_instances, sample_mesh_instances->instances.count++) = ludo::mat4(sample * 1.0001f, rotation);
  }

  // SCRIPTS

  auto mouse_movement_accumulator = new std::array<int32_t, 2>();
  ludo::add<ludo::script>(inst, [&](ludo::instance& inst)
  {
    (*mouse_movement_accumulator)[0] += window->mouse_movement[0];
    (*mouse_movement_accumulator)[1] += window->mouse_movement[1];

    auto camera = ludo::get_camera(*rendering_context);
    camera.view = ludo::mat4(ludo::vec3_zero, ludo::mat3(ludo::quat(float((*mouse_movement_accumulator)[0]) / 500.0f, float((*mouse_movement_accumulator)[1]) / 500.0f, 0.0f)));
    ludo::translate(camera.view, { 0.0f, 0.0f, 3.0f });
    ludo::set_camera(*rendering_context, camera);
  });

  ludo::add<ludo::script>(inst, ludo::prepare_render);
  ludo::add<ludo::script>(inst, ludo::update_windows);
  ludo::add<ludo::script, ludo::render_options>(inst, ludo::render, {});
  ludo::add<ludo::script>(inst, ludo::finalize_render);

  // PLAY

  ludo::play(inst);
}
