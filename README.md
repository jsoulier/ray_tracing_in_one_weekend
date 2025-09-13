# Ray Tracing In One Weekend

![](image.bmp)

Implementation of [Ray Tracing in One Weekend](https://raytracing.github.io/) using the new SDL3 GPU API with compute shaders

### Building

#### Windows

```bash
git clone https://github.com/jsoulier/ray_tracing_in_one_weekend --recurse-submodules
cd ray_tracing_in_one_weekend
mkdir build
cd build
cmake ..
cmake --build . --parallel 8 --config Release
cd bin
./rtiow.exe
```

#### Linux

```bash
git clone https://github.com/jsoulier/ray_tracing_in_one_weekend --recurse-submodules
cd ray_tracing_in_one_weekend
mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . --parallel 8
cd bin
./rtiow
```