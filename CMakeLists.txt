cmake_minimum_required(VERSION 3.24)
project(sdl3_ray_tracing)

set(BINARY_DIR ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${BINARY_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${BINARY_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${BINARY_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${BINARY_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE ${BINARY_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG ${BINARY_DIR})
make_directory(${BINARY_DIR})

add_subdirectory(lib/SDL)
add_executable(sdl3_ray_tracing WIN32
    lib/SPIRV-Reflect/spirv_reflect.c
    src/helpers.c
    src/main.c
)
target_link_libraries(sdl3_ray_tracing PUBLIC SDL3::SDL3)
target_include_directories(sdl3_ray_tracing PUBLIC lib/SPIRV-Reflect)
set_target_properties(sdl3_ray_tracing PROPERTIES C_STANDARD 11)

function(spirv FILE)
    set(SOURCE shaders/${FILE})
    set(OUTPUT ${BINARY_DIR}/${FILE})
    add_custom_command(
        OUTPUT ${OUTPUT}
        COMMAND glslc ${SOURCE} -o ${OUTPUT} -I src
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        DEPENDS ${SOURCE}
        BYPRODUCTS ${OUTPUT}
        COMMENT ${SOURCE}
    )
    string(REPLACE . _ NAME ${FILE})
    add_custom_target(${NAME} DEPENDS ${OUTPUT})
    add_dependencies(sdl3_ray_tracing ${NAME})
endfunction()
spirv(ray.comp)

configure_file(LICENSE.txt ${BINARY_DIR} COPYONLY)
configure_file(README.md ${BINARY_DIR} COPYONLY)