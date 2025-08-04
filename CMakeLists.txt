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
configure_file(LICENSE.txt ${BINARY_DIR} COPYONLY)
configure_file(README.md ${BINARY_DIR} COPYONLY)

add_subdirectory(SDL)
add_executable(rtiow WIN32 main.c shader.c)
set_target_properties(rtiow PROPERTIES C_STANDARD 11)
target_link_libraries(rtiow PUBLIC SDL3::SDL3)

if(MSVC)
    find_package(Vulkan)
endif()
function(add_shader FILE)
    set(DEPENDS ${ARGN})
    set(GLSL ${CMAKE_SOURCE_DIR}/${FILE})
    set(SPV ${CMAKE_SOURCE_DIR}/bin/${FILE}.spv)
    set(DXIL ${CMAKE_SOURCE_DIR}/bin/${FILE}.dxil)
    set(MSL ${CMAKE_SOURCE_DIR}/bin/${FILE}.msl)
    set(JSON ${CMAKE_SOURCE_DIR}/bin/${FILE}.json)
    function(compile PROGRAM SOURCE OUTPUT)
        add_custom_command(
            OUTPUT ${OUTPUT}
            COMMAND ${PROGRAM} ${SOURCE} -o ${OUTPUT} -I src
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
            DEPENDS ${SOURCE} ${DEPENDS}
            COMMENT ${OUTPUT}
        )
        get_filename_component(NAME ${OUTPUT} NAME)
        string(REPLACE . _ NAME ${NAME})
        set(NAME compile_${NAME})
        add_custom_target(${NAME} DEPENDS ${OUTPUT})
        add_dependencies(rtiow ${NAME})
    endfunction()
    if (MSVC AND ${Vulkan_glslc_FOUND})
        set(SHADERCROSS SDL_shadercross/msvc/shadercross.exe)
        compile(${Vulkan_GLSLC_EXECUTABLE} ${GLSL} ${SPV} -g)
        compile(${SHADERCROSS} ${SPV} ${DXIL})
        compile(${SHADERCROSS} ${SPV} ${MSL})
        compile(${SHADERCROSS} ${SPV} ${JSON})
    endif()
    function(package OUTPUT)
        get_filename_component(NAME ${OUTPUT} NAME)
        set(BINARY ${BINARY_DIR}/${NAME})
        add_custom_command(
            OUTPUT ${BINARY}
            COMMAND ${CMAKE_COMMAND} -E copy ${OUTPUT} ${BINARY}
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
            DEPENDS ${OUTPUT}
            COMMENT ${BINARY}
        )
        string(REPLACE . _ NAME ${NAME})
        set(NAME package_${NAME})
        add_custom_target(${NAME} DEPENDS ${BINARY})
        add_dependencies(rtiow ${NAME})
    endfunction()
    package(${SPV})
    if(WIN32)
        package(${DXIL})
    elseif(APPLE)
        package(${MSL})
    endif()
    package(${JSON})
endfunction()
add_shader(shader.comp random.glsl config.h)