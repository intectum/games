/*
 * This file is part of ludo. See the LICENSE file for the full license governing this code.
 */

#include "../meshes/ico_faces.h"
#include "ico_tree.h"

namespace astrum
{
  uint32_t find_cell(const ico_tree& ico_tree, const ico_tree_test& test, uint32_t divisions, const std::array<ludo::vec3, 3>& face, uint32_t cumulative_index);
  void find_cells(const ico_tree& ico_tree, const ico_tree_test& test, uint32_t divisions, const std::array<ludo::vec3, 3>& face, uint32_t cumulative_index, std::vector<uint32_t>& results);
  uint32_t cell_element_index(const ico_tree& ico_tree, uint32_t cell_index, const std::byte* element_data);
  uint64_t cell_offset(const ico_tree& ico_tree, uint32_t cell_index);
  std::array<std::array<ludo::vec3, 3>, 4> divided_faces(const std::array<ludo::vec3, 3>& face);
  bool within(const std::array<ludo::vec3, 3>& face, const ludo::vec3& position);

  void init(ico_tree& ico_tree)
  {
    ico_tree.id = ludo::next_id++;

    auto cell_size = sizeof(uint32_t) + ico_tree.cell_capacity * ico_tree.cell_element_size;

    auto cell_count = astrum::cell_count(ico_tree);
    auto data_size = cell_count * cell_size;
    ico_tree.buffer = ludo::allocate(data_size);

    auto offset = uint32_t(0);
    for (auto cell_index = uint32_t(0); cell_index < cell_count; cell_index++)
    {
      cast<uint32_t>(ico_tree.buffer, offset) = 0;
      offset += cell_size;
    }
  }

  void destroy(ico_tree& ico_tree)
  {
    deallocate(ico_tree.buffer);
  }

  void add(ico_tree& ico_tree, const std::byte* element_data, const ludo::vec3& position)
  {
    auto cell_index = find_cell(ico_tree, [&](const std::array<ludo::vec3, 3>& face) { return within(face, position); });
    assert(cell_index != std::numeric_limits<uint32_t>::max() && "cell not found");

    auto offset = cell_offset(ico_tree, cell_index);

    auto& element_count = cast<uint32_t>(ico_tree.buffer, offset);
    assert(element_count < ico_tree.cell_capacity && "cell is full");

    offset += sizeof(uint32_t) + element_count * ico_tree.cell_element_size;
    std::memcpy(ico_tree.buffer.data + offset, element_data, ico_tree.cell_element_size);

    element_count++;
  }

  void remove(ico_tree& ico_tree, const std::byte* element_data, const ludo::vec3& position)
  {
    auto cell_index = find_cell(ico_tree, [&](const std::array<ludo::vec3, 3>& face) { return within(face, position); });
    assert(cell_index != std::numeric_limits<uint32_t>::max() && "cell not found");

    auto element_index = cell_element_index(ico_tree, cell_index, element_data);
    assert(element_index < ico_tree.cell_capacity && "element not found");

    auto offset = cell_offset(ico_tree, cell_index);
    auto stream = ludo::stream(ico_tree.buffer, offset);

    auto element_count = peek<uint32_t>(stream) - 1;
    write(stream, element_count);
    stream.position += element_index * ico_tree.cell_element_size;
    std::memmove(
      ico_tree.buffer.data + stream.position,
      ico_tree.buffer.data + stream.position + ico_tree.cell_element_size,
      (element_count - element_index) * ico_tree.cell_element_size
    );
  }

  uint32_t find_cell(const ico_tree& ico_tree, const ico_tree_test& test)
  {
    auto& ico_faces = get_ico_faces();

    for (auto face_index = uint32_t(0); face_index < ico_faces.size(); face_index++)
    {
      auto cell_index = find_cell(ico_tree, test, ico_tree.divisions, ico_faces[face_index], face_index);
      if (cell_index != std::numeric_limits<uint32_t>::max())
      {
        return cell_index;
      }
    }

    return std::numeric_limits<uint32_t>::max();
  }

  std::vector<uint32_t> find_cells(const ico_tree& ico_tree, const ico_tree_test& test)
  {
    auto results = std::vector<uint32_t>();
    auto& ico_faces = get_ico_faces();

    for (auto face_index = uint32_t(0); face_index < ico_faces.size(); face_index++)
    {
      find_cells(ico_tree, test, ico_tree.divisions, ico_faces[face_index], face_index, results);
    }

    return results;
  }

  void add_render_commands(ludo::array<ico_tree>& ico_trees, ludo::array<ludo::render_program>& render_programs, const ludo::heap& render_commands, const ludo::camera& camera)
  {
    auto frustum_planes = ludo::frustum_planes(camera);

    for (auto& ico_tree : ico_trees)
    {
      // TODO ico_tree needs to have a position and radius for correct frustum culling!
      auto cell_indices = find_cells(ico_tree, [&](const std::array<ludo::vec3, 3>& face)
      {
        auto bounds = ludo::aabb3
        {
          .min =
          {
            std::min(face[0][0], std::min(face[1][0], face[2][0])),
            std::min(face[0][1], std::min(face[1][1], face[2][1])),
            std::min(face[0][2], std::min(face[1][2], face[2][2]))
          },
          .max =
          {
            std::max(face[0][0], std::max(face[1][0], face[2][0])),
            std::max(face[0][1], std::max(face[1][1], face[2][1])),
            std::max(face[0][2], std::max(face[1][2], face[2][2]))
          }
        };

        return ludo::frustum_test(frustum_planes, bounds) != -1;
      });

      for (auto cell_index : cell_indices)
      {
        auto element_data = cell_element_data(ico_tree, cell_index);
        auto stream = ludo::stream(element_data);
        while (!ludo::ended(stream))
        {
          auto& render_mesh = ludo::read<ludo::render_mesh>(stream);
          auto render_program = ludo::find_by_id(render_programs.begin(), render_programs.end(), render_mesh.render_program_id);
          ludo::add_render_command(*render_program, render_mesh);
        }
      }
    }
  }

