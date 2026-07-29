#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include "config.h"
#include "shader.h"

typedef struct Sphere
{
    float x;
    float y;
    float z;
    float radius;
    float r;
    float g;
    float b;
    Uint32 padding1;
    Uint32 type;
    float fuzz;
    float refraction;
    Uint32 padding2;
} Sphere;

static SDL_GPUBuffer* CreateSpheres(SDL_GPUDevice* device, Uint32* size)
{
    Sphere spheres[4 + 484] =
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
    SDL_srand(0);
    for (int a = -11; a < 11; a++)
    for (int b = -11; b < 11; b++)
    {
        const int i = 4 + (a + 11) * 22 + b + 11;
        const float material = SDL_randf();
        spheres[i].x = a + 0.9f * SDL_randf();
        spheres[i].y = 0.2f;
        spheres[i].z = b + 0.9f * SDL_randf();
        spheres[i].radius = 0.2f;
        if (material < 0.8f)
        {
            spheres[i].type = LAMBERTIAN;
            spheres[i].r = SDL_randf();
            spheres[i].g = SDL_randf();
            spheres[i].b = SDL_randf();
            spheres[i].fuzz = 0.0f;
            spheres[i].refraction = 0.0f;
        }
        else if (material < 0.95f)
        {
            spheres[i].type = METAL;
            spheres[i].r = SDL_randf();
            spheres[i].g = SDL_randf();
            spheres[i].b = SDL_randf();
            spheres[i].fuzz = SDL_randf() * 0.5f;
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
    SDL_memcpy(data, spheres, sizeof(spheres));
    SDL_UnmapGPUTransferBuffer(device, tbo);
    SDL_GPUCommandBuffer* command_buffer = SDL_AcquireGPUCommandBuffer(device);
    if (!command_buffer)
    {
        SDL_Log("Failed to acquire command buffer: %s", SDL_GetError());
        return NULL;
    }
    SDL_GPUCopyPass* copy_pass = SDL_BeginGPUCopyPass(command_buffer);
    if (!copy_pass)
    {
        SDL_Log("Failed to begin copy pass: %s", SDL_GetError());
        return NULL;
    }
    SDL_GPUTransferBufferLocation src = {0};
    SDL_GPUBufferRegion dst = {0};
    src.transfer_buffer = tbo;
    dst.buffer = sbo;
    dst.size = sizeof(spheres);
    SDL_UploadToGPUBuffer(copy_pass, &src, &dst, false);
    SDL_EndGPUCopyPass(copy_pass);
    SDL_SubmitGPUCommandBuffer(command_buffer);
    SDL_ReleaseGPUTransferBuffer(device, tbo);
    *size = sizeof(spheres) / sizeof(spheres[0]);
    return sbo;
}

int main(int argc, char** argv)
{
#ifndef NDEBUG
    bool debug = true;
#else
    bool debug = false;
#endif
    if (debug)
    {
        SDL_SetLogPriorities(SDL_LOG_PRIORITY_VERBOSE);
    }
    SDL_SetAppMetadata("RTIOW", NULL, NULL);
    if (!SDL_Init(SDL_INIT_VIDEO))
    {
        SDL_Log("Failed to initialize SDL: %s", SDL_GetError());
        return 1;
    }
    SDL_Window* window = SDL_CreateWindow("", WIDTH, HEIGHT, 0);
    if (!window)
    {
        SDL_Log("Failed to create window: %s", SDL_GetError());
        return 1;
    }
    SDL_GPUDevice* device = SDL_CreateGPUDevice(SDL_GPU_SHADERFORMAT_DXIL | SDL_GPU_SHADERFORMAT_SPIRV | SDL_GPU_SHADERFORMAT_MSL, debug, NULL);
    if (!device)
    {
        SDL_Log("Failed to create device: %s", SDL_GetError());
        return 1;
    }
    if (!SDL_ClaimWindowForGPUDevice(device, window))
    {
        SDL_Log("Failed to create swapchain: %s", SDL_GetError());
        return 1;
    }
    SDL_RaiseWindow(window);
    SDL_GPUComputePipeline* pipeline = Shader_Load(device, "shader.comp");
    if (!pipeline)
    {
        SDL_Log("Failed to load compute pipeline");
        return 1;
    }
    SDL_GPUTextureCreateInfo tci = {0};
    tci.usage = SDL_GPU_TEXTUREUSAGE_COMPUTE_STORAGE_WRITE | SDL_GPU_TEXTUREUSAGE_SAMPLER;
    tci.type = SDL_GPU_TEXTURETYPE_2D;
    tci.format = SDL_GPU_TEXTUREFORMAT_R32G32B32A32_FLOAT;
    tci.width = WIDTH;
    tci.height = HEIGHT;
    tci.num_levels = 1;
    tci.layer_count_or_depth = 1;
    SDL_GPUTexture* texture1 = SDL_CreateGPUTexture(device, &tci);
    if (!texture1)
    {
        SDL_Log("Failed to create texture: %s", SDL_GetError());
        return 1;
    }
    SDL_GPUTexture* texture2 = SDL_CreateGPUTexture(device, &tci);
    if (!texture2)
    {
        SDL_Log("Failed to create texture: %s", SDL_GetError());
        return 1;
    }
    tci.usage = SDL_GPU_TEXTUREUSAGE_COLOR_TARGET;
    tci.format = SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM;
    SDL_GPUTexture* texture3 = SDL_CreateGPUTexture(device, &tci);
    if (!texture3)
    {
        SDL_Log("Failed to create texture: %s", SDL_GetError());
        return 1;
    }
    Uint32 num_spheres;
    SDL_GPUBuffer* spheres = CreateSpheres(device, &num_spheres);
    if (!spheres)
    {
        SDL_Log("Failed to create spheres");
        return 1;
    }
    for (Uint32 batch = 0; batch < BATCHES; batch++)
    {
        char title[128] = {0};
        int samples = batch * SAMPLES / BATCHES;
        SDL_snprintf(title, sizeof(title), "%s: %d/%d samples", SDL_GetGPUDeviceDriver(device), samples, SAMPLES);
        SDL_SetWindowTitle(window, title);
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            switch (event.type)
            {
            case SDL_EVENT_QUIT:
                batch = BATCHES;
                break;
            }
        }
        SDL_GPUCommandBuffer* command_buffer = SDL_AcquireGPUCommandBuffer(device);
        if (!command_buffer)
        {
            SDL_Log("Failed to acquire command buffer: %s", SDL_GetError());
            return 1;
        }
        SDL_GPUStorageTextureReadWriteBinding stb[2] = {0};
        stb[0].texture = texture1;
        stb[1].texture = texture2;
        SDL_GPUComputePass* compute_pass = SDL_BeginGPUComputePass(command_buffer, stb, 2, NULL, 0);
        if (!compute_pass)
        {
            SDL_Log("Failed to begin compute pass: %s", SDL_GetError());
            return 1;
        }
        SDL_BindGPUComputePipeline(compute_pass, pipeline);
        SDL_BindGPUComputeStorageBuffers(compute_pass, 0, &spheres, 1);
        SDL_PushGPUComputeUniformData(command_buffer, 0, &num_spheres, sizeof(num_spheres));
        SDL_PushGPUComputeUniformData(command_buffer, 1, &batch, sizeof(batch));
        SDL_DispatchGPUCompute(compute_pass, (WIDTH + THREADS - 1) / THREADS, HEIGHT, 1);
        SDL_EndGPUComputePass(compute_pass);
        SDL_GPUTexture* swapchain;
        Uint32 width;
        Uint32 height;
        SDL_WaitForGPUSwapchain(device, window);
        if (!SDL_AcquireGPUSwapchainTexture(command_buffer, window, &swapchain, &width, &height))
        {
            SDL_Log("Failed to acquire swapchain texture: %s", SDL_GetError());
            continue;
        }
        if (!swapchain)
        {
            continue;
        }
        SDL_GPUBlitInfo blit = {0};
        blit.source.texture = texture2;
        blit.source.w = WIDTH;
        blit.source.h = HEIGHT;
        blit.destination.texture = swapchain;
        blit.destination.w = width;
        blit.destination.h = height;
        SDL_BlitGPUTexture(command_buffer, &blit);
        SDL_SubmitGPUCommandBuffer(command_buffer);
        SDL_Delay(DELAY);
    }
    SDL_GPUTransferBufferCreateInfo tbci = {0};
    tbci.usage = SDL_GPU_TRANSFERBUFFERUSAGE_DOWNLOAD;
    tbci.size = WIDTH * HEIGHT * 4;
    SDL_GPUTransferBuffer* tbo = SDL_CreateGPUTransferBuffer(device, &tbci);
    if (!tbo)
    {
        SDL_Log("Failed to create transfer buffer: %s", SDL_GetError());
        return 1;
    }
    SDL_GPUCommandBuffer* command_buffer = SDL_AcquireGPUCommandBuffer(device);
    if (!command_buffer)
    {
        SDL_Log("Failed to acquire command buffer: %s", SDL_GetError());
        return 1;
    }
    SDL_GPUBlitInfo blit = {0};
    blit.source.texture = texture1;
    blit.source.w = WIDTH;
    blit.source.h = HEIGHT;
    blit.destination.texture = texture3;
    blit.destination.w = WIDTH;
    blit.destination.h = HEIGHT;
    SDL_BlitGPUTexture(command_buffer, &blit);
    SDL_GPUCopyPass* copy_pass = SDL_BeginGPUCopyPass(command_buffer);
    if (!copy_pass)
    {
        SDL_Log("Failed to begin copy pass: %s", SDL_GetError());
        return 1;
    }
    SDL_GPUTextureRegion src = {0};
    SDL_GPUTextureTransferInfo dst = {0};
    src.texture = texture3;
    src.w = WIDTH;
    src.h = HEIGHT;
    src.d = 1;
    dst.transfer_buffer = tbo;
    SDL_DownloadFromGPUTexture(copy_pass, &src, &dst);
    SDL_EndGPUCopyPass(copy_pass);
    SDL_GPUFence* fence = SDL_SubmitGPUCommandBufferAndAcquireFence(command_buffer);
    if (!fence)
    {
        SDL_Log("Failed to acquire fence: %s", SDL_GetError());
        return 1;
    }
    SDL_WaitForGPUFences(device, true, &fence, 1);
    SDL_ReleaseGPUFence(device, fence);
    void* data = SDL_MapGPUTransferBuffer(device, tbo, false);
    if (!data)
    {
        SDL_Log("Failed to map transfer buffer: %s", SDL_GetError());
        return 1;
    }
    SDL_Surface* surface = SDL_CreateSurfaceFrom(WIDTH, HEIGHT, SDL_PIXELFORMAT_RGBA32, data, WIDTH * 4);
    if (!surface)
    {
        SDL_Log("Failed to create surface: %s", SDL_GetError());
        return 1;
    }
    SDL_SavePNG(surface, IMAGE);
    SDL_DestroySurface(surface);
    SDL_UnmapGPUTransferBuffer(device, tbo);
    SDL_ReleaseGPUTransferBuffer(device, tbo);
    SDL_ReleaseGPUTexture(device, texture1);
    SDL_ReleaseGPUTexture(device, texture2);
    SDL_ReleaseGPUTexture(device, texture3);
    SDL_ReleaseGPUBuffer(device, spheres);
    SDL_ReleaseGPUComputePipeline(device, pipeline);
    SDL_ReleaseWindowFromGPUDevice(device, window);
    SDL_DestroyGPUDevice(device);
    SDL_DestroyWindow(window);
    SDL_OpenURL(IMAGE);
    SDL_Quit();
    return 0;
}
