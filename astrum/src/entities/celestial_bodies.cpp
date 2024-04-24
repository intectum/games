#include <fstream>
#include <map>
#include <sstream>

#include "ludo/opengl/util.h"

#include "../constants.h"
#include "celestial_bodies.h"
#include "meshes/patchwork.h"
#include "meshes/sphere_ico.h"

namespace astrum
{
  void build_variants(const celestial_body& celestial_body, patchwork& patchwork);
  void build_patches(const celestial_body& celestial_body, patchwork& patchwork);
  std::vector<ludo::vec3> build_positions(uint32_t index, uint32_t patch_divisions, uint32_t divisions);
  void raw_positions(std::vector<ludo::vec3>& positions, const std::array<ludo::vec3, 3>& corner_positions, uint32_t divisions);
  int32_t nearest_celestial_body_index(const ludo::vec3& target_position, ludo::array_buffer<point_mass>& point_masses);
  std::function<uint32_t(const patchwork& patchwork, uint32_t patch_index)> build_variant_index(const celestial_body& celestial_body, const ludo::vec3& camera_position, const ludo::vec3& position);
  void load_patch(ludo::instance& inst, const celestial_body& celestial_body, const patchwork& patchwork, uint32_t patch_index, uint32_t variant_index, ludo::mesh& mesh);
  void sew_patch(ludo::instance& inst, const celestial_body& celestial_body, const patchwork& patchwork, uint32_t anchor_patch_index, uint32_t patch_index);
  const std::vector<uint32_t>& border_indices(const patchwork& patchwork, uint32_t patch_index, uint32_t adjacent_patch_index);
  void update_static_bodies(ludo::instance& inst, celestial_body& celestial_body, const patchwork& patchwork);

  void relativize_to_nearest_celestial_body(ludo::instance& inst)
  {
    auto& all_dynamic_bodies = ludo::data<ludo::dynamic_body>(inst);
    auto& rendering_context = *ludo::first<ludo::rendering_context>(inst);
    auto camera = ludo::get_camera(rendering_context);

    auto& all_point_masses = ludo::data<point_mass>(inst);
    auto& solar_system = *ludo::first<astrum::solar_system>(inst);

    auto& point_masses = ludo::data<point_mass>(inst, "celestial-bodies");

    auto new_relative_index = nearest_celestial_body_index(ludo::position(camera.view), point_masses);
    if (new_relative_index == solar_system.relative_celestial_body_index)
    {
      return;
    }

    solar_system.relative_celestial_body_index = new_relative_index;

    auto target_linear_velocity = point_masses[solar_system.relative_celestial_body_index].linear_velocity;

    for (auto& body: all_dynamic_bodies)
    {
      ludo::pull(body);
      body.linear_velocity -= target_linear_velocity;
      ludo::push(body);
    }

    for (auto& point_mass: all_point_masses)
    {
      point_mass.linear_velocity -= target_linear_velocity;
    }
  }

