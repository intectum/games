#include <ludo/assimp/api.h>

#include "../types.h"
#include "trees.h"

namespace astrum
{
  auto lod_max_distances = std::vector<float> { 10.0f, 0.0f };

  void add_trees(ludo::instance& inst, uint32_t celestial_body_index)
  {
    auto& linear_octree = *ludo::first<ludo::linear_octree>(inst, "default");
    auto& rendering_context = *ludo::first<ludo::rendering_context>(inst);

    auto& patchwork = ludo::data<astrum::patchwork>(inst, "celestial-bodies")[celestial_body_index];
    auto& point_mass = ludo::data<astrum::point_mass>(inst, "celestial-bodies")[celestial_body_index];
    auto& celestial_body = ludo::data<astrum::celestial_body>(inst, "celestial-bodies")[celestial_body_index];
    auto& celestial_body_mesh_buffer = ludo::data<ludo::mesh_buffer>(inst, "celestial-bodies")[celestial_body_index];

    auto max_tree_count = static_cast<uint32_t>(patchwork.patches.size()) * 25;
    auto max_tree_1_count = static_cast<uint32_t>(patchwork.patches.size()) * 25;

    auto tree_import_options = ludo::import_options
    {
      .merge = true,
      .instance_count = max_tree_count
    };

    auto tree_1_import_options = ludo::import_options
    {
      .merge = true,
      .instance_count = max_tree_1_count
    };

    auto tree_mesh_buffer_options = ludo::build_mesh_buffer_options("assets/models/fruit-tree.dae", tree_import_options);
    auto tree_1_mesh_buffer_options = ludo::build_mesh_buffer_options("assets/models/fruit-tree-1.dae", tree_1_import_options);

    auto mesh_buffer = ludo::add(
      inst,
      ludo::mesh_buffer(),
      {
        .instance_count = tree_mesh_buffer_options.instance_count + tree_1_mesh_buffer_options.instance_count,
        .index_count = tree_mesh_buffer_options.index_count + tree_1_mesh_buffer_options.index_count,
        .vertex_count = tree_mesh_buffer_options.vertex_count + tree_1_mesh_buffer_options.vertex_count,
        .normals = true,
        .colors = tree_mesh_buffer_options.colors || tree_1_mesh_buffer_options.colors,
        .texture_count = tree_mesh_buffer_options.texture_count + tree_1_mesh_buffer_options.texture_count,
        .bone_count = 0
      },
      "trees"
    );

    tree_import_options.mesh_buffer = mesh_buffer;
    tree_1_import_options.mesh_buffer = mesh_buffer;
    tree_1_import_options.index_start = tree_mesh_buffer_options.index_count;
    tree_1_import_options.vertex_start = tree_mesh_buffer_options.vertex_count;

    ludo::import(inst, "assets/models/fruit-tree.dae", tree_import_options, "trees");
    ludo::import(inst, "assets/models/fruit-tree-1.dae", tree_1_import_options, "trees");

    auto& meshes = ludo::data<ludo::mesh>(inst, "trees");
    meshes[1].instance_start = max_tree_count;

    auto camera = ludo::get_camera(rendering_context);
    auto to_camera = ludo::position(camera.view) - ludo::position(ludo::get_transform(celestial_body_mesh_buffer, 0));

    auto tree_instance_index = uint32_t(0);
    auto tree_1_instance_index = uint32_t(0);

    for (auto patch_index = 0; patch_index < patchwork.patches.size(); patch_index++)
    {
      auto trees = celestial_body.tree_func(patch_index);
      for (auto& tree : trees)
      {
        auto tree_final_position = point_mass.transform.position + tree.position * celestial_body.height_func(tree.position) * celestial_body.radius;

        auto rotation = ludo::mat3(ludo::vec3_unit_y, tree.position) * ludo::mat3(ludo::vec3_unit_y, tree.rotation);
        auto transform = ludo::mat4(tree_final_position, rotation);
        ludo::scale(transform, { tree.scale, tree.scale, tree.scale });

        if (ludo::length(to_camera - tree_final_position) <= lod_max_distances[0])
        {
          auto mesh_instance = meshes[0];
          mesh_instance.instance_start = tree_instance_index;
          mesh_instance.instance_count = 1;

          ludo::set_transform(*mesh_buffer, tree_instance_index, transform);
          ludo::add(linear_octree, mesh_instance, tree_final_position);

          tree_instance_index++;
        }
        else
        {
          auto mesh_instance = meshes[1];
          mesh_instance.instance_start = meshes[1].instance_start + tree_1_instance_index;
          mesh_instance.instance_count = 1;

          ludo::set_transform(*mesh_buffer, meshes[1].instance_start + tree_1_instance_index, transform);
          ludo::add(linear_octree, mesh_instance, tree_final_position);

          tree_1_instance_index++;
        }
      }
    }

    meshes[0].instance_count = tree_instance_index;
    meshes[1].instance_count = tree_1_instance_index;
  }

