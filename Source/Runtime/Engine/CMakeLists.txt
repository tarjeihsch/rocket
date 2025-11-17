cmake_minimum_required(VERSION 3.16)

project(Engine LANGUAGES CXX)

add_library(Engine STATIC)
add_library(Rocket::Engine ALIAS Engine)

file(GLOB_RECURSE ENGINE_PUBLIC_HEADERS
    ${CMAKE_CURRENT_LIST_DIR}/Public/*.h
    ${CMAKE_CURRENT_LIST_DIR}/Public/*.hpp
)

file(GLOB_RECURSE ENGINE_PRIVATE_HEADERS
    ${CMAKE_CURRENT_LIST_DIR}/Private/*.h
    ${CMAKE_CURRENT_LIST_DIR}/Private/*.hpp
)

file(GLOB_RECURSE ENGINE_PRIVATE_SOURCES
    ${CMAKE_CURRENT_LIST_DIR}/Private/*.c
    ${CMAKE_CURRENT_LIST_DIR}/Private/*.cpp
)

target_sources(Engine PRIVATE 
    ${ENGINE_PUBLIC_HEADERS}
    ${ENGINE_PRIVATE_HEADERS}
    ${ENGINE_PRIVATE_SOURCES}
)

target_include_directories(Engine
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/Public>
        $<INSTALL_INTERFACE:include/Engine>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Private
)

target_link_libraries(Engine
    PUBLIC Rocket::Core
    PUBLIC Rocket::Window
    PUBLIC Rocket::Object
    PRIVATE Rocket::RHI
    PRIVATE Rocket::RenderCore
    PRIVATE Rocket::Rendering
    PRIVATE $<BUILD_INTERFACE:glm::glm>
)

set_target_properties(Engine PROPERTIES POSITION_INDEPENDENT_CODE ON)
