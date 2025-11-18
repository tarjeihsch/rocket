cmake_minimum_required(VERSION 3.16)

project(IMGUIModule LANGUAGES CXX)

add_library(IMGUIModule STATIC)
add_library(Rocket::IMGUIModule ALIAS IMGUIModule)

file(GLOB_RECURSE IMGUIMODULE_PUBLIC_HEADERS
    ${CMAKE_CURRENT_LIST_DIR}/Public/*.h
    ${CMAKE_CURRENT_LIST_DIR}/Public/*.hpp
)

file(GLOB_RECURSE IMGUIMODULE_PRIVATE_HEADERS
    ${CMAKE_CURRENT_LIST_DIR}/Private/*.h
    ${CMAKE_CURRENT_LIST_DIR}/Private/*.hpp
)

file(GLOB_RECURSE IMGUIMODULE_PRIVATE_SOURCES
    ${CMAKE_CURRENT_LIST_DIR}/Private/*.c
    ${CMAKE_CURRENT_LIST_DIR}/Private/*.cpp
)

target_sources(IMGUIModule PRIVATE
    ${IMGUIMODULE_PUBLIC_HEADERS}
    ${IMGUIMODULE_PRIVATE_HEADERS}
    ${IMGUIMODULE_PRIVATE_SOURCES}
)

target_include_directories(IMGUIModule
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/Public>
        $<INSTALL_INTERFACE:include/IMGUIModule>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Private
)

target_link_libraries(IMGUIModule
    PUBLIC Rocket::Core
    PUBLIC Rocket::Engine
    PUBLIC Rocket::Rendering
    PRIVATE imgui::imgui
)

set_target_properties(IMGUIModule PROPERTIES POSITION_INDEPENDENT_CODE ON)
