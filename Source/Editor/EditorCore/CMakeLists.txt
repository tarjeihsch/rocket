cmake_minimum_required(VERSION 3.16)

project(EditorCore LANGUAGES CXX)

add_library(EditorCore STATIC)
add_library(Rocket::EditorCore ALIAS EditorCore)

file(GLOB_RECURSE EDITORCORE_PUBLIC_HEADERS
    ${CMAKE_CURRENT_LIST_DIR}/Public/*.h
    ${CMAKE_CURRENT_LIST_DIR}/Public/*.hpp
)

file(GLOB_RECURSE EDITORCORE_PRIVATE_HEADERS
    ${CMAKE_CURRENT_LIST_DIR}/Private/*.h
    ${CMAKE_CURRENT_LIST_DIR}/Private/*.hpp
)

file(GLOB_RECURSE EDITORCORE_PRIVATE_SOURCES
    ${CMAKE_CURRENT_LIST_DIR}/Private/*.c
    ${CMAKE_CURRENT_LIST_DIR}/Private/*.cpp
)

target_sources(EditorCore PRIVATE ${EDITORCORE_PRIVATE_SOURCES})

target_include_directories(EditorCore
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/Public>
        $<INSTALL_INTERFACE:include/EditorCore>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Private
)

target_link_libraries(EditorCore
	PUBLIC Rocket::Core
    PUBLIC Rocket::Engine
	PUBLIC Rocket::Renderer
)

set_target_properties(EditorCore PROPERTIES POSITION_INDEPENDENT_CODE ON)
