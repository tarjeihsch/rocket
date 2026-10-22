cmake_minimum_required(VERSION 3.23)

include(CMakePackageConfigHelpers)

project(Rocket LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

set(BUILD_SHARED_LIBS OFF)

set(RK_ENABLE_DEBUG_OVERLAY 0 CACHE BOOL "Enable Rocket debug overlay")
set(RK_ENABLE_PROFILER 0 CACHE BOOL "Enable Rocket profiler")
set(RK_DEBUG 0 CACHE BOOL "Enable Vulkan validation layers")

foreach(cfg Debug Release RelWithDebInfo MinSizeRel)
    string(TOUPPER "${cfg}" CFG)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${CFG} ${CMAKE_BINARY_DIR})
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_${CFG} ${CMAKE_BINARY_DIR})
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${CFG} ${CMAKE_BINARY_DIR})
endforeach()

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    add_compile_definitions(RK_PLATFORM_LINUX)
endif()

if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
	add_compile_options(/MP)
    add_compile_definitions(RK_PLATFORM_WINDOWS)
endif()

add_compile_definitions(RK_DEBUG=$<BOOL:${RK_DEBUG}>)
add_compile_definitions($<$<CONFIG:Release>:RK_RELEASE=1>)
add_compile_definitions(ROCKET_BINARIES=\"${CMAKE_CURRENT_SOURCE_DIR}/Binaries\")
add_compile_definitions(ROCKET_SRC=\"${CMAKE_CURRENT_SOURCE_DIR}/Source\")
add_compile_definitions(ROCKET_CONTENT=\"${CMAKE_CURRENT_SOURCE_DIR}/Content\")

# Third Party modules
add_subdirectory(Source/ThirdParty/glfw EXCLUDE_FROM_ALL)
add_subdirectory(Source/ThirdParty/vulkan EXCLUDE_FROM_ALL)

# Rocket modules
add_subdirectory(Source/Runtime/Core)
add_subdirectory(Source/Runtime/Platform)
add_subdirectory(Source/Runtime/RHI)
add_subdirectory(Source/Runtime/Geometry)
add_subdirectory(Source/Runtime/Graphics)

enable_testing()
add_subdirectory(Source/Tests/Core)
add_test(NAME Color COMMAND ColorTest)
add_test(NAME Delegate COMMAND DelegateTest)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set_target_properties(glfw PROPERTIES FOLDER "ThirdParty/glfw")

set(ROCKET_EXPORT_LIBS
    Core
    Platform
    RHI
	Geometry
	Graphics
)

configure_package_config_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/CMake/RocketConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/RocketConfig.cmake
    INSTALL_DESTINATION cmake
)

install(TARGETS ${ROCKET_EXPORT_LIBS} glfw
    EXPORT RocketTargets
    ARCHIVE DESTINATION lib/$<CONFIG>
    LIBRARY DESTINATION lib/$<CONFIG>
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

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/Generated/RocketConfig.h
    DESTINATION include/Rocket
)
