cmake_minimum_required(VERSION 3.16)

project(Asset LANGUAGES CXX)

add_library(Asset STATIC)
add_library(Rocket::Asset ALIAS Asset)

file(GLOB_RECURSE ASSET_PUBLIC_HEADERS
    ${CMAKE_CURRENT_LIST_DIR}/Public/*.h
    ${CMAKE_CURRENT_LIST_DIR}/Public/*.hpp
)

file(GLOB_RECURSE ASSET_PRIVATE_HEADERS
    ${CMAKE_CURRENT_LIST_DIR}/Private/*.h
    ${CMAKE_CURRENT_LIST_DIR}/Private/*.hpp
)

file(GLOB_RECURSE ASSET_PRIVATE_SOURCES
    ${CMAKE_CURRENT_LIST_DIR}/*.c
    ${CMAKE_CURRENT_LIST_DIR}/*.cpp
)

target_sources(Asset PRIVATE 
    ${ASSET_PUBLIC_HEADERS}
    ${ASSET_PRIVATE_HEADERS}
    ${ASSET_PRIVATE_SOURCES}
)

target_include_directories(Asset
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/Public>
        $<INSTALL_INTERFACE:include/Asset>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Private
)

target_link_libraries(Asset
    PUBLIC Rocket::Core
    PUBLIC Rocket::RenderCore
    PRIVATE $<BUILD_INTERFACE:RapidJSON>
    PRIVATE $<BUILD_INTERFACE:glm::glm>
    PRIVATE $<BUILD_INTERFACE:vulkan::vulkan>
    PRIVATE $<BUILD_INTERFACE:tinygltf>
)

set_target_properties(Core PROPERTIES POSITION_INDEPENDENT_CODE ON)
