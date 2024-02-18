/*
 * This file is part of ludo. See the LICENSE file for the full license governing this code.
 */

#include <thread>

#include "patchwork.h"

namespace astrum
{
  ludo::mesh* allocate(ludo::instance& inst, patchwork& patchwork, uint32_t patch_index, uint32_t variant_index, const std::string& partition);
  void deallocate(ludo::instance& inst, patchwork& patchwork, uint64_t mesh_id, const std::string& partition);
  void sew(patchwork& patchwork, uint32_t patch_index, bool sew_same_variant);

  void save(const patchwork& patchwork, std::ostream& stream)
  {
    auto variant_count = patchwork.variants.size();
    stream.write(reinterpret_cast<char*>(&variant_count), sizeof(std::size_t));
    for (auto& variant : patchwork.variants)
    {
      auto border_indices_count = variant.border_indices.size();
      stream.write(reinterpret_cast<char*>(&border_indices_count), sizeof(std::size_t));
      for (auto& border_indices : variant.border_indices)
      {
        auto border_indices_count_inner = border_indices.size();
        stream.write(reinterpret_cast<char*>(&border_indices_count_inner), sizeof(std::size_t));
        for (auto border_index : border_indices)
        {
          stream.write(reinterpret_cast<char*>(&border_index), sizeof(uint32_t));
        }
      }

      auto first_unique_indices_count = variant.first_unique_indices.size();
      stream.write(reinterpret_cast<char*>(&first_unique_indices_count), sizeof(std::size_t));
      for (auto& first_unique_index : variant.first_unique_indices)
      {
        stream.write(reinterpret_cast<const char*>(&first_unique_index), sizeof(uint32_t));
      }
    }

    auto patch_count = patchwork.patches.size();
    stream.write(reinterpret_cast<char*>(&patch_count), sizeof(uint64_t));
    for (auto& patch : patchwork.patches)
    {
      stream.write(reinterpret_cast<const char*>(&patch.center), sizeof(ludo::vec3));
      stream.write(reinterpret_cast<const char*>(&patch.normal), sizeof(ludo::vec3));

      auto adjacent_patch_indices_count = patch.adjacent_patch_indices.size();
      stream.write(reinterpret_cast<char*>(&adjacent_patch_indices_count), sizeof(std::size_t));
      for (auto adjacent_patch_index : patch.adjacent_patch_indices)
      {
        stream.write(reinterpret_cast<char*>(&adjacent_patch_index), sizeof(uint32_t));
      }
    }
  }

  void load(patchwork& patchwork, std::istream& stream)
  {
    auto variant_count = size_t();
    stream.read(reinterpret_cast<char*>(&variant_count), sizeof(std::size_t));
    patchwork.variants = std::vector<patch_variant>(variant_count);
    for (auto& variant : patchwork.variants)
    {
      auto border_indices_count = size_t();
      stream.read(reinterpret_cast<char*>(&border_indices_count), sizeof(std::size_t));
      variant.border_indices = std::vector<std::vector<uint32_t>>(border_indices_count);
      for (auto& border_indices : variant.border_indices)
      {
        auto border_indices_count_inner = size_t();
        stream.read(reinterpret_cast<char*>(&border_indices_count_inner), sizeof(std::size_t));
        border_indices = std::vector<uint32_t>(border_indices_count_inner);
        for (auto& border_index : border_indices)
        {
          stream.read(reinterpret_cast<char*>(&border_index), sizeof(uint32_t));
        }
      }

      auto first_unique_indices_count = size_t();
      stream.read(reinterpret_cast<char*>(&first_unique_indices_count), sizeof(std::size_t));
      variant.first_unique_indices = std::vector<uint32_t>(first_unique_indices_count);
      for (auto& first_unique_index : variant.first_unique_indices)
      {
        stream.read(reinterpret_cast<char*>(&first_unique_index), sizeof(uint32_t));
      }
    }

    auto patch_count = uint64_t();
    stream.read(reinterpret_cast<char*>(&patch_count), sizeof(uint64_t));
    patchwork.patches = std::vector<patch>(patch_count);
    for (auto& patch : patchwork.patches)
    {
      stream.read(reinterpret_cast<char*>(&patch.center), sizeof(ludo::vec3));
      stream.read(reinterpret_cast<char*>(&patch.normal), sizeof(ludo::vec3));

      auto adjacent_patch_indices_count = size_t();
      stream.read(reinterpret_cast<char*>(&adjacent_patch_indices_count), sizeof(size_t));
      patch.adjacent_patch_indices = std::vector<uint32_t>(adjacent_patch_indices_count);
      for (auto& adjacent_patch_id : patch.adjacent_patch_indices)
      {
        stream.read(reinterpret_cast<char*>(&adjacent_patch_id), sizeof(uint32_t));
      }
    }
  }

