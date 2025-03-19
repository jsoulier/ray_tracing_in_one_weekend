#ifndef CONFIG_H
#define CONFIG_H

#define THREADS 256
#define WIDTH 960.0f
#define HEIGHT 540.0f
#define SAMPLES 50
#define BATCHES 10
#define DEPTH 20
#define FOV 20
#define SOURCE vec3(13.0f, 2.0f, 3.0f)
#define TARGET vec3(0.0f, 0.0f, -1.0f)
#define UP vec3(0.0f, 1.0f, 0.0f)
#define FOCUS 10.0f
#define OOF 0.6f
#define IMAGE "image.bmp"
#define SKY vec3(0.5f, 0.7f, 1.0f)
#define HORIZON vec3(1.0f, 1.0f, 1.0f)

#define LAMBERTIAN 0
#define METAL 1
#define DIAELECTRIC 2

#endif