#include <cmath>
#include <thread>

#include "constants.h"
#include "ecs.h"
#include "meshes/lod_shaders.h"
#include "terrain.h"
//#include "static_bodies.h" TODO
#include "terrain_chunk.h"

namespace astrum
{
  void init_terrain(ludo::container& container, ludo::buffer& indices, ludo::buffer& vertices, ludo::pool* index_pools, ludo::pool* vertex_pools, const ludo::vec3& camera_position, const terrain_funcs& terrain_funcs, const ludo::vertex_format& format, const ludo::vec3& position, float radius, const std::vector<lod>& lods)
  {
    auto lod_arena = get_components(
      container,
      "celestial_body",
      "lods_vram"
    );

    auto stream = ludo::stream(*lod_arena);
    for (auto lod_index = uint32_t(0); lod_index < lods.size(); lod_index++)
    {
      auto is_highest_detail = lod_index == lods.size() - 1;

      auto max_distance = lods[lod_index].max_distance;
      auto min_distance = is_highest_detail ? 0.0f : lods[lod_index + 1].max_distance;
      auto distance_range = max_distance - min_distance;

      auto low_detail_distance = min_distance + distance_range * 0.66f;
      auto high_detail_distance = min_distance + distance_range * 0.33f;

      ludo::write(stream, low_detail_distance);
      ludo::write(stream, high_detail_distance);
    }

    // TODO
    /*auto bounds_half_dimensions = ludo::vec3 { radius * 1.1f, radius * 1.1f, radius * 1.1f };
    auto grid = ludo::grid3
    {
      .bounds =
      {
        .min = position - bounds_half_dimensions,
        .max = position + bounds_half_dimensions
      },
      .cell_count_1d = 16
    };
    grid->compute_program_id = ludo::build_compute_program(grid).id;
    ludo::init(grid);*/

    auto pool_mutex = std::mutex();

    auto containers = std::vector { container };
    ludo::run(
      containers,
      {
        ludo::job
        {
          .read_component_names = { "local_position", "normal" },
          .write_component_names = { "mesh", "lod_index_vram" },
          .kernel = [&](uint32_t entity_start, uint32_t entity_count, const std::vector<ludo::arena>& read_component_data, std::vector<ludo::arena>& write_component_data)
          {
            auto local_positions = reinterpret_cast<ludo::vec3*>(read_component_data[0].data);
            auto normals = reinterpret_cast<ludo::vec3*>(read_component_data[1].data);
            auto meshes = reinterpret_cast<ludo::mesh*>(write_component_data[0].data);
            auto lod_indices = reinterpret_cast<uint32_t*>(write_component_data[1].data);

            for (auto index = 0; index < entity_count; index++)
            {
              auto& mesh = meshes[index];
              auto& lod_index = lod_indices[index];

              lod_index = find_lod_index(lods, camera_position, position + local_positions[index], normals[index]);

              {
                auto lock = std::lock_guard(pool_mutex);

                auto index_data = ludo::allocate(index_pools[lod_index]);
                auto vertex_data = ludo::allocate(vertex_pools[lod_index]);
                mesh.indices.start = (index_data - indices.data) / sizeof(uint32_t);
                mesh.vertices.start = (vertex_data - vertices.data) / format.size;
                mesh.vertex_size = format.size;
              }

              load_terrain_chunk(mesh, indices, vertices, terrain_funcs, radius, lods, format, index, lod_index);

              // TODO
              //ludo::add(grid, *render_mesh, position + chunk.center);
            }
          }
        }
      }
    );

    // TODO
    //ludo::commit(grid);

    // TODO
    //update_terrain_static_bodies(container, terrain_funcs, radius, position, radius * 1.25f);
  }

  std::pair<uint32_t, uint32_t> terrain_counts(const std::vector<lod>& lods)
  {
    // TODO I think this will supply waaaay more space than is needed...

    auto total = uint32_t(0);
    for (auto& lod : lods)
    {
      total += static_cast<uint32_t>(20 * 3 * std::pow(4, lod.level - 1));
    }
    auto unique = total;

    return { total, unique };
  }

