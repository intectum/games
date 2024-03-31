/*
 * This file is part of ludo. See the LICENSE file for the full license governing this code.
 */

#include <ludo/graphs.h>
#include <ludo/testing.h>

#include "dynamic_octree.h"

namespace ludo
{
  void test_graphs_dynamic_octree()
  {
    test_group("dynamic_octree");

    auto bounds_1 = aabb { .min = { -1.0f, -1.0f, -1.0f }, .max = { 1.0f, 1.0f, 1.0f } };
    auto bounds_2 = aabb { .min = { -0.5f, -0.5f, -0.5f }, .max = { 0.5f, 0.5f, 0.5f } };
    auto bounds_3 = aabb { .min = { -0.2f, -0.2f, -0.2f }, .max = { -0.8f, -0.8f, -0.8f } };
    auto bounds_4 = aabb { .min = { -0.25f, -0.25f, -0.25f }, .max = { -0.75f, -0.75f, -0.75f } };
    auto bounds_5 = aabb { .min = { 0.25f, 0.25f, 0.25f }, .max = { 0.75f, 0.75f, 0.75f } };

    auto octree_1 = dynamic_octree { .root = { .bounds = bounds_1 }, .split_threshold = 1 };
    auto element_1 = dynamic_octree_element { .mesh_instance = { .id = 1 }, .bounds = bounds_2 };
    add(octree_1, element_1);
    test_equal("dynamic_octree: add (child count)", octree_1.root.children.size(), std::size_t(0));
    test_equal("dynamic_octree: add (root element count)", octree_1.root.elements.size(), std::size_t(1));
    test_equal("dynamic_octree: add (element-node count)", octree_1.element_nodes.size(), std::size_t(1));
    test_equal("dynamic_octree: add (element-node 1)", octree_1.element_nodes[element_1.mesh_instance.id], &octree_1.root);

    auto element_2 = dynamic_octree_element { .mesh_instance = { .id = 2 }, .bounds = bounds_2 };
    add(octree_1, element_2);
    test_equal("dynamic_octree: add with split (child count)", octree_1.root.children.size(), std::size_t(8));
    test_equal("dynamic_octree: add with split (root element count)", octree_1.root.elements.size(), std::size_t(2));
    test_equal("dynamic_octree: add with split (child 0 element count)", octree_1.root.children[0].elements.size(), std::size_t(0));
    test_equal("dynamic_octree: add with split (child 1 element count)", octree_1.root.children[1].elements.size(), std::size_t(0));
    test_equal("dynamic_octree: add with split (child 2 element count)", octree_1.root.children[2].elements.size(), std::size_t(0));
    test_equal("dynamic_octree: add with split (child 3 element count)", octree_1.root.children[3].elements.size(), std::size_t(0));
    test_equal("dynamic_octree: add with split (child 4 element count)", octree_1.root.children[4].elements.size(), std::size_t(0));
    test_equal("dynamic_octree: add with split (child 5 element count)", octree_1.root.children[5].elements.size(), std::size_t(0));
    test_equal("dynamic_octree: add with split (child 6 element count)", octree_1.root.children[6].elements.size(), std::size_t(0));
    test_equal("dynamic_octree: add with split (child 7 element count)", octree_1.root.children[7].elements.size(), std::size_t(0));
    test_equal("dynamic_octree: add with split (element-node count)", octree_1.element_nodes.size(), std::size_t(2));
    test_equal("dynamic_octree: add with split (element-node 1)", octree_1.element_nodes.find(element_1.mesh_instance.id)->second, &octree_1.root);
    test_equal("dynamic_octree: add with split (element-node 2)", octree_1.element_nodes.find(element_2.mesh_instance.id)->second, &octree_1.root);

    auto element_3 = dynamic_octree_element { .mesh_instance = { .id = 3 }, .bounds = bounds_3 };
    add(octree_1, element_3);
    test_equal("dynamic_octree: add into child (child count)", octree_1.root.children.size(), std::size_t(8));
    test_equal("dynamic_octree: add into child (root element count)", octree_1.root.elements.size(), std::size_t(2));
    test_equal("dynamic_octree: add into child (child 0 element count)", octree_1.root.children[0].elements.size(), std::size_t(1));
    test_equal("dynamic_octree: add into child (child 1 element count)", octree_1.root.children[1].elements.size(), std::size_t(0));
    test_equal("dynamic_octree: add into child (child 2 element count)", octree_1.root.children[2].elements.size(), std::size_t(0));
    test_equal("dynamic_octree: add into child (child 3 element count)", octree_1.root.children[3].elements.size(), std::size_t(0));
    test_equal("dynamic_octree: add into child (child 4 element count)", octree_1.root.children[4].elements.size(), std::size_t(0));
    test_equal("dynamic_octree: add into child (child 5 element count)", octree_1.root.children[5].elements.size(), std::size_t(0));
    test_equal("dynamic_octree: add into child (child 6 element count)", octree_1.root.children[6].elements.size(), std::size_t(0));
    test_equal("dynamic_octree: add into child (child 7 element count)", octree_1.root.children[7].elements.size(), std::size_t(0));
    test_equal("dynamic_octree: add into child (element-node count)", octree_1.element_nodes.size(), std::size_t(3));
    test_equal("dynamic_octree: add into child (element-node 1)", octree_1.element_nodes.find(element_1.mesh_instance.id)->second, &octree_1.root);
    test_equal("dynamic_octree: add into child (element-node 2)", octree_1.element_nodes.find(element_2.mesh_instance.id)->second, &octree_1.root);
    test_equal("dynamic_octree: add into child (element-node 3)", octree_1.element_nodes.find(element_3.mesh_instance.id)->second, &octree_1.root.children[0]);

    remove(octree_1, element_2);
    test_equal("dynamic_octree: remove (child count)", octree_1.root.children.size(), std::size_t(8));
    test_equal("dynamic_octree: remove (root element count)", octree_1.root.elements.size(), std::size_t(1));
    test_equal("dynamic_octree: remove (child 0 element count)", octree_1.root.children[0].elements.size(), std::size_t(1));
    test_equal("dynamic_octree: remove (child 1 element count)", octree_1.root.children[1].elements.size(), std::size_t(0));
    test_equal("dynamic_octree: remove (child 2 element count)", octree_1.root.children[2].elements.size(), std::size_t(0));
    test_equal("dynamic_octree: remove (child 3 element count)", octree_1.root.children[3].elements.size(), std::size_t(0));
    test_equal("dynamic_octree: remove (child 4 element count)", octree_1.root.children[4].elements.size(), std::size_t(0));
    test_equal("dynamic_octree: remove (child 5 element count)", octree_1.root.children[5].elements.size(), std::size_t(0));
    test_equal("dynamic_octree: remove (child 6 element count)", octree_1.root.children[6].elements.size(), std::size_t(0));
    test_equal("dynamic_octree: remove (child 7 element count)", octree_1.root.children[7].elements.size(), std::size_t(0));
    test_equal("dynamic_octree: remove (element-node count)", octree_1.element_nodes.size(), std::size_t(2));
    test_equal("dynamic_octree: remove (element-node 1)", octree_1.element_nodes.find(element_1.mesh_instance.id)->second, &octree_1.root);
    test_equal("dynamic_octree: remove (element-node 2)", octree_1.element_nodes.find(element_2.mesh_instance.id) == octree_1.element_nodes.end(), true);
    test_equal("dynamic_octree: remove (element-node 3)", octree_1.element_nodes.find(element_3.mesh_instance.id)->second, &octree_1.root.children[0]);

    remove(octree_1, element_1);
    test_equal("dynamic_octree: remove with merge (child count)", octree_1.root.children.size(), std::size_t(0));
    test_equal("dynamic_octree: remove with merge (root element count)", octree_1.root.elements.size(), std::size_t(1));
    test_equal("dynamic_octree: remove with merge (element-node count)", octree_1.element_nodes.size(), std::size_t(1));
    test_equal("dynamic_octree: remove with merge (element-node 1)", octree_1.element_nodes.find(element_1.mesh_instance.id) == octree_1.element_nodes.end(), true);
    test_equal("dynamic_octree: remove with merge (element-node 2)", octree_1.element_nodes.find(element_2.mesh_instance.id) == octree_1.element_nodes.end(), true);
    test_equal("dynamic_octree: remove with merge (element-node 3)", octree_1.element_nodes.find(element_3.mesh_instance.id)->second, &octree_1.root);

    auto element_3_got = get(octree_1, element_3.mesh_instance);
    test_equal("dynamic_octree: get", element_3_got.mesh_instance.id, element_3.mesh_instance.id);

    add(octree_1, element_1);
    add(octree_1, element_2);
    element_3.bounds = bounds_4;
    update(octree_1, element_3);
    test_equal("dynamic_octree: update (child count)", octree_1.root.children.size(), std::size_t(8));
    test_equal("dynamic_octree: update (root element count)", octree_1.root.elements.size(), std::size_t(2));
    test_equal("dynamic_octree: update (child 0 element count)", octree_1.root.children[0].elements.size(), std::size_t(1));
    test_equal("dynamic_octree: update (child 1 element count)", octree_1.root.children[1].elements.size(), std::size_t(0));
    test_equal("dynamic_octree: update (child 2 element count)", octree_1.root.children[2].elements.size(), std::size_t(0));
    test_equal("dynamic_octree: update (child 3 element count)", octree_1.root.children[3].elements.size(), std::size_t(0));
    test_equal("dynamic_octree: update (child 4 element count)", octree_1.root.children[4].elements.size(), std::size_t(0));
    test_equal("dynamic_octree: update (child 5 element count)", octree_1.root.children[5].elements.size(), std::size_t(0));
    test_equal("dynamic_octree: update (child 6 element count)", octree_1.root.children[6].elements.size(), std::size_t(0));
    test_equal("dynamic_octree: update (child 7 element count)", octree_1.root.children[7].elements.size(), std::size_t(0));
    test_equal("dynamic_octree: update (element-node count)", octree_1.element_nodes.size(), std::size_t(3));
    test_equal("dynamic_octree: update (element-node 1)", octree_1.element_nodes.find(element_1.mesh_instance.id)->second, &octree_1.root);
    test_equal("dynamic_octree: update (element-node 2)", octree_1.element_nodes.find(element_2.mesh_instance.id)->second, &octree_1.root);
    test_equal("dynamic_octree: update (element-node 3)", octree_1.element_nodes.find(element_3.mesh_instance.id)->second, &octree_1.root.children[0]);

    element_3.bounds = bounds_5;
    update(octree_1, element_3);
    test_equal("dynamic_octree: update move to sibling (child count)", octree_1.root.children.size(), std::size_t(8));
    test_equal("dynamic_octree: update move to sibling (root element count)", octree_1.root.elements.size(), std::size_t(2));
    test_equal("dynamic_octree: update move to sibling (child 0 element count)", octree_1.root.children[0].elements.size(), std::size_t(0));
    test_equal("dynamic_octree: update move to sibling (child 1 element count)", octree_1.root.children[1].elements.size(), std::size_t(0));
    test_equal("dynamic_octree: update move to sibling (child 2 element count)", octree_1.root.children[2].elements.size(), std::size_t(0));
    test_equal("dynamic_octree: update move to sibling (child 3 element count)", octree_1.root.children[3].elements.size(), std::size_t(0));
    test_equal("dynamic_octree: update move to sibling (child 4 element count)", octree_1.root.children[4].elements.size(), std::size_t(0));
    test_equal("dynamic_octree: update move to sibling (child 5 element count)", octree_1.root.children[5].elements.size(), std::size_t(0));
    test_equal("dynamic_octree: update move to sibling (child 6 element count)", octree_1.root.children[6].elements.size(), std::size_t(0));
    test_equal("dynamic_octree: update move to sibling (child 7 element count)", octree_1.root.children[7].elements.size(), std::size_t(1));
    test_equal("dynamic_octree: update move to sibling (element-node count)", octree_1.element_nodes.size(), std::size_t(3));
    test_equal("dynamic_octree: update move to sibling (element-node 1)", octree_1.element_nodes.find(element_1.mesh_instance.id)->second, &octree_1.root);
    test_equal("dynamic_octree: update move to sibling (element-node 2)", octree_1.element_nodes.find(element_2.mesh_instance.id)->second, &octree_1.root);
    test_equal("dynamic_octree: update move to sibling (element-node 3)", octree_1.element_nodes.find(element_3.mesh_instance.id)->second, &octree_1.root.children[7]);

    element_3.bounds = bounds_2;
    update(octree_1, element_3);
    test_equal("dynamic_octree: update move to parent (child count)", octree_1.root.children.size(), std::size_t(8));
    test_equal("dynamic_octree: update move to parent (root element count)", octree_1.root.elements.size(), std::size_t(3));
    test_equal("dynamic_octree: update move to parent (child 0 element count)", octree_1.root.children[0].elements.size(), std::size_t(0));
    test_equal("dynamic_octree: update move to parent (child 1 element count)", octree_1.root.children[1].elements.size(), std::size_t(0));
    test_equal("dynamic_octree: update move to parent (child 2 element count)", octree_1.root.children[2].elements.size(), std::size_t(0));
    test_equal("dynamic_octree: update move to parent (child 3 element count)", octree_1.root.children[3].elements.size(), std::size_t(0));
    test_equal("dynamic_octree: update move to parent (child 4 element count)", octree_1.root.children[4].elements.size(), std::size_t(0));
    test_equal("dynamic_octree: update move to parent (child 5 element count)", octree_1.root.children[5].elements.size(), std::size_t(0));
    test_equal("dynamic_octree: update move to parent (child 6 element count)", octree_1.root.children[6].elements.size(), std::size_t(0));
    test_equal("dynamic_octree: update move to parent (child 7 element count)", octree_1.root.children[7].elements.size(), std::size_t(0));
    test_equal("dynamic_octree: update move to parent (element-node count)", octree_1.element_nodes.size(), std::size_t(3));
    test_equal("dynamic_octree: update move to parent (element-node 1)", octree_1.element_nodes.find(element_1.mesh_instance.id)->second, &octree_1.root);
    test_equal("dynamic_octree: update move to parent (element-node 2)", octree_1.element_nodes.find(element_2.mesh_instance.id)->second, &octree_1.root);
    test_equal("dynamic_octree: update move to parent (element-node 3)", octree_1.element_nodes.find(element_3.mesh_instance.id)->second, &octree_1.root);

    element_3.bounds = bounds_3;
    update(octree_1, element_3);
    test_equal("dynamic_octree: update move to child (child count)", octree_1.root.children.size(), std::size_t(8));
    test_equal("dynamic_octree: update move to child (root element count)", octree_1.root.elements.size(), std::size_t(2));
    test_equal("dynamic_octree: update move to child (child 0 element count)", octree_1.root.children[0].elements.size(), std::size_t(1));
    test_equal("dynamic_octree: update move to child (child 1 element count)", octree_1.root.children[1].elements.size(), std::size_t(0));
    test_equal("dynamic_octree: update move to child (child 2 element count)", octree_1.root.children[2].elements.size(), std::size_t(0));
    test_equal("dynamic_octree: update move to child (child 3 element count)", octree_1.root.children[3].elements.size(), std::size_t(0));
    test_equal("dynamic_octree: update move to child (child 4 element count)", octree_1.root.children[4].elements.size(), std::size_t(0));
    test_equal("dynamic_octree: update move to child (child 5 element count)", octree_1.root.children[5].elements.size(), std::size_t(0));
    test_equal("dynamic_octree: update move to child (child 6 element count)", octree_1.root.children[6].elements.size(), std::size_t(0));
    test_equal("dynamic_octree: update move to child (child 7 element count)", octree_1.root.children[7].elements.size(), std::size_t(0));
    test_equal("dynamic_octree: update move to child (element-node count)", octree_1.element_nodes.size(), std::size_t(3));
    test_equal("dynamic_octree: update move to child (element-node 1)", octree_1.element_nodes.find(element_1.mesh_instance.id)->second, &octree_1.root);
    test_equal("dynamic_octree: update move to child (element-node 2)", octree_1.element_nodes.find(element_2.mesh_instance.id)->second, &octree_1.root);
    test_equal("dynamic_octree: update move to child (element-node 3)", octree_1.element_nodes.find(element_3.mesh_instance.id)->second, &octree_1.root.children[0]);

    auto meshes_1 = find(octree_1, [&bounds_1](const aabb& bounds)
    {
      return contains(bounds_1, bounds) ? 1 : -1;
    });
    test_equal("dynamic_octree: find", meshes_1.size(), std::size_t(3));

    auto meshes_2 = find(octree_1, [&bounds_5](const aabb& bounds)
    {
      return contains(bounds_5, bounds) ? 1 : -1;
    });
    test_equal("dynamic_octree: find 2", meshes_2.size(), std::size_t(0));
  }
}
