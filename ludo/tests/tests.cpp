#include <ludo/rendering.h>
#include <ludo/spatial/grid3.h>
#include <ludo/testing.h>

#include "data/arrays.h"
#include "data/buffers.h"
#include "math/mat.h"
#include "math/projection.h"
#include "math/quat.h"
#include "math/vec.h"
#include "spatial/grid2.h"
#include "spatial/grid3.h"
#include "spatial/octree.h"
#include "spatial/quadtree.h"

int main()
{
  ludo::test_arrays();
  ludo::test_buffers();
  ludo::test_math_mat();
  ludo::test_math_projection();
  ludo::test_math_quat();
  ludo::test_math_vec();
  ludo::test_spatial_grid2();
  ludo::test_spatial_grid3();
  ludo::test_spatial_octree();
  ludo::test_spatial_quadtree();

  return ludo::test_finalize();
}

// stubs
namespace ludo
{
  compute_program* add_grid_compute_program(instance& instance, const grid3& octree)
  {
    return new compute_program();
  }

  buffer allocate_vram(uint64_t size, vram_buffer_access_hint access_hint)
  {
    return allocate(size);
  }

  void deallocate_vram(buffer& buffer)
  {
    deallocate(buffer);
  }

  void set_texture(mesh_instance& mesh_instance, const texture& texture, uint32_t instance_index)
  {
  }
}
