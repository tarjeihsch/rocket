cmake_minimum_required(VERSION 3.16)

project(Window LANGUAGES CXX)

add_library(Window STATIC)
add_library(Rocket::Window ALIAS Window)

file(GLOB_RECURSE WINDOW_PUBLIC_HEADERS
    ${CMAKE_CURRENT_LIST_DIR}/Public/*.h
    ${CMAKE_CURRENT_LIST_DIR}/Public/*.hpp
)

file(GLOB_RECURSE WINDOW_PRIVATE_HEADERS
    ${CMAKE_CURRENT_LIST_DIR}/Private/*.h
    ${CMAKE_CURRENT_LIST_DIR}/Private/*.hpp
)

file(GLOB_RECURSE WINDOW_PRIVATE_SOURCES
    ${CMAKE_CURRENT_LIST_DIR}/Private/*.c
    ${CMAKE_CURRENT_LIST_DIR}/Private/*.cpp
)

target_sources(Window PRIVATE ${WINDOW_PRIVATE_SOURCES})

target_include_directories(Window
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/Public>
        $<INSTALL_INTERFACE:include/Window>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Private
)

target_link_libraries(Window
    PUBLIC Rocket::Core
    PRIVATE $<BUILD_INTERFACE:glfw>
)

set_target_properties(Core PROPERTIES POSITION_INDEPENDENT_CODE ON)
