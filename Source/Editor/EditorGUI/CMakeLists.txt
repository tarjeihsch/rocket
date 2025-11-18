cmake_minimum_required(VERSION 3.16)

project(EditorGUI LANGUAGES CXX)

add_library(EditorGUI STATIC)
add_library(Rocket::EditorGUI ALIAS EditorGUI)

file(GLOB_RECURSE EDITORGUI_PUBLIC_HEADERS
    ${CMAKE_CURRENT_LIST_DIR}/Public/*.h
    ${CMAKE_CURRENT_LIST_DIR}/Public/*.hpp
)

file(GLOB_RECURSE EDITORGUI_PRIVATE_HEADERS
    ${CMAKE_CURRENT_LIST_DIR}/Private/*.h
    ${CMAKE_CURRENT_LIST_DIR}/Private/*.hpp
)

file(GLOB_RECURSE EDITORGUI_PRIVATE_SOURCES
    ${CMAKE_CURRENT_LIST_DIR}/Private/*.c
    ${CMAKE_CURRENT_LIST_DIR}/Private/*.cpp
)

target_sources(EditorGUI PRIVATE
    ${EDITORGUI_PUBLIC_HEADERS}
    ${EDITORGUI_PRIVATE_HEADERS}
    ${EDITORGUI_PRIVATE_SOURCES}
)

target_include_directories(EditorGUI
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/Public>
        $<INSTALL_INTERFACE:include/EditorGUI>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Private
)

target_link_libraries(EditorGUI
	PUBLIC Rocket::Core
    PUBLIC Rocket::Engine
	PUBLIC Rocket::Rendering
	PUBLIC Rocket::IMGUIModule
)

set_target_properties(EditorGUI PROPERTIES POSITION_INDEPENDENT_CODE ON)
