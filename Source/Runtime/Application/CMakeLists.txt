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

target_sources(Application PRIVATE ${APPLICATION_PRIVATE_SOURCES})

target_include_directories(Application
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/Public>
        $<INSTALL_INTERFACE:include/Application>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Private
)

target_link_libraries(Application
    PUBLIC Rocket::Engine
    PRIVATE -Wl,--export-dynamic
)

add_custom_command(TARGET Application
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    ${CMAKE_BINARY_DIR}/compile_commands.json
    ${CMAKE_BINARY_DIR}/../compile_commands.json
    COMMENT "Copying compile_commands.json to project root"
)