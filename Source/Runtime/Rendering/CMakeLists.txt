cmake_minimum_required(VERSION 3.16)

project(Rendering LANGUAGES CXX)

add_library(Rendering STATIC)
add_library(Rocket::Rendering ALIAS Rendering)

file(GLOB_RECURSE RENDERING_PUBLIC_HEADERS
    ${CMAKE_CURRENT_LIST_DIR}/Public/*.h
    ${CMAKE_CURRENT_LIST_DIR}/Public/*.hpp
)

file(GLOB_RECURSE RENDERING_PRIVATE_HEADERS
    ${CMAKE_CURRENT_LIST_DIR}/Private/*.h
    ${CMAKE_CURRENT_LIST_DIR}/Private/*.hpp
)

file(GLOB_RECURSE RENDERING_PRIVATE_SOURCES
    ${CMAKE_CURRENT_LIST_DIR}/*.c
    ${CMAKE_CURRENT_LIST_DIR}/*.cpp
)

target_sources(Rendering PRIVATE
    ${RENDERING_PUBLIC_HEADERS}
    ${RENDERING_PRIVATE_HEADERS}
    ${RENDERING_PRIVATE_SOURCES}
)

target_include_directories(Rendering
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/Public>
        $<INSTALL_INTERFACE:include/Rendering>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Private
)

target_link_libraries(Rendering
    PUBLIC Rocket::Core
    PUBLIC Rocket::RenderCore
    PUBLIC Rocket::RHI
)

set_target_properties(Rendering PROPERTIES POSITION_INDEPENDENT_CODE ON)
