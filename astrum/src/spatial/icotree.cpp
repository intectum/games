/*
 * This file is part of ludo. See the LICENSE file for the full license governing this code.
 */

#include "../meshes/ico_faces.h"
#include "icotree.h"

namespace astrum
{
  uint32_t find_cell(const icotree& icotree, const std::function<int32_t(const std::array<ludo::vec3, 3>& face)>& test, uint32_t divisions, const std::array<ludo::vec3, 3>& face, uint32_t cumulative_index);
  void find_cells(const icotree& icotree, const std::function<int32_t(const std::array<ludo::vec3, 3>& face)>& test, uint32_t divisions, const std::array<ludo::vec3, 3>& face, uint32_t cumulative_index, std::vector<uint32_t>& results);
  uint32_t cell_element_index(const icotree& icotree, uint32_t cell_index, const ludo::vec3& element);
  uint64_t cell_offset(const icotree& icotree, uint32_t cell_index);
  std::array<std::array<ludo::vec3, 3>, 4> divided_faces(const std::array<ludo::vec3, 3>& face);
  bool within(const std::array<ludo::vec3, 3>& face, const ludo::vec3& position);

  void init(icotree& icotree)
  {
    auto cell_size = sizeof(uint32_t) + icotree.cell_capacity * sizeof(ludo::vec3);

    auto cell_count = astrum::cell_count(icotree);
    auto data_size = cell_count * cell_size;
    icotree.buffer = ludo::allocate(data_size);

    auto offset = uint32_t(0);
    for (auto cell_index = uint32_t(0); cell_index < cell_count; cell_index++)
    {
      cast<uint32_t>(icotree.buffer, offset) = 0;
      offset += cell_size;
    }
  }

  void destroy(icotree& icotree)
  {
    deallocate(icotree.buffer);
  }

  void add(icotree& icotree, const ludo::vec3& element, const ludo::vec3& position)
  {
    auto cell_index = find_cell(icotree, [&](const std::array<ludo::vec3, 3>& face)
    {
      return within(face, position) ? 1 : -1;
    });

    assert(cell_index != std::numeric_limits<uint32_t>::max() && "cell not found");

    auto offset = cell_offset(icotree, cell_index);
    auto stream = ludo::stream(icotree.buffer, offset);

    auto element_count = peek<uint32_t>(stream);
    assert(element_count < icotree.cell_capacity && "cell is full");

    write(stream, element_count + 1);
    stream.position += element_count * sizeof(ludo::vec3);
    write(stream, element);
  }

  void remove(icotree& icotree, const ludo::vec3& element, const ludo::vec3& position)
  {
    auto cell_index = find_cell(icotree, [&](const std::array<ludo::vec3, 3>& face)
    {
      return within(face, position) ? 1 : -1;
    });

    assert(cell_index != std::numeric_limits<uint32_t>::max() && "cell not found");

    auto element_index = cell_element_index(icotree, cell_index, element);
    assert(element_index < icotree.cell_capacity && "element not found");

    // TODO check adjacent cell in case of floating point inaccuracy?

    auto offset = cell_offset(icotree, cell_index);
    auto stream = ludo::stream(icotree.buffer, offset);

    auto element_count = peek<uint32_t>(stream) - 1;
    write(stream, element_count);
    stream.position += element_index * sizeof(ludo::vec3);
    std::memmove(
      icotree.buffer.data + stream.position,
      icotree.buffer.data + stream.position + sizeof(ludo::vec3),
      (element_count - element_index) * sizeof(ludo::vec3)
    );
  }

  uint32_t find_cell(const icotree& icotree, const std::function<int32_t(const std::array<ludo::vec3, 3>& face)>& test)
  {
    auto& ico_faces = get_ico_faces();

    for (auto face_index = uint32_t(0); face_index < ico_faces.size(); face_index++)
    {
      auto cell_index = find_cell(icotree, test, icotree.divisions, ico_faces[face_index], face_index);
      if (cell_index != std::numeric_limits<uint32_t>::max())
      {
        return cell_index;
      }
    }

    return std::numeric_limits<uint32_t>::max();
  }

  std::vector<uint32_t> find_cells(const icotree& icotree, const std::function<int32_t(const std::array<ludo::vec3, 3>& face)>& test)
  {
    auto results = std::vector<uint32_t>();
    auto& ico_faces = get_ico_faces();

    for (auto face_index = uint32_t(0); face_index < ico_faces.size(); face_index++)
    {
      find_cells(icotree, test, icotree.divisions, ico_faces[face_index], face_index, results);
    }

    return results;
  }

  uint32_t find_cell(const icotree& icotree, const std::function<int32_t(const std::array<ludo::vec3, 3>& face)>& test, uint32_t divisions, const std::array<ludo::vec3, 3>& face, uint32_t cumulative_index)
  {
    auto test_result = test(face);
    if (test_result == -1)
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
      auto cell_index = find_cell(icotree, test, divisions - 1, divided_faces[face_index], cumulative_index * 4 + face_index);
      if (cell_index != std::numeric_limits<uint32_t>::max())
      {
        return cell_index;
      }
    }

    return std::numeric_limits<uint32_t>::max();
  }

  void find_cells(const icotree& icotree, const std::function<int32_t(const std::array<ludo::vec3, 3>& face)>& test, uint32_t divisions, const std::array<ludo::vec3, 3>& face, uint32_t cumulative_index, std::vector<uint32_t>& results)
  {
    auto test_result = test(face);
    if (test_result == -1)
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
      find_cells(icotree, test, divisions - 1, divided_faces[face_index], cumulative_index * 4 + face_index, results);
    }
  }

  uint32_t cell_count(const icotree& icotree)
  {
    return static_cast<uint32_t>(20 * std::pow(4, icotree.divisions));
  }

  ludo::buffer cell_element_data(const icotree& icotree, uint32_t cell_index)
  {
    auto offset = cell_offset(icotree, cell_index);
    auto stream = ludo::stream(icotree.buffer, offset);

    auto element_count = read<uint32_t>(stream);

    return
    {
      .data = stream.data + stream.position,
      .size = element_count * sizeof(ludo::vec3)
    };
  }

  uint32_t cell_element_index(const icotree& icotree, uint32_t cell_index, const ludo::vec3& element)
  {
    auto offset = cell_offset(icotree, cell_index);
    auto stream = ludo::stream(icotree.buffer, offset);

    auto element_count = read<uint32_t>(stream);
    for (auto element_index = uint32_t(0); element_index < element_count; element_index++)
    {
      if (read<ludo::vec3>(stream) == element)
      {
        return element_index;
      }
    }

    return icotree.cell_capacity;
  }

  uint64_t cell_offset(const icotree& icotree, uint32_t cell_index)
  {
    return cell_index * (sizeof(uint32_t) + icotree.cell_capacity * sizeof(ludo::vec3));
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
