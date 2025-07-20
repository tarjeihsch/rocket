#TODO: Create one CMakeLists.txt per sub-directory of Source/Runtime. Example: RHI gets it's own CMakeLists.txt, same goes for IO, Application, Core. These are modules, and this CMakeLists.txt file would collect and manage all.
# We are currently setting the target_sources for Rocket, which should not be done. We should let each directory beneath Source/Runtime to set it's own rules and includes.

#TODO: Rename Rocket::vulkan and Rocket::imgui. Rocket is reserved for our own modules.

cmake_minimum_required(VERSION 3.22)
project(Rocket LANGUAGES C CXX)

include(FetchContent)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

file(GLOB_RECURSE ROCKET_PUBLIC_HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/Runtime/*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/Runtime/*.hpp
)

file(GLOB_RECURSE ROCKET_PRIVATE_HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/Runtime/*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/Runtime/*.hpp
)

file(GLOB_RECURSE ROCKET_PRIVATE_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/Runtime/*.cpp
)

add_subdirectory(Source/ThirdParty/glfw)
add_subdirectory(Source/ThirdParty/vulkan)
add_subdirectory(Source/ThirdParty/imgui)
add_subdirectory(Source/ThirdParty/glm)
add_subdirectory(Source/ThirdParty/entt)
add_subdirectory(Source/ThirdParty/spdlog)
add_executable(Rocket)

target_sources(Rocket PRIVATE
    ${ROCKET_PRIVATE_SOURCES}
    ${ROCKET_PRIVATE_HEADERS}
    ${ROCKET_PUBLIC_HEADERS}
)

target_include_directories(Rocket
    PUBLIC  ${CMAKE_CURRENT_SOURCE_DIR}/Source/Runtime
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/Source/Runtime
)

target_compile_definitions(Rocket PRIVATE
    RK_ROOT_DIR=\"${CMAKE_CURRENT_SOURCE_DIR}\"
    RK_SOURCE_DIR=\"${CMAKE_CURRENT_SOURCE_DIR}/Source\"
)

target_link_libraries(Rocket PRIVATE 
    glfw
    glm::glm 
    EnTT::EnTT 
    spdlog::spdlog
    Rocket::vulkan
    Rocket::imgui 
)

target_precompile_headers(Rocket PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/Runtime/RocketPCH.h
)

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    target_compile_definitions(Rocket PRIVATE RK_PLATFORM_LINUX)
endif()

if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    target_compile_definitions(Rocket PRIVATE RK_PLATFORM_WINDOWS)
endif()

if (MINGW)
    target_link_options(Rocket PRIVATE -static -static-libgcc -static-libstdc++)
endif()

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_definitions(Rocket PRIVATE RK_DEBUG=1)
endif()

if (CMAKE_BUILD_TYPE STREQUAL "Release")
    target_compile_definitions(Rocket PRIVATE RK_RELEASE=1)
endif()

add_custom_command(
    TARGET Rocket POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_BINARY_DIR}/compile_commands.json ${CMAKE_BINARY_DIR}/../compile_commands.json
)
