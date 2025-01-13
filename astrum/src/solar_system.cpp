#include <ludo/api.h>
#include <ludo/bullet/api.h>

#include "constants.h"
#include "containers.h"
#include "controllers/game.h"
#include "ecs.h"
#include "entities/luna.h"
#include "entities/people.h"
#include "entities/sol.h"
#include "entities/spaceships.h"
#include "entities/terra.h"
#include "entities/trees.h"
#include "meshes/lods.h"
#include "paths.h"
#include "physics/centering.h"
#include "physics/gravity.h"
#include "physics/point_masses.h"
#include "physics/relativity.h"
#include "physics/util.h"
#include "solar_system.h"
#include "terrain/terrain.h"

namespace astrum
{
  void add_solar_system(ludo::instance& inst, btDynamicsWorld* bullet_world, ludo::pool* sol_index_pools, ludo::pool* sol_vertex_pools, ludo::pool* terra_index_pools, ludo::pool* terra_vertex_pools, ludo::pool* luna_index_pools, ludo::pool* luna_vertex_pools, const ludo::import_results& minifig, const ludo::import_results& spaceship)
  {
    // TODO
    /*auto tree_collapse_iterations = std::vector<uint32_t> { 200, 50, 12 };
    auto tree_lod_meshes = std::array<std::vector<ludo::mesh>, tree_type_count>();
    if (import_assets)
    {
      for (auto tree_type_index = uint32_t(0); tree_type_index < tree_types.size(); tree_type_index++)
      {
        auto tree_import = ludo::import(ludo::asset_folder + "/models/" + tree_types[tree_type_index] + "-tree.dae", { .merge_meshes = true });
        tree_lod_meshes[tree_type_index] = build_lod_meshes(tree_import.meshes[0], ludo::vertex_format_pnc, tree_collapse_iterations);
        std::reverse(tree_lod_meshes[tree_type_index].begin(), tree_lod_meshes[tree_type_index].end());
        for (auto lod_index = uint32_t(0); lod_index < tree_collapse_iterations.size(); lod_index++)
        {
          ludo::save(tree_lod_meshes[tree_type_index][lod_index], ludo::asset_folder + "/meshes/" + tree_types[tree_type_index] + "-tree-" + std::to_string(lod_index) + ".lmesh");
        }
      }
    }
    else
    {
      for (auto tree_type_index = uint32_t(0); tree_type_index < tree_types.size(); tree_type_index++)
      {
        for (auto lod_index = uint32_t(0); lod_index < tree_collapse_iterations.size(); lod_index++)
        {
          tree_lod_meshes[tree_type_index].emplace_back(ludo::load(ludo::asset_folder + "/meshes/" + tree_types[tree_type_index] + "-tree-" + std::to_string(lod_index) + ".lmesh"));
        }
      }
    }

    for (auto tree_type_index = uint32_t(0); tree_type_index < tree_types.size(); tree_type_index++)
    {
      for (auto lod_index = uint32_t(0); lod_index < tree_collapse_iterations.size(); lod_index++)
      {
        ludo::add(inst, tree_lod_meshes[tree_type_index][lod_index], tree_types[tree_type_index] + "-trees");
      }
    }*/

    // TODO
    /*auto person_surface_position = ludo::vec3 { 4138.0f, -3054.0f, 3800.0f };
    const auto person_initial_position = terra_initial_position + person_surface_position;
    const auto person_initial_velocity = ludo::vec3 { 0.0f, orbital_speed(ludo::length(person_initial_position), sol_mass), 0.0f };

    const auto spaceship_initial_position = terra_initial_position + ludo::vec3 { 4154.0f, -3046.0f, 3785.0f };
    const auto spaceship_initial_velocity = ludo::vec3 { 0.0f, orbital_speed(ludo::length(spaceship_initial_position), sol_mass), 0.0f };*/

    // TODO
    //add_trees(inst, 1);

    // TODO
    /*auto person_render_program = ludo::render_program();
    ludo::init(person_render_program, ludo::format(true, true, true, true));

    auto grid = ludo::first<ludo::grid3>(inst, "default");
    ludo::add(*grid, *render_mesh, initial_transform.position);

    auto person_kinematic_body = new btRigidBody(0.0f, nullptr, nullptr);
    person_kinematic_body->setCollisionFlags(person_kinematic_body->getCollisionFlags() | btCollisionObject::CF_KINEMATIC_OBJECT);
    person_kinematic_body->setActivationState(DISABLE_DEACTIVATION);
    bullet_world->addRigidBody(person_kinematic_body);

    ludo::add(
      container,
      "person",
      person_initial_position,
      ludo::quat_identity,
      ludo::mat4_identity,
      0.001f * gravitational_constant,
      person_initial_velocity,
      false,
      person_kinematic_body,
      person { .turn_angle = ludo::pi },
      person_controls { .camera_rotation = { 0, ludo::pi } }
    );*/

    // TODO
    /*auto spaceship_render_program = ludo::render_program();
    ludo::init(spaceship_render_program, ludo::vertex_format_pn);

    auto grid = ludo::first<ludo::grid3>(inst, "default");
    ludo::add(*grid, *render_mesh, initial_transform.position);

    auto spaceship_up = spaceship_initial_position - terra_initial_position;
    ludo::normalize(spaceship_up);

    auto spaceship_kinematic_body = new btRigidBody(0.0f, nullptr, nullptr);
    spaceship_kinematic_body->setCollisionFlags(person_kinematic_body->getCollisionFlags() | btCollisionObject::CF_KINEMATIC_OBJECT);
    spaceship_kinematic_body->setActivationState(DISABLE_DEACTIVATION);
    bullet_world->addRigidBody(spaceship_kinematic_body);

    auto spaceship_ghost_body = new btRigidBody(0.0f, nullptr, nullptr);
    spaceship_ghost_body->setCollisionFlags(spaceship_ghost_body->getCollisionFlags() | btCollisionObject::CF_KINEMATIC_OBJECT);
    spaceship_ghost_body->setActivationState(DISABLE_DEACTIVATION);
    spaceship_ghost_body->setCollisionFlags(spaceship_ghost_body->getCollisionFlags() | btCollisionObject::CF_NO_CONTACT_RESPONSE);
    bullet_world->addRigidBody(spaceship_ghost_body);

    ludo::add(
      container,
      "spaceship",
      spaceship_initial_position,
      ludo::quat(ludo::vec3_unit_y, spaceship_up) * ludo::quat(ludo::vec3_unit_y, ludo::pi * 1.5f),
      ludo::mat4_identity,
      0.1f * gravitational_constant,
      spaceship_initial_velocity,
      false,
      spaceship_kinematic_body,
      spaceship_ghost_body,
      spaceship_controls()
    );*/

    // TODO
    /*ludo::run(
      container,
      {
        ludo::job
        {
          .read_component_names = { "astrum::person_controls" }, // hack to ensure only people
          .write_component_names = { "ludo::kinematic_body" },
          .kernel = [&](uint32_t entity_start, uint32_t entity_count, const std::vector<ludo::arena>& read_component_data, std::vector<ludo::arena>& write_component_data)
          {
            auto kinematic_bodies_start = reinterpret_cast<ludo::kinematic_body*>(write_component_data[0].start);

            for (auto index = 0; index < entity_count; index++)
            {
              ludo::init(kinematic_bodies_start[index], physics_context);
              ludo::connect(kinematic_bodies_start[index], physics_context, { minifig.dynamic_body_shapes[0] });
            }
          }
        },
        ludo::job
        {
          .read_component_names = { "astrum::spaceship_controls" }, // hack to ensure only spaceships
          .write_component_names = { "ludo::kinematic_body" },
          .kernel = [&](uint32_t entity_start, uint32_t entity_count, const std::vector<ludo::arena>& read_component_data, std::vector<ludo::arena>& write_component_data)
          {
            auto kinematic_bodies_start = reinterpret_cast<ludo::kinematic_body*>(write_component_data[0].start);

            for (auto index = 0; index < entity_count; index++)
            {
              ludo::init(kinematic_bodies_start[index], physics_context);
              ludo::connect(kinematic_bodies_start[index], physics_context, { spaceship.dynamic_body_shapes[1] });
            }
          }
        },
        ludo::job
        {
          .read_component_names = { "astrum::spaceship_controls" }, // hack to ensure only spaceships
          .write_component_names = { "ludo::ghost_body" },
          .kernel = [&](uint32_t entity_start, uint32_t entity_count, const std::vector<ludo::arena>& read_component_data, std::vector<ludo::arena>& write_component_data)
          {
            auto ghost_bodies_start = reinterpret_cast<ludo::ghost_body*>(write_component_data[0].start);

            for (auto index = 0; index < entity_count; index++)
            {
              ludo::init(ghost_bodies_start[index], physics_context);
              ludo::connect(ghost_bodies_start[index], physics_context, { spaceship.dynamic_body_shapes[0] });
            }
          }
        }
      }
    );*/

    // TODO
    /*if (show_paths)
    {
      add_prediction_paths(
        inst,
        {
          { 1.0f, 1.0f, 0.0f, 1.0f },
          { 0.0f, 0.5f, 1.0f, 1.0f },
          { 1.0f, 1.0f, 1.0f, 1.0f },
          { 1.0f, 0.0f, 0.0f, 1.0f },
          { 0.0f, 1.0f, 0.0f, 1.0f }
        }
      );
    }*/
  }

  struct empty_lod
  {
    float a;
    float b;
    float c;
    float d;
    float e;
    float f;
    float g;
    float h;
    float i;
    float j;
  };

  ludo::container build_celestial_body(
    const std::string& name,
    const ludo::vec3& position,
    float mass,
    const ludo::vec3& linear_velocity,
    float radius,
    ludo::buffer& indices,
    ludo::buffer& vertices,
    ludo::pool* index_pools,
    ludo::pool* vertex_pools,
    const ludo::vec3& camera_position,
    const ludo::vertex_format& format,
    const std::vector<lod>& lods,
    const terrain_funcs& funcs
  )
  {
    auto container = build_container(name, radius, lods, funcs);

    ludo::add(
      container,
      "celestial_body",
      position,
      ludo::quat_identity,
      ludo::mat4_identity,
      mass,
      linear_velocity,
      ludo::vec3_zero,
      false,
      radius,
      empty_lod()
    );

    init_terrain(container, indices, vertices, index_pools, vertex_pools, camera_position, funcs, format, position, radius, lods);

    return container;
  }
}
