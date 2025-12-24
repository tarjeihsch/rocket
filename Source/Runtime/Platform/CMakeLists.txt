cmake_minimum_required(VERSION 3.16)

project(Platform LANGUAGES CXX)

add_library(Platform STATIC)
add_library(Rocket::Platform ALIAS Platform)

file(GLOB_RECURSE PLATFORM_PUBLIC_HEADERS
    ${CMAKE_CURRENT_LIST_DIR}/Public/*.h
    ${CMAKE_CURRENT_LIST_DIR}/Public/*.hpp
)

file(GLOB_RECURSE PLATFORM_PRIVATE_HEADERS
    ${CMAKE_CURRENT_LIST_DIR}/Private/*.h
    ${CMAKE_CURRENT_LIST_DIR}/Private/*.hpp
)

file(GLOB_RECURSE PLATFORM_PRIVATE_SOURCES
    ${CMAKE_CURRENT_LIST_DIR}/Private/*.c
    ${CMAKE_CURRENT_LIST_DIR}/Private/*.cpp
)

target_sources(Platform PRIVATE 
    ${PLATFORM_PRIVATE_SOURCES}
    ${PLATFORM_PRIVATE_HEADERS}
    ${PLATFORM_PUBLIC_HEADERS}
)

source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}/Public"  PREFIX "Public"  FILES ${PLATFORM_PUBLIC_HEADERS})
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}/Private" PREFIX "Private" FILES ${PLATFORM_PRIVATE_HEADERS} ${PLATFORM_PRIVATE_SOURCES})

target_include_directories(Platform
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/Public>
        $<INSTALL_INTERFACE:include/Platform>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Private
)

target_link_libraries(Platform
    PUBLIC 
        Rocket::Core
    PRIVATE 
        $<BUILD_INTERFACE:glfw>
)

set_target_properties(Platform PROPERTIES POSITION_INDEPENDENT_CODE ON)