  patchwork* add(ludo::instance& inst, const patchwork& init, const std::string& partition)
  {
    auto patchwork = ludo::add(ludo::data<astrum::patchwork>(inst), init, partition);
    patchwork->id = ludo::next_id++;

    auto mesh_buffer = ludo::get<ludo::mesh_buffer>(inst, partition, patchwork->mesh_buffer_id);
    assert(mesh_buffer && "mesh buffer not found");

    patchwork->available_ranges.emplace_back(ludo::mesh {
      .mesh_buffer_id = mesh_buffer->id,
      .index_buffer = mesh_buffer->index_buffer,
      .vertex_buffer = mesh_buffer->vertex_buffer
    });

    for (auto index = 0; index < patchwork->patches.size(); index++)
    {
      auto& patch = patchwork->patches[index];

      patch.variant_index = patchwork->variant_index(*patchwork, index);
      auto mesh = allocate(inst, *patchwork, index, patch.variant_index, partition);
      patch.mesh_id = mesh->id;
      patchwork->load(*patchwork, index, patch.variant_index, *mesh);
    }

    for (auto index = 0; index < patchwork->patches.size(); index++)
    {
      sew(*patchwork, index, false);
    }

    for (auto index = 0; index < patchwork->patches.size(); index++)
    {
      patchwork->on_load(*patchwork, index);
    }

    return patchwork;
  }

  void update_patchworks(ludo::instance& inst)
  {
    auto& partitions = ludo::data<patchwork>(inst).partitions;

    for (auto& partition : partitions)
    {
      auto partition_name = partition.first;

      for (auto& patchwork : partition.second)
      {
        auto mutex = std::mutex();
        ludo::divide_and_conquer(patchwork.patches.size(), [&](uint32_t start, uint32_t end)
        {
          for (auto index = start; index < end; index++)
          {
            auto& patch = patchwork.patches[index];
            if (patch.locked)
            {
              continue;
            }

            auto new_variant_index = patchwork.variant_index(patchwork, index);
            if (new_variant_index != patch.variant_index)
            {
              patch.locked = true;

              // Multiple concurrent allocations would be bad...
              auto lock = std::lock_guard(mutex);
              auto& new_mesh = *allocate(inst, patchwork, index, new_variant_index, partition_name);

              // Purposely take a copy of the new mesh!
              // Otherwise, it may get shifted in the partitioned_buffer and cause all sorts of havoc.
              ludo::enqueue_background(inst, [&inst, &patchwork, index, new_variant_index, new_mesh, partition_name]()
              {
                auto local_new_mesh = new_mesh;
                patchwork.load(patchwork, index, new_variant_index, local_new_mesh);

                return [&inst, &patchwork, index, new_variant_index, local_new_mesh, partition_name]()
                {
                  auto& patch = patchwork.patches[index];

                  patchwork.on_unload(patchwork, index);
                  deallocate(inst, patchwork, patch.mesh_id, partition_name);

                  patch.mesh_id = local_new_mesh.id;
                  patch.variant_index = new_variant_index;
                  patch.locked = false;

                  sew(patchwork, index, true);
                  patchwork.on_load(patchwork, index);
                };
              });
            }
          }

          return []() {};
        });
      }
    }
  }

