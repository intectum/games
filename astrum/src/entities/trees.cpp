#include "../meshes/lod_shaders.h"
#include "../types.h"
#include "trees.h"

namespace astrum
{
  std::array<ludo::render_mesh*, tree_type_count> add_trees(
    ludo::instance& inst,
    ludo::heap& indices,
    ludo::heap& vertices,
    terrain& terrain,
    ludo::render_program& render_program,
    float radius,
    const ludo::vec3& position,
    uint32_t chunk_index,
    uint32_t lod_index,
    const std::array<ludo::array<ludo::mesh>, tree_type_count>& meshes
  );

  const auto tree_instance_size = sizeof(ludo::mat4) + sizeof(uint32_t) + 12; // align 16

  void add_trees(ludo::instance& inst, uint32_t celestial_body_index)
  {
    auto& fruit_tree_meshes = ludo::data<ludo::mesh>(inst, "fruit-trees");
    auto& oak_tree_meshes = ludo::data<ludo::mesh>(inst, "oak-trees");
    auto& pine_tree_meshes = ludo::data<ludo::mesh>(inst, "pine-trees");
    auto rendering_context = ludo::first<ludo::rendering_context>(inst);

    auto& render_commands = ludo::data_heap(inst, "ludo::vram_render_commands");
    auto& indices = ludo::data_heap(inst, "ludo::vram_indices");
    auto& vertices = ludo::data_heap(inst, "ludo::vram_vertices");

    auto& celestial_body = ludo::data<astrum::celestial_body>(inst, "celestial-bodies")[celestial_body_index];
    auto& point_mass = ludo::data<astrum::point_mass>(inst, "celestial-bodies")[celestial_body_index];
    auto& terrain = ludo::data<astrum::terrain>(inst, "celestial-bodies")[celestial_body_index];

    auto camera_position = ludo::position(ludo::get_camera(*rendering_context).view);

    auto lod_format = ludo::vertex_format_pnc;
    lod_format.components.insert(lod_format.components.end(), ludo::vertex_format_pnc.components.begin(), ludo::vertex_format_pnc.components.end());
    lod_format.size *= 2;

    auto vertex_shader_code = lod_vertex_shader_code(ludo::vertex_format_pnc, false);
    auto fragment_shader_code = lod_fragment_shader_code(ludo::vertex_format_pnc, false);
    auto render_program = ludo::add(
      inst,
      ludo::render_program
      {
        .format = lod_format,
        .shader_buffer = ludo::allocate_dual(tree_lods.size() * 2 * sizeof(float)),
        .instance_size = tree_instance_size,
        .push_on_bind = false
      },
      "trees"
    );
    ludo::init(*render_program, vertex_shader_code, fragment_shader_code, render_commands, terrain.chunks.size() * 200);

    auto stream = ludo::stream(render_program->shader_buffer.back);
    for (auto lod_index = uint32_t(0); lod_index < tree_lods.size(); lod_index++)
    {
      auto is_highest_detail = lod_index == tree_lods.size() - 1;

      auto max_distance = tree_lods.at(lod_index).max_distance;
      auto min_distance = is_highest_detail ? 0.0f : tree_lods[lod_index + 1].max_distance;
      auto distance_range = max_distance - min_distance;

      auto low_detail_distance = min_distance + distance_range * 0.66f;
      auto high_detail_distance = min_distance + distance_range * 0.33f;

      ludo::write(stream, low_detail_distance);
      ludo::write(stream, high_detail_distance);
    }

    auto bounds_half_dimensions = ludo::vec3 { celestial_body.radius * 1.1f, celestial_body.radius * 1.1f, celestial_body.radius * 1.1f };
    auto grid = ludo::add(
      inst,
      ludo::grid3
        {
          .bounds =
          {
            .min = point_mass.transform.position - bounds_half_dimensions,
            .max = point_mass.transform.position + bounds_half_dimensions
          },
          .cell_count_1d = 16,
          .cell_capacity = 48,
        },
      "trees"
    );
    grid->compute_program_id = ludo::add(inst, ludo::build_compute_program(*grid))->id;
    ludo::init(*grid);

    for (auto chunk_index = 0; chunk_index < terrain.chunks.size(); chunk_index++)
    {
      auto& chunk = terrain.chunks.at(chunk_index);
      auto chunk_position = point_mass.transform.position + chunk.center;

      auto lod_index = find_lod_index(tree_lods, camera_position, chunk_position, chunk.normal);
      if (lod_index == 0)
      {
        continue;
      }

      auto render_meshes = add_trees(
        inst,
        indices,
        vertices,
        terrain,
        *render_program,
        celestial_body.radius,
        point_mass.transform.position,
        chunk_index,
        lod_index,
        { fruit_tree_meshes, oak_tree_meshes, pine_tree_meshes }
      );

      chunk.trees_loaded = true;
      chunk.treeless = true;
      for (auto tree_type = 0; tree_type < render_meshes.size(); tree_type++)
      {
        if (!render_meshes.at(tree_type))
        {
          chunk.tree_render_mesh_ids.at(tree_type) = 0;
          continue;
        }

        chunk.treeless = false;
        chunk.tree_render_mesh_ids.at(tree_type) = render_meshes.at(tree_type)->id;
        ludo::add(*grid, *render_meshes.at(tree_type), chunk_position);
      }
    }

    ludo::commit(*render_program);
    ludo::commit(*grid);
  }

