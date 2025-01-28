#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include "helpers.h"

static SDL_Window* window;
static SDL_GPUDevice* device;
static SDL_GPUComputePipeline* ray_pipeline;

int main(
    int argc,
    char** argv)
{
    if (!SDL_Init(SDL_INIT_VIDEO))
    {
        SDL_Log("Failed to initialize SDL: %s", SDL_GetError());
        return EXIT_FAILURE;
    }
    device = SDL_CreateGPUDevice(SDL_GPU_SHADERFORMAT_SPIRV, true, NULL);
    if (!device)
    {
        SDL_Log("Failed to create device: %s", SDL_GetError());
        return EXIT_FAILURE;
    }
    ray_pipeline = load_compute_pipeline(device, "ray.comp");
    if (!ray_pipeline)
    {
        SDL_Log("Failed to load ray pipeline");
        return EXIT_FAILURE;
    }
    SDL_ReleaseGPUComputePipeline(device, ray_pipeline);
    SDL_DestroyGPUDevice(device);
    SDL_Quit();
    return EXIT_SUCCESS;
}