  void add_celestial_body(ludo::instance& inst, const celestial_body& init, const ludo::transform& initial_transform, const ludo::vec3& initial_velocity)
  {
    auto& rendering_context = *ludo::first<ludo::rendering_context>(inst);

    auto& meshes = ludo::data<ludo::mesh>(inst, "celestial-bodies");

    // Indexes are stable in this case since celestial bodies are only added to a single partition and never removed.
    auto index = ludo::data<celestial_body>(inst, "celestial-bodies").array_size;
    auto celestial_body = ludo::add(inst, init, "celestial-bodies");

    auto camera = ludo::get_camera(rendering_context);
    auto camera_position = ludo::position(camera.view);

    auto counts = [init](const patchwork& patchwork, uint32_t patch_index, uint32_t variant_index)
    {
      auto count = 3 * static_cast<uint32_t>(std::pow(4, init.lods[variant_index].level - init.lods[0].level));
      return std::pair<uint32_t, uint32_t> { count, count };
    };

    auto load = [&inst, index](const patchwork& patchwork, uint32_t patch_index, uint32_t variant_index, ludo::mesh& mesh)
    {
      auto& celestial_body = ludo::data<astrum::celestial_body>(inst, "celestial-bodies")[index];

      load_patch(inst, celestial_body, patchwork, patch_index, variant_index, mesh);
    };

    auto sew = [&inst, index](const patchwork& patchwork, uint32_t anchor_patch_index, uint32_t patch_index)
    {
      auto& celestial_body = ludo::data<astrum::celestial_body>(inst, "celestial-bodies")[index];

      sew_patch(inst, celestial_body, patchwork, anchor_patch_index, patch_index);
    };

    auto on_load = [&inst, index](const patchwork& patchwork, uint32_t patch_index)
    {
      auto& linear_octree = ludo::data<ludo::linear_octree>(inst, "celestial-bodies")[index];

      auto& patch = patchwork.patches[patch_index];
      auto& mesh_instance = *ludo::get<ludo::mesh_instance>(inst, patch.mesh_instance_id);

      ludo::add(linear_octree, mesh_instance, patchwork.transform.position + patch.center);
    };

    auto on_unload = [&inst, index](const patchwork& patchwork, uint32_t patch_index)
    {
      auto& linear_octree = ludo::data<ludo::linear_octree>(inst, "celestial-bodies")[index];

      auto& patch = patchwork.patches[patch_index];
      auto& mesh_instance = *ludo::get<ludo::mesh_instance>(inst, patch.mesh_instance_id);

      ludo::remove(linear_octree, mesh_instance, patchwork.transform.position + patch.center);
    };

    auto render_program = ludo::add(inst, ludo::render_program(), celestial_body->format, "celestial-bodies");

    auto bounds_half_dimensions = ludo::vec3 { celestial_body->radius * 1.1f, celestial_body->radius * 1.1f, celestial_body->radius * 1.1f };
    ludo::add(
      inst,
      ludo::linear_octree
      {
        .bounds =
        {
          .min = initial_transform.position - bounds_half_dimensions,
          .max = initial_transform.position + bounds_half_dimensions
        },
        .depth = 4
      },
      "celestial-bodies"
    );

    auto patchwork_init = astrum::patchwork
    {
      .render_program_id = render_program->id,
      .transform = initial_transform,
      .variant_index = build_variant_index(*celestial_body, camera_position, initial_transform.position),
      .counts = counts,
      .load = load,
      .sew = sew,
      .on_load = on_load,
      .on_unload = on_unload
    };

    auto patchwork_file_name = std::stringstream();
    patchwork_file_name << "assets/meshes/" << celestial_body->name << ".patchwork";

    auto pmesh_istream = std::ifstream(patchwork_file_name.str(), std::ios::binary);
    if (pmesh_istream.is_open())
    {
      astrum::load(patchwork_init, pmesh_istream);
    }
    else
    {
      build_variants(*celestial_body, patchwork_init);
      build_patches(*celestial_body, patchwork_init);

      auto pmesh_ostream = std::ofstream(patchwork_file_name.str(), std::ios::binary);
      save(patchwork_init, pmesh_ostream);
    }

    auto patchwork = add(inst, patchwork_init, "celestial-bodies");

    ludo::add(
      inst,
      astrum::point_mass
      {
        .mass = celestial_body->mass,
        .transform = initial_transform,
        .linear_velocity = initial_velocity
      },
      "celestial-bodies"
    );

    update_static_bodies(inst, *celestial_body, *patchwork);
  }

  void update_celestial_bodies(ludo::instance& inst)
  {
    auto& rendering_context = *ludo::first<ludo::rendering_context>(inst);

    auto& linear_octrees = ludo::data<ludo::linear_octree>(inst, "celestial-bodies");

    auto& celestial_bodies = ludo::data<celestial_body>(inst, "celestial-bodies");
    auto& patchworks = ludo::data<patchwork>(inst, "celestial-bodies");
    auto& point_masses = ludo::data<point_mass>(inst, "celestial-bodies");

    auto camera = ludo::get_camera(rendering_context);
    auto camera_position = ludo::position(camera.view);

    ludo::divide_and_conquer(celestial_bodies.array_size, [&](uint32_t start, uint32_t end)
    {
      for (auto index = start; index < end; index++)
      {
        auto& patchwork = patchworks[index];
        auto& point_mass = point_masses[index];

        auto movement = point_mass.transform.position - patchwork.transform.position;
        if (ludo::length2(movement) > 0.0f)
        {
          ludo::move(linear_octrees[index], movement);
        }

        patchwork.transform = point_mass.transform;
        patchwork.variant_index = build_variant_index(celestial_bodies[index], camera_position, patchwork.transform.position);
      }

      return []() {};
    });

    for (auto index = 0; index < celestial_bodies.array_size; index++)
    {
      update_static_bodies(inst, celestial_bodies[index], patchworks[index]);
    }
  }