  void stream_trees(ludo::instance& inst, uint32_t celestial_body_index)
  {
    auto grid = ludo::first<ludo::grid3>(inst, "trees");
    auto& fruit_tree_meshes = ludo::data<ludo::mesh>(inst, "fruit-trees");
    auto& oak_tree_meshes = ludo::data<ludo::mesh>(inst, "oak-trees");
    auto& pine_tree_meshes = ludo::data<ludo::mesh>(inst, "pine-trees");
    auto& render_meshes = ludo::data<ludo::render_mesh>(inst, "trees");
    auto rendering_context = ludo::first<ludo::rendering_context>(inst);
    auto render_program = ludo::first<ludo::render_program>(inst, "trees");

    auto& indices = ludo::data_heap(inst, "ludo::vram_indices");
    auto& vertices = ludo::data_heap(inst, "ludo::vram_vertices");

    auto& celestial_body = ludo::data<astrum::celestial_body>(inst, "celestial-bodies")[celestial_body_index];
    auto& point_mass = ludo::data<astrum::point_mass>(inst, "celestial-bodies")[celestial_body_index];
    auto& terrain = ludo::data<astrum::terrain>(inst, "celestial-bodies")[celestial_body_index];

    auto camera_position = ludo::position(ludo::get_camera(*rendering_context).view);
    auto meshes = std::array<ludo::array<ludo::mesh>, tree_type_count> { fruit_tree_meshes, oak_tree_meshes, pine_tree_meshes };

    auto push_required = false;
    for (auto chunk_index = uint32_t(0); chunk_index < terrain.chunks.size(); chunk_index++)
    {
      auto& chunk = terrain.chunks.at(chunk_index);
      if (chunk.treeless)
      {
        continue;
      }

      auto chunk_position = point_mass.transform.position + chunk.center;
      auto lod_index = find_lod_index(tree_lods, camera_position, chunk_position, chunk.normal);

      if (lod_index > 0 && !chunk.trees_loaded)
      {
        auto added_render_meshes = add_trees(
          inst,
          indices,
          vertices,
          terrain,
          *render_program,
          celestial_body.radius,
          point_mass.transform.position,
          chunk_index,
          lod_index,
          meshes
        );

        chunk.trees_loaded = true;
        chunk.treeless = true;
        for (auto tree_type = 0; tree_type < added_render_meshes.size(); tree_type++)
        {
          if (!added_render_meshes.at(tree_type))
          {
            chunk.tree_render_mesh_ids.at(tree_type) = 0;
            continue;
          }

          chunk.treeless = false;
          chunk.tree_render_mesh_ids.at(tree_type) = added_render_meshes.at(tree_type)->id;
          ludo::add(*grid, *added_render_meshes.at(tree_type), chunk_position);
        }

        push_required = true;
      }
      else if (lod_index == 0 && chunk.trees_loaded)
      {
        for (auto& tree_render_mesh_id : chunk.tree_render_mesh_ids)
        {
          if (tree_render_mesh_id)
          {
            auto render_mesh = ludo::find_by_id(render_meshes.begin(), render_meshes.end(), tree_render_mesh_id);

            ludo::remove(*grid, *render_mesh, chunk_position);
            tree_render_mesh_id = 0;
            push_required = true;

            ludo::disconnect(*render_mesh, *render_program);
            ludo::de_init(*render_mesh);
            ludo::remove(inst, render_mesh, "trees");
          }
        }

        chunk.trees_loaded = false;
      }
      else if (chunk.trees_loaded)
      {
        for (auto tree_type = 0; tree_type < meshes.size(); tree_type++)
        {
          if (chunk.tree_render_mesh_ids.at(tree_type))
          {
            auto render_mesh = ludo::find_by_id(render_meshes.begin(), render_meshes.end(), chunk.tree_render_mesh_ids.at(tree_type));
            if (lod_index != ludo::cast<uint32_t>(render_mesh->instance_buffer, sizeof(ludo::mat4)))
            {
              ludo::connect(*render_mesh, meshes.at(tree_type)[lod_index - 1], indices, vertices);

              auto instance_stream = ludo::stream(render_mesh->instance_buffer, sizeof(ludo::mat4));
              while (!ludo::ended(instance_stream))
              {
                ludo::write(instance_stream, lod_index);
                instance_stream.position += tree_instance_size - sizeof(uint32_t);
              }

              ludo::remove(*grid, *render_mesh, chunk_position);
              ludo::add(*grid, *render_mesh, chunk_position);
              push_required = true;
            }
          }
        }
      }
    }

    if (push_required)
    {
      // TODO not while render could be happening!!!
      ludo::commit(*render_program);
      ludo::commit(*grid);
    }
  }

