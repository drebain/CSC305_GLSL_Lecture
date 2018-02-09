cmake_minimum_required(VERSION 2.8)

project(glsl_lecture_examples)

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/cmake)

include(cmake/configure.cmake)

add_subdirectory(apps/Ex1_Simple)
add_subdirectory(apps/Ex2_Shaders)
add_subdirectory(apps/Ex3_Geometry)
add_subdirectory(apps/Ex4_Advanced_Geometry)
add_subdirectory(apps/Ex5_Textures)
add_subdirectory(apps/Ex6_Shading)
add_subdirectory(apps/Ex7_Framebuffers)