  std::pair<uint32_t, uint32_t> celestial_body_counts(const std::vector<lod>& lods)
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

  void build_variants(const celestial_body& celestial_body, patchwork& patchwork)
  {
    auto& lowest_detail_lod = celestial_body.lods[0];
    patchwork.variants.reserve(celestial_body.lods.size());

    for (auto& lod : celestial_body.lods)
    {
      auto positions = std::vector<ludo::vec3>();
      raw_positions(positions, { ludo::vec3_zero, ludo::vec3_unit_x, ludo::vec3_unit_y }, lod.level - lowest_detail_lod.level);

      auto border_0_indices = std::vector<uint32_t>();
      auto border_2_indices = std::vector<uint32_t>();
      for (auto index = 0; index < positions.size(); index++)
      {
        if (positions[index][1] == 0.0f)
        {
          border_0_indices.emplace_back(index);
        }

        if (positions[index][0] == 0.0f)
        {
          border_2_indices.emplace_back(index);
        }
      }

      std::sort(border_0_indices.begin(), border_0_indices.end(), [&positions](uint32_t index_a, uint32_t index_b)
      {
        return positions[index_a][0] < positions[index_b][0];
      });

      std::sort(border_2_indices.begin(), border_2_indices.end(), [&positions](uint32_t index_a, uint32_t index_b)
      {
        return positions[index_a][1] < positions[index_b][1];
      });

      positions = std::vector<ludo::vec3>();
      raw_positions(positions, { ludo::vec3_unit_y, ludo::vec3_zero, ludo::vec3_unit_x }, lod.level - lowest_detail_lod.level);

      auto border_1_indices = std::vector<uint32_t>();
      for (auto index = 0; index < positions.size(); index++)
      {
        if (positions[index][1] == 0.0f)
        {
          border_1_indices.emplace_back(index);
        }
      }

      std::sort(border_1_indices.begin(), border_1_indices.end(), [&positions](uint32_t index_a, uint32_t index_b)
      {
        return positions[index_a][0] < positions[index_b][0];
      });

      auto first_unique_indices = std::vector<uint32_t>();
      first_unique_indices.reserve(positions.size());
      auto first_unique_map = std::map<ludo::vec3, uint32_t>();

      for (auto index = 0; index < positions.size(); index++)
      {
        auto& position = positions[index];

        if (!first_unique_map.contains(position))
        {
          first_unique_map[position] = index;
        }

        first_unique_indices.emplace_back(first_unique_map[position]);
      }

      patchwork.variants.emplace_back(patch_variant
      {
        .border_indices = { border_0_indices, border_1_indices, border_2_indices },
        .first_unique_indices = first_unique_indices
      });
    }
  }

