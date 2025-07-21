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

target_sources(Engine PRIVATE ${ENGINE_PRIVATE_SOURCES})

target_include_directories(Engine 
    PRIVATE ${CMAKE_CURRENT_LIST_DIR}/Private
    PUBLIC ${CMAKE_CURRENT_LIST_DIR}/Public
)

target_link_libraries(Engine
    PUBLIC Rocket::Core
    PUBLIC Rocket::Window
    PUBLIC Rocket::Scene
    PRIVATE Rocket::RHI
)
