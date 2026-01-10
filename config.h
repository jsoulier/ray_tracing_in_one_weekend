#ifndef CONFIG_H
#define CONFIG_H

#define THREADS 256
#define WIDTH 960.0f
#define HEIGHT 540.0f
#define SAMPLES 500
#define BATCHES 50
#define DELAY 200
#define DEPTH 20
#define FOV 20
#define SOURCE float3(13.0f, 2.0f, 3.0f)
#define TARGET float3(0.0f, 0.0f, -1.0f)
#define UP float3(0.0f, 1.0f, 0.0f)
#define FOCUS 10.0f
#define OOF 0.6f
#define IMAGE "image.png"
#define SKY float3(0.5f, 0.7f, 1.0f)
#define HORIZON float3(1.0f, 1.0f, 1.0f)
#define LAMBERTIAN 0
#define METAL 1
#define DIAELECTRIC 2

#endif