  ludo::mesh* allocate(ludo::instance& inst, patchwork& patchwork, uint32_t patch_index, uint32_t variant_index, const std::string& partition)
  {
    // TODO allocate in the smallest available range that fits it...

    auto [ index_size, vertex_size] = patchwork.size(patchwork, patch_index, variant_index);

    for (auto available_ranges_iter = patchwork.available_ranges.begin(); available_ranges_iter < patchwork.available_ranges.end(); available_ranges_iter++)
    {
      auto& available_range = *available_ranges_iter;

      if (available_range.index_buffer.size < index_size || available_range.vertex_buffer.size < vertex_size)
      {
        continue;
      }

      auto mesh = ludo::add(
        inst,
        ludo::mesh
        {
          .mesh_buffer_id = patchwork.mesh_buffer_id,
          .index_buffer =
          {
            .data = available_range.index_buffer.data,
            .size = index_size,
          },
          .vertex_buffer =
          {
            .data = available_range.vertex_buffer.data,
            .size = vertex_size
          }
        },
        partition
      );

      if (available_range.index_buffer.size == index_size && available_range.vertex_buffer.size == vertex_size)
      {
        patchwork.available_ranges.erase(available_ranges_iter);
      }
      else
      {
        available_range.index_buffer.data += index_size;
        available_range.index_buffer.size -= index_size;
        available_range.vertex_buffer.data += vertex_size;
        available_range.vertex_buffer.size -= vertex_size;
      }

      return mesh;
    }

    assert(false && "could not fit mesh");
  }

  void deallocate(ludo::instance& inst, patchwork& patchwork, uint64_t mesh_id, const std::string& partition)
  {
    auto mesh = ludo::get<ludo::mesh>(inst, mesh_id);
    assert(mesh && "mesh not found");

    for (auto available_ranges_iter = patchwork.available_ranges.begin(); available_ranges_iter < patchwork.available_ranges.end(); available_ranges_iter++)
    {
      auto& available_range = *available_ranges_iter;

      if (available_range.index_buffer.data == mesh->index_buffer.data + mesh->index_buffer.size &&
          available_range.vertex_buffer.data == mesh->vertex_buffer.data + mesh->vertex_buffer.size)
      {
        available_range.index_buffer.data = mesh->index_buffer.data;
        available_range.index_buffer.size += mesh->index_buffer.size;
        available_range.vertex_buffer.data = mesh->vertex_buffer.data;
        available_range.vertex_buffer.size += mesh->vertex_buffer.size;

        ludo::remove<ludo::mesh>(inst, mesh, partition);
        return;
      }

      if (available_range.index_buffer.data + available_range.index_buffer.size == mesh->index_buffer.data &&
          available_range.vertex_buffer.data + available_range.vertex_buffer.size == mesh->vertex_buffer.data)
      {
        available_range.index_buffer.size += mesh->index_buffer.size;
        available_range.vertex_buffer.size += mesh->vertex_buffer.size;

        if (available_ranges_iter + 1 != patchwork.available_ranges.end())
        {
          auto& next_available_range = *(available_ranges_iter + 1);
          if (next_available_range.index_buffer.data == mesh->index_buffer.data + mesh->index_buffer.size &&
              next_available_range.vertex_buffer.data == mesh->vertex_buffer.data + mesh->vertex_buffer.size)
          {
            available_range.index_buffer.size += next_available_range.index_buffer.size;
            available_range.vertex_buffer.size += next_available_range.vertex_buffer.size;
            patchwork.available_ranges.erase(available_ranges_iter + 1);
          }
        }

        ludo::remove<ludo::mesh>(inst, mesh, partition);
        return;
      }
    }

    auto new_available_mesh = *mesh;
    new_available_mesh.id = 0;
    ludo::remove<ludo::mesh>(inst, mesh, partition);

    patchwork.available_ranges.emplace_back(new_available_mesh);
    std::sort(patchwork.available_ranges.begin(), patchwork.available_ranges.end(),
      [](const ludo::mesh& a, const ludo::mesh& b)
      {
        return a.index_buffer.data < b.index_buffer.data;
      }
    );
  }

  void sew(patchwork& patchwork, uint32_t patch_index, bool sew_same_variant)
  {
    auto& patch = patchwork.patches[patch_index];

    for (auto adjacent_patch_index : patch.adjacent_patch_indices)
    {
      auto& adjacent_patch = patchwork.patches[adjacent_patch_index];

      if (adjacent_patch.variant_index < patch.variant_index)
      {
        patchwork.sew(patchwork, adjacent_patch_index, patch_index);
      }
      else if (adjacent_patch.variant_index > patch.variant_index)
      {
        patchwork.sew(patchwork, patch_index, adjacent_patch_index);
      }
      else if (sew_same_variant && adjacent_patch.variant_index == patch.variant_index)
      {
        patchwork.sew(patchwork, patch_index, adjacent_patch_index);
      }
    }
  }
}
