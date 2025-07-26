cmake_minimum_required(VERSION 3.22)
project(Rocket LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY  ${CMAKE_BINARY_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY  ${CMAKE_BINARY_DIR})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY  ${CMAKE_BINARY_DIR})

foreach(cfg Debug Release RelWithDebInfo MinSizeRel)
    string(TOUPPER "${cfg}" CFG)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${CFG}  ${CMAKE_BINARY_DIR})
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_${CFG}  ${CMAKE_BINARY_DIR})
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${CFG}  ${CMAKE_BINARY_DIR})
endforeach()

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    add_compile_definitions(RK_PLATFORM_LINUX)
endif()

if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    add_compile_definitions(RK_PLATFORM_WINDOWS)
endif()

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_definitions(RK_DEBUG=1)
endif()

if (CMAKE_BUILD_TYPE STREQUAL "Release")
    add_compile_definitions(RK_RELEASE=1)
endif()

if (MINGW)
    add_link_options(-static -static-libgcc -static-libstdc++)
endif()

add_compile_definitions(ROCKET_BINARIES="${CMAKE_CURRENT_SOURCE_DIR}/Binaries")
add_compile_definitions(ROCKET_SRC="${CMAKE_CURRENT_SOURCE_DIR}/Source")
add_compile_definitions(ROCKET_CONTENT="${CMAKE_CURRENT_SOURCE_DIR}/Content")

# Third Party modules
add_subdirectory(Source/ThirdParty/glfw)
add_subdirectory(Source/ThirdParty/vulkan)
add_subdirectory(Source/ThirdParty/imgui)
add_subdirectory(Source/ThirdParty/glm)
add_subdirectory(Source/ThirdParty/entt)
add_subdirectory(Source/ThirdParty/spdlog)
add_subdirectory(Source/ThirdParty/fastgltf)

# Rocket modules
add_subdirectory(Source/Runtime/Core)
add_subdirectory(Source/Runtime/Engine)
add_subdirectory(Source/Runtime/RHI)
add_subdirectory(Source/Runtime/Window)
add_subdirectory(Source/Runtime/Scene)
add_subdirectory(Source/Runtime/Asset)
add_subdirectory(Source/Runtime/Application)

# Game entrypoint module
if (DEFINED PRIMARY_MODULE)
    add_subdirectory("${PRIMARY_MODULE}" "${PRIMARY_MODULE_DIR}")
endif()