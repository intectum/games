#pragma once

#include <btBulletDynamicsCommon.h>

#include <ludo/api.h>

#include "types.h"

namespace astrum
{
  void add_solar_system(ludo::instance& inst, btDynamicsWorld* bullet_world, ludo::pool* sol_index_pools, ludo::pool* sol_vertex_pools, ludo::pool* terra_index_pools, ludo::pool* terra_vertex_pools, ludo::pool* luna_index_pools, ludo::pool* luna_vertex_pools, const ludo::import_results& minifig, const ludo::import_results& spaceship);

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
  );
}
