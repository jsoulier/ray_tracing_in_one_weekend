#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#define THREADS 256
#define LAMBERTIAN 0
#define METAL 1
#define DIAELECTRIC 2
#define SPHERES (4 + 22 * 22)
#define SAMPLES 500

typedef struct Config
{
    float source_x;
    float source_y;
    float source_z;
    float fov;
    float target_x;
    float target_y;
    float target_z;
    float focus;
    float up_x;
    float up_y;
    float up_z;
    float oof;
    float sky_r;
    float sky_g;
    float sky_b;
    Uint32 width;
    float horizon_r;
    float horizon_g;
    float horizon_b;
    Uint32 height;
    Uint32 sample;
    Uint32 depth;
    Uint32 num_spheres;
    Uint32 padding[1];
} Config;

typedef struct Sphere
{
    float x;
    float y;
    float z;
    float radius;
    float r;
    float g;
    float b;
    float fuzz;
    float refraction;
    Uint32 type;
    Uint32 padding[2];
} Sphere;

static SDL_GPUComputePipeline* CreateComputePipeline(SDL_GPUDevice* device)
{
    SDL_GPUShaderFormat formats = SDL_GetGPUShaderFormats(device);
    const char* path;
    const char* entrypoint;
    SDL_GPUShaderFormat format;
    if (formats & SDL_GPU_SHADERFORMAT_SPIRV)
    {
        path = "shader.comp.spv";
        entrypoint = "main";
        format = SDL_GPU_SHADERFORMAT_SPIRV;
    }
    else if (formats & SDL_GPU_SHADERFORMAT_DXIL)
    {
        path = "shader.comp.dxil";
        entrypoint = "main";
        format = SDL_GPU_SHADERFORMAT_DXIL;
    }
    else if (formats & SDL_GPU_SHADERFORMAT_MSL)
    {
        path = "shader.comp.msl";
        entrypoint = "main0";
        format = SDL_GPU_SHADERFORMAT_MSL;
    }
    else
    {
        SDL_Log("No supported shader format");
        return NULL;
    }
    size_t size;
    void* data = SDL_LoadFile(path, &size);
    if (!data)
    {
        SDL_Log("Failed to load shader: %s", SDL_GetError());
        return NULL;
    }
    SDL_GPUComputePipelineCreateInfo cpci = {0};
    cpci.code = data;
    cpci.code_size = size;
    cpci.entrypoint = entrypoint;
    cpci.format = format;
    cpci.num_readonly_storage_buffers = 1;
    cpci.num_readwrite_storage_textures = 1;
    cpci.num_uniform_buffers = 1;
    cpci.threadcount_x = THREADS;
    cpci.threadcount_y = 1;
    cpci.threadcount_z = 1;
    SDL_GPUComputePipeline* pipeline = SDL_CreateGPUComputePipeline(device, &cpci);
    SDL_free(data);
    if (!pipeline)
    {
        SDL_Log("Failed to create compute pipeline: %s", SDL_GetError());
        return NULL;
    }
    return pipeline;
}

