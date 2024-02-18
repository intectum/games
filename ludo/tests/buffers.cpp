/*
 * This file is part of ludo. See the LICENSE file for the full license governing this code.
 */

#include <ludo/buffers.h>
#include <ludo/testing.h>

#include "buffers.h"

namespace ludo
{
  void test_buffers()
  {
    test_group("buffers");

    auto buffer = allocate(sizeof(int32_t) * 10);
    test_not_equal<void*>("buffer: allocate (data)", buffer.data, nullptr);
    test_equal("buffer: allocate (size)", buffer.size, sizeof(int32_t) * 10);

    deallocate(buffer);
    test_equal<void*>("buffer: deallocate (data)", buffer.data, nullptr);
    test_equal("buffer: deallocate (size)", buffer.size, 0ul);

    auto array_buffer = allocate_array<int32_t>(10);
    test_not_equal<void*>("array_buffer: allocate (data)", array_buffer.data, nullptr);
    test_equal("array_buffer: allocate (size)", array_buffer.size, sizeof(int32_t) * 10);
    test_equal("array_buffer: allocate (array_size)", array_buffer.array_size, 0ul);
    test_equal("array_buffer: begin", array_buffer.begin(), reinterpret_cast<int32_t*>(array_buffer.data));
    test_equal("array_buffer: end", array_buffer.end(), reinterpret_cast<int32_t*>(array_buffer.data));

    deallocate(array_buffer);
    test_equal<void*>("array_buffer: deallocate (data)", array_buffer.data, nullptr);
    test_equal("array_buffer: deallocate (size)", array_buffer.size, 0ul);
    test_equal("array_buffer: deallocate (array_size)", array_buffer.array_size, 0ul);

    auto array_buffer_2 = allocate_array<int32_t>(10);
    auto array_buffer_2_first_element = add(array_buffer_2, 0);
    test_equal("array_buffer: add (element)", *array_buffer_2_first_element, 0);
    test_equal("array_buffer: add (element address)", array_buffer_2_first_element, reinterpret_cast<int32_t*>(array_buffer_2.data));
    test_equal("array_buffer: add (array_size)", array_buffer_2.array_size, 1ul);
    test_equal("array_buffer: add (begin)", array_buffer_2.begin(), reinterpret_cast<int32_t*>(array_buffer_2.data));
    test_equal("array_buffer: add (end)", array_buffer_2.end(), reinterpret_cast<int32_t*>(array_buffer_2.data) + 1);

    test_equal("array_buffer: array index operator", array_buffer_2[0], 0);

    add(array_buffer_2, 1);
    add(array_buffer_2, 2);
    remove(array_buffer_2, array_buffer_2.begin());
    test_equal("array_buffer: remove front (array_size)", array_buffer_2.array_size, 2ul);
    test_equal("array_buffer: remove front (remaining element 0)", array_buffer_2[0], 1);
    test_equal("array_buffer: remove front (remaining element 1)", array_buffer_2[1], 2);

    add(array_buffer_2, 3);
    remove(array_buffer_2, array_buffer_2.begin() + 1);
    test_equal("array_buffer: remove middle (array_size)", array_buffer_2.array_size, 2ul);
    test_equal("array_buffer: remove middle (remaining element 0)", array_buffer_2[0], 1);
    test_equal("array_buffer: remove middle (remaining element 1)", array_buffer_2[1], 3);

    add(array_buffer_2, 4);
    remove(array_buffer_2, array_buffer_2.begin() + 2);
    test_equal("array_buffer: remove end (array_size)", array_buffer_2.array_size, 2ul);
    test_equal("array_buffer: remove end (remaining element 0)", array_buffer_2[0], 1);
    test_equal("array_buffer: remove end (remaining element 1)", array_buffer_2[1], 3);

    auto partitioned_buffer = allocate_partitioned<int32_t>(10);
    test_not_equal<void*>("partitioned_buffer: allocate (data)", partitioned_buffer.data, nullptr);
    test_equal("partitioned_buffer: allocate (size)", partitioned_buffer.size, sizeof(int32_t) * 10);
    test_equal("partitioned_buffer: allocate (array_size)", partitioned_buffer.array_size, 0ul);
    test_equal("partitioned_buffer: begin", partitioned_buffer.begin(), reinterpret_cast<int32_t*>(partitioned_buffer.data));
    test_equal("partitioned_buffer: end", partitioned_buffer.end(), reinterpret_cast<int32_t*>(partitioned_buffer.data));

    deallocate(partitioned_buffer);
    test_equal<void*>("partitioned_buffer: deallocate (data)", partitioned_buffer.data, nullptr);
    test_equal("partitioned_buffer: deallocate (size)", partitioned_buffer.size, 0ul);
    test_equal("partitioned_buffer: deallocate (array_size)", partitioned_buffer.array_size, 0ul);

    auto partitioned_buffer_2 = allocate_partitioned<int32_t>(10);
    auto partitioned_buffer_2_first_element = add(partitioned_buffer_2, 0);
    test_equal("partitioned_buffer: add (element)", *partitioned_buffer_2_first_element, 0);
    test_equal("partitioned_buffer: add (element address)", partitioned_buffer_2_first_element, reinterpret_cast<int32_t*>(partitioned_buffer_2.data));
    test_equal("partitioned_buffer: add (array_size)", partitioned_buffer_2.array_size, 1ul);
    test_equal("partitioned_buffer: add (begin)", partitioned_buffer_2.begin(), reinterpret_cast<int32_t*>(partitioned_buffer_2.data));
    test_equal("partitioned_buffer: add (end)", partitioned_buffer_2.end(), reinterpret_cast<int32_t*>(partitioned_buffer_2.data) + 1);

    test_equal("partitioned_buffer: array index operator", partitioned_buffer_2[0], 0);

    add(partitioned_buffer_2, 1);
    add(partitioned_buffer_2, 2);
    remove(partitioned_buffer_2, partitioned_buffer_2.begin());
    test_equal("partitioned_buffer: remove front (array_size)", partitioned_buffer_2.array_size, 2ul);
    test_equal("partitioned_buffer: remove front (remaining element 0)", partitioned_buffer_2[0], 1);
    test_equal("partitioned_buffer: remove front (remaining element 1)", partitioned_buffer_2[1], 2);

    add(partitioned_buffer_2, 3);
    remove(partitioned_buffer_2, partitioned_buffer_2.begin() + 1);
    test_equal("partitioned_buffer: remove middle (array_size)", partitioned_buffer_2.array_size, 2ul);
    test_equal("partitioned_buffer: remove middle (remaining element 0)", partitioned_buffer_2[0], 1);
    test_equal("partitioned_buffer: remove middle (remaining element 1)", partitioned_buffer_2[1], 3);

    add(partitioned_buffer_2, 4);
    remove(partitioned_buffer_2, partitioned_buffer_2.begin() + 2);
    test_equal("partitioned_buffer: remove end (array_size)", partitioned_buffer_2.array_size, 2ul);
    test_equal("partitioned_buffer: remove end (remaining element 0)", partitioned_buffer_2[0], 1);
    test_equal("partitioned_buffer: remove end (remaining element 1)", partitioned_buffer_2[1], 3);

    auto partitioned_buffer_3 = allocate_partitioned<int32_t>(10);
    add(partitioned_buffer_3, 0, "my-partition-0");
    add(partitioned_buffer_3, 1, "my-partition-1");
    add(partitioned_buffer_3, 2, "my-partition-2");
    test_equal("partitioned_buffer: partitions size", partitioned_buffer_3.partitions.size(), 3ul);
    test_equal("partitioned_buffer: partition 0 name", partitioned_buffer_3.partitions[0].first, std::string("my-partition-0"));
    test_equal("partitioned_buffer: partition 0 data", partitioned_buffer_3.partitions[0].second.data, partitioned_buffer_3.data);
    test_equal("partitioned_buffer: partition 0 array size", partitioned_buffer_3.partitions[0].second.array_size, 1ul);
    test_equal("partitioned_buffer: partition 0 element 0", partitioned_buffer_3.partitions[0].second[0], 0);
    test_equal("partitioned_buffer: partition 1 name", partitioned_buffer_3.partitions[1].first, std::string("my-partition-1"));
    test_equal<void*>("partitioned_buffer: partition 1 data", partitioned_buffer_3.partitions[1].second.data, reinterpret_cast<int32_t*>(partitioned_buffer_3.data) + 1);
    test_equal("partitioned_buffer: partition 1 array size", partitioned_buffer_3.partitions[1].second.array_size, 1ul);
    test_equal("partitioned_buffer: partition 1 element 0", partitioned_buffer_3.partitions[1].second[0], 1);
    test_equal("partitioned_buffer: partition 2 name", partitioned_buffer_3.partitions[2].first, std::string("my-partition-2"));
    test_equal<void*>("partitioned_buffer: partition 2 data", partitioned_buffer_3.partitions[2].second.data, reinterpret_cast<int32_t*>(partitioned_buffer_3.data) + 2);
    test_equal("partitioned_buffer: partition 2 array size", partitioned_buffer_3.partitions[2].second.array_size, 1ul);
    test_equal("partitioned_buffer: partition 2 element 0", partitioned_buffer_3.partitions[2].second[0], 2);

    add(partitioned_buffer_3, 1, "my-partition-0");
    test_equal("partitioned_buffer: add front (array_size)", partitioned_buffer_3.array_size, 4ul);
    test_equal("partitioned_buffer: add front (element 0)", partitioned_buffer_3[0], 0);
    test_equal("partitioned_buffer: add front (element 1)", partitioned_buffer_3[1], 1);
    test_equal("partitioned_buffer: add front (element 2)", partitioned_buffer_3[2], 1);
    test_equal("partitioned_buffer: add front (element 3)", partitioned_buffer_3[3], 2);
    test_equal("partitioned_buffer: add front (partitions size)", partitioned_buffer_3.partitions.size(), 3ul);
    test_equal("partitioned_buffer: add front (partition 0 name)", partitioned_buffer_3.partitions[0].first, std::string("my-partition-0"));
    test_equal("partitioned_buffer: add front (partition 0 data)", partitioned_buffer_3.partitions[0].second.data, partitioned_buffer_3.data);
    test_equal("partitioned_buffer: add front (partition 0 array size)", partitioned_buffer_3.partitions[0].second.array_size, 2ul);
    test_equal("partitioned_buffer: add front (partition 0 element 0)", partitioned_buffer_3.partitions[0].second[0], 0);
    test_equal("partitioned_buffer: add front (partition 0 element 1)", partitioned_buffer_3.partitions[0].second[1], 1);
    test_equal("partitioned_buffer: add front (partition 1 name)", partitioned_buffer_3.partitions[1].first, std::string("my-partition-1"));
    test_equal<void*>("partitioned_buffer: add front (partition 1 data)", partitioned_buffer_3.partitions[1].second.data, reinterpret_cast<int32_t*>(partitioned_buffer_3.data) + 2);
    test_equal("partitioned_buffer: add front (partition 1 array size)", partitioned_buffer_3.partitions[1].second.array_size, 1ul);
    test_equal("partitioned_buffer: add front (partition 1 element 0)", partitioned_buffer_3.partitions[1].second[0], 1);
    test_equal("partitioned_buffer: add front (partition 2 name)", partitioned_buffer_3.partitions[2].first, std::string("my-partition-2"));
    test_equal<void*>("partitioned_buffer: add front (partition 2 data)", partitioned_buffer_3.partitions[2].second.data, reinterpret_cast<int32_t*>(partitioned_buffer_3.data) + 3);
    test_equal("partitioned_buffer: add front (partition 2 array size)", partitioned_buffer_3.partitions[2].second.array_size, 1ul);
    test_equal("partitioned_buffer: add front (partition 2 element 0)", partitioned_buffer_3.partitions[2].second[0], 2);

    add(partitioned_buffer_3, 2, "my-partition-1");
    test_equal("partitioned_buffer: add middle (array_size)", partitioned_buffer_3.array_size, 5ul);
    test_equal("partitioned_buffer: add middle (element 0)", partitioned_buffer_3[0], 0);
    test_equal("partitioned_buffer: add middle (element 1)", partitioned_buffer_3[1], 1);
    test_equal("partitioned_buffer: add middle (element 2)", partitioned_buffer_3[2], 1);
    test_equal("partitioned_buffer: add middle (element 3)", partitioned_buffer_3[3], 2);
    test_equal("partitioned_buffer: add middle (element 4)", partitioned_buffer_3[4], 2);
    test_equal("partitioned_buffer: add middle (partitions size)", partitioned_buffer_3.partitions.size(), 3ul);
    test_equal("partitioned_buffer: add middle (partition 0 name)", partitioned_buffer_3.partitions[0].first, std::string("my-partition-0"));
    test_equal("partitioned_buffer: add middle (partition 0 data)", partitioned_buffer_3.partitions[0].second.data, partitioned_buffer_3.data);
    test_equal("partitioned_buffer: add middle (partition 0 array size)", partitioned_buffer_3.partitions[0].second.array_size, 2ul);
    test_equal("partitioned_buffer: add middle (partition 0 element 0)", partitioned_buffer_3.partitions[0].second[0], 0);
    test_equal("partitioned_buffer: add middle (partition 0 element 1)", partitioned_buffer_3.partitions[0].second[1], 1);
    test_equal("partitioned_buffer: add middle (partition 1 name)", partitioned_buffer_3.partitions[1].first, std::string("my-partition-1"));
    test_equal<void*>("partitioned_buffer: add middle (partition 1 data)", partitioned_buffer_3.partitions[1].second.data, reinterpret_cast<int32_t*>(partitioned_buffer_3.data) + 2);
    test_equal("partitioned_buffer: add middle (partition 1 array size)", partitioned_buffer_3.partitions[1].second.array_size, 2ul);
    test_equal("partitioned_buffer: add middle (partition 1 element 0)", partitioned_buffer_3.partitions[1].second[0], 1);
    test_equal("partitioned_buffer: add middle (partition 1 element 1)", partitioned_buffer_3.partitions[1].second[1], 2);
    test_equal("partitioned_buffer: add middle (partition 2 name)", partitioned_buffer_3.partitions[2].first, std::string("my-partition-2"));
    test_equal<void*>("partitioned_buffer: add middle (partition 2 data)", partitioned_buffer_3.partitions[2].second.data, reinterpret_cast<int32_t*>(partitioned_buffer_3.data) + 4);
    test_equal("partitioned_buffer: add middle (partition 2 array size)", partitioned_buffer_3.partitions[2].second.array_size, 1ul);
    test_equal("partitioned_buffer: add middle (partition 2 element 0)", partitioned_buffer_3.partitions[2].second[0], 2);

    add(partitioned_buffer_3, 3, "my-partition-2");
    test_equal("partitioned_buffer: add end (array_size)", partitioned_buffer_3.array_size, 6ul);
    test_equal("partitioned_buffer: add end (element 0)", partitioned_buffer_3[0], 0);
    test_equal("partitioned_buffer: add end (element 1)", partitioned_buffer_3[1], 1);
    test_equal("partitioned_buffer: add end (element 2)", partitioned_buffer_3[2], 1);
    test_equal("partitioned_buffer: add end (element 3)", partitioned_buffer_3[3], 2);
    test_equal("partitioned_buffer: add end (element 4)", partitioned_buffer_3[4], 2);
    test_equal("partitioned_buffer: add end (element 5)", partitioned_buffer_3[5], 3);
    test_equal("partitioned_buffer: add end (partitions size)", partitioned_buffer_3.partitions.size(), 3ul);
    test_equal("partitioned_buffer: add end (partition 0 name)", partitioned_buffer_3.partitions[0].first, std::string("my-partition-0"));
    test_equal("partitioned_buffer: add end (partition 0 data)", partitioned_buffer_3.partitions[0].second.data, partitioned_buffer_3.data);
    test_equal("partitioned_buffer: add end (partition 0 array size)", partitioned_buffer_3.partitions[0].second.array_size, 2ul);
    test_equal("partitioned_buffer: add end (partition 0 element 0)", partitioned_buffer_3.partitions[0].second[0], 0);
    test_equal("partitioned_buffer: add end (partition 0 element 1)", partitioned_buffer_3.partitions[0].second[1], 1);
    test_equal("partitioned_buffer: add end (partition 1 name)", partitioned_buffer_3.partitions[1].first, std::string("my-partition-1"));
    test_equal<void*>("partitioned_buffer: add end (partition 1 data)", partitioned_buffer_3.partitions[1].second.data, reinterpret_cast<int32_t*>(partitioned_buffer_3.data) + 2);
    test_equal("partitioned_buffer: add end (partition 1 array size)", partitioned_buffer_3.partitions[1].second.array_size, 2ul);
    test_equal("partitioned_buffer: add end (partition 1 element)", partitioned_buffer_3.partitions[1].second[0], 1);
    test_equal("partitioned_buffer: add end (partition 1 element 0)", partitioned_buffer_3.partitions[1].second[0], 1);
    test_equal("partitioned_buffer: add end (partition 1 element 1)", partitioned_buffer_3.partitions[1].second[1], 2);
    test_equal("partitioned_buffer: add end (partition 2 name)", partitioned_buffer_3.partitions[2].first, std::string("my-partition-2"));
    test_equal<void*>("partitioned_buffer: add end (partition 2 data)", partitioned_buffer_3.partitions[2].second.data, reinterpret_cast<int32_t*>(partitioned_buffer_3.data) + 4);
    test_equal("partitioned_buffer: add end (partition 2 array size)", partitioned_buffer_3.partitions[2].second.array_size, 2ul);
    test_equal("partitioned_buffer: add end (partition 2 element 0)", partitioned_buffer_3.partitions[2].second[0], 2);
    test_equal("partitioned_buffer: add end (partition 2 element 1)", partitioned_buffer_3.partitions[2].second[1], 3);

    auto my_partition_0_iter = find(partitioned_buffer_3, "my-partition-0");
    remove(partitioned_buffer_3, my_partition_0_iter->second.begin(), "my-partition-0");
    test_equal("partitioned_buffer: remove front (array_size)", partitioned_buffer_3.array_size, 5ul);
    test_equal("partitioned_buffer: remove front (element 0)", partitioned_buffer_3[0], 1);
    test_equal("partitioned_buffer: remove front (element 1)", partitioned_buffer_3[1], 1);
    test_equal("partitioned_buffer: remove front (element 2)", partitioned_buffer_3[2], 2);
    test_equal("partitioned_buffer: remove front (element 3)", partitioned_buffer_3[3], 2);
    test_equal("partitioned_buffer: remove front (element 4)", partitioned_buffer_3[4], 3);
    test_equal("partitioned_buffer: remove front (partitions size)", partitioned_buffer_3.partitions.size(), 3ul);
    test_equal("partitioned_buffer: remove front (partition 0 name)", partitioned_buffer_3.partitions[0].first, std::string("my-partition-0"));
    test_equal("partitioned_buffer: remove front (partition 0 data)", partitioned_buffer_3.partitions[0].second.data, partitioned_buffer_3.data);
    test_equal("partitioned_buffer: remove front (partition 0 array size)", partitioned_buffer_3.partitions[0].second.array_size, 1ul);
    test_equal("partitioned_buffer: remove front (partition 0 element 0)", partitioned_buffer_3.partitions[0].second[0], 1);
    test_equal("partitioned_buffer: remove front (partition 1 name)", partitioned_buffer_3.partitions[1].first, std::string("my-partition-1"));
    test_equal<void*>("partitioned_buffer: remove front (partition 1 data)", partitioned_buffer_3.partitions[1].second.data, reinterpret_cast<int32_t*>(partitioned_buffer_3.data) + 1);
    test_equal("partitioned_buffer: remove front (partition 1 array size)", partitioned_buffer_3.partitions[1].second.array_size, 2ul);
    test_equal("partitioned_buffer: remove front (partition 1 element 0)", partitioned_buffer_3.partitions[1].second[0], 1);
    test_equal("partitioned_buffer: remove front (partition 1 element 1)", partitioned_buffer_3.partitions[1].second[1], 2);
    test_equal("partitioned_buffer: remove front (partition 2 name)", partitioned_buffer_3.partitions[2].first, std::string("my-partition-2"));
    test_equal<void*>("partitioned_buffer: remove front (partition 2 data)", partitioned_buffer_3.partitions[2].second.data, reinterpret_cast<int32_t*>(partitioned_buffer_3.data) + 3);
    test_equal("partitioned_buffer: remove front (partition 2 array size)", partitioned_buffer_3.partitions[2].second.array_size, 2ul);
    test_equal("partitioned_buffer: remove front (partition 2 element 0)", partitioned_buffer_3.partitions[2].second[0], 2);
    test_equal("partitioned_buffer: remove front (partition 2 element 1)", partitioned_buffer_3.partitions[2].second[1], 3);

    auto my_partition_1_iter = find(partitioned_buffer_3, "my-partition-1");
    remove(partitioned_buffer_3, my_partition_1_iter->second.begin(), "my-partition-1");
    test_equal("partitioned_buffer: remove middle (array_size)", partitioned_buffer_3.array_size, 4ul);
    test_equal("partitioned_buffer: remove middle (element 0)", partitioned_buffer_3[0], 1);
    test_equal("partitioned_buffer: remove middle (element 1)", partitioned_buffer_3[1], 2);
    test_equal("partitioned_buffer: remove middle (element 2)", partitioned_buffer_3[2], 2);
    test_equal("partitioned_buffer: remove middle (element 3)", partitioned_buffer_3[3], 3);
    test_equal("partitioned_buffer: remove middle (partitions size)", partitioned_buffer_3.partitions.size(), 3ul);
    test_equal("partitioned_buffer: remove middle (partition 0 name)", partitioned_buffer_3.partitions[0].first, std::string("my-partition-0"));
    test_equal("partitioned_buffer: remove middle (partition 0 data)", partitioned_buffer_3.partitions[0].second.data, partitioned_buffer_3.data);
    test_equal("partitioned_buffer: remove middle (partition 0 array size)", partitioned_buffer_3.partitions[0].second.array_size, 1ul);
    test_equal("partitioned_buffer: remove middle (partition 0 element 0)", partitioned_buffer_3.partitions[0].second[0], 1);
    test_equal("partitioned_buffer: remove middle (partition 1 name)", partitioned_buffer_3.partitions[1].first, std::string("my-partition-1"));
    test_equal<void*>("partitioned_buffer: remove middle (partition 1 data)", partitioned_buffer_3.partitions[1].second.data, reinterpret_cast<int32_t*>(partitioned_buffer_3.data) + 1);
    test_equal("partitioned_buffer: remove middle (partition 1 array size)", partitioned_buffer_3.partitions[1].second.array_size, 1ul);
    test_equal("partitioned_buffer: remove middle (partition 1 element 0)", partitioned_buffer_3.partitions[1].second[0], 2);
    test_equal("partitioned_buffer: remove middle (partition 2 name)", partitioned_buffer_3.partitions[2].first, std::string("my-partition-2"));
    test_equal<void*>("partitioned_buffer: remove middle (partition 2 data)", partitioned_buffer_3.partitions[2].second.data, reinterpret_cast<int32_t*>(partitioned_buffer_3.data) + 2);
    test_equal("partitioned_buffer: remove middle (partition 2 array size)", partitioned_buffer_3.partitions[2].second.array_size, 2ul);
    test_equal("partitioned_buffer: remove middle (partition 2 element 0)", partitioned_buffer_3.partitions[2].second[0], 2);
    test_equal("partitioned_buffer: remove middle (partition 2 element 1)", partitioned_buffer_3.partitions[2].second[1], 3);

    auto my_partition_2_iter = find(partitioned_buffer_3, "my-partition-2");
    remove(partitioned_buffer_3, my_partition_2_iter->second.begin(), "my-partition-2");
    test_equal("partitioned_buffer: remove end (array_size)", partitioned_buffer_3.array_size, 3ul);
    test_equal("partitioned_buffer: remove end (element 0)", partitioned_buffer_3[0], 1);
    test_equal("partitioned_buffer: remove end (element 1)", partitioned_buffer_3[1], 2);
    test_equal("partitioned_buffer: remove end (element 2)", partitioned_buffer_3[2], 3);
    test_equal("partitioned_buffer: remove end (partitions size)", partitioned_buffer_3.partitions.size(), 3ul);
    test_equal("partitioned_buffer: remove end (partition 0 name)", partitioned_buffer_3.partitions[0].first, std::string("my-partition-0"));
    test_equal("partitioned_buffer: remove end (partition 0 data)", partitioned_buffer_3.partitions[0].second.data, partitioned_buffer_3.data);
    test_equal("partitioned_buffer: remove end (partition 0 array size)", partitioned_buffer_3.partitions[0].second.array_size, 1ul);
    test_equal("partitioned_buffer: remove end (partition 0 element 0)", partitioned_buffer_3.partitions[0].second[0], 1);
    test_equal("partitioned_buffer: remove end (partition 1 name)", partitioned_buffer_3.partitions[1].first, std::string("my-partition-1"));
    test_equal<void*>("partitioned_buffer: remove end (partition 1 data)", partitioned_buffer_3.partitions[1].second.data, reinterpret_cast<int32_t*>(partitioned_buffer_3.data) + 1);
    test_equal("partitioned_buffer: remove end (partition 1 array size)", partitioned_buffer_3.partitions[1].second.array_size, 1ul);
    test_equal("partitioned_buffer: remove end (partition 1 element 0)", partitioned_buffer_3.partitions[1].second[0], 2);
    test_equal("partitioned_buffer: remove end (partition 2 name)", partitioned_buffer_3.partitions[2].first, std::string("my-partition-2"));
    test_equal<void*>("partitioned_buffer: remove end (partition 2 data)", partitioned_buffer_3.partitions[2].second.data, reinterpret_cast<int32_t*>(partitioned_buffer_3.data) + 2);
    test_equal("partitioned_buffer: remove end (partition 2 array size)", partitioned_buffer_3.partitions[2].second.array_size, 1ul);
    test_equal("partitioned_buffer: remove end (partition 2 element 0)", partitioned_buffer_3.partitions[2].second[0], 3);

    auto partitioned_buffer_4 = allocate_partitioned<int32_t>(10);
    add(partitioned_buffer_4, 0, "existing-partition");
    auto existing_partition_iter = find_or_create(partitioned_buffer_4, "existing-partition");
    test_equal("partitioned_buffer: find or create existing (name)", existing_partition_iter->first, std::string("existing-partition"));
    test_equal("partitioned_buffer: find or create existing (data)", existing_partition_iter->second.data, partitioned_buffer_4.data);
    test_equal("partitioned_buffer: find or create existing (array size)", existing_partition_iter->second.array_size, 1ul);
    test_equal("partitioned_buffer: find or create existing (element)", existing_partition_iter->second[0], 0);

    auto new_partition_iter = find_or_create(partitioned_buffer_4, "new-partition");
    test_equal("partitioned_buffer: find or create new (name)", new_partition_iter->first, std::string("new-partition"));
    test_equal<void*>("partitioned_buffer: find or create new (data)", new_partition_iter->second.data, reinterpret_cast<int32_t*>(partitioned_buffer_4.data) + 1);
    test_equal("partitioned_buffer: find or create new (array size)", new_partition_iter->second.array_size, 0ul);
  }
}