  void build_patches(const celestial_body& celestial_body, patchwork& patchwork)
  {
    auto& lowest_detail_lod = celestial_body.lods[0];
    auto positions = std::unordered_map<uint32_t, std::vector<ludo::vec3>>();

    auto patch_count = 20 * static_cast<uint32_t>(std::pow(4, lowest_detail_lod.level - 1));
    patchwork.patches.reserve(patch_count);

    for (auto patch_index = uint32_t(0); patch_index < patch_count; patch_index++)
    {
      if (!positions.contains(patch_index))
      {
        positions[patch_index] = build_positions(patch_index, lowest_detail_lod.level, lowest_detail_lod.level);
      }

      auto patch_positions = positions[patch_index];
      auto final_patch_positions = std::array<ludo::vec3, 3>
      {
        patch_positions[0] * celestial_body.height_func(patch_positions[0]) * celestial_body.radius,
        patch_positions[1] * celestial_body.height_func(patch_positions[1]) * celestial_body.radius,
        patch_positions[2] * celestial_body.height_func(patch_positions[2]) * celestial_body.radius
      };

      auto normal = ludo::cross(final_patch_positions[1] - final_patch_positions[0], final_patch_positions[2] - final_patch_positions[0]);
      ludo::normalize(normal);

      auto& patch = patchwork.patches.emplace_back(astrum::patch
      {
        .center = (final_patch_positions[0] + final_patch_positions[1] + final_patch_positions[2]) / 3.0f,
        .normal = normal
      });

      patch.adjacent_patch_indices.reserve(patch_positions.size());

      for (auto position_index = 0; position_index < patch_positions.size(); position_index++)
      {
        auto position_0 = patch_positions[position_index];
        auto position_1 = patch_positions[(position_index + 1) % patch_positions.size()];

        for (auto other_patch_index = 0; other_patch_index < patch_count; other_patch_index++)
        {
          if (other_patch_index == patch_index)
          {
            continue;
          }

          if (!positions.contains(other_patch_index))
          {
            positions[other_patch_index] = build_positions(other_patch_index, lowest_detail_lod.level, lowest_detail_lod.level);
          }

          auto other_patch_positions = positions[other_patch_index];

          auto position_0_match = ludo::near(position_0, other_patch_positions[0]) ||
                                  ludo::near(position_0, other_patch_positions[1]) ||
                                  ludo::near(position_0, other_patch_positions[2]);

          auto position_1_match = ludo::near(position_1, other_patch_positions[0]) ||
                                  ludo::near(position_1, other_patch_positions[1]) ||
                                  ludo::near(position_1, other_patch_positions[2]);

          if (position_0_match && position_1_match)
          {
            patch.adjacent_patch_indices.emplace_back(other_patch_index);
          }
        }

        assert(patch.adjacent_patch_indices.size() == position_index + 1 && "adjacent patch not found");
      }
    }
  }

  std::vector<ludo::vec3> build_positions(uint32_t index, uint32_t patch_divisions, uint32_t divisions)
  {
    auto patch_count = 20 * static_cast<uint32_t>(std::pow(4, patch_divisions - 1));
    auto vertex_count = 20 * 3 * static_cast<uint32_t>(std::pow(4, divisions - 1)) / patch_count;
    auto patch_positions = std::vector<ludo::vec3>(vertex_count);

    auto temp_mesh = ludo::mesh
    {
      .index_buffer = ludo::allocate(vertex_count * sizeof(uint32_t)),
      .vertex_buffer = ludo::allocate(vertex_count * sizeof(ludo::vec3))
    };

    ico_section(temp_mesh, ludo::vertex_format_p, 0, index, patch_divisions, divisions, 0);
    std::memcpy(patch_positions.data(), temp_mesh.vertex_buffer.data, vertex_count * sizeof(ludo::vec3));

    ludo::deallocate(temp_mesh.index_buffer);
    ludo::deallocate(temp_mesh.vertex_buffer);

    return patch_positions;
  }

  void raw_positions(std::vector<ludo::vec3>& positions, const std::array<ludo::vec3, 3>& corner_positions, uint32_t divisions)
  {
    if (divisions == 0)
    {
      positions.emplace_back(corner_positions[0]);
      positions.emplace_back(corner_positions[1]);
      positions.emplace_back(corner_positions[2]);

      return;
    }

    auto position_01 = (corner_positions[0] + corner_positions[1]) * 0.5f;
    auto position_02 = (corner_positions[0] + corner_positions[2]) * 0.5f;
    auto position_12 = (corner_positions[1] + corner_positions[2]) * 0.5f;

    raw_positions(positions, { corner_positions[0], position_01, position_02 }, divisions - 1);
    raw_positions(positions, { position_01, corner_positions[1], position_12 }, divisions - 1);
    raw_positions(positions, { position_02, position_12, corner_positions[2] }, divisions - 1);
    raw_positions(positions, { position_01, position_12, position_02 }, divisions - 1);
  }

