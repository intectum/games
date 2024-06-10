cmake_minimum_required(VERSION 3.2)

include(../common.cmake)

# Project
#########################
project(ludo-stb)

# Source
#########################
set(SRC_FILES
    src/ludo/stb/textures.cpp)

# Target
#########################
add_library(ludo-stb ${LUDO_LINK_TYPE} ${SRC_FILES})
target_include_directories(ludo-stb PUBLIC src)

# Target Dependencies
#########################

# stb
target_include_directories(ludo-stb PUBLIC lib/stb)

# ludo
target_link_libraries(ludo-stb ludo)
