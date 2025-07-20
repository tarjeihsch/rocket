cmake_minimum_required(VERSION 3.15)

project(imgui LANGUAGES CXX)

add_library(imgui STATIC)
add_library(Rocket::imgui ALIAS imgui)

target_sources(imgui PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/imgui.cpp
    ${CMAKE_CURRENT_LIST_DIR}/imgui_draw.cpp
    ${CMAKE_CURRENT_LIST_DIR}/imgui_tables.cpp
    ${CMAKE_CURRENT_LIST_DIR}/imgui_widgets.cpp
)

target_include_directories(imgui PUBLIC ${CMAKE_CURRENT_LIST_DIR})
