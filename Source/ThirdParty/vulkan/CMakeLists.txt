cmake_minimum_required(VERSION 3.15)

project(vulkan LANGUAGES C)

add_library(vulkan INTERFACE)
add_library(vulkan::vulkan ALIAS vulkan)

target_include_directories(vulkan INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>
    $<INSTALL_INTERFACE:include/vulkan>
)

if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
target_compile_options(vulkan INTERFACE
        -Wno-nullability-completeness
        -Wno-nullability-extension
        -Wno-dynamic-class-memaccess
    )
endif()
