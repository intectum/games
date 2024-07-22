cmake_minimum_required(VERSION 3.2)

include(../common.cmake)

# Project
#########################
project(ludo-assimp)

# Project Dependencies
#########################

# Assimp
add_subdirectory(lib/assimp)

# Free Image Plus
set(FREEIMAGEPLUS_PATH ${CMAKE_CURRENT_SOURCE_DIR}/lib/freeimage/Dist/libfreeimageplus.a)
add_custom_command(
    OUTPUT ${FREEIMAGEPLUS_PATH}
    COMMAND make -f Makefile.fip
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/lib/freeimage
    COMMENT "Building Free Image Plus")
add_custom_target(free-image-plus DEPENDS ${FREEIMAGEPLUS_PATH})

# Source
#########################
set(SRC_FILES
    src/ludo/assimp/animation.cpp
    src/ludo/assimp/importing.cpp
    src/ludo/assimp/math.cpp
    src/ludo/assimp/meshes.cpp
    src/ludo/assimp/physics.cpp
    src/ludo/assimp/textures.cpp
    src/ludo/assimp/util.cpp)

# Target
#########################
add_library(ludo-assimp STATIC ${SRC_FILES})
target_include_directories(ludo-assimp PUBLIC src)

# Target Dependencies
#########################

# Assimp
target_include_directories(ludo-assimp PUBLIC lib/assimp/include)
target_link_libraries(ludo-assimp assimp)

# Free Image Plus
add_dependencies(ludo-assimp free-image-plus)
target_include_directories(ludo-assimp PUBLIC lib/freeimage/Dist)
target_link_libraries(ludo-assimp ${FREEIMAGEPLUS_PATH})

# ludo
target_link_libraries(ludo-assimp ludo)
