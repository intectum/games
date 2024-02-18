cmake_minimum_required(VERSION 3.2)

include(../common.cmake)

# Project
#########################
project(ludo-opengl)

# Project Dependencies
#########################

# GLEW
set(GLEW_PATH ${CMAKE_CURRENT_SOURCE_DIR}/lib/glew/lib/libGLEW.a)
set(GLEW32_PATH ${CMAKE_CURRENT_SOURCE_DIR}/lib/glew/lib/glew32.lib)
add_custom_command(
    OUTPUT ${GLEW_PATH}
    COMMAND make extensions
    COMMAND make
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/lib/glew
    COMMENT "Building GL Extension Wrangler")
add_custom_target(glew DEPENDS ${GLEW_PATH})

# Source
#########################
set(SRC_FILES
    src/ludo/opengl/buffers.cpp
    src/ludo/opengl/built_ins.cpp
    src/ludo/opengl/frame_buffers.cpp
    src/ludo/opengl/meshes.cpp
    src/ludo/opengl/render_programs.cpp
    src/ludo/opengl/rendering.cpp
    src/ludo/opengl/rendering_contexts.cpp
    src/ludo/opengl/shaders.cpp
    src/ludo/opengl/textures.cpp
    src/ludo/opengl/util.cpp)

# Target
#########################
add_library(ludo-opengl ${LUDO_LINK_TYPE} ${SRC_FILES})
target_include_directories(ludo-opengl PUBLIC src)

# Target Dependencies
#########################

# GLEW
add_dependencies(ludo-opengl glew)
target_include_directories(ludo-opengl PUBLIC lib/glew/include)
IF(WIN32)
    target_link_libraries(ludo-opengl ${GLEW32_PATH})
ENDIF(WIN32)
IF(UNIX)
    target_link_libraries(ludo-opengl ${GLEW_PATH})
ENDIF(UNIX)

# OpenGL
IF(WIN32)
    target_link_libraries(ludo-opengl GlU32)
    target_link_libraries(ludo-opengl OpenGL32)
ENDIF(WIN32)
IF(UNIX)
    target_link_libraries(ludo-opengl GLU)
    target_link_libraries(ludo-opengl GL)
ENDIF(UNIX)

# ludo
target_link_libraries(ludo-opengl ludo)
