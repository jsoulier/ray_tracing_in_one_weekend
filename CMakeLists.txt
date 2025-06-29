cmake_minimum_required(VERSION 3.24)
project(rtiow)

set(BINARY_DIR ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${BINARY_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${BINARY_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${BINARY_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${BINARY_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE ${BINARY_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG ${BINARY_DIR})
make_directory(${BINARY_DIR})

add_subdirectory(SDL)
add_executable(rtiow WIN32 main.c)
set_target_properties(rtiow PROPERTIES C_STANDARD 11)
target_link_libraries(rtiow PUBLIC SDL3::SDL3)
add_custom_command(
    OUTPUT ${BINARY_DIR}/shader.comp
    COMMAND glslc shader.comp -o ${BINARY_DIR}/shader.comp -I src
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    DEPENDS shader.comp config.h random.glsl
    BYPRODUCTS ${BINARY_DIR}/shader.comp
    COMMENT shader.comp
)
add_custom_target(shader_comp DEPENDS ${BINARY_DIR}/shader.comp)
add_dependencies(rtiow shader_comp)
configure_file(LICENSE.txt ${BINARY_DIR} COPYONLY)
configure_file(README.md ${BINARY_DIR} COPYONLY)