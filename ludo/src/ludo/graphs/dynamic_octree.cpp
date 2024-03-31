/*
 * This file is part of ludo. See the LICENSE file for the full license governing this code.
 */

#include "../graphs.h"

namespace ludo
{
  bool add(dynamic_octree& octree, dynamic_octree_node& node, const dynamic_octree_element& element);
  void find(std::set<mesh>& results, const dynamic_octree_node& node, const std::function<int32_t(const aabb& bounds)>& test, bool inside);
  uint32_t count(const dynamic_octree_node& node);
  void split(dynamic_octree& octree, dynamic_octree_node& node);
  void merge(dynamic_octree& octree, dynamic_octree_node& node);
  void merge(dynamic_octree& octree, dynamic_octree_node& merger, dynamic_octree_node& mergee);
  std::vector<dynamic_octree_element>::iterator find(dynamic_octree_node& node, const dynamic_octree_element& element);

  std::size_t dynamic_octree_element_node_hash::operator()(const std::pair<uint64_t, uint32_t>& pair) const noexcept
  {
    if (pair.first < pair.second)
    {
      return static_cast<std::size_t>(std::pow(pair.second, 2)) + pair.first;
    }
    else
    {
      return static_cast<std::size_t>(std::pow(pair.first, 2)) + pair.first + pair.second;
    }
  }

  template<>
  dynamic_octree* add(instance& instance, const dynamic_octree& init, const std::string& partition)
  {
    auto octree = add(data<ludo::dynamic_octree>(instance), init, partition);
    octree->id = next_id++;

    return octree;
  }

  void add(dynamic_octree& octree, const dynamic_octree_element& element)
  {
    auto result = add(octree, octree.root, element);
    assert(result && "could not fit element!");
  }

  bool add(dynamic_octree& octree, dynamic_octree_node& node, const dynamic_octree_element& element)
  {
    if (!contains(node.bounds, element.bounds))
    {
      return false;
    }

    auto max_depth_reached = node.depth == octree.max_depth;
    auto split_threshold_reached = node.elements.size() >= octree.split_threshold;

    if (split_threshold_reached && !max_depth_reached && node.children.empty())
    {
      split(octree, node);
    }

    for (auto& child : node.children)
    {
      if (add(octree, child, element))
      {
        return true;
      }
    }

    node.elements.emplace_back(element);
    octree.element_nodes[{ element.mesh.id, element.mesh.instance_start }] = &node;

    return true;
  }

  bool remove(dynamic_octree& octree, const dynamic_octree_element& element)
  {
    if (octree.element_nodes.find({ element.mesh.id, element.mesh.instance_start }) == octree.element_nodes.end())
    {
      return false;
    }

    auto& node = *octree.element_nodes[{ element.mesh.id, element.mesh.instance_start }];
    auto element_iter = find(node, element);

    octree.element_nodes.erase({ element.mesh.id, element.mesh.instance_start });
    node.elements.erase(element_iter);

    auto merge_node = &node;
    if (count(*merge_node) <= octree.split_threshold)
    {
      while (merge_node->parent && count(*merge_node->parent) < octree.split_threshold)
      {
        merge_node = merge_node->parent;
      }

      merge(octree, *merge_node);
    }

    return true;
  }

  dynamic_octree_element& get(dynamic_octree& octree, const mesh& mesh)
  {
    assert(octree.element_nodes.find({ mesh.id, mesh.instance_start }) != octree.element_nodes.end() && "element not found");

    auto& node = *octree.element_nodes[{ mesh.id, mesh.instance_start }];
    auto element_iter = find(node, dynamic_octree_element { mesh.id, mesh.instance_start });

    return *element_iter;
  }

