#include <fstream>

#include <btBulletDynamicsCommon.h>

#include <ludo/opengl/util.h>

#include "containers.h"
#include "ecs.h"
#include "terrain/mesh.h"
#include "types.h"

#include <iostream>

namespace astrum
{
  ludo::container build_container(const std::string& name, float radius, const std::vector<lod>& lods, const terrain_funcs& terrain_funcs)
  {
    auto terrain_count = 20 * static_cast<uint32_t>(std::pow(4, lods[0].level - 1));

    auto container = ludo::container
    {
      .components =
      {
        // common
        { .name = "position", .size = sizeof(ludo::vec3) },
        { .name = "position_vram", .size = sizeof(ludo::vec3), .vram = true },
        { .name = "rotation", .size = sizeof(ludo::quat) },
        { .name = "transform_vram", .size = sizeof(ludo::mat4), .vram = true },
        { .name = "mass", .size = sizeof(float) },
        { .name = "linear_velocity", .size = sizeof(ludo::vec3) },
        { .name = "angular_velocity", .size = sizeof(ludo::vec3) },
        { .name = "gravitational_acceleration", .size = sizeof(ludo::vec3) },
        { .name = "ghost_body", .size = sizeof(btRigidBody*) },
        { .name = "kinematic_body", .size = sizeof(btRigidBody*) },
        { .name = "static_body", .size = sizeof(btRigidBody*) },
        { .name = "resting", .size = sizeof(bool) }, // TODO bools should not be components! It means there will be conditional kernels somewhere...

        // camera
        { .name = "view_vram", .size = sizeof(ludo::mat4), .vram = true },
        { .name = "projection_vram", .size = sizeof(ludo::mat4), .vram = true },
        { .name = "view_projection_vram", .size = sizeof(ludo::mat4), .vram = true },
        { .name = "near_clipping_distance_vram", .size = sizeof(float), .vram = true },
        { .name = "far_clipping_distance_vram", .size = sizeof(float), .vram = true },

        // light
        { .name = "ambient_color_vram", .size = sizeof(ludo::vec4), .vram = true },
        { .name = "diffuse_color_vram", .size = sizeof(ludo::vec4), .vram = true },
        { .name = "specular_color_vram", .size = sizeof(ludo::vec4), .vram = true },
        { .name = "direction_vram", .size = sizeof(ludo::vec3), .vram = true },
        { .name = "attenuation_vram", .size = sizeof(ludo::vec3), .vram = true },
        { .name = "strength_vram", .size = sizeof(float), .vram = true },
        { .name = "range_vram", .size = sizeof(float), .vram = true },

        // celestial body
        { .name = "radius", .size = sizeof(float) },
        { .name = "lods_vram", .size = 5 * 2 * sizeof(float), .vram = true },
        { .name = "render_command_vram", .size = sizeof(ludo::render_command), .vram = true },

        // terrain
        { .name = "local_position", .size = sizeof(ludo::vec3) },
        { .name = "normal", .size = sizeof(ludo::vec3) },
        { .name = "mesh", .size = sizeof(ludo::mesh) },
        { .name = "loading_mesh", .size = sizeof(ludo::mesh) },
        { .name = "lod_index_vram", .size = sizeof(uint32_t), .vram = true },
        { .name = "loading_lod_index", .size = sizeof(uint32_t) },
        { .name = "loading", .size = sizeof(bool) }, // TODO bools should not be components! It means there will be conditional kernels somewhere...
        { .name = "loaded", .size = sizeof(bool) }, // TODO bools should not be components! It means there will be conditional kernels somewhere...

        // person
        { .name = "person", .size = sizeof(person) },
        { .name = "person_controls", .size = sizeof(person_controls) },

        // spaceship
        { .name = "spaceship_controls", .size = sizeof(spaceship_controls) }
      },
      .archetypes =
      {
        {
          .name = "camera",
          .component_names =
          {
            "position_vram",
            "rotation",
            "view_vram",
            "projection_vram",
            "view_projection_vram",
            "near_clipping_distance_vram",
            "far_clipping_distance_vram"
          },
          .capacity = 1
        },
        {
          .name = "light",
          .component_names =
          {
            "ambient_color_vram",
            "diffuse_color_vram",
            "specular_color_vram",
            "position_vram",
            "direction_vram",
            "attenuation_vram",
            "strength_vram",
            "range_vram",
          },
          .capacity = 1
        },
        {
          .name = "celestial_body",
          .component_names =
          {
            "position",
            "rotation",
            "transform_vram",
            "mass",
            "linear_velocity",
            "gravitational_acceleration",
            "resting",
            "radius",
            "lods_vram"
          },
          .capacity = 3
        },
        {
          .name = "terrain",
          .component_names =
          {
            "local_position",
            "normal",
            "mesh",
            "loading_mesh",
            "lod_index_vram",
            "loading_lod_index",
            "loading",
            "loaded",
            "render_command_vram"
          },
          .capacity = terrain_count
        },
        {
          .name = "person",
          .component_names =
          {
            "position",
            "rotation",
            "transform_vram",
            "mass",
            "linear_velocity",
            "gravitational_acceleration",
            "resting",
            "kinematic_body",
            "person",
            "person_controls"
          },
          .capacity = 1
        },
        {
          .name = "spaceship",
          .component_names =
          {
            "position",
            "rotation",
            "transform_vram",
            "mass",
            "linear_velocity",
            "gravitational_acceleration",
            "resting",
            "kinematic_body",
            "ghost_body",
            "spaceship_controls"
          },
          .capacity = 1
        }
      }
    };

    ludo::init(container);

    // TODO probably just a good old 0 init would be best...
    ludo::add(
      container,
      "terrain",
      ludo::vec3_zero,
      ludo::vec3_zero,
      ludo::mesh(),
      ludo::mesh(),
      uint32_t(0),
      uint32_t(0),
      false,
      false,
      ludo::render_command(),
      terrain_count
    );

    auto terrain_local_positions = get_components(container, "terrain", "local_position");
    auto terrain_normals = get_components(container, "terrain", "normal");

    auto metadata_file_name = ludo::asset_folder + "/meshes/" + name + ".terrain";
    auto read_stream = std::ifstream(metadata_file_name, std::ios::binary);
    if (read_stream.is_open())
    {
      read_stream.read(reinterpret_cast<char*>(terrain_local_positions->data), terrain_count * sizeof(ludo::vec3));
      read_stream.read(reinterpret_cast<char*>(terrain_normals->data), terrain_count * sizeof(ludo::vec3));
    }
    else
    {
      auto local_positions = reinterpret_cast<ludo::vec3*>(terrain_local_positions->data);
      auto normals = reinterpret_cast<ludo::vec3*>(terrain_normals->data);

      auto temp_indices = ludo::allocate_buffer(3 * sizeof(uint32_t));
      auto temp_vertices = ludo::allocate_buffer(3 * sizeof(ludo::vec3));

      for (auto index = 0; index < terrain_count; index++)
      {
        auto temp_mesh = ludo::mesh();
        terrain_mesh(temp_mesh, temp_indices, temp_vertices, terrain_funcs, radius, ludo::vertex_format_p, ludo::vertex_format_p, false, index, lods[0].level, lods[0].level, lods[0].level);
        auto corners = reinterpret_cast<ludo::vec3*>(temp_vertices.data);

        auto normal = ludo::cross(corners[1] - corners[0], corners[2] - corners[0]);
        ludo::normalize(normal);

        local_positions[index] = (corners[0] + corners[1] + corners[2]) / 3.0f;
        normals[index] = normal;
      }

      ludo::free_buffer(temp_indices);
      ludo::free_buffer(temp_vertices);

      auto write_stream = std::ofstream(metadata_file_name, std::ios::binary);
      write_stream.write(reinterpret_cast<const char*>(local_positions), terrain_count * sizeof(ludo::vec3));
      write_stream.write(reinterpret_cast<const char*>(normals), terrain_count * sizeof(ludo::vec3));
    }

    return container;
  }
}
