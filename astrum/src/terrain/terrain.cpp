#include <fstream>

#include "constants.h"
#include "metadata.h"
#include "terrain.h"
#include "shaders.h"
#include "static_bodies.h"
#include "terrain_chunk.h"

namespace astrum
{
  void add_terrain(ludo::instance& inst, const terrain& init, const celestial_body& celestial_body, const std::string& partition)
  {
    auto rendering_context = ludo::first<ludo::rendering_context>(inst);

    auto& render_commands = data_heap(inst, "ludo::vram_render_commands");
    auto& indices = data_heap(inst, "ludo::vram_indices");
    auto& vertices = data_heap(inst, "ludo::vram_vertices");

    auto& point_masses = ludo::data<point_mass>(inst, partition);
    auto& point_mass = point_masses[point_masses.length - 1];

    auto terrain = add(inst, init, partition);

    auto metadata_file_name = ludo::asset_folder + "/meshes/" + celestial_body.name + ".terrain";
    auto read_stream = std::ifstream(metadata_file_name, std::ios::binary);
    if (read_stream.is_open())
    {
      read_terrain_metadata(read_stream, *terrain);
    }
    else
    {
      build_terrain_metadata(*terrain, celestial_body.radius);
      auto write_stream = std::ofstream(metadata_file_name, std::ios::binary);
      write_terrain_metadata(write_stream, *terrain);
    }

    terrain->format.components.emplace_back(std::pair { 'f', 3 });
    terrain->format.size += sizeof(float) * 3;
    if (terrain->format.has_normal)
    {
      terrain->format.components.emplace_back(std::pair { 'f', 3 });
      terrain->format.size += sizeof(float) * 3;
    }
    if (terrain->format.has_color)
    {
      terrain->format.components.emplace_back(std::pair { 'f', 4 });
      terrain->format.size += sizeof(float) * 4;
    }

    auto render_program = ludo::add(
      inst,
      ludo::render_program
      {
        .format = terrain->format,
        .shader_buffer = ludo::allocate_dual(sizeof(ludo::mat4)),
        .instance_size = 2 * sizeof(float)
      },
      "terrain"
    );

    auto vertex_shader_code = terrain_vertex_shader_code(terrain->format);
    auto fragment_shader_code = terrain_fragment_shader_code(terrain->format);
    ludo::init(*render_program, vertex_shader_code, fragment_shader_code, render_commands, terrain->chunks.size());

    ludo::cast<ludo::mat4>(render_program->shader_buffer.back, 0) = ludo::mat4(point_mass.transform.position, ludo::mat3(point_mass.transform.rotation));

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
      "terrain"
    );
    grid->compute_program_id = ludo::add(inst, ludo::build_compute_program(*grid))->id;
    ludo::init(*grid);

    auto camera = ludo::get_camera(*rendering_context);
    auto camera_position = ludo::position(camera.view);

    for (auto chunk_index = uint32_t(0); chunk_index < terrain->chunks.size(); chunk_index++)
    {
      auto& chunk = terrain->chunks[chunk_index];
      chunk.lod_index = terrain_chunk_lod_index(*terrain, chunk_index, terrain->lods, camera_position, point_mass.transform.position);

      auto count = 3 * static_cast<uint32_t>(std::pow(4, init.lods[chunk.lod_index].level - init.lods[0].level));

      auto mesh = ludo::add(inst, ludo::mesh(), "terrain");
      ludo::init(*mesh, indices, vertices, count, count, render_program->format.size);

      auto render_mesh = add(
        inst,
        ludo::render_mesh
        {
          .render_program_id = render_program->id,
          .instances =
          {
            .start = chunk_index,
            .count = 1
          },
          .indices =
          {
            .start = static_cast<uint32_t>((mesh->index_buffer.data - indices.data) / sizeof(uint32_t)),
            .count = static_cast<uint32_t>(mesh->index_buffer.size / sizeof(uint32_t))
          },
          .vertices =
          {
            .start = static_cast<uint32_t>((mesh->vertex_buffer.data - vertices.data) / mesh->vertex_size),
            .count = static_cast<uint32_t>(mesh->vertex_buffer.size / mesh->vertex_size)
          },
          .instance_buffer = allocate(render_program->instance_buffer_back, render_program->instance_size),
          .instance_size = render_program->instance_size
        },
        "terrain"
      );
      ludo::init(*render_mesh);

      chunk.mesh_id = mesh->id;
      chunk.render_mesh_id = render_mesh->id;

      if (chunk.mesh_id == 33844)
      {
        std::cout << "33844 added" << std::endl;
      }

      load_terrain_chunk(*terrain, celestial_body.radius, chunk_index, chunk.lod_index, *mesh);
      init_terrain_chunk(*terrain, chunk_index, *render_mesh);

      ludo::add(*grid, *render_mesh, point_mass.transform.position + chunk.center);
    }

