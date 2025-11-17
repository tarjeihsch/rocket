cmake_minimum_required(VERSION 3.23)

include(CMakePackageConfigHelpers)

project(Rocket LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY  ${CMAKE_BINARY_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY  ${CMAKE_BINARY_DIR})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY  ${CMAKE_BINARY_DIR})

set(RK_ENABLE_DEBUG_OVERLAY 0 CACHE BOOL "Enable Rocket debug overlay")
set(RK_ENABLE_PROFILER 0 CACHE BOOL "Enable Rocket profiler")

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

add_compile_definitions(ROCKET_BINARIES=\"${CMAKE_CURRENT_SOURCE_DIR}/Binaries\")
add_compile_definitions(ROCKET_SRC=\"${CMAKE_CURRENT_SOURCE_DIR}/Source\")
add_compile_definitions(ROCKET_CONTENT=\"${CMAKE_CURRENT_SOURCE_DIR}/Content\")

# Third Party modules
add_subdirectory(Source/ThirdParty/glfw EXCLUDE_FROM_ALL)
add_subdirectory(Source/ThirdParty/vulkan EXCLUDE_FROM_ALL)
add_subdirectory(Source/ThirdParty/imgui EXCLUDE_FROM_ALL)
add_subdirectory(Source/ThirdParty/glm EXCLUDE_FROM_ALL)
add_subdirectory(Source/ThirdParty/spdlog EXCLUDE_FROM_ALL)
add_subdirectory(Source/ThirdParty/tinygltf EXCLUDE_FROM_ALL)
add_subdirectory(Source/ThirdParty/rapidjson EXCLUDE_FROM_ALL)

# Rocket modules
add_subdirectory(Source/Editor/Application)
add_subdirectory(Source/Editor/EditorCore)
add_subdirectory(Source/Editor/EditorUI)
add_subdirectory(Source/Runtime/Rendering)
add_subdirectory(Source/Runtime/Object)
add_subdirectory(Source/Runtime/Math)
add_subdirectory(Source/Runtime/Core)
add_subdirectory(Source/Runtime/Engine)
add_subdirectory(Source/Runtime/RHI)
add_subdirectory(Source/Runtime/RenderCore)
add_subdirectory(Source/Runtime/Scene)
add_subdirectory(Source/Runtime/Window)
add_subdirectory(Source/Runtime/Asset)
add_subdirectory(Source/Runtime/Application)

set(ROCKET_EXPORT_LIBS
    Core
    Object
    Engine
    RHI
    RenderCore
    Rendering
    Window
    Asset
    Math
)

configure_package_config_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/CMake/RocketConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/RocketConfig.cmake
    INSTALL_DESTINATION cmake
)

install(TARGETS ${ROCKET_EXPORT_LIBS}
    EXPORT RocketTargets
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
)

install(TARGETS Application
    RUNTIME DESTINATION bin
)

install(EXPORT RocketTargets
    FILE RocketTargets.cmake
    NAMESPACE Rocket::
    DESTINATION cmake
)

install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/RocketConfig.cmake
    DESTINATION cmake
)

foreach(module IN LISTS ROCKET_EXPORT_LIBS)
    install(DIRECTORY Source/Runtime/${module}/Public/
        DESTINATION include/${module}
        FILES_MATCHING PATTERN "*.h" PATTERN "*.hpp")
endforeach()

configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/CMake/RocketConfig.h.in
    ${CMAKE_CURRENT_BINARY_DIR}/Generated/RocketConfig.h
    @ONLY
)

#target_include_directories(Core PUBLIC
#    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/Generated>
#    $<INSTALL_INTERFACE:include>
#)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/Generated/RocketConfig.h
    DESTINATION include/Rocket
)
