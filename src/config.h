#ifndef CONFIG_H
#define CONFIG_H

#define THREADS 256
#define WIDTH 400.0f
#define HEIGHT 225.0f
#define SAMPLES 100
#define DEPTH 50
#define SOURCE vec3(-2.0f, 2.0f, 1.0f)
#define TARGET vec3(0.0f, 0.0f, -1.0f)
#define UP vec3(0.0f, 1.0f, 0.0f)
#define FOV 90
#define IMAGE "image.bmp"

#define LAMBERTIAN 0
#define METAL 1
#define DIAELECTRIC 2

#endif