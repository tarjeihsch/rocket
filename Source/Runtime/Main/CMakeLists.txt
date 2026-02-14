cmake_minimum_required(VERSION 3.16)

project(Main LANGUAGES CXX)

add_executable(Main)
add_executable(Rocket::Main ALIAS Main)

file(GLOB_RECURSE MAIN_PUBLIC_HEADERS
    ${CMAKE_CURRENT_LIST_DIR}/Public/*.h
    ${CMAKE_CURRENT_LIST_DIR}/Public/*.hpp
)

file(GLOB_RECURSE MAIN_PRIVATE_HEADERS
    ${CMAKE_CURRENT_LIST_DIR}/Private/*.h
    ${CMAKE_CURRENT_LIST_DIR}/Private/*.hpp
)

file(GLOB_RECURSE MAIN_PRIVATE_SOURCES
    ${CMAKE_CURRENT_LIST_DIR}/Private/*.c
    ${CMAKE_CURRENT_LIST_DIR}/Private/*.cpp
)

target_sources(Main PRIVATE 
    ${MAIN_PRIVATE_SOURCES}
    ${MAIN_PRIVATE_HEADERS}
    ${MAIN_PUBLIC_HEADERS}
)

source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}/Public"  PREFIX "Public"  FILES ${MAIN_PUBLIC_HEADERS})
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}/Private" PREFIX "Private" FILES ${MAIN_PRIVATE_HEADERS} ${MAIN_PRIVATE_SOURCES})

target_include_directories(Main
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/Public>
        $<INSTALL_INTERFACE:include/Main>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Private
)

target_link_libraries(Main
    PUBLIC 
        Rocket::Core
        Rocket::Platform
        Rocket::RHI
)

set_target_properties(Main PROPERTIES POSITION_INDEPENDENT_CODE ON)