  void stream_trees(ludo::instance& inst, uint32_t celestial_body_index)
  {
    auto& linear_octree = *ludo::first<ludo::linear_octree>(inst, "default");
    auto& rendering_context = *ludo::first<ludo::rendering_context>(inst);

    auto& mesh_buffer = *ludo::first<ludo::mesh_buffer>(inst, "trees");
    auto& meshes = ludo::data<ludo::mesh>(inst, "trees");

    auto& point_mass = ludo::data<astrum::point_mass>(inst, "celestial-bodies")[celestial_body_index];
    auto& celestial_body_mesh_buffer = ludo::data<ludo::mesh_buffer>(inst, "celestial-bodies")[celestial_body_index];

    auto camera_position = ludo::position(ludo::get_camera(rendering_context).view);
    auto old_position = ludo::position(ludo::get_transform(celestial_body_mesh_buffer, 0));
    auto movement = point_mass.transform.position - old_position;
    auto moved = ludo::length2(movement) > 0.0f;

    for (auto mesh_index = 0; mesh_index < meshes.array_size; mesh_index++)
    {
      auto& mesh = meshes[mesh_index];
      auto mesh_indices = std::vector<uint32_t>(mesh.instance_count, mesh_index);
      auto transforms = std::vector<ludo::mat4>(mesh.instance_count);

      ludo::divide_and_conquer(mesh.instance_count, [&](uint32_t start, uint32_t end)
      {
        for (auto instance_index = mesh.instance_start + start; instance_index < mesh.instance_start + end; instance_index++)
        {
          auto relative_instance_index = instance_index - mesh.instance_start;

          transforms[relative_instance_index] = ludo::get_transform(mesh_buffer, instance_index);
          auto position = ludo::position(transforms[relative_instance_index]);

          if (moved)
          {
            position += movement;
            ludo::position(transforms[relative_instance_index], position);
            ludo::set_transform(mesh_buffer, instance_index, transforms[relative_instance_index]);
          }

          auto distance_to_camera = ludo::length(camera_position - position);
          if (mesh_index > 0 && distance_to_camera <= lod_max_distances[mesh_index - 1])
          {
            mesh_indices[relative_instance_index]--;
          }
          else if (lod_max_distances[mesh_index] != 0.0f && distance_to_camera > lod_max_distances[mesh_index])
          {
            mesh_indices[relative_instance_index]++;
          }
        }

        return []() {};
      });

      for (auto instance_index = mesh.instance_start; instance_index < mesh.instance_start + mesh.instance_count; instance_index++)
      {
        auto relative_instance_index = instance_index - mesh.instance_start;

        auto new_mesh_index = mesh_indices[relative_instance_index];
        if (new_mesh_index == mesh_index)
        {
          continue;
        }

        auto& new_mesh = meshes[new_mesh_index];

        /* TODO!!!
        auto& element = ludo::get(linear_octree, mesh.id, instance_index);
        // TODO cannot do this! Does not update the element_nodes map!
        element.data_id = new_mesh.id;
        element.data_index = new_mesh.instance_count;

        std::memmove(mesh_buffer.data_buffers[0].data + instance_index * sizeof(ludo::mat4), mesh_buffer.data_buffers[0].data + (instance_index + 1) * sizeof(ludo::mat4), (mesh.instance_start + mesh.instance_count - (instance_index + 1)) * sizeof(ludo::mat4));
        mesh.instance_count--;

        ludo::set_transform(mesh_buffer, new_mesh.instance_start + new_mesh.instance_count, transforms[relative_instance_index]);
        new_mesh.instance_count++;

        // Account for removal from currently iterated instances.
        mesh_indices.erase(mesh_indices.begin() + relative_instance_index);
        transforms.erase(transforms.begin() + relative_instance_index);
        instance_index--;*/
      }
    }
  }
}
