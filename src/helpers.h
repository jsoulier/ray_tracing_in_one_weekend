#pragma once

#include <SDL3/SDL.h>
#include <assert.h>
#include <math.h>
#include <stdlib.h>

#undef assert
#undef max
#undef min

#define EPSILON 0.000001f
#define PI 3.14159265359f
#define max(a, b) ((a) > (b) ? (a) : (b))
#define min(a, b) ((a) < (b) ? (a) : (b))
#define clamp(x, a, b) min(b, max(a, x))
#define deg(rad) ((rad) * 180.0f / PI)
#define rad(deg) ((deg) * PI / 180.0f)
#define abs(x) ((x) > 0 ? (x) : -(x))
#define lerp(a, b, t) ((a) + ((b) - (a)) * (t))

#ifndef NDEBUG
#define assert(e) SDL_assert(e)
#else
#define assert(e)
#endif

SDL_GPUShader* load_shader(
    SDL_GPUDevice* device,
    const char* file);
SDL_GPUComputePipeline* load_compute_pipeline(
    SDL_GPUDevice* device,
    const char* file);