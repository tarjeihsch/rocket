cmake_minimum_required(VERSION 3.16)

project(Editor LANGUAGES CXX)

add_executable(Editor)
add_executable(Rocket::Editor ALIAS Editor)

file(GLOB_RECURSE EDITOR_PUBLIC_HEADERS
    ${CMAKE_CURRENT_LIST_DIR}/Public/*.h
    ${CMAKE_CURRENT_LIST_DIR}/Public/*.hpp
)

file(GLOB_RECURSE EDITOR_PRIVATE_HEADERS
    ${CMAKE_CURRENT_LIST_DIR}/Private/*.h
    ${CMAKE_CURRENT_LIST_DIR}/Private/*.hpp
)

file(GLOB_RECURSE EDITOR_PRIVATE_SOURCES
    ${CMAKE_CURRENT_LIST_DIR}/Private/*.c
    ${CMAKE_CURRENT_LIST_DIR}/Private/*.cpp
)

target_sources(Editor PRIVATE ${EDITOR_PRIVATE_SOURCES})

target_include_directories(Editor
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/Public>
        $<INSTALL_INTERFACE:include/Editor>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Private
)

target_link_libraries(Editor
    PRIVATE Rocket::EditorCore
)

set_target_properties(Editor PROPERTIES POSITION_INDEPENDENT_CODE ON)
