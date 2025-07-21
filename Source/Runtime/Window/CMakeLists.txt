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
    PRIVATE ${CMAKE_CURRENT_LIST_DIR}/Private
    PUBLIC ${CMAKE_CURRENT_LIST_DIR}/Public
)

target_link_libraries(Window
    PUBLIC Rocket::Core
    PRIVATE glfw
)
