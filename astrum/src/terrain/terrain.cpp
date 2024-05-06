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
    auto& indices = data_heap(inst, "ludo::vram_indices");
    auto& vertices = data_heap(inst, "ludo::vram_vertices");

    auto& point_masses = ludo::data<point_mass>(inst, partition);
    auto& point_mass = point_masses[point_masses.length - 1];

    auto terrain = add(inst, init, partition);

    auto metadata_file_name = std::string("assets/meshes/") + celestial_body.name + ".terrain";
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

    auto vertex_shader = add_terrain_vertex_shader(inst, terrain->format, "terrain");
    auto fragment_shader = add_terrain_fragment_shader(inst, terrain->format, "terrain");

    auto render_program = ludo::add(
      inst,
      ludo::render_program
      {
        .vertex_shader_id = vertex_shader->id,
        .fragment_shader_id = fragment_shader->id,
        .format = terrain->format,
        .shader_buffer = ludo::allocate_vram(sizeof(ludo::mat4)),
        .instance_size = 2 * sizeof(float)
      },
      terrain->format,
      terrain->chunks.size(),
      "terrain"
    );

    ludo::cast<ludo::mat4>(render_program->shader_buffer, 0) = ludo::mat4(point_mass.transform.position, ludo::mat3(point_mass.transform.rotation));

    auto bounds_half_dimensions = ludo::vec3 { celestial_body.radius * 1.1f, celestial_body.radius * 1.1f, celestial_body.radius * 1.1f };
    auto linear_octree = ludo::add(
      inst,
      ludo::linear_octree
      {
        .bounds =
        {
          .min = point_mass.transform.position - bounds_half_dimensions,
          .max = point_mass.transform.position + bounds_half_dimensions
        },
        .depth = 4
      },
      "terrain"
    );

    auto camera = ludo::get_camera(*rendering_context);
    auto camera_position = ludo::position(camera.view);

    for (auto chunk_index = uint32_t(0); chunk_index < terrain->chunks.size(); chunk_index++)
    {
      auto& chunk = terrain->chunks[chunk_index];
      chunk.lod_index = terrain_chunk_lod_index(*terrain, chunk_index, camera_position, point_mass.transform.position);

      auto count = 3 * static_cast<uint32_t>(std::pow(4, init.lods[chunk.lod_index].level - init.lods[0].level));

      auto mesh = ludo::add(
        inst,
        ludo::mesh { .render_program_id = render_program->id },
        count,
        count,
        render_program->format.size,
        "terrain"
      );

      auto mesh_instance = add(
        inst,
        ludo::mesh_instance
        {
          .mesh_id = mesh->id,
          .render_program_id = render_program->id,
          .instance_index = chunk_index,
          .instance_buffer = allocate(render_program->instance_buffer_back, render_program->instance_size),
          .indices =
          {
            .start = static_cast<uint32_t>((mesh->index_buffer.data - indices.data) / sizeof(uint32_t)),
            .count = static_cast<uint32_t>(mesh->index_buffer.size / sizeof(uint32_t))
          },
          .vertices =
          {
            .start = static_cast<uint32_t>((mesh->vertex_buffer.data - vertices.data) / mesh->vertex_size),
            .count = static_cast<uint32_t>(mesh->vertex_buffer.size / mesh->vertex_size)
          }
        },
        "terrain"
      );

      chunk.mesh_instance_id = mesh_instance->id;

      load_terrain_chunk(*terrain, celestial_body.radius, chunk_index, chunk.lod_index, *mesh);
      init_terrain_chunk(*terrain, chunk_index, *mesh_instance);

      ludo::add(*linear_octree, *mesh_instance, point_mass.transform.position + chunk.center);
    }

    /* TODO ludo::divide_and_conquer(terrain->chunks.size(), [&](uint32_t start, uint32_t end)
    {
      for (auto chunk_index = start; chunk_index < end; chunk_index++)
      {
        auto& chunk = terrain->chunks[chunk_index];
        auto mesh_instance = ludo::get<ludo::mesh_instance>(inst, chunk.mesh_instance_id);
        auto mesh = ludo::get<ludo::mesh>(inst, mesh_instance->mesh_id);

        load_terrain_chunk(*terrain, celestial_body.radius, chunk_index, chunk.lod_index, *mesh);
        init_terrain_chunk(*terrain, chunk_index, *mesh_instance);
      }

      return [] {};
    });*/

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

    auto& linear_octrees = ludo::data<ludo::linear_octree>(inst, "terrain");
    auto& render_programs = ludo::data<ludo::render_program>(inst, "terrain");

    auto& celestial_bodies = ludo::data<celestial_body>(inst, "celestial-bodies");
    auto& point_masses = ludo::data<point_mass>(inst, "celestial-bodies");
    auto& terrains = ludo::data<terrain>(inst, "celestial-bodies");

    auto camera = ludo::get_camera(rendering_context);
    auto camera_position = ludo::position(camera.view);

    for (auto index = uint32_t(0); index < terrains.length; index++)
    {
      auto& linear_octree = linear_octrees[index];
      auto& render_program = render_programs[index];

      auto& celestial_body = celestial_bodies[index];
      auto& point_mass = point_masses[index];
      auto& terrain = terrains[index];

      auto old_position = ludo::position(ludo::cast<ludo::mat4>(render_program.shader_buffer, 0));
      auto new_position = point_mass.transform.position;

      auto movement = new_position - old_position;
      if (ludo::length2(movement) > 0.0f)
      {
        linear_octree.bounds.min += movement;
        linear_octree.bounds.max += movement;

        ludo::cast<ludo::mat4>(render_program.shader_buffer, 0) = ludo::mat4(new_position, ludo::mat3(point_mass.transform.rotation));
      }

      update_terrain_static_bodies(inst, terrain, celestial_body.radius, new_position, celestial_body.radius * 1.25f);

      auto mutex = std::mutex();
      ludo::divide_and_conquer(terrain.chunks.size(), [&](uint32_t start, uint32_t end)
      {
        for (auto chunk_index = start; chunk_index < end; chunk_index++)
        {
          auto& chunk = terrain.chunks[chunk_index];
          if (chunk.locked)
          {
            continue;
          }

          auto new_lod_index = terrain_chunk_lod_index(terrain, chunk_index, camera_position, new_position);
          if (new_lod_index != chunk.lod_index)
          {
            chunk.locked = true;

            auto count = 3 * static_cast<uint32_t>(std::pow(4, terrain.lods[new_lod_index].level - terrain.lods[0].level));

            // Multiple concurrent allocations would be bad...
            auto lock = std::lock_guard(mutex);
            auto new_mesh = *ludo::add(
              inst,
              ludo::mesh { .render_program_id = render_program.id },
              count,
              count,
              render_program.format.size,
              "terrain"
            );

            // Purposely take a copy of the new mesh!
            // Otherwise, it may get shifted in the partitioned_buffer and cause all sorts of havoc.
            ludo::enqueue_background(inst, [&inst, &celestial_body, &terrain, &linear_octree, chunk_index, new_mesh, new_lod_index, new_position]()
            {
              auto local_new_mesh = new_mesh;
              load_terrain_chunk(terrain, celestial_body.radius, chunk_index, new_lod_index, local_new_mesh);

              return [&inst, &terrain, &linear_octree, chunk_index, new_mesh, new_lod_index, new_position]()
              {
                auto& chunk = terrain.chunks[chunk_index];

                auto mesh_instance = ludo::get<ludo::mesh_instance>(inst, chunk.mesh_instance_id);
                auto mesh = ludo::get<ludo::mesh>(inst, mesh_instance->mesh_id);
                ludo::remove(inst, mesh, "terrain");

                mesh_instance->mesh_id = new_mesh.id;

                auto& indices = data_heap(inst, "ludo::vram_indices");
                mesh_instance->indices.start = (new_mesh.index_buffer.data - indices.data) / sizeof(uint32_t);
                mesh_instance->indices.count = new_mesh.index_buffer.size / sizeof(uint32_t);

                auto& vertices = data_heap(inst, "ludo::vram_vertices");
                mesh_instance->vertices.start = (new_mesh.vertex_buffer.data - vertices.data) / new_mesh.vertex_size;
                mesh_instance->vertices.count = new_mesh.vertex_buffer.size / new_mesh.vertex_size;

                chunk.lod_index = new_lod_index;
                chunk.locked = false;

                ludo::remove(linear_octree, *mesh_instance, new_position + chunk.center);
                ludo::add(linear_octree, *mesh_instance, new_position + chunk.center);

                init_terrain_chunk(terrain, chunk_index, *mesh_instance);
              };
            });
          }
        }

        return [] {};
      });
    }
  }
}
