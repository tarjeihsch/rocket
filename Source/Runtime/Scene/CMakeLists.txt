cmake_minimum_required(VERSION 3.16)

project(Scene LANGUAGES CXX)

add_library(Scene STATIC)
add_library(Rocket::Scene ALIAS Scene)

file(GLOB_RECURSE SCENE_PUBLIC_HEADERS
    ${CMAKE_CURRENT_LIST_DIR}/Public/*.h
    ${CMAKE_CURRENT_LIST_DIR}/Public/*.hpp
)

file(GLOB_RECURSE SCENE_PRIVATE_HEADERS
    ${CMAKE_CURRENT_LIST_DIR}/Private/*.h
    ${CMAKE_CURRENT_LIST_DIR}/Private/*.hpp
)

file(GLOB_RECURSE SCENE_PRIVATE_SOURCES
    ${CMAKE_CURRENT_LIST_DIR}/Private/*.c
    ${CMAKE_CURRENT_LIST_DIR}/Private/*.cpp
)

target_sources(Scene PRIVATE ${SCENE_PRIVATE_SOURCES})

target_include_directories(Scene
    PRIVATE ${CMAKE_CURRENT_LIST_DIR}/Private
    PUBLIC ${CMAKE_CURRENT_LIST_DIR}/Public
)

target_link_libraries(Scene
    PUBLIC Rocket::Core
    PUBLIC EnTT::EnTT
    PUBLIC glm::glm
)

set_target_properties(Core PROPERTIES POSITION_INDEPENDENT_CODE ON)