  std::array<ludo::render_mesh*, tree_type_count> add_trees(
    ludo::instance& inst,
    ludo::heap& indices,
    ludo::heap& vertices,
    terrain& terrain,
    ludo::render_program& render_program,
    float radius,
    const ludo::vec3& position,
    uint32_t chunk_index,
    uint32_t lod_index,
    const std::array<ludo::array<ludo::mesh>, tree_type_count>& meshes
  )
  {
    auto render_meshes = std::array<ludo::render_mesh*, tree_type_count>();

    auto trees = terrain.tree_func(terrain, radius, chunk_index);
    for (auto tree_type = 0; tree_type < trees.size(); tree_type++)
    {
      auto& meshes_of_type = meshes.at(tree_type);
      auto& trees_of_type = trees.at(tree_type);
      if (trees_of_type.empty())
      {
        render_meshes.at(tree_type) = nullptr;
        continue;
      }

      auto render_mesh = ludo::add(inst, ludo::render_mesh(), "trees");
      ludo::init(*render_mesh, render_program, meshes_of_type[lod_index - 1], indices, vertices, trees_of_type.size());
      render_mesh->instances =
      {
        .start = static_cast<uint32_t>((render_mesh->instance_buffer.data - render_program.instance_buffer_back.data) / render_program.instance_size),
        .count = static_cast<uint32_t>(trees_of_type.size())
      };

      for (auto tree_index = uint32_t(0); tree_index < trees_of_type.size(); tree_index++)
      {
        auto& tree = trees_of_type.at(tree_index);
        auto tree_position = position + tree.position * terrain.height_func(tree.position) * radius;
        auto tree_rotation = ludo::mat3(ludo::vec3_unit_y, tree.position) * ludo::mat3(ludo::vec3_unit_y, tree.rotation);
        auto tree_transform = ludo::mat4(tree_position, tree_rotation);
        ludo::scale(tree_transform, { tree.scale, tree.scale, tree.scale });
        ludo::instance_transform(*render_mesh, tree_index) = tree_transform;
        ludo::cast<uint32_t>(render_mesh->instance_buffer, tree_index * tree_instance_size + sizeof(ludo::mat4)) = lod_index;
      }

      render_meshes.at(tree_type) = render_mesh;
    }

    return render_meshes;
  }
}
