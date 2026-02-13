cmake_minimum_required(VERSION 3.16)

project(RHI LANGUAGES CXX)

add_library(RHI STATIC)
add_library(Rocket::RHI ALIAS RHI)

file(GLOB_RECURSE RHI_PUBLIC_HEADERS
    ${CMAKE_CURRENT_LIST_DIR}/Public/*.h
    ${CMAKE_CURRENT_LIST_DIR}/Public/*.hpp
)

file(GLOB_RECURSE RHI_PRIVATE_HEADERS
    ${CMAKE_CURRENT_LIST_DIR}/Private/*.h
    ${CMAKE_CURRENT_LIST_DIR}/Private/*.hpp
)

file(GLOB_RECURSE RHI_PRIVATE_SOURCES
    ${CMAKE_CURRENT_LIST_DIR}/*.c
    ${CMAKE_CURRENT_LIST_DIR}/*.cpp
)

target_sources(RHI PRIVATE 
    ${RHI_PUBLIC_HEADERS}
    ${RHI_PRIVATE_HEADERS}
    ${RHI_PRIVATE_SOURCES}
)

source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}/Public"  PREFIX "Public"  FILES ${RHI_PUBLIC_HEADERS})
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}/Private" PREFIX "Private" FILES ${RHI_PRIVATE_HEADERS} ${RHI_PRIVATE_SOURCES})

target_include_directories(RHI
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/Public>
        $<INSTALL_INTERFACE:include/RHI>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Private
)

target_link_libraries(RHI
    PUBLIC Rocket::Core
    PRIVATE Rocket::Platform
    PRIVATE $<BUILD_INTERFACE:vulkan::vulkan>
)

set_target_properties(RHI PROPERTIES POSITION_INDEPENDENT_CODE ON)