static void Download(SDL_GPUDevice* device, SDL_GPUTexture* texture, Uint32 width, Uint32 height, const char* path)
{
    SDL_GPUTextureCreateInfo tci = {0};
    tci.usage = SDL_GPU_TEXTUREUSAGE_COLOR_TARGET;
    tci.type = SDL_GPU_TEXTURETYPE_2D;
    tci.format = SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM;
    tci.width = width;
    tci.height = height;
    tci.num_levels = 1;
    tci.layer_count_or_depth = 1;
    SDL_GPUTransferBufferCreateInfo tbci = {0};
    tbci.usage = SDL_GPU_TRANSFERBUFFERUSAGE_DOWNLOAD;
    tbci.size = width * height * 4;
    SDL_GPUTexture* target = SDL_CreateGPUTexture(device, &tci);
    SDL_GPUTransferBuffer* tbo = SDL_CreateGPUTransferBuffer(device, &tbci);
    if (!target || !tbo)
    {
        SDL_Log("Failed to create texture or transfer buffer: %s", SDL_GetError());
        return;
    }
    SDL_GPUCommandBuffer* command_buffer = SDL_AcquireGPUCommandBuffer(device);
    if (!command_buffer)
    {
        SDL_Log("Failed to acquire command buffer: %s", SDL_GetError());
        return;
    }
    SDL_GPUBlitInfo blit = {0};
    blit.source.texture = texture;
    blit.source.w = width;
    blit.source.h = height;
    blit.destination.texture = target;
    blit.destination.w = width;
    blit.destination.h = height;
    SDL_BlitGPUTexture(command_buffer, &blit);
    SDL_GPUCopyPass* copy_pass = SDL_BeginGPUCopyPass(command_buffer);
    if (!copy_pass)
    {
        SDL_Log("Failed to begin copy pass: %s", SDL_GetError());
        return;
    }
    SDL_GPUTextureRegion src = {0};
    SDL_GPUTextureTransferInfo dst = {0};
    src.texture = target;
    src.w = width;
    src.h = height;
    src.d = 1;
    dst.transfer_buffer = tbo;
    SDL_DownloadFromGPUTexture(copy_pass, &src, &dst);
    SDL_EndGPUCopyPass(copy_pass);
    SDL_GPUFence* fence = SDL_SubmitGPUCommandBufferAndAcquireFence(command_buffer);
    if (!fence)
    {
        SDL_Log("Failed to acquire fence: %s", SDL_GetError());
        return;
    }
    SDL_WaitForGPUFences(device, true, &fence, 1);
    SDL_ReleaseGPUFence(device, fence);
    void* data = SDL_MapGPUTransferBuffer(device, tbo, false);
    if (!data)
    {
        SDL_Log("Failed to map transfer buffer: %s", SDL_GetError());
        return;
    }
    SDL_Surface* surface = SDL_CreateSurfaceFrom(width, height, SDL_PIXELFORMAT_RGBA32, data, width * 4);
    if (!surface)
    {
        SDL_Log("Failed to create surface: %s", SDL_GetError());
        return;
    }
    if (!SDL_SavePNG(surface, path))
    {
        SDL_Log("Failed to save image: %s", SDL_GetError());
    }
    SDL_DestroySurface(surface);
    SDL_UnmapGPUTransferBuffer(device, tbo);
    SDL_ReleaseGPUTransferBuffer(device, tbo);
    SDL_ReleaseGPUTexture(device, target);
}

static SDL_GPUBuffer* CreateSpheres(SDL_GPUDevice* device)
{
    Sphere spheres[SPHERES] =
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
    return sbo;
}

