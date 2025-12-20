cmake_minimum_required(VERSION 3.16)

project(Object LANGUAGES CXX)

add_library(Object STATIC)
add_library(Rocket::Object ALIAS Object)

file(GLOB_RECURSE OBJECT_PUBLIC_HEADERS
    ${CMAKE_CURRENT_LIST_DIR}/Public/*.h
    ${CMAKE_CURRENT_LIST_DIR}/Public/*.hpp
)

file(GLOB_RECURSE OBJECT_PRIVATE_HEADERS
    ${CMAKE_CURRENT_LIST_DIR}/Private/*.h
    ${CMAKE_CURRENT_LIST_DIR}/Private/*.hpp
)

file(GLOB_RECURSE OBJECT_PRIVATE_SOURCES
    ${CMAKE_CURRENT_LIST_DIR}/Private/*.c
    ${CMAKE_CURRENT_LIST_DIR}/Private/*.cpp
)

target_sources(Object PRIVATE 
    ${OBJECT_PUBLIC_HEADERS}
    ${OBJECT_PRIVATE_HEADERS}
    ${OBJECT_PRIVATE_SOURCES}
)

target_include_directories(Object
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/Public>
        $<INSTALL_INTERFACE:include/Object>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Private
)

target_link_libraries(Object
    PUBLIC Rocket::Core
    PUBLIC Rocket::Window
    PUBLIC $<BUILD_INTERFACE:glm::glm>
    PRIVATE Rocket::RHI
    PRIVATE Rocket::RenderCore
    PRIVATE Rocket::Rendering
)

set_target_properties(Object PROPERTIES POSITION_INDEPENDENT_CODE ON)

target_compile_options(Object PRIVATE
  $<$<CXX_COMPILER_ID:Clang,GNU>:-Wno-invalid-offsetof>
)