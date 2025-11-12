cmake_minimum_required(VERSION 3.16)
project(Application LANGUAGES CXX)

add_executable(Application)
add_executable(Rocket::Application ALIAS Application)

file(GLOB_RECURSE APPLICATION_PUBLIC_HEADERS
    ${CMAKE_CURRENT_LIST_DIR}/Public/*.h
    ${CMAKE_CURRENT_LIST_DIR}/Public/*.hpp
)

file(GLOB_RECURSE APPLICATION_PRIVATE_HEADERS
    ${CMAKE_CURRENT_LIST_DIR}/Private/*.h
    ${CMAKE_CURRENT_LIST_DIR}/Private/*.hpp
)

file(GLOB_RECURSE APPLICATION_PRIVATE_SOURCES
    ${CMAKE_CURRENT_LIST_DIR}/Private/*.c
    ${CMAKE_CURRENT_LIST_DIR}/Private/*.cpp
)

target_sources(Application PRIVATE 
    ${APPLICATION_PUBLIC_HEADERS}
    ${APPLICATION_PRIVATE_HEADERS}
    ${APPLICATION_PRIVATE_SOURCES}
)

target_include_directories(Application
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/Public>
        $<INSTALL_INTERFACE:include/Application>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Private
)

target_link_libraries(Application
    PRIVATE Rocket::Engine
    PRIVATE Rocket::Core
    PRIVATE Rocket::Window
    PRIVATE -Wl,--export-dynamic
)

if (EXISTS ${CMAKE_BINARY_DIR}/compile_commands.json)
    file(COPY ${CMAKE_BINARY_DIR}/compile_commands.json DESTINATION ${CMAKE_SOURCE_DIR})
endif()