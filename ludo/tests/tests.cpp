#include <ludo/testing.h>

#include "buffers.h"
#include "graphs/linear_octree.h"
#include "graphs/dynamic_octree.h"
#include "math/mat.h"
#include "math/quat.h"
#include "math/vec.h"

int main()
{
  ludo::test_buffers();
  ludo::test_graphs_dynamic_octree();
  ludo::test_graphs_linear_octree();
  ludo::test_math_mat();
  ludo::test_math_quat();
  ludo::test_math_vec();

  return ludo::test_finalize();
}
