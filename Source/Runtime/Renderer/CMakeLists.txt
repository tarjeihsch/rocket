cmake_minimum_required(VERSION 3.16)

project(Renderer LANGUAGES CXX)

add_library(Renderer STATIC)
add_library(Rocket::Renderer ALIAS Renderer)

file(GLOB_RECURSE RENDERER_PUBLIC_HEADERS
    ${CMAKE_CURRENT_LIST_DIR}/Public/*.h
    ${CMAKE_CURRENT_LIST_DIR}/Public/*.hpp
)

file(GLOB_RECURSE RENDERER_PRIVATE_HEADERS
    ${CMAKE_CURRENT_LIST_DIR}/Private/*.h
    ${CMAKE_CURRENT_LIST_DIR}/Private/*.hpp
)

file(GLOB_RECURSE RENDERER_PRIVATE_SOURCES
    ${CMAKE_CURRENT_LIST_DIR}/*.c
    ${CMAKE_CURRENT_LIST_DIR}/*.cpp
)

target_sources(Renderer PRIVATE ${RENDERER_PRIVATE_SOURCES})

target_include_directories(Renderer
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/Public>
        $<INSTALL_INTERFACE:include/Renderer>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Private
)

target_link_libraries(Renderer
    PUBLIC Rocket::Core
    PUBLIC Rocket::RenderCore
    PUBLIC Rocket::RHI
    PRIVATE $<BUILD_INTERFACE:glm::glm>
)

set_target_properties(RenderCore PROPERTIES POSITION_INDEPENDENT_CODE ON)
