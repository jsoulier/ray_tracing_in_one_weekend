# SDL3 Ray Tracing In One Weekend

Implementation of [Ray Tracing in One Weekend](https://raytracing.github.io/) using the new SDL3 GPU API with compute shaders

![](image.bmp)
*200 samples, 20 bounces, and a 960x540 image taking around 30 seconds on integrated AMD Ryzen 7 4700U Graphics*

### Building

#### Windows

Install the [Vulkan SDK](https://www.lunarg.com/vulkan-sdk/) for glslc

```bash
git clone https://github.com/jsoulier/sdl3_rtiow --recurse-submodules
cd sdl3_rtiow
mkdir build
cd build
cmake ..
cmake --build . --parallel 8 --config Release
cd bin
./sdl3_rtiow.exe
```

#### Linux

```bash
sudo apt install glslc
```

```bash
git clone https://github.com/jsoulier/sdl3_rtiow --recurse-submodules
cd sdl3_rtiow
mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . --parallel 8
cd bin
./sdl3_rtiow
```

### Bugs

Drivers may throw `VK_ERROR_DEVICE_LOST`.
To fix, you can try either of the following:
1. Lower the number of samples and bounces (see [config.h](src/config.h))
2. (Windows only) Set `TdrDelay` in the Windows Registry (see [here](https://learn.microsoft.com/en-us/windows-hardware/drivers/display/tdr-registry-keys))