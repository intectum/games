#include "../constants.h"
#include "mesh.h"
#include "sphere_ico.h"
#include "static_bodies.h"

namespace astrum
{
  void update_terrain_static_bodies(ludo::instance& inst, terrain& terrain, float radius, const ludo::vec3& position, float point_mass_max_distance)
  {
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
      if (!terrain.static_body_ids.contains(section.first))
      {
        auto mesh_count = 3 * static_cast<uint32_t>(std::pow(4, most_detailed_lod.level - second_most_detailed_lod.level));
        auto mesh = ludo::add(
          inst,
          ludo::mesh(),
          "celestial-bodies"
        );

        mesh->index_buffer = ludo::allocate(mesh_count * sizeof(uint32_t));
        mesh->vertex_buffer = ludo::allocate(mesh_count * ludo::vertex_format_p.size);

        auto chunks_per_ico_face = static_cast<uint32_t>(std::pow(4, second_most_detailed_lod.level));
        auto index = static_cast<uint32_t>(static_cast<float>(section.first) / static_cast<float>(chunks_per_ico_face));

        terrain_mesh(terrain, radius, *mesh, ludo::vertex_format_p, ludo::vertex_format_p, index, false, 0, most_detailed_lod.level - second_most_detailed_lod.level, most_detailed_lod.level - second_most_detailed_lod.level, section.second);

        auto body = ludo::add(
          inst,
          ludo::static_body{ { .transform = { .position = position } } },
          "celestial-bodies"
        );

        auto build_shape = ludo::build_shape(inst, *body, *mesh, ludo::vertex_format_p, 0, mesh->index_buffer.size / sizeof(uint32_t));
        ludo::execute(build_shape);

        terrain.static_body_ids[section.first] = body->id;
        terrain.static_body_mesh_ids[section.first] = mesh->id;
      }
    }

    for (auto static_body_iter = terrain.static_body_ids.begin(); static_body_iter != terrain.static_body_ids.end();)
    {
      if (!sections.contains(static_body_iter->first))
      {
        auto mesh = ludo::get<ludo::mesh>(inst, "celestial-bodies", terrain.static_body_mesh_ids[static_body_iter->first]);

        ludo::deallocate(mesh->index_buffer);
        ludo::deallocate(mesh->vertex_buffer);

        ludo::remove(inst, mesh, "celestial-bodies");
        terrain.static_body_mesh_ids.erase(static_body_iter->first);

        auto static_body = ludo::get<ludo::static_body>(inst, "celestial-bodies", static_body_iter->second);
        ludo::remove(inst, static_body, "celestial-bodies");
        static_body_iter = terrain.static_body_ids.erase(static_body_iter);
      }
      else
      {
        static_body_iter++;
      }
    }
  }
}
