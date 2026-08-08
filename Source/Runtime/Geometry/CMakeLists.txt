cmake_minimum_required(VERSION 3.16)

project(Geometry LANGUAGES CXX)

add_library(Geometry STATIC)
add_library(Rocket::Geometry ALIAS Geometry)

file(GLOB_RECURSE GEOMETRY_PUBLIC_HEADERS
    ${CMAKE_CURRENT_LIST_DIR}/Public/*.h
    ${CMAKE_CURRENT_LIST_DIR}/Public/*.hpp
)

file(GLOB_RECURSE GEOMETRY_PRIVATE_HEADERS
    ${CMAKE_CURRENT_LIST_DIR}/Private/*.h
    ${CMAKE_CURRENT_LIST_DIR}/Private/*.hpp
)

file(GLOB_RECURSE GEOMETRY_PRIVATE_SOURCES
    ${CMAKE_CURRENT_LIST_DIR}/*.c
    ${CMAKE_CURRENT_LIST_DIR}/*.cpp
)

target_sources(Geometry PRIVATE 
    ${GEOMETRY_PUBLIC_HEADERS}
    ${GEOMETRY_PRIVATE_HEADERS}
    ${GEOMETRY_PRIVATE_SOURCES}
)

source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}/Public"  PREFIX "Public"  FILES ${GEOMETRY_PUBLIC_HEADERS})
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}/Private" PREFIX "Private" FILES ${GEOMETRY_PRIVATE_HEADERS} ${GEOMETRY_PRIVATE_SOURCES})

target_include_directories(Geometry
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/Public>
        $<INSTALL_INTERFACE:include/Geometry>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Private
)

target_link_libraries(Geometry
    PUBLIC
        Rocket::Core
)

set_target_properties(Geometry PROPERTIES POSITION_INDEPENDENT_CODE ON)