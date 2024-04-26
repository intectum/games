cmake_minimum_required(VERSION 3.2)

include(../common.cmake)

# Project
#########################
project(astrum)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/build/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/build/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/build/lib)

set(LUDO_LINK_TYPE SHARED)
add_subdirectory(../ludo lib/ludo)
add_subdirectory(../ludo-assimp lib/ludo-assimp)
add_subdirectory(../ludo-bullet lib/ludo-bullet)
add_subdirectory(../ludo-glfw lib/ludo-glfw)
add_subdirectory(../ludo-opengl lib/ludo-opengl)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/build)

# Source
#########################
set(SRC_FILES
    src/controllers/game.cpp
    src/controllers/map.cpp
    src/controllers/person.cpp
    src/controllers/spaceship.cpp
    src/entities/luna.cpp
    src/entities/people.cpp
    src/entities/sol.cpp
    src/entities/spaceships.cpp
    src/entities/terra.cpp
    #src/entities/trees.cpp
    src/main.cpp
    src/paths.cpp
    src/physics/centering.cpp
    src/physics/gravity.cpp
    src/physics/point_masses.cpp
    src/physics/relativity.cpp
    src/physics/util.cpp
    src/post-processing/atmosphere.cpp
    src/post-processing/bloom.cpp
    src/post-processing/hdr_resolve.cpp
    src/post-processing/pass.cpp
    src/post-processing/util.cpp
    src/solar_system.cpp
    src/terrain/mesh.cpp
    src/terrain/metadata.cpp
    src/terrain/shaders.cpp
    src/terrain/sphere_ico.cpp
    src/terrain/static_bodies.cpp
    src/terrain/terrain.cpp
    src/terrain/terrain_chunk.cpp
    src/terrain/mesh.cpp
    src/util.cpp)

# Standalone Target
#########################
add_executable(astrum ${SRC_FILES})
set_target_properties(astrum PROPERTIES INSTALL_RPATH "./lib")
target_include_directories(astrum PUBLIC src)

# Standalone Target Dependencies
#########################

# ludo
target_link_libraries(astrum ludo-assimp)
target_link_libraries(astrum ludo-bullet)
target_link_libraries(astrum ludo-glfw)
target_link_libraries(astrum ludo-opengl)
target_link_libraries(astrum ludo)

# noise
target_link_libraries(astrum noise)

# pthread
IF(UNIX)
    target_link_libraries(astrum pthread)
ENDIF(UNIX)

# Standalone Target Resources
#########################
add_custom_command(TARGET astrum PRE_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy_directory ${PROJECT_SOURCE_DIR}/assets ${PROJECT_SOURCE_DIR}/build/assets)

# Spinny Cube Target
#########################
add_executable(spinny-cube src/spinny_cube.cpp)
set_target_properties(spinny-cube PROPERTIES INSTALL_RPATH "./lib")

# Spinny Cube Target Dependencies
#########################

# ludo
target_link_libraries(spinny-cube ludo-assimp)
target_link_libraries(spinny-cube ludo-glfw)
target_link_libraries(spinny-cube ludo-opengl)
target_link_libraries(spinny-cube ludo)
