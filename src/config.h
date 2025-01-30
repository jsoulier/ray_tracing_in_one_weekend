#ifndef CONFIG_H
#define CONFIG_H

#define THREADS 256
#define WIDTH 800.0f
#define HEIGHT 450.0f
#define SAMPLES 20
#define DEPTH 30
#define SOURCE vec3(13.0f, 2.0f, 3.0f)
#define TARGET vec3(0.0f, 0.0f, 0.0f)
#define UP vec3(0.0f, 1.0f, 0.0f)
#define FOV 20
#define DEFOCUS_ANGLE 0.6f
#define FOCUS_LENGTH 10.0f
#define IMAGE "image.bmp"

#define LAMBERTIAN 0
#define METAL 1
#define DIAELECTRIC 2

#endif