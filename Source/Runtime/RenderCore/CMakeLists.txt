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
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/Public>
        $<INSTALL_INTERFACE:include/RenderCore>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Private
)

target_link_libraries(RenderCore
    PUBLIC Rocket::Core
    PUBLIC Rocket::Asset
    PUBLIC Rocket::RHI
    PUBLIC Rocket::Window
    PUBLIC Rocket::Engine
    PUBLIC Rocket::Math
    PRIVATE $<BUILD_INTERFACE:glm::glm>
)

set_target_properties(RenderCore PROPERTIES POSITION_INDEPENDENT_CODE ON)
