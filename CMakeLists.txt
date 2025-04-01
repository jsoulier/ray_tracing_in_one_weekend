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

function(spirv FILE)
    set(OUTPUT ${BINARY_DIR}/${FILE})
    add_custom_command(
        OUTPUT ${OUTPUT}
        COMMAND glslc ${FILE} -o ${OUTPUT} -I src
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        DEPENDS ${FILE} config.h random.glsl
        BYPRODUCTS ${OUTPUT}
        COMMENT ${FILE}
    )
    string(REPLACE . _ NAME ${FILE})
    add_custom_target(${NAME} DEPENDS ${OUTPUT})
    add_dependencies(rtiow ${NAME})
endfunction()
spirv(shader.comp)

configure_file(LICENSE.txt ${BINARY_DIR} COPYONLY)
configure_file(README.md ${BINARY_DIR} COPYONLY)