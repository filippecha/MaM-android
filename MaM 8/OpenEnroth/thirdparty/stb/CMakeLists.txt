cmake_minimum_required(VERSION 3.27 FATAL_ERROR)

# stb_image.h, public domain single-header image loader. Only its JPEG decoder is used, for the HD texture pack.
add_library(stb INTERFACE)
target_include_directories(stb INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