int main(int argc, char** argv)
{
    Config config =
    {
        .source_x = 13.0f,
        .source_y = 2.0f,
        .source_z = 3.0f,
        .fov = 20.0f,
        .target_x = 0.0f,
        .target_y = 0.0f,
        .target_z = -1.0f,
        .focus = 10.0f,
        .up_x = 0.0f,
        .up_y = 1.0f,
        .up_z = 0.0f,
        .oof = 0.6f,
        .sky_r = 0.5f,
        .sky_g = 0.7f,
        .sky_b = 1.0f,
        .width = 960,
        .horizon_r = 1.0f,
        .horizon_g = 1.0f,
        .horizon_b = 1.0f,
        .height = 540,
        .depth = 20,
        .num_spheres = SPHERES,
    };
#ifndef NDEBUG
    SDL_SetLogPriorities(SDL_LOG_PRIORITY_VERBOSE);
#endif
    SDL_SetAppMetadata("RTIOW", NULL, NULL);
    if (!SDL_Init(SDL_INIT_VIDEO))
    {
        SDL_Log("Failed to initialize SDL: %s", SDL_GetError());
        return 1;
    }
    SDL_Window* window = SDL_CreateWindow("", config.width, config.height, 0);
    if (!window)
    {
        SDL_Log("Failed to create window: %s", SDL_GetError());
        return 1;
    }
#ifndef NDEBUG
    SDL_GPUDevice* device = SDL_CreateGPUDevice(SDL_GPU_SHADERFORMAT_DXIL | SDL_GPU_SHADERFORMAT_SPIRV | SDL_GPU_SHADERFORMAT_MSL, true, NULL);
#else
    SDL_GPUDevice* device = SDL_CreateGPUDevice(SDL_GPU_SHADERFORMAT_DXIL | SDL_GPU_SHADERFORMAT_SPIRV | SDL_GPU_SHADERFORMAT_MSL, false, NULL);
#endif
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
    SDL_GPUComputePipeline* pipeline = CreateComputePipeline(device);
    if (!pipeline)
    {
        SDL_Log("Failed to create compute pipeline");
        return 1;
    }
    SDL_GPUTextureCreateInfo tci = {0};
    tci.usage = SDL_GPU_TEXTUREUSAGE_COMPUTE_STORAGE_WRITE | SDL_GPU_TEXTUREUSAGE_SAMPLER;
    tci.type = SDL_GPU_TEXTURETYPE_2D;
    tci.format = SDL_GPU_TEXTUREFORMAT_R32G32B32A32_FLOAT;
    tci.width = config.width;
    tci.height = config.height;
    tci.num_levels = 1;
    tci.layer_count_or_depth = 1;
    SDL_GPUTexture* texture1 = SDL_CreateGPUTexture(device, &tci);
    if (!texture1)
    {
        SDL_Log("Failed to create texture: %s", SDL_GetError());
        return 1;
    }
    SDL_GPUBuffer* spheres = CreateSpheres(device);
    if (!spheres)
    {
        SDL_Log("Failed to create spheres");
        return 1;
    }
    SDL_GPUFence* fence = NULL;
    bool running = true;
    config.sample = 0;
    while (running)
    {
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            switch (event.type)
            {
            case SDL_EVENT_QUIT:
                running = false;
                break;
            }
        }
        if (!running)
        {
            break;
        }
        if (fence)
        {
            if (!SDL_QueryGPUFence(device, fence))
            {
                SDL_Delay(1);
                continue;
            }
            SDL_ReleaseGPUFence(device, fence);
            fence = NULL;
            config.sample++;
            if (config.sample >= SAMPLES)
            {
                break;
            }
        }
        char title[128] = {0};
        SDL_snprintf(title, sizeof(title), "%s: %u/%u samples", SDL_GetGPUDeviceDriver(device), config.sample, SAMPLES);
        SDL_SetWindowTitle(window, title);
        SDL_GPUCommandBuffer* command_buffer = SDL_AcquireGPUCommandBuffer(device);
        if (!command_buffer)
        {
            SDL_Log("Failed to acquire command buffer: %s", SDL_GetError());
            return 1;
        }
        SDL_PushGPUComputeUniformData(command_buffer, 0, &config, sizeof(config));
        SDL_GPUStorageTextureReadWriteBinding stb = {0};
        stb.texture = texture1;
        SDL_GPUComputePass* compute_pass = SDL_BeginGPUComputePass(command_buffer, &stb, 1, NULL, 0);
        if (!compute_pass)
        {
            SDL_Log("Failed to begin compute pass: %s", SDL_GetError());
            return 1;
        }
        SDL_BindGPUComputePipeline(compute_pass, pipeline);
        SDL_BindGPUComputeStorageBuffers(compute_pass, 0, &spheres, 1);
        SDL_DispatchGPUCompute(compute_pass, (config.width + THREADS - 1) / THREADS, config.height, 1);
        SDL_EndGPUComputePass(compute_pass);
        SDL_GPUTexture* swapchain;
        Uint32 width;
        Uint32 height;
        if (!SDL_WaitAndAcquireGPUSwapchainTexture(command_buffer, window, &swapchain, &width, &height))
        {
            SDL_Log("Failed to acquire swapchain texture: %s", SDL_GetError());
            SDL_CancelGPUCommandBuffer(command_buffer);
            continue;
        }
        if (!swapchain)
        {
            SDL_CancelGPUCommandBuffer(command_buffer);
            continue;
        }
        SDL_GPUBlitInfo blit = {0};
        blit.source.texture = texture1;
        blit.source.w = config.width;
        blit.source.h = config.height;
        blit.destination.texture = swapchain;
        blit.destination.w = width;
        blit.destination.h = height;
        SDL_BlitGPUTexture(command_buffer, &blit);
        fence = SDL_SubmitGPUCommandBufferAndAcquireFence(command_buffer);
        if (!fence)
        {
            SDL_Log("Failed to acquire fence: %s", SDL_GetError());
            return 1;
        }
    }
    SDL_HideWindow(window);
    SDL_ReleaseGPUFence(device, fence);
    Download(device, texture1, config.width, config.height, "image.png");
    SDL_ReleaseGPUTexture(device, texture1);
    SDL_ReleaseGPUBuffer(device, spheres);
    SDL_ReleaseGPUComputePipeline(device, pipeline);
    SDL_ReleaseWindowFromGPUDevice(device, window);
    SDL_DestroyGPUDevice(device);
    SDL_DestroyWindow(window);
    SDL_OpenURL("image.png");
    SDL_Quit();
    return 0;
}
