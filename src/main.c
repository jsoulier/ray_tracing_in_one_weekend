#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "config.h"

static SDL_GPUBuffer* create_spheres(
    SDL_GPUDevice* device,
    uint32_t* size)
{
    struct
    {
        float x;
        float y;
        float z;
        float radius;
        float r;
        float g;
        float b;
        uint32_t type;
        float fuzz;
        float refraction;
        uint32_t padding1;
        uint32_t padding2;
    }
    spheres[4 + 484] =
    {{
        .x = 0.0f,
        .y = -1000.0f,
        .z = 0.0f,
        .radius = 1000.0f,
        .r = 0.5f,
        .g = 0.5f,
        .b = 0.5f,
        .type = LAMBERTIAN,
        .fuzz = 0.0f,
        .refraction = 0.0f,
    },
    {
        .x = 0.0f,
        .y = 1.0f,
        .z = 0.0f,
        .radius = 1.0f,
        .r = 0.0f,
        .g = 0.0f,
        .b = 0.0f,
        .type = DIAELECTRIC,
        .fuzz = 0.0f,
        .refraction = 1.5f,
    },
    {
        .x = -4.0f,
        .y = 1.0f,
        .z = 0.0f,
        .radius = 1.0f,
        .r = 0.4f,
        .g = 0.2f,
        .b = 0.1f,
        .type = LAMBERTIAN,
        .fuzz = 0.0f,
        .refraction = 1.5f,
    },
    {
        .x = 4.0f,
        .y = 1.0f,
        .z = 0.0f,
        .radius = 1.0f,
        .r = 0.7f,
        .g = 0.6f,
        .b = 0.5f,
        .type = METAL,
        .fuzz = 0.0f,
        .refraction = 1.5f,
    }};
    srand(time(NULL));
    for (int a = -11; a < 11; a++)
    for (int b = -11; b < 11; b++)
    {
        const int i = 4 + (a + 11) * 22 + b + 11;
        const float material = rand() / (RAND_MAX + 1.0f);
        spheres[i].x = a + 0.9f * rand() / (RAND_MAX + 1.0f);
        spheres[i].y = 0.2f;
        spheres[i].z = b + 0.9f * rand() / (RAND_MAX + 1.0f);
        spheres[i].radius = 0.2f;
        if (material < 0.8f)
        {
            spheres[i].type = LAMBERTIAN;
            spheres[i].r = rand() / (RAND_MAX + 1.0f);
            spheres[i].g = rand() / (RAND_MAX + 1.0f);
            spheres[i].b = rand() / (RAND_MAX + 1.0f);
            spheres[i].fuzz = 0.0f;
            spheres[i].refraction = 0.0f;
        }
        else if (material < 0.95f)
        {
            spheres[i].type = METAL;
            spheres[i].r = rand() / (RAND_MAX + 1.0f);
            spheres[i].g = rand() / (RAND_MAX + 1.0f);
            spheres[i].b = rand() / (RAND_MAX + 1.0f);
            spheres[i].fuzz = rand() / (RAND_MAX + 1.0f) * 0.5f;
            spheres[i].refraction = 0.0f;
        }
        else
        {
            spheres[i].type = DIAELECTRIC;
            spheres[i].r = 0.0f,
            spheres[i].g = 0.0f,
            spheres[i].b = 0.0f,
            spheres[i].fuzz = 0.0f;
            spheres[i].refraction = 1.5f;
        }
    }
    SDL_GPUTransferBufferCreateInfo tbci = {0};
    tbci.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD;
    tbci.size = sizeof(spheres);
    SDL_GPUBufferCreateInfo bci = {0};
    bci.usage = SDL_GPU_BUFFERUSAGE_COMPUTE_STORAGE_READ;
    bci.size = sizeof(spheres);
    SDL_GPUTransferBuffer* tbo = SDL_CreateGPUTransferBuffer(device, &tbci);
    SDL_GPUBuffer* sbo = SDL_CreateGPUBuffer(device, &bci);
    if (!tbo || !sbo)
    {
        SDL_Log("Failed to create buffer(s): %s", SDL_GetError());
        return NULL;
    }
    void* data = SDL_MapGPUTransferBuffer(device, tbo, false);
    if (!data)
    {
        SDL_Log("Failed to map transfer buffer: %s", SDL_GetError());
        return NULL;
    }
    memcpy(data, spheres, sizeof(spheres));
    SDL_UnmapGPUTransferBuffer(device, tbo);
    SDL_GPUCommandBuffer* commands = SDL_AcquireGPUCommandBuffer(device);
    if (!commands)
    {
        SDL_Log("Failed to acquire command buffer: %s", SDL_GetError());
        return NULL;
    }
    SDL_GPUCopyPass* pass = SDL_BeginGPUCopyPass(commands);
    if (!pass)
    {
        SDL_Log("Failed to begin copy pass: %s", SDL_GetError());
        return NULL;
    }
    SDL_GPUTransferBufferLocation src = {0};
    SDL_GPUBufferRegion dst = {0};
    src.transfer_buffer = tbo;
    dst.buffer = sbo;
    dst.size = sizeof(spheres);
    SDL_UploadToGPUBuffer(pass, &src, &dst, false);
    SDL_EndGPUCopyPass(pass);
    SDL_SubmitGPUCommandBuffer(commands);
    SDL_ReleaseGPUTransferBuffer(device, tbo);
    *size = sizeof(spheres) / sizeof(spheres[0]);
    return sbo;
}