  int32_t nearest_celestial_body_index(const ludo::vec3& target_position, ludo::array_buffer<point_mass>& point_masses)
  {
    auto central_index = int32_t(-1);
    auto shortest_length2 = std::numeric_limits<float>::max();
    for (auto index = 0; index < point_masses.array_size; index++)
    {
      auto relative_length2 = length2(point_masses[index].transform.position - target_position);
      if (relative_length2 < shortest_length2)
      {
        shortest_length2 = relative_length2;
        central_index = index;
      }
    }

    return central_index;
  }

  std::function<uint32_t(const patchwork& patchwork, uint32_t patch_index)> build_variant_index(const celestial_body& celestial_body, const ludo::vec3& camera_position, const ludo::vec3& position)
  {
    return [celestial_body, camera_position, position](const patchwork& patchwork, uint32_t patch_index)
    {
      auto& patch = patchwork.patches[patch_index];

      auto to_camera = camera_position - (position + patch.center);
      auto to_camera_unit = to_camera;
      ludo::normalize(to_camera_unit);

      if (ludo::dot(to_camera_unit, patch.normal) < -0.5f)
      {
        return uint32_t(0);
      }

      auto distance_from_camera = ludo::length(to_camera);

      for (auto variant_index = uint32_t(celestial_body.lods.size() - 1); variant_index < celestial_body.lods.size(); variant_index--)
      {
        if (celestial_body.lods[variant_index].max_distance == 0.0f || distance_from_camera < celestial_body.lods[variant_index].max_distance)
        {
          return variant_index;
        }
      }

      assert(false && "variant not found");

      return uint32_t(0);
    };
  }

  void load_patch(ludo::instance& inst, const celestial_body& celestial_body, const patchwork& patchwork, uint32_t patch_index, uint32_t variant_index, ludo::mesh& mesh)
  {
    auto& variant = patchwork.variants[variant_index];
    auto& lowest_detail_lod = celestial_body.lods[0];
    auto& lod = celestial_body.lods[variant_index];
    auto index_count = mesh.index_buffer.size / sizeof(uint32_t);
    auto heights = std::vector<float>(index_count);

    ico_section(mesh, celestial_body.format, 0, patch_index, lowest_detail_lod.level, lod.level, celestial_body.format.position_offset);

    for (auto index = 0; index < index_count; index += 3)
    {
      auto index_0 = ludo::read<uint32_t>(mesh.index_buffer, index * sizeof(uint32_t));
      auto index_1 = ludo::read<uint32_t>(mesh.index_buffer, (index + 1) * sizeof(uint32_t));
      auto index_2 = ludo::read<uint32_t>(mesh.index_buffer, (index + 2) * sizeof(uint32_t));

      auto position_0 = ludo::read<ludo::vec3>(mesh.vertex_buffer, index_0 * celestial_body.format.size + celestial_body.format.position_offset);
      auto position_1 = ludo::read<ludo::vec3>(mesh.vertex_buffer, index_1 * celestial_body.format.size + celestial_body.format.position_offset);
      auto position_2 = ludo::read<ludo::vec3>(mesh.vertex_buffer, index_2 * celestial_body.format.size + celestial_body.format.position_offset);

      auto position_0_normalized = position_0;

      if (variant.first_unique_indices[index] == index)
      {
        heights[index] = celestial_body.height_func(position_0);
      }
      if (variant.first_unique_indices[index + 1] == index + 1)
      {
        heights[index + 1] = celestial_body.height_func(position_1);
      }
      if (variant.first_unique_indices[index + 2] == index + 2)
      {
        heights[index + 2] = celestial_body.height_func(position_2);
      }

      auto height_0 = heights[variant.first_unique_indices[index]];
      auto height_1 = heights[variant.first_unique_indices[index + 1]];
      auto height_2 = heights[variant.first_unique_indices[index + 2]];

      position_0 *= height_0 * celestial_body.radius;
      position_1 *= height_1 * celestial_body.radius;
      position_2 *= height_2 * celestial_body.radius;

      ludo::write(mesh.vertex_buffer, index_0 * celestial_body.format.size + celestial_body.format.position_offset, position_0);
      ludo::write(mesh.vertex_buffer, index_1 * celestial_body.format.size + celestial_body.format.position_offset, position_1);
      ludo::write(mesh.vertex_buffer, index_2 * celestial_body.format.size + celestial_body.format.position_offset, position_2);

      auto normal = ludo::cross(position_1 - position_0, position_2 - position_0);
      ludo::normalize(normal);

      if (celestial_body.format.has_normal)
      {
        ludo::write(mesh.vertex_buffer, index_0 * celestial_body.format.size + celestial_body.format.normal_offset, normal);
        ludo::write(mesh.vertex_buffer, index_1 * celestial_body.format.size + celestial_body.format.normal_offset, normal);
        ludo::write(mesh.vertex_buffer, index_2 * celestial_body.format.size + celestial_body.format.normal_offset, normal);
      }

      if (celestial_body.format.has_color)
      {
        auto color = celestial_body.color_func(position_0_normalized[1], { height_0, height_1, height_2 }, ludo::dot(normal, position_0_normalized));

        ludo::write(mesh.vertex_buffer, index_0 * celestial_body.format.size + celestial_body.format.color_offset, color);
        ludo::write(mesh.vertex_buffer, index_1 * celestial_body.format.size + celestial_body.format.color_offset, color);
        ludo::write(mesh.vertex_buffer, index_2 * celestial_body.format.size + celestial_body.format.color_offset, color);
      }
    }
  }