  void stream_terrain(ludo::container& container, ludo::buffer& indices, ludo::buffer& vertices, ludo::pool* index_pools, ludo::pool* vertex_pools, const ludo::vec3& camera_position, const terrain_funcs& terrain_funcs, const ludo::vertex_format& format, float radius, const std::vector<lod>& lods)
  {
    auto celestial_body_positions = reinterpret_cast<const ludo::vec3*>(
      astrum::get_components(
        container,
        "celestial_body",
        "position"
      )->data
    );

    auto pool_mutex = std::mutex();

    auto containers = std::vector { container };
    ludo::run(
      containers,
      {
        ludo::job
        {
          // inclusion of 'radius' is a hack to ensure only celestial bodies are selected
          .read_component_names = { "position", "radius" },
          .write_component_names = { "transform_vram" },
          .kernel = [&](uint32_t entity_start, uint32_t entity_count, const std::vector<ludo::arena>& read_component_data, std::vector<ludo::arena>& write_component_data)
          {
            auto positions = reinterpret_cast<const ludo::vec3*>(read_component_data[0].data);
            auto transforms = reinterpret_cast<ludo::mat4*>(write_component_data[0].data);

            for (auto index = uint32_t(0); index < entity_count; index++)
            {
              auto old_position = ludo::position(transforms[index]);
              auto& position = positions[index];

              auto movement = position - old_position;
              if (ludo::length2(movement) > 0.0f)
              {
                // TODO
                /*grid.bounds.min += movement;
                grid.bounds.max += movement;
                ludo::commit_header(grid);*/
              }

              // TODO
              //update_terrain_static_bodies(container, terrain_funcs, radius, position, radius * 1.25f);
            }
          }
        },
        ludo::job
        {
          .read_component_names = { "local_position", "normal", "lod_index_vram" },
          .write_component_names = { "loading_mesh", "loading_lod_index", "loading", "loaded" },
          .kernel = [&](uint32_t entity_start, uint32_t entity_count, const std::vector<ludo::arena>& read_component_data, std::vector<ludo::arena>& write_component_data)
          {
            auto local_positions = reinterpret_cast<const ludo::vec3*>(read_component_data[0].data);
            auto normals = reinterpret_cast<const ludo::vec3*>(read_component_data[1].data);
            auto lod_indices = reinterpret_cast<const uint32_t*>(read_component_data[2].data);
            auto loading_meshes = reinterpret_cast<ludo::mesh*>(write_component_data[0].data);
            auto loading_lod_indices = reinterpret_cast<uint32_t*>(write_component_data[1].data);
            auto loadings = reinterpret_cast<bool*>(write_component_data[2].data);
            auto loadeds = reinterpret_cast<bool*>(write_component_data[3].data);

            for (auto index = uint32_t(0); index < entity_count; index++)
            {
              auto& lod_index = lod_indices[index];
              auto& loading_mesh = loading_meshes[index];
              auto& loading_lod_index = loading_lod_indices[index];
              auto& loading = loadings[index];
              auto& loaded = loadeds[index];

              if (loading || loaded) continue;

              auto new_lod_index = find_lod_index(lods, camera_position, celestial_body_positions[0] + local_positions[index], normals[index]);
              if (new_lod_index == lod_index) return; // TODO

              loading_lod_index = new_lod_index;
              loading = true;

              {
                auto lock = std::lock_guard(pool_mutex);

                auto index_data = ludo::allocate(index_pools[loading_lod_index]);
                auto vertex_data = ludo::allocate(vertex_pools[loading_lod_index]);
                loading_mesh.indices.start = (index_data - indices.data) / sizeof(uint32_t);
                loading_mesh.vertices.start = (vertex_data - vertices.data) / format.size;
                loading_mesh.vertex_size = format.size;
              }

              ludo::thread_pool_enqueue([&, index]
              {
                load_terrain_chunk(loading_mesh, indices, vertices, terrain_funcs, radius, lods, format, index, loading_lod_index);
                loading = false;
                loaded = true;
              });
            }

            // TODO not while render could be happening!!!
            //ludo::commit(grid);
          }
        },
        ludo::job
        {
          .write_component_names = { "mesh", "loading_mesh", "lod_index_vram", "loading_lod_index", "loaded" },
          .kernel = [&](uint32_t entity_start, uint32_t entity_count, const std::vector<ludo::arena>& read_component_data, std::vector<ludo::arena>& write_component_data)
          {
            auto meshes = reinterpret_cast<ludo::mesh*>(write_component_data[0].data);
            auto loading_meshes = reinterpret_cast<ludo::mesh*>(write_component_data[1].data);
            auto lod_indices = reinterpret_cast<uint32_t*>(write_component_data[2].data);
            auto loading_lod_indices = reinterpret_cast<uint32_t*>(write_component_data[3].data);
            auto loadeds = reinterpret_cast<bool*>(write_component_data[4].data);

            for (auto index = uint32_t(0); index < entity_count; index++)
            {
              auto& mesh = meshes[index];
              auto& loading_mesh = loading_meshes[index];
              auto& lod_index = lod_indices[index];
              auto& loading_lod_index = loading_lod_indices[index];
              auto& loaded = loadeds[index];

              if (!loaded) continue;

              {
                auto lock = std::lock_guard(pool_mutex);

                auto index_data = indices.data + mesh.indices.start * sizeof(uint32_t);
                auto vertex_data = vertices.data + mesh.vertices.start * mesh.vertex_size;
                ludo::free(index_pools[lod_index], index_data);
                ludo::free(vertex_pools[lod_index], vertex_data);

                mesh = loading_mesh;
                loading_mesh = ludo::mesh();
                lod_index = loading_lod_index; // TODO this could write to VRAM while rendering...
                loading_lod_index = 0;
                loaded = false;

                // TODO
                /*ludo::remove(grid, *render_mesh, point_mass.transform.position + chunk.center);
                ludo::add(grid, *render_mesh, point_mass.transform.position + chunk.center);*/

                // TODO not while render could be happening!!!
                //ludo::commit(grid);
              }
            }
          }
        }
      }
    );
  }
}
