cmake_minimum_required(VERSION 3.16)

project(Editor LANGUAGES CXX)

add_library(Editor STATIC)
add_library(Rocket::Editor ALIAS Editor)

file(GLOB_RECURSE CORE_PUBLIC_HEADERS
    ${CMAKE_CURRENT_LIST_DIR}/Public/*.h
    ${CMAKE_CURRENT_LIST_DIR}/Public/*.hpp
)

file(GLOB_RECURSE CORE_PRIVATE_HEADERS
    ${CMAKE_CURRENT_LIST_DIR}/Private/*.h
    ${CMAKE_CURRENT_LIST_DIR}/Private/*.hpp
)

file(GLOB_RECURSE CORE_PRIVATE_SOURCES
    ${CMAKE_CURRENT_LIST_DIR}/Private/*.c
    ${CMAKE_CURRENT_LIST_DIR}/Private/*.cpp
)

target_sources(Editor PRIVATE ${EDITOR_PRIVATE_SOURCES})

target_include_directories(Editor 
    PRIVATE ${CMAKE_CURRENT_LIST_DIR}/Private
    PUBLIC ${CMAKE_CURRENT_LIST_DIR}/Public
)

target_link_libraries(Editor
	
)

set_target_properties(Editor PROPERTIES POSITION_INDEPENDENT_CODE ON)
