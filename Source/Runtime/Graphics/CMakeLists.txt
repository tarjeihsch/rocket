cmake_minimum_required(VERSION 3.16)

project(Graphics LANGUAGES CXX)

add_library(Graphics STATIC)
add_library(Rocket::Graphics ALIAS Graphics)

file(GLOB_RECURSE GRAPHICS_PUBLIC_HEADERS
    ${CMAKE_CURRENT_LIST_DIR}/Public/*.h
    ${CMAKE_CURRENT_LIST_DIR}/Public/*.hpp
)

file(GLOB_RECURSE GRAPHICS_PRIVATE_HEADERS
    ${CMAKE_CURRENT_LIST_DIR}/Private/*.h
    ${CMAKE_CURRENT_LIST_DIR}/Private/*.hpp
)

file(GLOB_RECURSE GRAPHICS_PRIVATE_SOURCES
    ${CMAKE_CURRENT_LIST_DIR}/*.c
    ${CMAKE_CURRENT_LIST_DIR}/*.cpp
)

target_sources(Graphics PRIVATE 
    ${GRAPHICS_PUBLIC_HEADERS}
    ${GRAPHICS_PRIVATE_HEADERS}
    ${GRAPHICS_PRIVATE_SOURCES}
)

source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}/Public"  PREFIX "Public"  FILES ${GRAPHICS_PUBLIC_HEADERS})
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}/Private" PREFIX "Private" FILES ${GRAPHICS_PRIVATE_HEADERS} ${GRAPHICS_PRIVATE_SOURCES})

target_include_directories(Graphics
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/Public>
        $<INSTALL_INTERFACE:include/Graphics>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Private
)

target_link_libraries(Graphics
    PUBLIC
        Rocket::Core
        Rocket::Geometry
    PRIVATE
        Rocket::RHI
)

set_target_properties(Graphics PROPERTIES POSITION_INDEPENDENT_CODE ON)
