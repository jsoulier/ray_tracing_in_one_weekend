#pragma once

#include <SDL3/SDL.h>

SDL_GPUComputePipeline* Shader_Load(SDL_GPUDevice* device, const char* path);