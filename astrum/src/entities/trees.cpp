#include "../constants.h"
#include "../spatial/icotree.h"
#include "../terrain/terrain_chunk.h"
#include "trees.h"

namespace astrum
{
  ludo::mesh_instance* add_trees(
    ludo::instance& inst,
    terrain& terrain,
    ludo::render_program& render_program,
    float radius,
    const ludo::vec3& position,
    uint32_t chunk_index,
    uint32_t lod_index,
    const std::array<std::array<ludo::range, 2>, 2>& lod_ranges
  );

  void add_trees(ludo::instance& inst, uint32_t celestial_body_index)
  {
    auto meshes = ludo::first<ludo::mesh>(inst, "trees");
    auto rendering_context = ludo::first<ludo::rendering_context>(inst);

    auto& indices = ludo::data_heap(inst, "ludo::vram_indices");
    auto& vertices = ludo::data_heap(inst, "ludo::vram_vertices");

    auto& celestial_body = ludo::data<astrum::celestial_body>(inst, "celestial-bodies")[celestial_body_index];
    auto& point_mass = ludo::data<astrum::point_mass>(inst, "celestial-bodies")[celestial_body_index];
    auto& terrain = ludo::data<astrum::terrain>(inst, "celestial-bodies")[celestial_body_index];

    auto camera_position = ludo::position(ludo::get_camera(*rendering_context).view);

    auto render_program = ludo::add(
      inst,
      ludo::render_program { .push_on_bind = false },
      ludo::vertex_format_pnc,
      terrain.chunks.size() * 200,
      "trees"
    );

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
          .cell_count_1d = 16
        },
      "trees"
    );

    auto mesh_1_index_range = ludo::range
    {
      .start = static_cast<uint32_t>((meshes[0].index_buffer.data - indices.data) / sizeof(uint32_t)),
      .count = static_cast<uint32_t>(meshes[0].index_buffer.size / sizeof(uint32_t))
    };
    auto mesh_1_vertex_range = ludo::range
    {
      .start = static_cast<uint32_t>((meshes[0].vertex_buffer.data - vertices.data) / meshes[0].vertex_size),
      .count = static_cast<uint32_t>(meshes[0].vertex_buffer.size / meshes[0].vertex_size)
    };

    auto mesh_2_index_range = ludo::range
    {
      .start = static_cast<uint32_t>((meshes[1].index_buffer.data - indices.data) / sizeof(uint32_t)),
      .count = static_cast<uint32_t>(meshes[1].index_buffer.size / sizeof(uint32_t))
    };
    auto mesh_2_vertex_range = ludo::range
    {
      .start = static_cast<uint32_t>((meshes[1].vertex_buffer.data - vertices.data) / meshes[1].vertex_size),
      .count = static_cast<uint32_t>(meshes[1].vertex_buffer.size / meshes[1].vertex_size)
    };

    for (auto chunk_index = 0; chunk_index < terrain.chunks.size(); chunk_index++)
    {
      auto& chunk = terrain.chunks[chunk_index];

      auto lod_index = terrain_chunk_lod_index(terrain, chunk_index, tree_lods, camera_position, point_mass.transform.position);
      if (lod_index == 0)
      {
        continue;
      }

      auto mesh_instance = add_trees(
        inst,
        terrain,
        *render_program,
        celestial_body.radius,
        point_mass.transform.position,
        chunk_index,
        lod_index,
        {{
          { mesh_1_index_range, mesh_1_vertex_range },
          { mesh_2_index_range, mesh_2_vertex_range }
        }}
      );

      chunk.tree_mesh_instance_id = mesh_instance->id;

      auto chunk_position = point_mass.transform.position + chunk.center;
      ludo::add(*grid, *mesh_instance, chunk_position);
    }

    ludo::push(*render_program);
    ludo::push(*grid);
  }

  void stream_trees(ludo::instance& inst, uint32_t celestial_body_index)
  {
    auto grid = ludo::first<ludo::grid3>(inst, "trees");
    auto& meshes = ludo::data<ludo::mesh>(inst, "trees");
    auto& mesh_instances = ludo::data<ludo::mesh_instance>(inst, "trees");
    auto rendering_context = ludo::first<ludo::rendering_context>(inst);
    auto render_program = ludo::first<ludo::render_program>(inst, "trees");

    auto& indices = ludo::data_heap(inst, "ludo::vram_indices");
    auto& vertices = ludo::data_heap(inst, "ludo::vram_vertices");

    auto& celestial_body = ludo::data<astrum::celestial_body>(inst, "celestial-bodies")[celestial_body_index];
    auto& point_mass = ludo::data<astrum::point_mass>(inst, "celestial-bodies")[celestial_body_index];
    auto& terrain = ludo::data<astrum::terrain>(inst, "celestial-bodies")[celestial_body_index];

    auto camera_position = ludo::position(ludo::get_camera(*rendering_context).view);

    auto mesh_1_index_range = ludo::range
    {
      .start = static_cast<uint32_t>((meshes[0].index_buffer.data - indices.data) / sizeof(uint32_t)),
      .count = static_cast<uint32_t>(meshes[0].index_buffer.size / sizeof(uint32_t))
    };
    auto mesh_1_vertex_range = ludo::range
    {
      .start = static_cast<uint32_t>((meshes[0].vertex_buffer.data - vertices.data) / meshes[0].vertex_size),
      .count = static_cast<uint32_t>(meshes[0].vertex_buffer.size / meshes[0].vertex_size)
    };

    auto mesh_2_index_range = ludo::range
    {
      .start = static_cast<uint32_t>((meshes[1].index_buffer.data - indices.data) / sizeof(uint32_t)),
      .count = static_cast<uint32_t>(meshes[1].index_buffer.size / sizeof(uint32_t))
    };
    auto mesh_2_vertex_range = ludo::range
    {
      .start = static_cast<uint32_t>((meshes[1].vertex_buffer.data - vertices.data) / meshes[1].vertex_size),
      .count = static_cast<uint32_t>(meshes[1].vertex_buffer.size / meshes[1].vertex_size)
    };

    auto push_required = false;
    for (auto chunk_index = uint32_t(0); chunk_index < terrain.chunks.size(); chunk_index++)
    {
      auto& chunk = terrain.chunks[chunk_index];

      auto lod_index = terrain_chunk_lod_index(terrain, chunk_index, tree_lods, camera_position, point_mass.transform.position);

      if (lod_index == 0)
      {
        if (chunk.tree_mesh_instance_id != 0)
        {
          auto mesh_instance = ludo::find_by_id(mesh_instances.begin(), mesh_instances.end(), chunk.tree_mesh_instance_id);

          auto chunk_position = point_mass.transform.position + chunk.center;
          ludo::remove(*grid, *mesh_instance, chunk_position);
          chunk.tree_mesh_instance_id = 0;
          push_required = true;

          ludo::remove(inst, mesh_instance, "trees");
        }
      }
      else
      {
        if (chunk.tree_mesh_instance_id == 0)
        {
          auto mesh_instance = add_trees(
            inst,
            terrain,
            *render_program,
            celestial_body.radius,
            point_mass.transform.position,
            chunk_index,
            lod_index,
            {{
              { mesh_1_index_range, mesh_1_vertex_range },
              { mesh_2_index_range, mesh_2_vertex_range }
            }}
          );

          chunk.tree_mesh_instance_id = mesh_instance->id;

          auto chunk_position = point_mass.transform.position + chunk.center;
          ludo::add(*grid, *mesh_instance, chunk_position);
          push_required = true;
        }

        auto mesh_instance = ludo::find_by_id(mesh_instances.begin(), mesh_instances.end(), chunk.tree_mesh_instance_id);

        if (lod_index == 1 && mesh_instance->indices.start != mesh_1_index_range.start)
        {
          mesh_instance->indices = mesh_1_index_range;
          mesh_instance->vertices = mesh_1_vertex_range;

          auto chunk_position = point_mass.transform.position + chunk.center;
          ludo::remove(*grid, *mesh_instance, chunk_position);
          ludo::add(*grid, *mesh_instance, chunk_position);
          push_required = true;
        }
        else if (lod_index == 2 && mesh_instance->indices.start != mesh_2_index_range.start)
        {
          mesh_instance->indices = mesh_2_index_range;
          mesh_instance->vertices = mesh_2_vertex_range;

          auto chunk_position = point_mass.transform.position + chunk.center;
          ludo::remove(*grid, *mesh_instance, chunk_position);
          ludo::add(*grid, *mesh_instance, chunk_position);
          push_required = true;
        }
      }
    }

    if (push_required)
    {
      // TODO not while render could be happening!!!
      ludo::push(*render_program);
      ludo::push(*grid);
    }
  }

  ludo::mesh_instance* add_trees(
    ludo::instance& inst,
    terrain& terrain,
    ludo::render_program& render_program,
    float radius,
    const ludo::vec3& position,
    uint32_t chunk_index,
    uint32_t lod_index,
    const std::array<std::array<ludo::range, 2>, 2>& lod_ranges
  )
  {
    auto trees = terrain.tree_func(terrain, radius, chunk_index);
    auto instance_buffer = allocate(render_program.instance_buffer_back, render_program.instance_size * trees.size());

    auto mesh_instance = ludo::add(
      inst,
      ludo::mesh_instance
      {
        .render_program_id = render_program.id,
        .instances =
        {
          .start = static_cast<uint32_t>((instance_buffer.data - render_program.instance_buffer_back.data) / render_program.instance_size),
          .count = static_cast<uint32_t>(trees.size())
        },
        .indices = lod_ranges[lod_index - 1][0],
        .vertices = lod_ranges[lod_index - 1][1],
        .instance_buffer = instance_buffer,
        .instance_size = render_program.instance_size
      },
      "trees"
    );

    for (auto tree_index = uint32_t(0); tree_index < trees.size(); tree_index++)
    {
      auto& tree = trees[tree_index];
      auto tree_position = position + tree.position * terrain.height_func(tree.position) * radius;
      auto rotation = ludo::mat3(ludo::vec3_unit_y, tree.position) * ludo::mat3(ludo::vec3_unit_y, tree.rotation);
      auto transform = ludo::mat4(tree_position, rotation);
      ludo::scale(transform, { tree.scale, tree.scale, tree.scale });
      ludo::instance_transform(*mesh_instance, tree_index) = transform;
    }

    return mesh_instance;
  }
}
