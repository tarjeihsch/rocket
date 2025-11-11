cmake_minimum_required(VERSION 3.16)

project(Shader LANGUAGES NONE)

add_library(Shader STATIC)
add_library(Rocket::Shader ALIAS Shader)

file(GLOB_RECURSE SHADER_FILES
    ${CMAKE_CURRENT_LIST_DIR}/*.hlsl
)

target_sources(Shader PRIVATE ${SHADER_FILES})

set_source_files_properties(${SHADER_FILES} PROPERTIES HEADER_FILE_ONLY TRUE)

source_group(TREE ${CMAKE_CURRENT_LIST_DIR}/Shader PREFIX "Shader Files" FILES ${SHADER_FILES})

set_target_properties(Shader PROPERTIES
    LINKER_LANGUAGE CXX
    FOLDER "Shader"
)
