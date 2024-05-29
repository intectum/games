#include "../constants.h"
#include "mesh.h"
#include "sphere_ico.h"
#include "static_bodies.h"

namespace astrum
{
  void update_terrain_static_bodies(ludo::instance& inst, terrain& terrain, float radius, const ludo::vec3& position, float point_mass_max_distance)
  {
    auto physics_context = ludo::first<ludo::physics_context>(inst);

    auto& indices = ludo::data_heap(inst, "ludo::vram_indices");
    auto& vertices = ludo::data_heap(inst, "ludo::vram_vertices");

    auto& point_masses = ludo::data<point_mass>(inst);

    auto& most_detailed_lod = terrain.lods[terrain.lods.size() - 1];
    auto& second_most_detailed_lod = terrain.lods[terrain.lods.size() - 2];

    auto test_positions = std::vector<ludo::vec3>();
    for (auto& point_mass : point_masses)
    {
      if (point_mass.mass > 100.0f * gravitational_constant)
      {
        continue;
      }

      auto relative_position = point_mass.transform.position - position;
      if (ludo::length(relative_position) > point_mass_max_distance)
      {
        continue;
      }

      ludo::normalize(relative_position);
      test_positions.push_back(relative_position);
    }

    auto sections = find_sphere_ico_chunks(second_most_detailed_lod.level, [&](const std::array<ludo::vec3, 3>& triangle)
    {
      auto center = (triangle[0] + triangle[1] + triangle[2]) / 3.0f;
      auto range = ludo::length(triangle[1] - triangle[0]);

      return std::any_of(test_positions.begin(), test_positions.end(), [&center, range](const ludo::vec3& test_position)
      {
        return ludo::length(test_position - center) < range;
      });
    });

    for (auto& section : sections)
    {
      if (terrain.static_body_ids.contains(section.first))
      {
        continue;
      }

      auto mesh_count = 3 * static_cast<uint32_t>(std::pow(4, most_detailed_lod.level - second_most_detailed_lod.level));
      auto static_body_mesh = ludo::add(inst, ludo::mesh(), "celestial-bodies");
      static_body_mesh->id = ludo::next_id++; // TODO!
      static_body_mesh->index_buffer = ludo::allocate(mesh_count * sizeof(uint32_t));
      static_body_mesh->vertex_buffer = ludo::allocate(mesh_count * ludo::vertex_format_p.size);

      auto chunks_per_ico_face = static_cast<uint32_t>(std::pow(4, second_most_detailed_lod.level));
      auto index = static_cast<uint32_t>(static_cast<float>(section.first) / static_cast<float>(chunks_per_ico_face));

      terrain_mesh(terrain, radius, *static_body_mesh, ludo::vertex_format_p, ludo::vertex_format_p, false, index, 0, most_detailed_lod.level - second_most_detailed_lod.level, most_detailed_lod.level - second_most_detailed_lod.level, section.second);

      auto static_body = ludo::add(inst, ludo::static_body { .transform = { .position = position } }, "celestial-bodies");
      ludo::init(*static_body, *physics_context);
      ludo::connect(*static_body, *physics_context, *static_body_mesh, ludo::vertex_format_p);

      terrain.static_body_ids[section.first] = static_body->id;
      terrain.static_body_mesh_ids[section.first] = static_body_mesh->id;
    }

    for (auto static_body_iter = terrain.static_body_ids.begin(); static_body_iter != terrain.static_body_ids.end();)
    {
      if (sections.contains(static_body_iter->first))
      {
        static_body_iter++;
        continue;
      }

      auto static_body_mesh = ludo::get<ludo::mesh>(inst, "celestial-bodies", terrain.static_body_mesh_ids[static_body_iter->first]);
      ludo::deallocate(static_body_mesh->index_buffer);
      ludo::deallocate(static_body_mesh->vertex_buffer);
      ludo::de_init(*static_body_mesh, indices, vertices);
      ludo::remove(inst, static_body_mesh, "celestial-bodies");
      terrain.static_body_mesh_ids.erase(static_body_iter->first);

      auto static_body = ludo::get<ludo::static_body>(inst, "celestial-bodies", static_body_iter->second);
      ludo::de_init(*static_body, *physics_context);
      ludo::remove(inst, static_body, "celestial-bodies");
      static_body_iter = terrain.static_body_ids.erase(static_body_iter);
    }
  }
}