int main(
    int argc,
    char** argv)
{
    SDL_SetLogPriorities(SDL_LOG_PRIORITY_TRACE);
    if (!SDL_Init(SDL_INIT_VIDEO))
    {
        SDL_Log("Failed to initialize SDL: %s", SDL_GetError());
        return EXIT_FAILURE;
    }
    SDL_GPUDevice* device = SDL_CreateGPUDevice(SDL_GPU_SHADERFORMAT_SPIRV, true, NULL);
    if (!device)
    {
        SDL_Log("Failed to create device: %s", SDL_GetError());
        return EXIT_FAILURE;
    }
    size_t size;
    void* code = SDL_LoadFile("shader.comp", &size);
    if (!code)
    {
        SDL_Log("Failed to load compute pipeline:  %s", SDL_GetError());
        return EXIT_FAILURE;
    }
    SDL_GPUComputePipelineCreateInfo gpci = {0};
    gpci.code = code;
    gpci.code_size = size;
    gpci.threadcount_x = THREADS;
    gpci.threadcount_y = 1;
    gpci.threadcount_z = 1;
    gpci.num_uniform_buffers = 1;
    gpci.num_samplers = 0;
    gpci.num_readwrite_storage_buffers = 0;
    gpci.num_readonly_storage_buffers = 1;
    gpci.num_readwrite_storage_textures = 1;
    gpci.num_readonly_storage_textures = 0;
    gpci.format = SDL_GPU_SHADERFORMAT_SPIRV;
    gpci.entrypoint = "main";
    SDL_GPUComputePipeline* pipeline = SDL_CreateGPUComputePipeline(device, &gpci);
    if (!pipeline)
    {
        SDL_Log("Failed to load compute pipeline");
        return EXIT_FAILURE;
    }
    SDL_free(code);
    SDL_GPUCommandBuffer* commands = SDL_AcquireGPUCommandBuffer(device);
    if (!commands)
    {
        SDL_Log("Failed to acquire command buffer: %s", SDL_GetError());
        return EXIT_FAILURE;
    }
    SDL_GPUTextureCreateInfo tci = {0};
    tci.usage = SDL_GPU_TEXTUREUSAGE_COMPUTE_STORAGE_WRITE;
    tci.type = SDL_GPU_TEXTURETYPE_2D;
    tci.format = SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM;
    tci.width = WIDTH;
    tci.height = HEIGHT;
    tci.num_levels = 1;
    tci.layer_count_or_depth = 1;
    SDL_GPUTexture* texture = SDL_CreateGPUTexture(device, &tci);
    if (!texture)
    {
        SDL_Log("Failed to create texture: %s", SDL_GetError());
        return EXIT_FAILURE;
    }
    SDL_GPUStorageTextureReadWriteBinding stb = {0};
    stb.texture = texture;
    SDL_GPUComputePass* pass = SDL_BeginGPUComputePass(commands, &stb, 1, NULL, 0);
    if (!pass)
    {
        SDL_Log("Failed to begin compute pass: %s", SDL_GetError());
        return EXIT_FAILURE;
    }
    uint32_t num_spheres;
    SDL_GPUBuffer* spheres = create_spheres(device, &num_spheres);
    if (!spheres)
    {
        SDL_Log("Failed to create spheres");
        return EXIT_FAILURE;
    }
    SDL_BindGPUComputePipeline(pass, pipeline);
    SDL_BindGPUComputeStorageBuffers(pass, 0, &spheres, 1);
    SDL_PushGPUComputeUniformData(commands, 0, &num_spheres, sizeof(num_spheres));
    SDL_DispatchGPUCompute(pass, (WIDTH + THREADS - 1) / THREADS, HEIGHT, 1);
    SDL_EndGPUComputePass(pass);
    SDL_GPUTransferBufferCreateInfo tbci = {0};
    tbci.usage = SDL_GPU_TRANSFERBUFFERUSAGE_DOWNLOAD;
    tbci.size = WIDTH * HEIGHT * 4;
    SDL_GPUTransferBuffer* results = SDL_CreateGPUTransferBuffer(device, &tbci);
    if (!results)
    {
        SDL_Log("Failed to create transfer buffer: %s", SDL_GetError());
        return EXIT_FAILURE;
    }
    SDL_GPUCopyPass* copy = SDL_BeginGPUCopyPass(commands);
    if (!copy)
    {
        SDL_Log("Failed to begin copy pass: %s", SDL_GetError());
        return EXIT_FAILURE;
    }
    SDL_GPUTextureRegion src = {0};
    SDL_GPUTextureTransferInfo dst = {0};
    src.texture = texture;
    src.w = WIDTH;
    src.h = HEIGHT;
    src.d = 1;
    dst.transfer_buffer = results;
    SDL_DownloadFromGPUTexture(copy, &src, &dst);
    SDL_EndGPUCopyPass(copy);
    SDL_GPUFence* fence = SDL_SubmitGPUCommandBufferAndAcquireFence(commands);
    if (!fence)
    {
        SDL_Log("Failed to acquire fence: %s", SDL_GetError());
        return EXIT_FAILURE;
    }
    SDL_WaitForGPUFences(device, true, &fence, 1);
    SDL_ReleaseGPUFence(device, fence);
    void* data = SDL_MapGPUTransferBuffer(device, results, false);
    if (!data)
    {
        SDL_Log("Failed to map transfer buffer: %s", SDL_GetError());
        return EXIT_FAILURE;
    }
    SDL_Surface* surface = SDL_CreateSurfaceFrom(WIDTH, HEIGHT, SDL_PIXELFORMAT_RGBA32, data, WIDTH * 4);
    if (!surface)
    {
        SDL_Log("Failed to create surface: %s", SDL_GetError());
        return EXIT_FAILURE;
    }
    SDL_SaveBMP(surface, IMAGE);
    SDL_DestroySurface(surface);
    SDL_UnmapGPUTransferBuffer(device, results);
    SDL_ReleaseGPUTransferBuffer(device, results);
    SDL_ReleaseGPUTexture(device, texture);
    SDL_ReleaseGPUBuffer(device, spheres);
    SDL_ReleaseGPUComputePipeline(device, pipeline);
    SDL_DestroyGPUDevice(device);
    SDL_OpenURL(IMAGE);
    SDL_Quit();
    return EXIT_SUCCESS;
}