  uint32_t find_cell(const ico_tree& ico_tree, const ico_tree_test& test, uint32_t divisions, const std::array<ludo::vec3, 3>& face, uint32_t cumulative_index)
  {
    if (!test(face))
    {
      return std::numeric_limits<uint32_t>::max();
    }

    if (divisions == 0)
    {
      return cumulative_index;
    }

    auto divided_faces = astrum::divided_faces(face);
    for (auto face_index = uint32_t(0); face_index < divided_faces.size(); face_index++)
    {
      auto cell_index = find_cell(ico_tree, test, divisions - 1, divided_faces[face_index], cumulative_index * 4 + face_index);
      if (cell_index != std::numeric_limits<uint32_t>::max())
      {
        return cell_index;
      }
    }

    return std::numeric_limits<uint32_t>::max();
  }

  void find_cells(const ico_tree& ico_tree, const ico_tree_test& test, uint32_t divisions, const std::array<ludo::vec3, 3>& face, uint32_t cumulative_index, std::vector<uint32_t>& results)
  {
    if (!test(face))
    {
      return;
    }

    if (divisions == 0)
    {
      results.push_back(cumulative_index);
      return;
    }

    auto divided_faces = astrum::divided_faces(face);
    for (auto face_index = uint32_t(0); face_index < divided_faces.size(); face_index++)
    {
      find_cells(ico_tree, test, divisions - 1, divided_faces[face_index], cumulative_index * 4 + face_index, results);
    }
  }

  uint32_t cell_count(const ico_tree& ico_tree)
  {
    return static_cast<uint32_t>(20 * std::pow(4, ico_tree.divisions));
  }

  ludo::buffer cell_element_data(const ico_tree& ico_tree, uint32_t cell_index)
  {
    auto offset = cell_offset(ico_tree, cell_index);
    auto stream = ludo::stream(ico_tree.buffer, offset);

    auto element_count = read<uint32_t>(stream);

    return
    {
      .data = stream.data + stream.position,
      .size = element_count * ico_tree.cell_element_size
    };
  }

  uint32_t cell_element_index(const ico_tree& ico_tree, uint32_t cell_index, const std::byte* element_data)
  {
    auto offset = cell_offset(ico_tree, cell_index);
    auto stream = ludo::stream(ico_tree.buffer, offset);

    auto element_count = read<uint32_t>(stream);
    for (auto element_index = uint32_t(0); element_index < element_count; element_index++)
    {
      if (std::memcmp(stream.data + stream.position, element_data, ico_tree.cell_element_size) == 0)
      {
        return element_index;
      }

      stream.position += ico_tree.cell_element_size;
    }

    return ico_tree.cell_capacity;
  }

  uint64_t cell_offset(const ico_tree& ico_tree, uint32_t cell_index)
  {
    return cell_index * (sizeof(uint32_t) + ico_tree.cell_capacity * ico_tree.cell_element_size);
  }

  std::array<std::array<ludo::vec3, 3>, 4> divided_faces(const std::array<ludo::vec3, 3>& face)
  {
    auto center_positions = std::array<ludo::vec3, 3>
    {{
       (face[0] + face[1]) * 0.5f,
       (face[0] + face[2]) * 0.5f,
       (face[1] + face[2]) * 0.5f,
    }};

    normalize(center_positions[0]);
    normalize(center_positions[1]);
    normalize(center_positions[2]);

    return
    {{
      { face[0], center_positions[0], center_positions[1] },
      { center_positions[0], face[1], center_positions[2] },
      { center_positions[1], center_positions[2], face[2] },
      { center_positions[0], center_positions[2], center_positions[1] }
    }};
  }

  bool within(const std::array<ludo::vec3, 3>& face, const ludo::vec3& position)
  {
    // TODO refine
    if (ludo::dot(face[0], position) < 0.0f)
    {
      return false;
    }

    auto u = face[1] - face[0];
    auto v = face[2] - face[0];
    auto normal = ludo::cross(u, v);
    ludo::normalize(normal);

    auto projected_position = ludo::project_point_onto_plane(position, face[0], normal);
    auto w = projected_position - face[0];

    auto uu = ludo::dot(u, u);
    auto uv = ludo::dot(u, v);
    auto vv = ludo::dot(v, v);
    auto wu = ludo::dot(w, u);
    auto wv = ludo::dot(w, v);

    // Barycentric coordinates
    auto denominator = uu * vv - uv * uv;
    auto s = (vv * wu - uv * wv) / denominator;
    auto t = (uu * wv - uv * wu) / denominator;

    // TODO why is this epsilon so f***ing big?
    auto epsilon = 0.05f;
    return s >= -epsilon && t >= -epsilon && s + t <= 1.0f + epsilon;
  }
}
