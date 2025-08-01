cmake_minimum_required(VERSION 3.16)

project(RenderCore LANGUAGES CXX)

add_library(RenderCore STATIC)
add_library(Rocket::RenderCore ALIAS RenderCore)

file(GLOB_RECURSE RENDERCORE_PUBLIC_HEADERS
    ${CMAKE_CURRENT_LIST_DIR}/Public/*.h
    ${CMAKE_CURRENT_LIST_DIR}/Public/*.hpp
)

file(GLOB_RECURSE RENDERCORE_PRIVATE_HEADERS
    ${CMAKE_CURRENT_LIST_DIR}/Private/*.h
    ${CMAKE_CURRENT_LIST_DIR}/Private/*.hpp
)

file(GLOB_RECURSE RENDERCORE_PRIVATE_SOURCES
    ${CMAKE_CURRENT_LIST_DIR}/*.c
    ${CMAKE_CURRENT_LIST_DIR}/*.cpp
)

target_sources(RenderCore PRIVATE ${RENDERCORE_PRIVATE_SOURCES})

target_include_directories(RenderCore
    PUBLIC ${CMAKE_CURRENT_LIST_DIR}/Public
    PRIVATE ${CMAKE_CURRENT_LIST_DIR}/Private
)

target_link_libraries(RenderCore
    PUBLIC Rocket::Core
    PUBLIC Rocket::Scene
    PUBLIC Rocket::Asset
    PRIVATE Rocket::RHI
)

set_target_properties(RenderCore PROPERTIES POSITION_INDEPENDENT_CODE ON)