  void update(dynamic_octree& octree, const dynamic_octree_element& element)
  {
    assert(octree.element_nodes.find({ element.mesh.id, element.mesh.instance_start }) != octree.element_nodes.end() && "element not found");

    auto& node = *octree.element_nodes[{ element.mesh.id, element.mesh.instance_start }];

    if (contains(node.bounds, element.bounds))
    {
      auto element_iter = find(node, element);

      auto added = false;
      for (auto& child : node.children)
      {
        if (add(octree, child, element))
        {
          added = true;
          node.elements.erase(element_iter);
          break;
        }
      }

      if (!added)
      {
        *element_iter = element;
      }
    }
    else
    {
      auto element_copy = element;
      remove(octree, element);
      add(octree, element_copy);
    }
  }

  std::set<mesh> find(const dynamic_octree& octree, const std::function<int32_t(const aabb& bounds)>& test)
  {
    auto meshes = std::set<mesh>();
    find(meshes, octree.root, test, false);

    return meshes;
  }

  void find(std::set<mesh>& meshes, const dynamic_octree_node& node, const std::function<int32_t(const aabb& bounds)>& test, bool inside)
  {
    auto test_result = inside ? 1 : test(node.bounds);
    if (test_result == -1)
    {
      return;
    }

    for (auto& element : node.elements)
    {
      auto element_test_result = test_result == 1 ? 1 : test(element.bounds);
      if (element_test_result == -1)
      {
        continue;
      }

      meshes.insert(element.mesh);
    }

    for (auto& child : node.children)
    {
      find(meshes, child, test, test_result == 1);
    }
  }

  uint32_t count(const dynamic_octree_node& node)
  {
    auto element_count = node.elements.size();

    for (auto& child : node.children)
    {
      element_count += count(child);
    }

    return element_count;
  }

  void split(dynamic_octree& octree, dynamic_octree_node& node)
  {
    auto center = (node.bounds.min + node.bounds.max) / 2.0f;

    auto offset_x = center[0] - node.bounds.min[0];
    auto offset_y = center[1] - node.bounds.min[1];
    auto offset_z = center[2] - node.bounds.min[2];
    auto offsets = std::array<vec3, 8>
    {
      vec3 { 0.0f, 0.0f, 0.0f },
      vec3 { offset_x, 0.0f, 0.0f },
      vec3 { 0.0f, offset_y, 0.0f },
      vec3 { offset_x, offset_y, 0.0f },
      vec3 { 0.0f, 0.0f, offset_z },
      vec3 { offset_x, 0.0f, offset_z },
      vec3 { 0.0f, offset_y, offset_z },
      vec3 { offset_x, offset_y, offset_z }
    };

    for (auto& offset : offsets)
    {
      node.children.emplace_back(dynamic_octree_node
      {
        .depth = node.depth + 1,
        .bounds = { .min = node.bounds.min + offset, .max = center + offset },
        .parent = &node
      });
    }

    for (auto element_iter = node.elements.begin(); element_iter != node.elements.end();)
    {
      auto added = false;
      for (auto& child : node.children)
      {
        if (add(octree, child, *element_iter))
        {
          added = true;
          element_iter = node.elements.erase(element_iter);
          break;
        }
      }

      if (!added)
      {
        element_iter++;
      }
    }
  }

  void merge(dynamic_octree& octree, dynamic_octree_node& node)
  {
    for (auto& child : node.children)
    {
      merge(octree, node, child);
    }

    node.children.clear();
  }

  void merge(dynamic_octree& octree, dynamic_octree_node& merger, dynamic_octree_node& mergee)
  {
    for (auto& element : mergee.elements)
    {
      merger.elements.emplace_back(element);
      octree.element_nodes[{ element.mesh.id, element.mesh.instance_start }] = &merger;
    }

    for (auto& child : mergee.children)
    {
      merge(octree, merger, child);
    }
  }

  std::vector<dynamic_octree_element>::iterator find(dynamic_octree_node& node, const dynamic_octree_element& element)
  {
    return std::find_if(node.elements.begin(), node.elements.end(),
      [&element](const dynamic_octree_element& node_element)
      {
        return node_element.mesh.id == element.mesh.id && node_element.mesh.instance_start == element.mesh.instance_start;
      }
    );
  }
}
