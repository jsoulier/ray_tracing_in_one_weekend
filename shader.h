#pragma once

#include <SDL3/SDL.h>

SDL_GPUShader* load_shader(SDL_GPUDevice* device, const char* path);
SDL_GPUComputePipeline* load_compute_pipeline(SDL_GPUDevice* device, const char* path);