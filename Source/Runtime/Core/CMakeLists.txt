cmake_minimum_required(VERSION 3.16)

project(Core LANGUAGES CXX)

add_library(Core STATIC)
add_library(Rocket::Core ALIAS Core)

file(GLOB_RECURSE CORE_PUBLIC_HEADERS
    ${CMAKE_CURRENT_LIST_DIR}/Public/*.h
    ${CMAKE_CURRENT_LIST_DIR}/Public/*.hpp
)

file(GLOB_RECURSE CORE_PRIVATE_HEADERS
    ${CMAKE_CURRENT_LIST_DIR}/Private/*.h
    ${CMAKE_CURRENT_LIST_DIR}/Private/*.hpp
)

file(GLOB_RECURSE CORE_PRIVATE_SOURCES
    ${CMAKE_CURRENT_LIST_DIR}/Private/*.c
    ${CMAKE_CURRENT_LIST_DIR}/Private/*.cpp
)

target_sources(Core PRIVATE
    ${CORE_PUBLIC_HEADERS}
    ${CORE_PRIVATE_HEADERS}
    ${CORE_PRIVATE_SOURCES}
)

source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}/Public"  PREFIX "Public"  FILES ${CORE_PUBLIC_HEADERS})
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}/Private" PREFIX "Private" FILES ${CORE_PRIVATE_HEADERS} ${CORE_PRIVATE_SOURCES})

target_include_directories(Core
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/Public>
        $<INSTALL_INTERFACE:include/Core>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Private
)

target_link_libraries(Core
)

set_target_properties(Core PROPERTIES POSITION_INDEPENDENT_CODE ON)
