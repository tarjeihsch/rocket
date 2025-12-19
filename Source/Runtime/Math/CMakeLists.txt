cmake_minimum_required(VERSION 3.16)

project(Math LANGUAGES CXX)

add_library(Math STATIC)
add_library(Rocket::Math ALIAS Math)

file(GLOB_RECURSE MATH_PUBLIC_HEADERS
    ${CMAKE_CURRENT_LIST_DIR}/Public/*.h
    ${CMAKE_CURRENT_LIST_DIR}/Public/*.hpp
)

file(GLOB_RECURSE MATH_PRIVATE_HEADERS
    ${CMAKE_CURRENT_LIST_DIR}/Private/*.h
    ${CMAKE_CURRENT_LIST_DIR}/Private/*.hpp
)

file(GLOB_RECURSE MATH_PRIVATE_SOURCES
    ${CMAKE_CURRENT_LIST_DIR}/Private/*.c
    ${CMAKE_CURRENT_LIST_DIR}/Private/*.cpp
)

target_sources(Math PRIVATE 
    ${MATH_PUBLIC_HEADERS}
    ${MATH_PRIVATE_HEADERS}
    ${MATH_PRIVATE_SOURCES}
)

target_include_directories(Math
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/Public>
        $<INSTALL_INTERFACE:include/Math>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Private
)

target_link_libraries(Math
    PRIVATE $<BUILD_INTERFACE:glm::glm>
)

target_compile_definitions(Math 
    PUBLIC
        MATH_REAL_64
)

set_target_properties(Math PROPERTIES POSITION_INDEPENDENT_CODE ON)
