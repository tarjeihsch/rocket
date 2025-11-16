cmake_minimum_required(VERSION 3.16)

project(EditorUI LANGUAGES CXX)

add_library(EditorUI STATIC)
add_library(Rocket::EditorUI ALIAS EditorUI)

file(GLOB_RECURSE EDITORUI_PUBLIC_HEADERS
    ${CMAKE_CURRENT_LIST_DIR}/Public/*.h
    ${CMAKE_CURRENT_LIST_DIR}/Public/*.hpp
)

file(GLOB_RECURSE EDITORUI_PRIVATE_HEADERS
    ${CMAKE_CURRENT_LIST_DIR}/Private/*.h
    ${CMAKE_CURRENT_LIST_DIR}/Private/*.hpp
)

file(GLOB_RECURSE EDITORUI_PRIVATE_SOURCES
    ${CMAKE_CURRENT_LIST_DIR}/Private/*.c
    ${CMAKE_CURRENT_LIST_DIR}/Private/*.cpp
)

target_sources(EditorUI PRIVATE
    ${EDITORUI_PUBLIC_HEADERS}
    ${EDITORUI_PRIVATE_HEADERS}
    ${EDITORUI_PRIVATE_SOURCES}
)

target_include_directories(EditorUI
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/Public>
        $<INSTALL_INTERFACE:include/EditorUI>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Private
)

target_link_libraries(EditorUI
	PUBLIC Rocket::Core
    PUBLIC Rocket::Engine
	PUBLIC Rocket::Rendering
	PRIVATE imgui::imgui
)

set_target_properties(EditorUI PROPERTIES POSITION_INDEPENDENT_CODE ON)
