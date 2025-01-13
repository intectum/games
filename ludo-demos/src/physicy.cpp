#include <ludo/api.h>
#include <ludo/opengl/util.h>

int main()
{
  // SETUP

  auto inst = ludo::instance();

  auto window = ludo::window { .title = "physicy!", .width = 1920, .height = 1080, .v_sync = false };
  ludo::init(window);
  ludo::init_rendering();

  auto box_counts = ludo::box_counts(ludo::vertex_format_p);

  // CAMERA

  auto camera_arena = ludo::allocate_arena_vram(256 /* TODO */);
  ludo::write_camera_aligned(ludo::allocate(camera_arena, 256 /* TODO */), ludo::camera
  {
    .view = ludo::mat4_identity,
    .projection = ludo::perspective(60.0f, 16.0f / 9.0f, 0.1f, 1000.0f)
  });

  // RENDER PROGRAMS

  auto render_program = ludo::render_program { .primitive = ludo::mesh_primitive::LINE_LIST };
  ludo::init(render_program, ludo::vertex_format_pc);

  // MESHES

  auto floor_mesh = ludo::mesh();
  ludo::mesh_alloc(floor_mesh, box_counts.first, box_counts.second, ludo::vertex_format_p.size);
  ludo::box(floor_mesh, ludo::vertex_format_p, 0, 0, { .dimensions = ludo::vec3(25.0f, 1.0f, 25.0f) });

  auto box_mesh = ludo::mesh();
  ludo::mesh_alloc(box_mesh, box_counts.first, box_counts.second, ludo::vertex_format_p.size);
  ludo::box(box_mesh, ludo::vertex_format_p, 0, 0);

  auto box_positions = std::vector<ludo::vec3>(8);
  std::memcpy(box_positions.data(), box_mesh.vertices.start, box_mesh.vertices.end - box_mesh.vertices.start);

  // PHYSICS

  auto physics_context = ludo::physics_context();
  ludo::init(physics_context);

  auto floor = ludo::static_body { .transform = { .position = ludo::vec3(0.0f, -2.0f, 0.0f) } };
  ludo::init(floor, physics_context);
  ludo::connect(floor, physics_context, floor_mesh, ludo::vertex_format_p);

  auto box_shape = new btCompoundShape(true, static_cast<int>(dynamic_body_shape.convex_hulls.size()));
  box_shape->addChildShape(ludo::to_btTransform(ludo::mat4_identity), new btConvexHullShape(box_positions.data()->begin(), static_cast<int>(box_positions.size()), sizeof(vec3)));

  auto box = ludo::dynamic_body { .transform = { .position = ludo::vec3(0.0f, 2.0f, -10.0f) }, .mass = 1.0f };
  ludo::init(box, physics_context);
  ludo::connect(box, physics_context, { box_shape });

  auto kinematic_box = ludo::kinematic_body { .transform = { .position = ludo::vec3(-5.0f, 2.0f, -10.0f) } };
  ludo::init(kinematic_box, physics_context);
  ludo::connect(kinematic_box, physics_context, { box_shape });

  auto ghost_box = ludo::ghost_body { .transform = { .position = ludo::vec3(5.0f, 2.0f, -10.0f) } };
  ludo::init(ghost_box, physics_context);
  ludo::connect(ghost_box, physics_context, { box_shape });

  auto physics_mesh = ludo::mesh();
  ludo::mesh_alloc(physics_mesh, 10000, 10000, ludo::vertex_format_pc.size);

  // TRANSFORMS

  auto transform_arena = ludo::allocate_arena_vram(sizeof(ludo::mat4));
  *((ludo::mat4*) ludo::allocate(transform_arena, sizeof(ludo::mat4))) = ludo::mat4_identity;

  // RENDER COMMANDS

  auto render_command_arena = ludo::allocate_arena_vram(sizeof(ludo::render_command));
  *((ludo::render_command*) ludo::allocate(render_command_arena, sizeof(ludo::render_command))) = ludo::render_command
  {
    .index_count = 10000
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

    if (window.active_mouse_button_states[ludo::mouse_button::LEFT] == ludo::button_state::HOLD)
    {
      kinematic_box.transform.position[0] += float(window.mouse_movement[0]) / 100.0f;
      kinematic_box.transform.position[1] += float(window.mouse_movement[1]) / -100.0f;
      ludo::commit(kinematic_box);
    }

    if (window.active_mouse_button_states[ludo::mouse_button::RIGHT] == ludo::button_state::HOLD)
    {
      ghost_box.transform.position[0] += float(window.mouse_movement[0]) / 100.0f;
      ghost_box.transform.position[1] += float(window.mouse_movement[1]) / -100.0f;
      ludo::commit(ghost_box);
    }

    ludo::simulate(physics_context, inst.delta_time);

    if (fence.id) ludo::wait(fence);
    ludo::swap_buffers(window);
    ludo::use_and_clear({ .width = window.width, .height = window.height });

    ludo::bind_indices(physics_mesh.indices.id);
    ludo::bind_vertices(physics_mesh.vertices.id);
    ludo::bind_data(0, camera_arena.id);
    ludo::bind_data(2, transform_arena.id);
    ludo::bind_commands(render_command_arena.id);

    ludo::visualize(physics_context, physics_mesh);
    ludo::render(render_program, 0, 1);

    ludo::init(fence);
  });
}
