#include <SDL3/SDL.h>
#include <spirv_reflect.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include "helpers.h"

SDL_GPUShader* load_shader(
    SDL_GPUDevice* device,
    const char* file)
{
    assert(device);
    assert(file);
    SDL_GPUShaderCreateInfo info = {0};
    size_t size;
    void* code = SDL_LoadFile(file, &size);
    if (!code)
    {
        SDL_Log("Failed to load shader: %s, %s", file, SDL_GetError());
        return NULL;
    }
    info.code = code;
    info.code_size = size;
    SpvReflectShaderModule module;
    SpvReflectResult result = spvReflectCreateShaderModule(size, code, &module);
    if (result != SPV_REFLECT_RESULT_SUCCESS)
    {
        SDL_Log("Failed to reflect shader: %s", file);
        SDL_free(code);
        return NULL;
    }
    for (int i = 0; i < module.descriptor_binding_count; i++)
    {
        const SpvReflectDescriptorBinding* binding = &module.descriptor_bindings[i];
        switch (module.descriptor_bindings[i].descriptor_type)
        {
        case SPV_REFLECT_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
            info.num_uniform_buffers++;
            break;
        case SPV_REFLECT_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
            info.num_samplers++;
            break;
        case SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_BUFFER:
            info.num_storage_buffers++;
            break;
        case SPV_REFLECT_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
            info.num_storage_textures++;
            break;
        }
    }
    spvReflectDestroyShaderModule(&module);
    if (strstr(file, ".vert"))
    {
        info.stage = SDL_GPU_SHADERSTAGE_VERTEX;
    }
    else
    {
        info.stage = SDL_GPU_SHADERSTAGE_FRAGMENT;
    }
    info.format = SDL_GPU_SHADERFORMAT_SPIRV;
    info.entrypoint = "main";
    SDL_GPUShader* shader = SDL_CreateGPUShader(device, &info);
    SDL_free(code);
    if (!shader)
    {
        SDL_Log("Failed to create shader: %s, %s", file, SDL_GetError());
        return NULL;
    }
    return shader;
}

SDL_GPUComputePipeline* load_compute_pipeline(
    SDL_GPUDevice* device,
    const char* file)
{
    assert(device);
    assert(file);
    SDL_GPUComputePipelineCreateInfo info = {0};
    size_t size;
    void* code = SDL_LoadFile(file, &size);
    if (!code)
    {
        SDL_Log("Failed to load compute pipeline: %s, %s", file, SDL_GetError());
        return NULL;
    }
    info.code = code;
    info.code_size = size;
    SpvReflectShaderModule module;
    SpvReflectResult result = spvReflectCreateShaderModule(size, code, &module);
    if (result != SPV_REFLECT_RESULT_SUCCESS)
    {
        SDL_Log("Failed to reflect compute pipeline: %s", file);
        SDL_free(code);
        return NULL;
    }
    const SpvReflectEntryPoint* entry = spvReflectGetEntryPoint(&module, "main");
    if (!entry)
    {
        SDL_Log("Failed to reflect compute pipeline: %s", file);
        SDL_free(code);
        return NULL;
    }
    info.threadcount_x = entry->local_size.x;
    info.threadcount_y = entry->local_size.y;
    info.threadcount_z = entry->local_size.z;
    for (int i = 0; i < module.descriptor_binding_count; i++)
    {
        const SpvReflectDescriptorBinding* binding = &module.descriptor_bindings[i];
        switch (binding->descriptor_type)
        {
        case SPV_REFLECT_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
            info.num_uniform_buffers++;
            break;
        case SPV_REFLECT_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
            info.num_samplers++;
            break;
        case SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_BUFFER:
            if (binding->accessed)
            {
                info.num_readwrite_storage_buffers++;
            }
            else
            {
                info.num_readonly_storage_buffers++;
            }
            break;
        case SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_IMAGE:
            if (binding->accessed)
            {
                info.num_readwrite_storage_textures++;
            }
            else
            {
                info.num_readonly_storage_textures++;
            }
            break;
        }
    }
    spvReflectDestroyShaderModule(&module);
    info.format = SDL_GPU_SHADERFORMAT_SPIRV;
    info.entrypoint = "main";
    SDL_GPUComputePipeline* pipeline = SDL_CreateGPUComputePipeline(device, &info);
    SDL_free(code);
    if (!pipeline)
    {
        SDL_Log("Failed to create compute pipeline: %s, %s", file, SDL_GetError());
        return NULL;
    }
    return pipeline;
}