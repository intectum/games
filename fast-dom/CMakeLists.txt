cmake_minimum_required(VERSION 3.2)

# Compiling
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Project
#########################
project(fast-dom)

# Project Dependencies
#########################

# ludo
add_subdirectory(../ludo lib/ludo)
add_subdirectory(../ludo-glfw lib/ludo-glfw)
add_subdirectory(../ludo-opengl lib/ludo-opengl)
add_subdirectory(../ludo-stb lib/ludo-stb)

# Source
#########################
set(SRC_FILES
    src/css/parsing.cpp
    src/html/attributes.cpp
    src/html/layout.cpp
    src/html/parsing.cpp
    src/main.cpp
)

# Target
#########################
add_executable(fast-dom ${SRC_FILES})

# Target Dependencies
#########################

# ludo
target_link_libraries(fast-dom ludo)
target_link_libraries(fast-dom ludo-glfw)
target_link_libraries(fast-dom ludo-opengl)
target_link_libraries(fast-dom ludo-stb)

# Target Resources
#########################
add_custom_command(TARGET fast-dom PRE_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${PROJECT_SOURCE_DIR}/assets ${PROJECT_BINARY_DIR}/assets)
