cmake_minimum_required(VERSION 3.15)

project(vulkan LANGUAGES C)

add_library(vulkan INTERFACE)
add_library(vulkan::vulkan ALIAS vulkan)

target_include_directories(vulkan INTERFACE ${CMAKE_CURRENT_LIST_DIR}/include)