  // TODO currently this makes no attempt to update normals, should it?
  void sew_patch(ludo::instance& inst, const celestial_body& celestial_body, const patchwork& patchwork, uint32_t anchor_patch_index, uint32_t patch_index)
  {
    auto& anchor_patch = patchwork.patches[anchor_patch_index];
    auto& patch = patchwork.patches[patch_index];
    auto& anchor_mesh_instance = *ludo::get<ludo::mesh_instance>(inst, anchor_patch.mesh_instance_id);
    auto& mesh_instance = *ludo::get<ludo::mesh_instance>(inst, patch.mesh_instance_id);

    assert(anchor_patch.variant_index <= patch.variant_index && "anchor patch LOD level must be equal or less than the aligning patch LOD level");

    auto& anchor_border_indices = border_indices(patchwork, anchor_patch_index, patch_index);
    auto& aligning_border_indices = border_indices(patchwork, patch_index, anchor_patch_index);

    auto anchor_border_position_0 = ludo::read<ludo::vec3>(anchor_mesh_instance.vertex_buffer, anchor_border_indices[0] * celestial_body.format.size + celestial_body.format.position_offset);
    auto aligning_border_position_0 = ludo::read<ludo::vec3>(mesh_instance.vertex_buffer, aligning_border_indices[0] * celestial_body.format.size + celestial_body.format.position_offset);
    auto reverse_borders = !ludo::near(anchor_border_position_0, aligning_border_position_0);

    if (anchor_patch.variant_index == patch.variant_index)
    {
      // Each mid-point along the edge has 3 indices referring to it (since 3 triangles meet at them). The two end-points have only 1 index referring to them.
      auto anchor_index = uint32_t(1);
      auto aligning_index = uint32_t(1);

      while (anchor_index < anchor_border_indices.size() - 1)
      {
        auto anchor_index_0 = anchor_index;
        if (reverse_borders)
        {
          anchor_index_0 = anchor_border_indices.size() - 1 - anchor_index_0;
        }

        auto position = ludo::read<ludo::vec3>(anchor_mesh_instance.vertex_buffer, anchor_border_indices[anchor_index_0] * celestial_body.format.size + celestial_body.format.position_offset);

        ludo::write(mesh_instance.vertex_buffer, aligning_border_indices[aligning_index++] * celestial_body.format.size + celestial_body.format.position_offset, position);
        ludo::write(mesh_instance.vertex_buffer, aligning_border_indices[aligning_index++] * celestial_body.format.size + celestial_body.format.position_offset, position);
        ludo::write(mesh_instance.vertex_buffer, aligning_border_indices[aligning_index++] * celestial_body.format.size + celestial_body.format.position_offset, position);

        anchor_index += 3;
      }
    }
    else
    {
      auto lod_level_diff = celestial_body.lods[patch.variant_index].level - celestial_body.lods[anchor_patch.variant_index].level;
      auto divisions_per_anchor = std::pow(lod_level_diff - 1, 2) + lod_level_diff;

      // Each mid-point along the edge has 3 indices referring to it (since 3 triangles meet at them). The two end-points have only 1 index referring to them.
      auto anchor_index = uint32_t(0);
      auto aligning_index = uint32_t(1);

      while (anchor_index < anchor_border_indices.size() - 1)
      {
        auto anchor_index_0 = anchor_index;
        auto anchor_index_1 = anchor_index + 1;
        if (reverse_borders)
        {
          anchor_index_0 = anchor_border_indices.size() - 1 - anchor_index_0;
          anchor_index_1 = anchor_border_indices.size() - 1 - anchor_index_1;
        }

        auto start_position = ludo::read<ludo::vec3>(anchor_mesh_instance.vertex_buffer, anchor_border_indices[anchor_index_0] * celestial_body.format.size + celestial_body.format.position_offset);
        auto end_position = ludo::read<ludo::vec3>(anchor_mesh_instance.vertex_buffer, anchor_border_indices[anchor_index_1] * celestial_body.format.size + celestial_body.format.position_offset);
        auto alignment_vector = end_position - start_position;

        for (auto division_index = 0; division_index < divisions_per_anchor; division_index++)
        {
          auto alignment_position = start_position + alignment_vector * static_cast<float>(division_index + 1) / static_cast<float>(divisions_per_anchor + 1);
          ludo::write(mesh_instance.vertex_buffer, aligning_border_indices[aligning_index++] * celestial_body.format.size + celestial_body.format.position_offset, alignment_position);
          ludo::write(mesh_instance.vertex_buffer, aligning_border_indices[aligning_index++] * celestial_body.format.size + celestial_body.format.position_offset, alignment_position);
          ludo::write(mesh_instance.vertex_buffer, aligning_border_indices[aligning_index++] * celestial_body.format.size + celestial_body.format.position_offset, alignment_position);
        }

        anchor_index += 3;

        if (anchor_index < anchor_border_indices.size() - 1)
        {
          ludo::write(mesh_instance.vertex_buffer, aligning_border_indices[aligning_index++] * celestial_body.format.size + celestial_body.format.position_offset, end_position);
          ludo::write(mesh_instance.vertex_buffer, aligning_border_indices[aligning_index++] * celestial_body.format.size + celestial_body.format.position_offset, end_position);
          ludo::write(mesh_instance.vertex_buffer, aligning_border_indices[aligning_index++] * celestial_body.format.size + celestial_body.format.position_offset, end_position);
        }
      }
    }
  }

