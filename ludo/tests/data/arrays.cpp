/*
 * This file is part of ludo. See the LICENSE file for the full license governing this code.
 */

#include <ludo/data/arrays.h>
#include <ludo/testing.h>

#include "arrays.h"

namespace ludo
{
  void test_arrays()
  {
    test_group("arrays");

    auto array = allocate_array<int32_t>(10);
    test_not_equal<int32_t*>("array: allocate (data)", array.data, nullptr);
    test_equal("array: allocate (length)", array.length, 0u);
    test_equal("array: allocate (capacity)", array.capacity, 10u);
    test_equal("array: allocate (begin)", array.begin(), array.data);
    test_equal("array: allocate (end)", array.end(), array.data);

    deallocate(array);
    test_equal<int32_t*>("array: deallocate (data)", array.data, nullptr);
    test_equal("array: deallocate (length)", array.length, 0u);
    test_equal("array: deallocate (capacity)", array.capacity, 0u);

    auto array_2 = allocate_array<int32_t>(10);
    auto array_2_first_element = add(array_2, 0);
    test_equal("array: add (element)", *array_2_first_element, 0);
    test_equal("array: add (element address)", array_2_first_element, array_2.data);
    test_equal("array: add (length)", array_2.length, 1u);
    test_equal("array: add (capacity)", array_2.capacity, 10u);
    test_equal("array: add (begin)", array_2.begin(), array_2.data);
    test_equal("array: add (end)", array_2.end(), array_2.data + 1);

    test_equal("array: array index operator", array_2[0], 0);

    add(array_2, 1);
    add(array_2, 2);
    remove(array_2, array_2.begin());
    test_equal("array: remove front (length)", array_2.length, 2u);
    test_equal("array: remove front (capacity)", array_2.capacity, 10u);
    test_equal("array: remove front (remaining element 0)", array_2[0], 1);
    test_equal("array: remove front (remaining element 1)", array_2[1], 2);

    add(array_2, 3);
    remove(array_2, array_2.begin() + 1);
    test_equal("array: remove middle (length)", array_2.length, 2u);
    test_equal("array: remove middle (capacity)", array_2.capacity, 10u);
    test_equal("array: remove middle (remaining element 0)", array_2[0], 1);
    test_equal("array: remove middle (remaining element 1)", array_2[1], 3);

    add(array_2, 4);
    remove(array_2, array_2.begin() + 2);
    test_equal("array: remove end (length)", array_2.length, 2u);
    test_equal("array: remove end (capacity)", array_2.capacity, 10u);
    test_equal("array: remove end (remaining element 0)", array_2[0], 1);
    test_equal("array: remove end (remaining element 1)", array_2[1], 3);

    auto partitioned_array = allocate_partitioned_array<int32_t>(10);
    test_not_equal<int32_t*>("partitioned_array: allocate (data)", partitioned_array.data, nullptr);
    test_equal("partitioned_array: allocate (length)", partitioned_array.length, 0u);
    test_equal("partitioned_array: allocate (capacity)", partitioned_array.capacity, 10u);
    test_equal("partitioned_array: allocate (begin)", partitioned_array.begin(), partitioned_array.data);
    test_equal("partitioned_array: allocate (end)", partitioned_array.end(), partitioned_array.data);

    deallocate(partitioned_array);
    test_equal<void*>("partitioned_array: deallocate (data)", partitioned_array.data, nullptr);
    test_equal("partitioned_array: deallocate (length)", partitioned_array.length, 0u);
    test_equal("partitioned_array: deallocate (capacity)", partitioned_array.capacity, 0u);

    auto partitioned_array_2 = allocate_partitioned_array<int32_t>(10);
    auto partitioned_array_2_first_element = add(partitioned_array_2, 0);
    test_equal("partitioned_array: add (element)", *partitioned_array_2_first_element, 0);
    test_equal("partitioned_array: add (element address)", partitioned_array_2_first_element, partitioned_array_2.data);
    test_equal("partitioned_array: add (length)", partitioned_array_2.length, 1u);
    test_equal("partitioned_array: add (capacity)", partitioned_array_2.capacity, 10u);
    test_equal("partitioned_array: add (begin)", partitioned_array_2.begin(), partitioned_array_2.data);
    test_equal("partitioned_array: add (end)", partitioned_array_2.end(), partitioned_array_2.data + 1);

    test_equal("partitioned_array: array index operator", partitioned_array_2[0], 0);

    add(partitioned_array_2, 1);
    add(partitioned_array_2, 2);
    remove(partitioned_array_2, partitioned_array_2.begin());
    test_equal("partitioned_array: remove front (length)", partitioned_array_2.length, 2u);
    test_equal("partitioned_array: remove front (capacity)", partitioned_array_2.capacity, 10u);
    test_equal("partitioned_array: remove front (remaining element 0)", partitioned_array_2[0], 1);
    test_equal("partitioned_array: remove front (remaining element 1)", partitioned_array_2[1], 2);

    add(partitioned_array_2, 3);
    remove(partitioned_array_2, partitioned_array_2.begin() + 1);
    test_equal("partitioned_array: remove middle (length)", partitioned_array_2.length, 2u);
    test_equal("partitioned_array: remove middle (capacity)", partitioned_array_2.capacity, 10u);
    test_equal("partitioned_array: remove middle (remaining element 0)", partitioned_array_2[0], 1);
    test_equal("partitioned_array: remove middle (remaining element 1)", partitioned_array_2[1], 3);

    add(partitioned_array_2, 4);
    remove(partitioned_array_2, partitioned_array_2.begin() + 2);
    test_equal("partitioned_array: remove end (length)", partitioned_array_2.length, 2u);
    test_equal("partitioned_array: remove end (capacity)", partitioned_array_2.capacity, 10u);
    test_equal("partitioned_array: remove end (remaining element 0)", partitioned_array_2[0], 1);
    test_equal("partitioned_array: remove end (remaining element 1)", partitioned_array_2[1], 3);

    auto partitioned_array_3 = allocate_partitioned_array<int32_t>(10);
    add(partitioned_array_3, 0, "my-partition-0");
    add(partitioned_array_3, 1, "my-partition-1");
    add(partitioned_array_3, 2, "my-partition-2");
    test_equal("partitioned_array: partitions size", partitioned_array_3.partitions.size(), 3ul);
    test_equal("partitioned_array: partition 0 name", partitioned_array_3.partitions[0].first, std::string("my-partition-0"));
    test_equal("partitioned_array: partition 0 data", partitioned_array_3.partitions[0].second.data, partitioned_array_3.data);
    test_equal("partitioned_array: partition 0 array length", partitioned_array_3.partitions[0].second.length, 1u);
    test_equal("partitioned_array: partition 0 element 0", partitioned_array_3.partitions[0].second[0], 0);
    test_equal("partitioned_array: partition 1 name", partitioned_array_3.partitions[1].first, std::string("my-partition-1"));
    test_equal<int32_t*>("partitioned_array: partition 1 data", partitioned_array_3.partitions[1].second.data, partitioned_array_3.data + 1);
    test_equal("partitioned_array: partition 1 array length", partitioned_array_3.partitions[1].second.length, 1u);
    test_equal("partitioned_array: partition 1 element 0", partitioned_array_3.partitions[1].second[0], 1);
    test_equal("partitioned_array: partition 2 name", partitioned_array_3.partitions[2].first, std::string("my-partition-2"));
    test_equal<int32_t*>("partitioned_array: partition 2 data", partitioned_array_3.partitions[2].second.data, partitioned_array_3.data + 2);
    test_equal("partitioned_array: partition 2 array length", partitioned_array_3.partitions[2].second.length, 1u);
    test_equal("partitioned_array: partition 2 element 0", partitioned_array_3.partitions[2].second[0], 2);

    add(partitioned_array_3, 1, "my-partition-0");
    test_equal("partitioned_array: add front (length)", partitioned_array_3.length, 4u);
    test_equal("partitioned_array: add front (element 0)", partitioned_array_3[0], 0);
    test_equal("partitioned_array: add front (element 1)", partitioned_array_3[1], 1);
    test_equal("partitioned_array: add front (element 2)", partitioned_array_3[2], 1);
    test_equal("partitioned_array: add front (element 3)", partitioned_array_3[3], 2);
    test_equal("partitioned_array: add front (partitions size)", partitioned_array_3.partitions.size(), 3ul);
    test_equal("partitioned_array: add front (partition 0 name)", partitioned_array_3.partitions[0].first, std::string("my-partition-0"));
    test_equal("partitioned_array: add front (partition 0 data)", partitioned_array_3.partitions[0].second.data, partitioned_array_3.data);
    test_equal("partitioned_array: add front (partition 0 array length)", partitioned_array_3.partitions[0].second.length, 2u);
    test_equal("partitioned_array: add front (partition 0 element 0)", partitioned_array_3.partitions[0].second[0], 0);
    test_equal("partitioned_array: add front (partition 0 element 1)", partitioned_array_3.partitions[0].second[1], 1);
    test_equal("partitioned_array: add front (partition 1 name)", partitioned_array_3.partitions[1].first, std::string("my-partition-1"));
    test_equal<int32_t*>("partitioned_array: add front (partition 1 data)", partitioned_array_3.partitions[1].second.data, partitioned_array_3.data + 2);
    test_equal("partitioned_array: add front (partition 1 array length)", partitioned_array_3.partitions[1].second.length, 1u);
    test_equal("partitioned_array: add front (partition 1 element 0)", partitioned_array_3.partitions[1].second[0], 1);
    test_equal("partitioned_array: add front (partition 2 name)", partitioned_array_3.partitions[2].first, std::string("my-partition-2"));
    test_equal<int32_t*>("partitioned_array: add front (partition 2 data)", partitioned_array_3.partitions[2].second.data, partitioned_array_3.data + 3);
    test_equal("partitioned_array: add front (partition 2 array length)", partitioned_array_3.partitions[2].second.length, 1u);
    test_equal("partitioned_array: add front (partition 2 element 0)", partitioned_array_3.partitions[2].second[0], 2);

    add(partitioned_array_3, 2, "my-partition-1");
    test_equal("partitioned_array: add middle (length)", partitioned_array_3.length, 5u);
    test_equal("partitioned_array: add middle (element 0)", partitioned_array_3[0], 0);
    test_equal("partitioned_array: add middle (element 1)", partitioned_array_3[1], 1);
    test_equal("partitioned_array: add middle (element 2)", partitioned_array_3[2], 1);
    test_equal("partitioned_array: add middle (element 3)", partitioned_array_3[3], 2);
    test_equal("partitioned_array: add middle (element 4)", partitioned_array_3[4], 2);
    test_equal("partitioned_array: add middle (partitions size)", partitioned_array_3.partitions.size(), 3ul);
    test_equal("partitioned_array: add middle (partition 0 name)", partitioned_array_3.partitions[0].first, std::string("my-partition-0"));
    test_equal("partitioned_array: add middle (partition 0 data)", partitioned_array_3.partitions[0].second.data, partitioned_array_3.data);
    test_equal("partitioned_array: add middle (partition 0 array length)", partitioned_array_3.partitions[0].second.length, 2u);
    test_equal("partitioned_array: add middle (partition 0 element 0)", partitioned_array_3.partitions[0].second[0], 0);
    test_equal("partitioned_array: add middle (partition 0 element 1)", partitioned_array_3.partitions[0].second[1], 1);
    test_equal("partitioned_array: add middle (partition 1 name)", partitioned_array_3.partitions[1].first, std::string("my-partition-1"));
    test_equal<int32_t*>("partitioned_array: add middle (partition 1 data)", partitioned_array_3.partitions[1].second.data, partitioned_array_3.data + 2);
    test_equal("partitioned_array: add middle (partition 1 array length)", partitioned_array_3.partitions[1].second.length, 2u);
    test_equal("partitioned_array: add middle (partition 1 element 0)", partitioned_array_3.partitions[1].second[0], 1);
    test_equal("partitioned_array: add middle (partition 1 element 1)", partitioned_array_3.partitions[1].second[1], 2);
    test_equal("partitioned_array: add middle (partition 2 name)", partitioned_array_3.partitions[2].first, std::string("my-partition-2"));
    test_equal<int32_t*>("partitioned_array: add middle (partition 2 data)", partitioned_array_3.partitions[2].second.data, partitioned_array_3.data + 4);
    test_equal("partitioned_array: add middle (partition 2 array length)", partitioned_array_3.partitions[2].second.length, 1u);
    test_equal("partitioned_array: add middle (partition 2 element 0)", partitioned_array_3.partitions[2].second[0], 2);

    add(partitioned_array_3, 3, "my-partition-2");
    test_equal("partitioned_array: add end (length)", partitioned_array_3.length, 6u);
    test_equal("partitioned_array: add end (element 0)", partitioned_array_3[0], 0);
    test_equal("partitioned_array: add end (element 1)", partitioned_array_3[1], 1);
    test_equal("partitioned_array: add end (element 2)", partitioned_array_3[2], 1);
    test_equal("partitioned_array: add end (element 3)", partitioned_array_3[3], 2);
    test_equal("partitioned_array: add end (element 4)", partitioned_array_3[4], 2);
    test_equal("partitioned_array: add end (element 5)", partitioned_array_3[5], 3);
    test_equal("partitioned_array: add end (partitions size)", partitioned_array_3.partitions.size(), 3ul);
    test_equal("partitioned_array: add end (partition 0 name)", partitioned_array_3.partitions[0].first, std::string("my-partition-0"));
    test_equal("partitioned_array: add end (partition 0 data)", partitioned_array_3.partitions[0].second.data, partitioned_array_3.data);
    test_equal("partitioned_array: add end (partition 0 array length)", partitioned_array_3.partitions[0].second.length, 2u);
    test_equal("partitioned_array: add end (partition 0 element 0)", partitioned_array_3.partitions[0].second[0], 0);
    test_equal("partitioned_array: add end (partition 0 element 1)", partitioned_array_3.partitions[0].second[1], 1);
    test_equal("partitioned_array: add end (partition 1 name)", partitioned_array_3.partitions[1].first, std::string("my-partition-1"));
    test_equal<int32_t*>("partitioned_array: add end (partition 1 data)", partitioned_array_3.partitions[1].second.data, partitioned_array_3.data + 2);
    test_equal("partitioned_array: add end (partition 1 array length)", partitioned_array_3.partitions[1].second.length, 2u);
    test_equal("partitioned_array: add end (partition 1 element)", partitioned_array_3.partitions[1].second[0], 1);
    test_equal("partitioned_array: add end (partition 1 element 0)", partitioned_array_3.partitions[1].second[0], 1);
    test_equal("partitioned_array: add end (partition 1 element 1)", partitioned_array_3.partitions[1].second[1], 2);
    test_equal("partitioned_array: add end (partition 2 name)", partitioned_array_3.partitions[2].first, std::string("my-partition-2"));
    test_equal<int32_t*>("partitioned_array: add end (partition 2 data)", partitioned_array_3.partitions[2].second.data, partitioned_array_3.data + 4);
    test_equal("partitioned_array: add end (partition 2 array length)", partitioned_array_3.partitions[2].second.length, 2u);
    test_equal("partitioned_array: add end (partition 2 element 0)", partitioned_array_3.partitions[2].second[0], 2);
    test_equal("partitioned_array: add end (partition 2 element 1)", partitioned_array_3.partitions[2].second[1], 3);

    auto my_partition_0_iter = find(partitioned_array_3, "my-partition-0");
    remove(partitioned_array_3, my_partition_0_iter->second.begin(), "my-partition-0");
    test_equal("partitioned_array: remove front (length)", partitioned_array_3.length, 5u);
    test_equal("partitioned_array: remove front (element 0)", partitioned_array_3[0], 1);
    test_equal("partitioned_array: remove front (element 1)", partitioned_array_3[1], 1);
    test_equal("partitioned_array: remove front (element 2)", partitioned_array_3[2], 2);
    test_equal("partitioned_array: remove front (element 3)", partitioned_array_3[3], 2);
    test_equal("partitioned_array: remove front (element 4)", partitioned_array_3[4], 3);
    test_equal("partitioned_array: remove front (partitions size)", partitioned_array_3.partitions.size(), 3ul);
    test_equal("partitioned_array: remove front (partition 0 name)", partitioned_array_3.partitions[0].first, std::string("my-partition-0"));
    test_equal("partitioned_array: remove front (partition 0 data)", partitioned_array_3.partitions[0].second.data, partitioned_array_3.data);
    test_equal("partitioned_array: remove front (partition 0 array length)", partitioned_array_3.partitions[0].second.length, 1u);
    test_equal("partitioned_array: remove front (partition 0 element 0)", partitioned_array_3.partitions[0].second[0], 1);
    test_equal("partitioned_array: remove front (partition 1 name)", partitioned_array_3.partitions[1].first, std::string("my-partition-1"));
    test_equal<int32_t*>("partitioned_array: remove front (partition 1 data)", partitioned_array_3.partitions[1].second.data, partitioned_array_3.data + 1);
    test_equal("partitioned_array: remove front (partition 1 array length)", partitioned_array_3.partitions[1].second.length, 2u);
    test_equal("partitioned_array: remove front (partition 1 element 0)", partitioned_array_3.partitions[1].second[0], 1);
    test_equal("partitioned_array: remove front (partition 1 element 1)", partitioned_array_3.partitions[1].second[1], 2);
    test_equal("partitioned_array: remove front (partition 2 name)", partitioned_array_3.partitions[2].first, std::string("my-partition-2"));
    test_equal<int32_t*>("partitioned_array: remove front (partition 2 data)", partitioned_array_3.partitions[2].second.data, partitioned_array_3.data + 3);
    test_equal("partitioned_array: remove front (partition 2 array length)", partitioned_array_3.partitions[2].second.length, 2u);
    test_equal("partitioned_array: remove front (partition 2 element 0)", partitioned_array_3.partitions[2].second[0], 2);
    test_equal("partitioned_array: remove front (partition 2 element 1)", partitioned_array_3.partitions[2].second[1], 3);

    auto my_partition_1_iter = find(partitioned_array_3, "my-partition-1");
    remove(partitioned_array_3, my_partition_1_iter->second.begin(), "my-partition-1");
    test_equal("partitioned_array: remove middle (length)", partitioned_array_3.length, 4u);
    test_equal("partitioned_array: remove middle (element 0)", partitioned_array_3[0], 1);
    test_equal("partitioned_array: remove middle (element 1)", partitioned_array_3[1], 2);
    test_equal("partitioned_array: remove middle (element 2)", partitioned_array_3[2], 2);
    test_equal("partitioned_array: remove middle (element 3)", partitioned_array_3[3], 3);
    test_equal("partitioned_array: remove middle (partitions size)", partitioned_array_3.partitions.size(), 3ul);
    test_equal("partitioned_array: remove middle (partition 0 name)", partitioned_array_3.partitions[0].first, std::string("my-partition-0"));
    test_equal("partitioned_array: remove middle (partition 0 data)", partitioned_array_3.partitions[0].second.data, partitioned_array_3.data);
    test_equal("partitioned_array: remove middle (partition 0 array length)", partitioned_array_3.partitions[0].second.length, 1u);
    test_equal("partitioned_array: remove middle (partition 0 element 0)", partitioned_array_3.partitions[0].second[0], 1);
    test_equal("partitioned_array: remove middle (partition 1 name)", partitioned_array_3.partitions[1].first, std::string("my-partition-1"));
    test_equal<int32_t*>("partitioned_array: remove middle (partition 1 data)", partitioned_array_3.partitions[1].second.data, partitioned_array_3.data + 1);
    test_equal("partitioned_array: remove middle (partition 1 array length)", partitioned_array_3.partitions[1].second.length, 1u);
    test_equal("partitioned_array: remove middle (partition 1 element 0)", partitioned_array_3.partitions[1].second[0], 2);
    test_equal("partitioned_array: remove middle (partition 2 name)", partitioned_array_3.partitions[2].first, std::string("my-partition-2"));
    test_equal<int32_t*>("partitioned_array: remove middle (partition 2 data)", partitioned_array_3.partitions[2].second.data, partitioned_array_3.data + 2);
    test_equal("partitioned_array: remove middle (partition 2 array length)", partitioned_array_3.partitions[2].second.length, 2u);
    test_equal("partitioned_array: remove middle (partition 2 element 0)", partitioned_array_3.partitions[2].second[0], 2);
    test_equal("partitioned_array: remove middle (partition 2 element 1)", partitioned_array_3.partitions[2].second[1], 3);

    auto my_partition_2_iter = find(partitioned_array_3, "my-partition-2");
    remove(partitioned_array_3, my_partition_2_iter->second.begin(), "my-partition-2");
    test_equal("partitioned_array: remove end (length)", partitioned_array_3.length, 3u);
    test_equal("partitioned_array: remove end (element 0)", partitioned_array_3[0], 1);
    test_equal("partitioned_array: remove end (element 1)", partitioned_array_3[1], 2);
    test_equal("partitioned_array: remove end (element 2)", partitioned_array_3[2], 3);
    test_equal("partitioned_array: remove end (partitions size)", partitioned_array_3.partitions.size(), 3ul);
    test_equal("partitioned_array: remove end (partition 0 name)", partitioned_array_3.partitions[0].first, std::string("my-partition-0"));
    test_equal("partitioned_array: remove end (partition 0 data)", partitioned_array_3.partitions[0].second.data, partitioned_array_3.data);
    test_equal("partitioned_array: remove end (partition 0 array length)", partitioned_array_3.partitions[0].second.length, 1u);
    test_equal("partitioned_array: remove end (partition 0 element 0)", partitioned_array_3.partitions[0].second[0], 1);
    test_equal("partitioned_array: remove end (partition 1 name)", partitioned_array_3.partitions[1].first, std::string("my-partition-1"));
    test_equal<int32_t*>("partitioned_array: remove end (partition 1 data)", partitioned_array_3.partitions[1].second.data, partitioned_array_3.data + 1);
    test_equal("partitioned_array: remove end (partition 1 array length)", partitioned_array_3.partitions[1].second.length, 1u);
    test_equal("partitioned_array: remove end (partition 1 element 0)", partitioned_array_3.partitions[1].second[0], 2);
    test_equal("partitioned_array: remove end (partition 2 name)", partitioned_array_3.partitions[2].first, std::string("my-partition-2"));
    test_equal<int32_t*>("partitioned_array: remove end (partition 2 data)", partitioned_array_3.partitions[2].second.data, partitioned_array_3.data + 2);
    test_equal("partitioned_array: remove end (partition 2 array length)", partitioned_array_3.partitions[2].second.length, 1u);
    test_equal("partitioned_array: remove end (partition 2 element 0)", partitioned_array_3.partitions[2].second[0], 3);

    auto partitioned_array_4 = allocate_partitioned_array<int32_t>(10);
    add(partitioned_array_4, 0, "existing-partition");
    auto existing_partition_iter = find_or_create(partitioned_array_4, "existing-partition");
    test_equal("partitioned_array: find or create existing (name)", existing_partition_iter->first, std::string("existing-partition"));
    test_equal("partitioned_array: find or create existing (data)", existing_partition_iter->second.data, partitioned_array_4.data);
    test_equal("partitioned_array: find or create existing (array length)", existing_partition_iter->second.length, 1u);
    test_equal("partitioned_array: find or create existing (element)", existing_partition_iter->second[0], 0);

    auto new_partition_iter = find_or_create(partitioned_array_4, "new-partition");
    test_equal("partitioned_array: find or create new (name)", new_partition_iter->first, std::string("new-partition"));
    test_equal<void*>("partitioned_array: find or create new (data)", new_partition_iter->second.data, partitioned_array_4.data + 1);
    test_equal("partitioned_array: find or create new (array length)", new_partition_iter->second.length, 0u);
  }
}