    ludo::commit(*grid);

    update_terrain_static_bodies(inst, *terrain, celestial_body.radius, point_mass.transform.position, celestial_body.radius * 1.25f);
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

  void stream_terrain(ludo::instance& inst)
  {
    auto& rendering_context = *ludo::first<ludo::rendering_context>(inst);

    auto& grids = ludo::data<ludo::grid3>(inst, "terrain");
    auto& render_programs = ludo::data<ludo::render_program>(inst, "terrain");

    auto& indices = data_heap(inst, "ludo::vram_indices");
    auto& vertices = data_heap(inst, "ludo::vram_vertices");

    auto& celestial_bodies = ludo::data<celestial_body>(inst, "celestial-bodies");
    auto& point_masses = ludo::data<point_mass>(inst, "celestial-bodies");
    auto& terrains = ludo::data<terrain>(inst, "celestial-bodies");

    auto camera = ludo::get_camera(rendering_context);
    auto camera_position = ludo::position(camera.view);

    for (auto index = uint32_t(0); index < terrains.length; index++)
    {
      auto& grid = grids[index];
      auto& render_program = render_programs[index];

      auto& celestial_body = celestial_bodies[index];
      auto& point_mass = point_masses[index];
      auto& terrain = terrains[index];

      auto old_position = ludo::position(ludo::cast<ludo::mat4>(render_program.shader_buffer.back, 0));
      auto new_position = point_mass.transform.position;

      auto movement = new_position - old_position;
      if (ludo::length2(movement) > 0.0f)
      {
        grid.bounds.min += movement;
        grid.bounds.max += movement;
        ludo::commit_header(grid);

        ludo::cast<ludo::mat4>(render_program.shader_buffer.back, 0) = ludo::mat4(new_position, ludo::mat3(point_mass.transform.rotation));
      }

      update_terrain_static_bodies(inst, terrain, celestial_body.radius, new_position, celestial_body.radius * 1.25f);

      for (auto chunk_index = uint32_t(0); chunk_index < terrain.chunks.size(); chunk_index++)
      {
        auto& chunk = terrain.chunks[chunk_index];
        if (chunk.locked)
        {
          continue;
        }

        auto new_lod_index = terrain_chunk_lod_index(terrain, chunk_index, terrain.lods, camera_position, new_position);
        if (new_lod_index != chunk.lod_index)
        {
          chunk.locked = true;

          auto count = 3 * static_cast<uint32_t>(std::pow(4, terrain.lods[new_lod_index].level - terrain.lods[0].level));

          auto new_mesh = ludo::add(inst, ludo::mesh(), "terrain");
          ludo::init(*new_mesh, indices, vertices, count, count, render_program.format.size);

          // Purposely take a copy of the new mesh!
          // Otherwise, it may get shifted in the partitioned_buffer and cause all sorts of havoc.
          auto new_mesh_copy = *new_mesh;

          ludo::enqueue_background_task(inst, [&inst, &celestial_body, &terrain, &grid, chunk_index, new_mesh_copy, new_lod_index, new_position]()
          {
            auto local_new_mesh = new_mesh_copy;
            load_terrain_chunk(terrain, celestial_body.radius, chunk_index, new_lod_index, local_new_mesh);

            return [&inst, &terrain, &grid, chunk_index, new_mesh_copy, new_lod_index, new_position]()
            {
              auto& indices = data_heap(inst, "ludo::vram_indices");
              auto& vertices = data_heap(inst, "ludo::vram_vertices");

              auto& chunk = terrain.chunks[chunk_index];

              auto render_mesh = ludo::get<ludo::render_mesh>(inst, "terrain", chunk.render_mesh_id);
              auto mesh = ludo::get<ludo::mesh>(inst, "terrain", chunk.mesh_id);
              ludo::de_init(*mesh, indices, vertices);
              ludo::remove(inst, mesh, "terrain");

              chunk.mesh_id = new_mesh_copy.id;

              render_mesh->indices.start = (new_mesh_copy.index_buffer.data - indices.data) / sizeof(uint32_t);
              render_mesh->indices.count = new_mesh_copy.index_buffer.size / sizeof(uint32_t);

              render_mesh->vertices.start = (new_mesh_copy.vertex_buffer.data - vertices.data) / new_mesh_copy.vertex_size;
              render_mesh->vertices.count = new_mesh_copy.vertex_buffer.size / new_mesh_copy.vertex_size;

              chunk.lod_index = new_lod_index;
              chunk.locked = false;

              ludo::remove(grid, *render_mesh, new_position + chunk.center);
              ludo::add(grid, *render_mesh, new_position + chunk.center);

              init_terrain_chunk(terrain, chunk_index, *render_mesh);
            };
          });
        }
      }

      // TODO not while render could be happening!!!
      ludo::commit(grid);
    }
  }
}
