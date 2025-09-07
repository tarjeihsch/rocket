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

target_sources(Object PRIVATE ${OBJECT_PRIVATE_SOURCES})

target_include_directories(Object 
    PRIVATE ${CMAKE_CURRENT_LIST_DIR}/Private
    PUBLIC ${CMAKE_CURRENT_LIST_DIR}/Public
)

target_link_libraries(Object
)

set_target_properties(Object PROPERTIES POSITION_INDEPENDENT_CODE ON)
