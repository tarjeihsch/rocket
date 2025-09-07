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
    PRIVATE ${CMAKE_CURRENT_LIST_DIR}/Private
    PUBLIC ${CMAKE_CURRENT_LIST_DIR}/Public
)

if (DEFINED PRIMARY_MODULE)
    target_link_libraries(Application
        PUBLIC Rocket::Engine
        PRIVATE $<$<CONFIG:Debug>:-Wl,--whole-archive>
        PRIVATE Game
        PRIVATE $<$<CONFIG:Debug>:-Wl,--no-whole-archive>
    )
else()
    target_link_libraries(Application
        PUBLIC Rocket::Engine
    )
endif()

add_custom_command(
    TARGET Application POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_BINARY_DIR}/compile_commands.json ${CMAKE_BINARY_DIR}/../compile_commands.json
)
