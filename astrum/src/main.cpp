#include <iomanip>
#include <iostream>

#include <btBulletDynamicsCommon.h>

#include <ludo/api.h>
#include <ludo/opengl/util.h>

#include "constants.h"
#include "controllers/game.h"
#include "ecs.h"
#include "entities/luna.h"
#include "entities/people.h"
#include "entities/spaceships.h"
#include "entities/sol.h"
#include "entities/terra.h"
#include "meshes/lod_shaders.h"
#include "physics/centering.h"
#include "physics/gravity.h"
#include "physics/point_masses.h"
#include "physics/relativity.h"
#include "physics/util.h"
#include "post-processing/atmosphere.h"
#include "post-processing/bloom.h"
#include "post-processing/tone_mapping.h"
#include "post-processing/util.h"
#include "solar_system.h"
#include "terrain/terrain.h"

int main()
{
  auto timer = ludo::timer();

  auto inst = ludo::instance();
  ludo::thread_pool_start();

  auto window = ludo::window { .title = "astrum", .width = 1920, .height = 1080, .v_sync = false };
  ludo::open(window);
  //ludo::capture_mouse(window);
  ludo::init_rendering();

  // TODO
  /*auto default_grid = ludo::add(
    inst,
    ludo::grid3
      {
        .bounds =
        {
          .min = { -2.0f * astrum::astronomical_unit, -2.0f * astrum::astronomical_unit, -2.0f * astrum::astronomical_unit },
          .max = { 2.0f * astrum::astronomical_unit, 2.0f * astrum::astronomical_unit, 2.0f * astrum::astronomical_unit }
        },
        .cell_count_1d = 16
      }
  );
  default_grid->compute_program_id = ludo::add(inst, ludo::build_compute_program(*default_grid))->id;
  ludo::init(*default_grid);*/

  auto multi_sample_color_texture = ludo::texture { .datatype = ludo::pixel_datatype::FLOAT16, .width = window.width, .height = window.height };
  ludo::init(multi_sample_color_texture, { .samples = astrum::multi_sample_count });
  auto multi_sample_depth_texture = ludo::texture { .components = ludo::pixel_components::DEPTH, .datatype = ludo::pixel_datatype::FLOAT32, .width = window.width, .height = window.height };
  ludo::init(multi_sample_depth_texture, { .samples = astrum::multi_sample_count });
  auto multi_sample_frame_buffer = ludo::frame_buffer { .width = window.width, .height = window.height, .color_texture_ids = { multi_sample_color_texture.id }, .depth_texture_id = multi_sample_depth_texture.id };
  ludo::init(multi_sample_frame_buffer);

  auto config = new btDefaultCollisionConfiguration();
  // config->setConvexConvexMultipointIterations();

  // We are just using the default collision dispatcher and constraint solver. For parallel processing see Extras/BulletMultiThreaded.
  auto dispatcher = new btCollisionDispatcher(config);
  auto broadphase = new btDbvtBroadphase();
  auto bullet_world = new btDiscreteDynamicsWorld(dispatcher, broadphase, nullptr, config);
  //bullet_world->setGravity(to_btVector3(ludo::vec3_zero));

  std::cout << std::fixed << std::setprecision(4) << "main load time: " << ludo::elapsed(timer) << "s" << std::endl;
  ludo::reset(timer);

  // TODO stars do not need lods... or even to be divided into chunks...
  auto star_format = ludo::vertex_format_p;
  star_format.components.insert(star_format.components.end(), star_format.components.begin(), star_format.components.end());
  star_format.size *= 2;

  auto star_render_program = ludo::render_program { .format = star_format };
  auto star_vertex_shader_code = astrum::lod_vertex_shader_code(star_format, true);
  auto star_fragment_shader_code = astrum::lod_fragment_shader_code(star_format, true);
  ludo::init(star_render_program, star_vertex_shader_code, star_fragment_shader_code);

  auto terrain_format = ludo::vertex_format_pnc;
  terrain_format.components.insert(terrain_format.components.end(), terrain_format.components.begin(), terrain_format.components.end());
  terrain_format.size *= 2;

  auto terrain_render_program = ludo::render_program { .format = terrain_format };
  auto terrain_vertex_shader_code = astrum::lod_vertex_shader_code(terrain_format, true);
  auto terrain_fragment_shader_code = astrum::lod_fragment_shader_code(terrain_format, true);
  ludo::init(terrain_render_program, terrain_vertex_shader_code, terrain_fragment_shader_code);

  auto minifig_format = ludo::format(true, true, true, true);
  auto minifig_counts = ludo::import_counts(ludo::asset_folder + "/models/minifig.dae");
  auto minifig_indices = ludo::allocate_buffer_vram(minifig_counts.first * sizeof(uint32_t));
  auto minifig_vertices = ludo::allocate_buffer_vram(minifig_counts.second * minifig_format.size);
  auto minifig = ludo::import(ludo::asset_folder + "/models/minifig.dae", minifig_indices, minifig_vertices);

  auto spaceship_counts = ludo::import_counts(ludo::asset_folder + "/models/spaceship.dae");
  auto spaceship_indices = ludo::allocate_buffer_vram(spaceship_counts.first * sizeof(uint32_t));
  auto spaceship_vertices = ludo::allocate_buffer_vram(spaceship_counts.second * ludo::vertex_format_pnc.size);
  auto spaceship = ludo::import(ludo::asset_folder + "/models/spaceship.dae", spaceship_indices, spaceship_vertices);

  auto sol_render_command_arena = ludo::allocate_arena_vram(5120 * sizeof(ludo::render_command));
  auto luna_render_command_arena = ludo::allocate_arena_vram(5120 * sizeof(ludo::render_command));
  for (auto terrain_index = uint32_t(0); terrain_index < 5120; terrain_index++)
  {
    *((ludo::render_command*) ludo::allocate(sol_render_command_arena, sizeof(ludo::render_command))) = ludo::render_command
    {
      .index_count = 3,
      .index_start = terrain_index * 3,
      .instance_start = terrain_index
    };

    *((ludo::render_command*) ludo::allocate(luna_render_command_arena, sizeof(ludo::render_command))) = ludo::render_command
    {
      .index_count = 3,
      .index_start = terrain_index * 3,
      .instance_start = terrain_index
    };
  }

  auto sol_indices = ludo::allocate_pool_vram(5120, 3 * sizeof(uint32_t));
  auto sol_vertices = ludo::allocate_pool_vram(5120, 3 * star_format.size);
  // TODO revise... we are allocating enough for all terrain to be at LOD 0
  // TODO ...and we allocate that same amount for every LOD, might be enough?
  auto terra_indices = ludo::allocate_buffer_vram(4 * 5120 * astrum::terra_lods.size() * 3 * sizeof(uint32_t));
  auto terra_vertices = ludo::allocate_buffer_vram(4 * 5120 * astrum::terra_lods.size() * 3 * terrain_format.size);
  auto luna_indices = ludo::allocate_buffer_vram(5120 * astrum::luna_lods.size() * 3 * sizeof(uint32_t));
  auto luna_vertices = ludo::allocate_buffer_vram(5120 * astrum::luna_lods.size() * 3 * terrain_format.size);
  uint32_t terra_lod_vertex_counts[] =
  {
    3 * static_cast<uint32_t>(std::pow(4, astrum::terra_lods[0].level - astrum::terra_lods[0].level)),
    3 * static_cast<uint32_t>(std::pow(4, astrum::terra_lods[1].level - astrum::terra_lods[0].level)),
    3 * static_cast<uint32_t>(std::pow(4, astrum::terra_lods[2].level - astrum::terra_lods[0].level)),
    3 * static_cast<uint32_t>(std::pow(4, astrum::terra_lods[3].level - astrum::terra_lods[0].level)),
    3 * static_cast<uint32_t>(std::pow(4, astrum::terra_lods[4].level - astrum::terra_lods[0].level))
  };
  uint32_t luna_lod_vertex_counts[] =
  {
    3 * static_cast<uint32_t>(std::pow(4, astrum::luna_lods[0].level - astrum::luna_lods[0].level)),
    3 * static_cast<uint32_t>(std::pow(4, astrum::luna_lods[1].level - astrum::luna_lods[0].level)),
    3 * static_cast<uint32_t>(std::pow(4, astrum::luna_lods[2].level - astrum::luna_lods[0].level)),
    3 * static_cast<uint32_t>(std::pow(4, astrum::luna_lods[3].level - astrum::luna_lods[0].level))
  };
  uint32_t terra_lod_block_counts[] =
  {
    4 * 5120 * terra_lod_vertex_counts[0] / terra_lod_vertex_counts[0],
    4 * 5120 * terra_lod_vertex_counts[0] / terra_lod_vertex_counts[1],
    4 * 5120 * terra_lod_vertex_counts[0] / terra_lod_vertex_counts[2],
    4 * 5120 * terra_lod_vertex_counts[0] / terra_lod_vertex_counts[3],
    4 * 5120 * terra_lod_vertex_counts[0] / terra_lod_vertex_counts[4],
  };
  uint32_t luna_lod_block_counts[] =
  {
    5120 * luna_lod_vertex_counts[0] / luna_lod_vertex_counts[0],
    5120 * luna_lod_vertex_counts[0] / luna_lod_vertex_counts[1],
    5120 * luna_lod_vertex_counts[0] / luna_lod_vertex_counts[2],
    5120 * luna_lod_vertex_counts[0] / luna_lod_vertex_counts[3]
  };
  ludo::pool terra_index_pools[] =
  {
    ludo::pool {
      {
        .data = terra_indices.data,
        .size = terra_indices.size / 5
      },
      terra_lod_block_counts[0],
      static_cast<uint32_t>(terra_lod_vertex_counts[0] * sizeof(uint32_t)),
      static_cast<bool*>(calloc(terra_lod_block_counts[0], sizeof(bool)))
    },
    ludo::pool {
      {
        .data = terra_indices.data + terra_indices.size / 5,
        .size = terra_indices.size / 5
      },
      terra_lod_block_counts[1],
      static_cast<uint32_t>(terra_lod_vertex_counts[1] * sizeof(uint32_t)),
      static_cast<bool*>(calloc(terra_lod_block_counts[1], sizeof(bool)))
    },
    ludo::pool {
      {
        .data = terra_indices.data + terra_indices.size / 5 * 2,
        .size = terra_indices.size / 5
      },
      terra_lod_block_counts[2],
      static_cast<uint32_t>(terra_lod_vertex_counts[2] * sizeof(uint32_t)),
      static_cast<bool*>(calloc(terra_lod_block_counts[2], sizeof(bool)))
    },
    ludo::pool {
      {
        .data = terra_indices.data + terra_indices.size / 5 * 3,
        .size = terra_indices.size / 5
      },
      terra_lod_block_counts[3],
      static_cast<uint32_t>(terra_lod_vertex_counts[3] * sizeof(uint32_t)),
      static_cast<bool*>(calloc(terra_lod_block_counts[3], sizeof(bool)))
    },
    ludo::pool {
      {
        .data = terra_indices.data + terra_indices.size / 5 * 4,
        .size = terra_indices.size / 5
      },
      terra_lod_block_counts[4],
      static_cast<uint32_t>(terra_lod_vertex_counts[4] * sizeof(uint32_t)),
      static_cast<bool*>(calloc(terra_lod_block_counts[4], sizeof(bool)))
    }
  };
  ludo::pool terra_vertex_pools[] =
  {
    ludo::pool {
        {
        .data = terra_vertices.data,
        .size = terra_vertices.size / 5
      },
      terra_lod_block_counts[0],
      terra_lod_vertex_counts[0] * terrain_format.size,
      static_cast<bool*>(calloc(terra_lod_block_counts[0], sizeof(bool)))
    },
    ludo::pool {
        {
        .data = terra_vertices.data + terra_vertices.size / 5,
        .size = terra_vertices.size / 5
      },
      terra_lod_block_counts[1],
      terra_lod_vertex_counts[1] * terrain_format.size,
      static_cast<bool*>(calloc(terra_lod_block_counts[1], sizeof(bool)))
    },
    ludo::pool {
      {
        .data = terra_vertices.data + terra_vertices.size / 5 * 2,
        .size = terra_vertices.size / 5
      },
      terra_lod_block_counts[2],
      terra_lod_vertex_counts[2] * terrain_format.size,
      static_cast<bool*>(calloc(terra_lod_block_counts[2], sizeof(bool)))
    },
    ludo::pool {
        {
        .data = terra_vertices.data + terra_vertices.size / 5 * 3,
        .size = terra_vertices.size / 5
      },
      terra_lod_block_counts[3],
      terra_lod_vertex_counts[3] * terrain_format.size,
      static_cast<bool*>(calloc(terra_lod_block_counts[3], sizeof(bool)))
    },
    ludo::pool {
        {
        .data = terra_vertices.data + terra_vertices.size / 5 * 4,
        .size = terra_vertices.size / 5
      },
      terra_lod_block_counts[4],
      terra_lod_vertex_counts[4] * terrain_format.size,
      static_cast<bool*>(calloc(terra_lod_block_counts[4], sizeof(bool)))
    }
  };
  ludo::pool luna_index_pools[] =
  {
    ludo::pool {
      {
        .data = luna_indices.data,
        .size = luna_indices.size / 5
      },
      luna_lod_block_counts[0],
      static_cast<uint32_t>(luna_lod_vertex_counts[0] * sizeof(uint32_t)),
      static_cast<bool*>(calloc(luna_lod_block_counts[0], sizeof(bool)))
    },
    ludo::pool {
      {
        .data = luna_indices.data + luna_indices.size / 5,
        .size = luna_indices.size / 5
      },
      luna_lod_block_counts[1],
      static_cast<uint32_t>(luna_lod_vertex_counts[1] * sizeof(uint32_t)),
      static_cast<bool*>(calloc(luna_lod_block_counts[1], sizeof(bool)))
    },
    ludo::pool {
      {
        .data = luna_indices.data + luna_indices.size / 5 * 2,
        .size = luna_indices.size / 5
      },
      luna_lod_block_counts[2],
      static_cast<uint32_t>(luna_lod_vertex_counts[2] * sizeof(uint32_t)),
      static_cast<bool*>(calloc(luna_lod_block_counts[2], sizeof(bool)))
    },
    ludo::pool {
      {
        .data = luna_indices.data + luna_indices.size / 5 * 3,
        .size = luna_indices.size / 5
      },
      luna_lod_block_counts[3],
      static_cast<uint32_t>(luna_lod_vertex_counts[3] * sizeof(uint32_t)),
      static_cast<bool*>(calloc(luna_lod_block_counts[3], sizeof(bool)))
    }
  };
  ludo::pool luna_vertex_pools[] =
  {
    ludo::pool {
      {
        .data = luna_vertices.data,
        .size = luna_vertices.size / 5
      },
      luna_lod_block_counts[0],
      luna_lod_vertex_counts[0] * terrain_format.size,
      static_cast<bool*>(calloc(luna_lod_block_counts[0], sizeof(bool)))
    },
    ludo::pool {
      {
        .data = luna_vertices.data + luna_vertices.size / 5,
        .size = luna_vertices.size / 5
      },
      luna_lod_block_counts[1],
      luna_lod_vertex_counts[1] * terrain_format.size,
      static_cast<bool*>(calloc(luna_lod_block_counts[1], sizeof(bool)))
    },
    ludo::pool {
      {
        .data = luna_vertices.data + luna_vertices.size / 5 * 2,
        .size = luna_vertices.size / 5
      },
      luna_lod_block_counts[2],
      luna_lod_vertex_counts[2] * terrain_format.size,
      static_cast<bool*>(calloc(luna_lod_block_counts[2], sizeof(bool)))
    },
    ludo::pool {
      {
        .data = luna_vertices.data + luna_vertices.size / 5 * 3,
        .size = luna_vertices.size / 5
      },
      luna_lod_block_counts[3],
      luna_lod_vertex_counts[3] * terrain_format.size,
      static_cast<bool*>(calloc(luna_lod_block_counts[3], sizeof(bool)))
    }
  };

  // TODO Initialize camera to roughly correct position to ensure the correct LODs are loaded
  const auto initial_camera_position = ludo::vec3_zero;

  auto sol_container = astrum::build_celestial_body(
    "sol",
    ludo::vec3_zero,
    astrum::sol_mass,
    ludo::vec3_zero,
    astrum::sol_radius,
    sol_indices,
    sol_vertices,
    &sol_indices,
    &sol_vertices,
    initial_camera_position,
    star_format,
    astrum::sol_lods,
    astrum::sol_funcs
  );

  const auto terra_initial_position = ludo::vec3 { -1.0f * astrum::astronomical_unit, 0.0f, 0.0f };
  const auto terra_initial_velocity = ludo::vec3 { 0.0f, astrum::orbital_speed(ludo::length(terra_initial_position), astrum::sol_mass), 0.0f };

  auto terra_container = astrum::build_celestial_body(
    "terra",
    terra_initial_position,
    astrum::terra_mass,
    terra_initial_velocity,
    astrum::terra_radius,
    terra_indices,
    terra_vertices,
    terra_index_pools,
    terra_vertex_pools,
    initial_camera_position,
    terrain_format,
    astrum::terra_lods,
    astrum::terra_funcs
  );

  const auto luna_initial_position = ludo::vec3 { -1.0f * astrum::astronomical_unit + astrum::luna_orbit_distance, 0.0f, 0.0f };
  const auto luna_initial_velocity = terra_initial_velocity + ludo::vec3 { 0.0f, astrum::orbital_speed(astrum::luna_orbit_distance, astrum::terra_mass), 0.0f };

  auto luna_container = astrum::build_celestial_body(
    "luna",
    luna_initial_position,
    astrum::luna_mass,
    luna_initial_velocity,
    astrum::luna_radius,
    luna_indices,
    luna_vertices,
    luna_index_pools,
    luna_vertex_pools,
    initial_camera_position,
    terrain_format,
    astrum::luna_lods,
    astrum::luna_funcs
  );

  ludo::add(
    sol_container,
    "camera",
    ludo::vec3_zero,
    ludo::quat_identity,
    ludo::mat4_identity,
    ludo::perspective(60.0f, 16.0f / 9.0f, 0.1f, 2.0f * astrum::astronomical_unit),
    ludo::mat4_identity,
    0.1f,
    2.0f * astrum::astronomical_unit
  );

  ludo::add(
    sol_container,
    "light",
    ludo::vec4 { 0.01f, 0.01f, 0.01f, 1.0f },
    ludo::vec4 { 0.7f, 0.7f, 0.7f, 1.0f },
    ludo::vec4 { 0.01f, 0.01f, 0.01f, 1.0f },
    ludo::vec3_zero,
    ludo::vec3_zero,
    ludo::vec3 { 1.0f, 0.0f, 0.0f },
    1.0f,
    5.0f * astrum::astronomical_unit
  );

  auto game_controls = astrum::game_controls { .mode = astrum::game_controls::mode::map };
  auto map_controls = astrum::map_controls { .target_radius = astrum::sol_radius };

  std::cout << std::fixed << std::setprecision(4) << "solar system load time: " << ludo::elapsed(timer) << "s" << std::endl;
  ludo::reset(timer);

  // TODO
  //ludo::commit(*default_grid);

  // TODO
  /*auto bullet_debug_render_program = ludo::render_program { .primitive = ludo::mesh_primitive::LINE_LIST };
  ludo::init(bullet_debug_render_program, ludo::vertex_format_pc);

  auto bullet_debug_mesh = ludo::mesh();
  ludo::mesh_alloc(bullet_debug_mesh, 25 * 48 * 2, 25 * 48 * 2, bullet_debug_render_program.format.size);*/

  // Post-processing
  auto single_sample_frame_buffer = astrum::build_post_processing_frame_buffer(window, true);
  auto post_processing_counts = ludo::rectangle_counts(ludo::vertex_format_pt);
  auto post_processing_indices = ludo::allocate_buffer_vram(post_processing_counts.first * sizeof(uint32_t));
  auto post_processing_vertices = ludo::allocate_buffer_vram(post_processing_counts.second * ludo::vertex_format_pt.size);
  auto post_processing_mesh = ludo::mesh { .vertex_size = ludo::vertex_format_pt.size };
  ludo::append_rectangle(post_processing_mesh, post_processing_indices, post_processing_vertices, ludo::vertex_format_pt, { .dimensions = { 2.0f, 2.0f, 0.0f } });
  auto post_processing_command_arena = ludo::allocate_arena_vram(sizeof(ludo::render_command));
  *((ludo::render_command*) ludo::allocate(post_processing_command_arena, sizeof(ludo::render_command))) = ludo::render_command
  {
    .index_count = post_processing_counts.first
  };

  //astrum::write_atmosphere_textures(astrum::terra_atmosphere_scale);
  auto [ atmosphere_render_program, atmosphere_data_buffer, atmosphere_frame_buffer ] = astrum::build_atmosphere(window, single_sample_frame_buffer, astrum::terra_radius, astrum::terra_radius * astrum::terra_atmosphere_scale);
  auto [ bloom_render_programs, bloom_data_buffers, bloom_frame_buffers ] = astrum::build_bloom(window, atmosphere_frame_buffer, 5, 0.1f);
  auto [ tone_mapping_render_program, tone_mapping_data_buffer, tone_mapping_frame_buffer ] = astrum::build_tone_mapper(window, bloom_frame_buffers.back());

  std::cout << std::fixed << std::setprecision(4) << "remaining load time: " << ludo::elapsed(timer) << "s" << std::endl;

  ludo::fence fence;
  auto last_print_time = 0.0f;
  auto frame_count = uint32_t(0);
  auto relative_celestial_body_index = uint32_t(0);

  ludo::play(inst, [&]
  {
    auto containers = std::vector { sol_container, terra_container, luna_container };

    auto camera_positions = astrum::get_components(
      sol_container,
      "camera",
      "position_vram"
    );

    auto camera_rotations = astrum::get_components(
      sol_container,
      "camera",
      "rotation"
    );

    auto camera_view_projections = astrum::get_components(
      sol_container,
      "camera",
      "view_projection_vram"
    );

    auto camera_views = astrum::get_components(
      sol_container,
      "camera",
      "view_vram"
    );

    auto camera_projections = astrum::get_components(
      sol_container,
      "camera",
      "projection_vram"
    );

    auto camera_near_clipping_distances = astrum::get_components(
      sol_container,
      "camera",
      "near_clipping_distance_vram"
    );

    auto camera_far_clipping_distances = astrum::get_components(
      sol_container,
      "camera",
      "far_clipping_distance_vram"
    );

    auto light_ambient_colors = astrum::get_components(
      sol_container,
      "light",
      "ambient_color_vram"
    );

    auto light_diffuse_colors = astrum::get_components(
      sol_container,
      "light",
      "diffuse_color_vram"
    );

    auto light_specular_colors = astrum::get_components(
      sol_container,
      "light",
      "specular_color_vram"
    );

    auto light_positions = astrum::get_components(
      sol_container,
      "light",
      "position_vram"
    );

    auto light_directions = astrum::get_components(
      sol_container,
      "light",
      "direction_vram"
    );

    auto light_attenuations = astrum::get_components(
      sol_container,
      "light",
      "attenuation_vram"
    );

    auto light_strengths = astrum::get_components(
      sol_container,
      "light",
      "strength_vram"
    );

    auto light_ranges = astrum::get_components(
      sol_container,
      "light",
      "range_vram"
    );

    auto sol_transforms = astrum::get_components(
      sol_container,
      "celestial_body",
      "transform_vram"
    );

    auto sol_lods = astrum::get_components(
      sol_container,
      "celestial_body",
      "lods_vram"
    );

    auto sol_lod_indices = astrum::get_components(
      sol_container,
      "terrain",
      "lod_index_vram"
    );

    auto terra_transforms = astrum::get_components(
      terra_container,
      "celestial_body",
      "transform_vram"
    );

    auto terra_lods = astrum::get_components(
      terra_container,
      "celestial_body",
      "lods_vram"
    );

    auto terra_lod_indices = astrum::get_components(
      terra_container,
      "terrain",
      "lod_index_vram"
    );

    auto terra_render_commands = astrum::get_components(
      terra_container,
      "terrain",
      "render_command_vram"
    );

    auto luna_transforms = astrum::get_components(
      luna_container,
      "celestial_body",
      "transform_vram"
    );

    auto luna_lods = astrum::get_components(
      luna_container,
      "celestial_body",
      "lods_vram"
    );

    auto luna_lod_indices = astrum::get_components(
      luna_container,
      "terrain",
      "lod_index_vram"
    );

    auto& camera_position = *reinterpret_cast<ludo::vec3*>(camera_positions->data);
    auto& camera_rotation = *reinterpret_cast<ludo::quat*>(camera_rotations->data);

    ludo::receive_input(window);

    if (window.frame_button_states[ludo::window_frame_button_close] == ludo::button_state_up)
    {
      ludo::stop(inst);
    }

    astrum::center_universe(containers, camera_position);
    relative_celestial_body_index = astrum::relativize_universe(containers, relative_celestial_body_index, camera_position);

    // TODO requires multi-threading...
    //astrum::simulate_gravity(inst, containers, relative_celestial_body_index);
    bullet_world->stepSimulation(inst.delta_time);
    astrum::simulate_point_mass_physics(inst, containers);

    astrum::stream_terrain(
      terra_container,
      terra_indices,
      terra_vertices,
      terra_index_pools,
      terra_vertex_pools,
      camera_position,
      astrum::terra_funcs,
      terrain_format,
      astrum::terra_radius,
      astrum::terra_lods
    );

    astrum::stream_terrain(
      luna_container,
      luna_indices,
      luna_vertices,
      luna_index_pools,
      luna_vertex_pools,
      camera_position,
      astrum::luna_funcs,
      terrain_format,
      astrum::luna_radius,
      astrum::luna_lods
    );

    // TODO
    //stream_trees(inst, 1);

    astrum::sync_light_with_sol(sol_container);

    astrum::simulate_people(inst, containers, minifig.animations[0], minifig.armatures[0], relative_celestial_body_index);
    astrum::simulate_spaceships(inst, containers);

    astrum::control_game(inst, containers, window, game_controls, map_controls, camera_position, camera_rotation);

    // TODO
    /*if (show_paths)
    {
      update_prediction_paths(sol_container);
    }*/

    if (fence.id) ludo::wait(fence);
    ludo::swap_buffers(window);
    ludo::use_and_clear(multi_sample_frame_buffer);

    ludo::run(
      containers,
      {
        ludo::job
        {
          .read_component_names = { "position", "rotation" },
          .write_component_names = { "transform_vram" },
          .kernel = [&](uint32_t entity_start, uint32_t entity_count, const std::vector<ludo::arena>& read_component_data, std::vector<ludo::arena>& write_component_data)
          {
            auto positions = reinterpret_cast<const ludo::vec3*>(read_component_data[0].data);
            auto rotations = reinterpret_cast<const ludo::quat*>(read_component_data[1].data);
            auto transforms = reinterpret_cast<ludo::mat4*>(write_component_data[0].data);

            for (auto index = 0; index < entity_count; index++)
            {
              transforms[index] = ludo::mat4(positions[index], ludo::mat3(rotations[index]));
            }
          }
        },
        ludo::job
        {
          .read_component_names = { "position_vram", "rotation", "projection_vram" },
          .write_component_names = { "view_vram", "view_projection_vram" },
          .kernel = [&](uint32_t entity_start, uint32_t entity_count, const std::vector<ludo::arena>& read_component_data, std::vector<ludo::arena>& write_component_data)
          {
            auto positions = reinterpret_cast<const ludo::vec3*>(read_component_data[0].data);
            auto rotations = reinterpret_cast<const ludo::quat*>(read_component_data[1].data);
            auto projections = reinterpret_cast<const ludo::mat4*>(read_component_data[2].data);
            auto views = reinterpret_cast<ludo::mat4*>(write_component_data[0].data);
            auto view_projections = reinterpret_cast<ludo::mat4*>(write_component_data[1].data);

            for (auto index = 0; index < entity_count; index++)
            {
              views[index] = ludo::mat4(positions[index], ludo::mat3(rotations[index]));
              view_projections[index] = ludo::view_projection(views[index], projections[index]);
            }
          }
        },
        ludo::job
        {
          .read_component_names = { "mesh" },
          .write_component_names = { "render_command_vram" },
          .kernel = [&](uint32_t entity_start, uint32_t entity_count, const std::vector<ludo::arena>& read_component_data, std::vector<ludo::arena>& write_component_data)
          {
            // terra only
            if (entity_start != 5120) return;

            auto meshes = reinterpret_cast<const ludo::mesh*>(read_component_data[0].data);
            auto render_commands = reinterpret_cast<ludo::render_command*>(write_component_data[0].data);

            for (auto index = uint32_t(0); index < entity_count; index++)
            {
              auto& mesh = meshes[index];

              render_commands[index] = ludo::render_command
              {
                .index_count = mesh.indices.count,
                .index_start = mesh.indices.start,
                .instance_start = index
              };
            }
          }
        }
      }
    );

    ludo::bind_data(0, camera_positions->id);
    ludo::bind_data(1, camera_view_projections->id);
    ludo::bind_data(2, camera_views->id);
    ludo::bind_data(3, camera_projections->id);
    ludo::bind_data(4, camera_near_clipping_distances->id);
    ludo::bind_data(5, camera_far_clipping_distances->id);
    ludo::bind_data(10, light_ambient_colors->id);
    ludo::bind_data(11, light_diffuse_colors->id);
    ludo::bind_data(12, light_specular_colors->id);
    ludo::bind_data(13, light_positions->id);
    ludo::bind_data(14, light_directions->id);
    ludo::bind_data(15, light_attenuations->id);
    ludo::bind_data(16, light_strengths->id);
    ludo::bind_data(17, light_ranges->id);

    ludo::bind_data(20, sol_transforms->id);
    ludo::bind_data(21, sol_lods->id);
    ludo::bind_data(22, sol_lod_indices->id);
    ludo::bind_commands(sol_render_command_arena.id);
    ludo::bind_indices(sol_indices.id);
    ludo::bind_vertices(sol_vertices.id);
    ludo::render(star_render_program, 0, 5120);

    ludo::bind_data(20, terra_transforms->id);
    ludo::bind_data(21, terra_lods->id);
    ludo::bind_data(22, terra_lod_indices->id);
    ludo::bind_commands(terra_render_commands->id);
    ludo::bind_indices(terra_indices.id);
    ludo::bind_vertices(terra_vertices.id);
    ludo::render(terrain_render_program, 0, 5120);

    ludo::bind_data(20, luna_transforms->id);
    ludo::bind_data(21, luna_lods->id);
    ludo::bind_data(22, luna_lod_indices->id);
    ludo::bind_commands(luna_render_command_arena.id);
    ludo::bind_indices(luna_indices.id);
    ludo::bind_vertices(luna_vertices.id);
    ludo::render(terrain_render_program, 0, 5120);

    /*if (astrum::visualize_physics)
    {
      ludo::visualize(physics_context, bullet_debug_mesh);
      ludo::add_render_command(*render_program, *render_mesh);
    }*/

    ludo::blit(multi_sample_frame_buffer, single_sample_frame_buffer); // Convert MSAA textures to regular textures

    ludo::bind_commands(post_processing_command_arena.id);
    ludo::bind_indices(post_processing_indices.id);
    ludo::bind_vertices(post_processing_vertices.id);

    auto terra_positions = reinterpret_cast<const ludo::vec3*>(
      astrum::get_components(
        terra_container,
        "celestial_body",
        "position"
      )->data
    );

    ludo::cast<ludo::vec3>(atmosphere_data_buffer, 5 * sizeof(uint64_t) + 8 /* align 16 */) = terra_positions[0];

    ludo::use_and_clear(atmosphere_frame_buffer);
    ludo::bind_data(20, atmosphere_data_buffer.id);
    ludo::render(atmosphere_render_program, 0, 1);

    auto bloom_pass_index = uint32_t(0);
    ludo::use_and_clear(bloom_frame_buffers[bloom_pass_index]);
    ludo::bind_data(20, bloom_data_buffers[bloom_pass_index].id);
    ludo::render(bloom_render_programs[bloom_pass_index], 0, 1);

    for (auto iteration = 0; iteration < 5; iteration++)
    {
      bloom_pass_index++;
      ludo::use_and_clear(bloom_frame_buffers[bloom_pass_index]);
      ludo::bind_data(20, bloom_data_buffers[bloom_pass_index].id);
      ludo::render(bloom_render_programs[bloom_pass_index], 0, 1);

      bloom_pass_index++;
      ludo::use_and_clear(bloom_frame_buffers[bloom_pass_index]);
      ludo::bind_data(20, bloom_data_buffers[bloom_pass_index].id);
      ludo::render(bloom_render_programs[bloom_pass_index], 0, 1);
    }

    bloom_pass_index++;
    ludo::use_and_clear(bloom_frame_buffers[bloom_pass_index]);
    ludo::bind_data(20, bloom_data_buffers[bloom_pass_index].id);
    ludo::render(bloom_render_programs[bloom_pass_index], 0, 1);

    ludo::use_and_clear(tone_mapping_frame_buffer);
    ludo::bind_data(20, tone_mapping_data_buffer.id);
    ludo::render(tone_mapping_render_program, 0, 1);

    ludo::blit(tone_mapping_frame_buffer, ludo::frame_buffer { .width = window.width, .height = window.height });

    ludo::init(fence);

    if (inst.total_time - last_print_time > 1.0f)
    {
      std::cout << "FPS: " << frame_count << std::endl;

      last_print_time = inst.total_time;
      frame_count = 0;
    }

    frame_count++;
  });

  ludo::thread_pool_stop();
}
