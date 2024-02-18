cmake_minimum_required(VERSION 3.2)

include(../common.cmake)

# Project
#########################
project(ludo)

# Source
#########################
set(SRC_FILES
    src/ludo/animation.cpp
    src/ludo/buffers.cpp
    src/ludo/core.cpp
    src/ludo/files.cpp
    src/ludo/graphs.cpp
    src/ludo/graphs/dynamic_octree.cpp
    src/ludo/graphs/linear_octree.cpp
    src/ludo/graphs/util.cpp
    src/ludo/logging.cpp
    src/ludo/math/distance.cpp
    src/ludo/math/mat.cpp
    src/ludo/math/quat.cpp
    src/ludo/math/util.cpp
    src/ludo/math/vec.cpp
    src/ludo/meshes.cpp
    src/ludo/meshes/box.cpp
    src/ludo/meshes/circle.cpp
    src/ludo/meshes/cylinder.cpp
    src/ludo/meshes/edit.cpp
    src/ludo/meshes/math.cpp
    src/ludo/meshes/rectangle.cpp
    src/ludo/meshes/sphere_cube.cpp
    src/ludo/meshes/sphere_ico.cpp
    src/ludo/meshes/sphere_uv.cpp
    src/ludo/meshes/util.cpp
    src/ludo/scripts.cpp
    src/ludo/tasks.cpp
    src/ludo/testing.cpp
    src/ludo/thread_pool.cpp
    src/ludo/timer.cpp)

set(TEST_SRC_FILES
    tests/buffers.cpp
    tests/graphs/dynamic_octree.cpp
    tests/graphs/linear_octree.cpp
    tests/math/mat.cpp
    tests/math/quat.cpp
    tests/math/vec.cpp
    tests/tests.cpp)

# Target
#########################
add_library(ludo ${LUDO_LINK_TYPE} ${SRC_FILES})
target_include_directories(ludo PUBLIC src)

# Test Target
#########################
add_executable(ludo-tests ${SRC_FILES} ${TEST_SRC_FILES})
target_include_directories(ludo-tests PUBLIC src tests)