  const std::vector<uint32_t>& border_indices(const patchwork& patchwork, uint32_t patch_index, uint32_t adjacent_patch_index)
  {
    auto& patch = patchwork.patches[patch_index];

    for (auto index = 0; index < patch.adjacent_patch_indices.size(); index++)
    {
      if (patch.adjacent_patch_indices[index] == adjacent_patch_index)
      {
        return patchwork.variants[patch.variant_index].border_indices[index];
      }
    }

    assert(false && "adjacent patch not found");

    return patchwork.variants[patch.variant_index].border_indices[0];
  }

  void update_static_bodies(ludo::instance& inst, celestial_body& celestial_body, const patchwork& patchwork)
  {
    auto& point_masses = ludo::data<point_mass>(inst);

    auto& most_detailed_lod = celestial_body.lods[celestial_body.lods.size() - 1];
    auto& second_most_detailed_lod = celestial_body.lods[celestial_body.lods.size() - 2];
    auto& variant = patchwork.variants[celestial_body.lods.size() - 2];

    auto test_positions = std::vector<ludo::vec3>();
    for (auto& point_mass : point_masses)
    {
      if (point_mass.mass > 100.0f * gravitational_constant)
      {
        continue;
      }

      auto relative_position = point_mass.transform.position - patchwork.transform.position;
      if (ludo::length(relative_position) > celestial_body.radius * 1.25f)
      {
        continue;
      }

      ludo::normalize(relative_position);
      test_positions.push_back(relative_position);
    }

    auto sections = find_ico_sections(second_most_detailed_lod.level, [&](const std::array<ludo::vec3, 3>& triangle)
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
      if (!celestial_body.static_body_ids.contains(section.first))
      {
        auto mesh_count = 3 * static_cast<uint32_t>(std::pow(4, most_detailed_lod.level - second_most_detailed_lod.level));
        auto mesh = ludo::add(
          inst,
          ludo::mesh(),
          "celestial-bodies"
        );

        mesh->index_buffer = ludo::allocate(mesh_count * sizeof(uint32_t));
        mesh->vertex_buffer = ludo::allocate(mesh_count * ludo::vertex_format_p.size);

        ico_section(*mesh, ludo::vertex_format_p, 0, 0, 0, most_detailed_lod.level - second_most_detailed_lod.level, ludo::vertex_format_p.position_offset, section.second);

        auto heights = std::vector<float>(mesh_count);
        for (auto index = 0; index < mesh_count; index += 3)
        {
          auto index_0 = ludo::read<uint32_t>(mesh->index_buffer, index * sizeof(uint32_t));
          auto index_1 = ludo::read<uint32_t>(mesh->index_buffer, (index + 1) * sizeof(uint32_t));
          auto index_2 = ludo::read<uint32_t>(mesh->index_buffer, (index + 2) * sizeof(uint32_t));

          auto position_0 = ludo::read<ludo::vec3>(mesh->vertex_buffer, index_0 * ludo::vertex_format_p.size);
          auto position_1 = ludo::read<ludo::vec3>(mesh->vertex_buffer, index_1 * ludo::vertex_format_p.size);
          auto position_2 = ludo::read<ludo::vec3>(mesh->vertex_buffer, index_2 * ludo::vertex_format_p.size);

          if (variant.first_unique_indices[index] == index)
          {
            heights[index] = celestial_body.height_func(position_0);
          }
          if (variant.first_unique_indices[index + 1] == index + 1)
          {
            heights[index + 1] = celestial_body.height_func(position_1);
          }
          if (variant.first_unique_indices[index + 2] == index + 2)
          {
            heights[index + 2] = celestial_body.height_func(position_2);
          }

          auto height_0 = heights[variant.first_unique_indices[index]];
          auto height_1 = heights[variant.first_unique_indices[index + 1]];
          auto height_2 = heights[variant.first_unique_indices[index + 2]];

          position_0 *= height_0 * celestial_body.radius;
          position_1 *= height_1 * celestial_body.radius;
          position_2 *= height_2 * celestial_body.radius;

          ludo::write(mesh->vertex_buffer, index_0 * ludo::vertex_format_p.size, position_0);
          ludo::write(mesh->vertex_buffer, index_1 * ludo::vertex_format_p.size, position_1);
          ludo::write(mesh->vertex_buffer, index_2 * ludo::vertex_format_p.size, position_2);
        }

        auto body = ludo::add(
          inst,
          ludo::static_body{ { .transform = { .position = patchwork.transform.position } } },
          "celestial-bodies"
        );

        auto build_shape = ludo::build_shape(inst, *body, *mesh, ludo::vertex_format_p, 0, mesh->index_buffer.size / sizeof(uint32_t));
        ludo::execute(build_shape);

        celestial_body.static_body_ids[section.first] = body->id;
        celestial_body.static_body_mesh_ids[section.first] = mesh->id;
      }
    }

    for (auto static_body_iter = celestial_body.static_body_ids.begin(); static_body_iter != celestial_body.static_body_ids.end();)
    {
      if (!sections.contains(static_body_iter->first))
      {
        auto mesh = ludo::get<ludo::mesh>(inst, "celestial-bodies", celestial_body.static_body_mesh_ids[static_body_iter->first]);

        ludo::deallocate(mesh->index_buffer);
        ludo::deallocate(mesh->vertex_buffer);

        ludo::remove(inst, mesh, "celestial-bodies");
        celestial_body.static_body_mesh_ids.erase(static_body_iter->first);

        auto static_body = ludo::get<ludo::static_body>(inst, "celestial-bodies", static_body_iter->second);
        ludo::remove(inst, static_body, "celestial-bodies");
        static_body_iter = celestial_body.static_body_ids.erase(static_body_iter);
      }
      else
      {
        static_body_iter++;
      }
    }
  }
}
