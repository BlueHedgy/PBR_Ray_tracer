Physically based ray tracer
==================

Introduction
------------------
This project is an expansion upon [Peter Shirley](https://www.petershirley.com/)'s [Ray tracing in one weekend](https://raytracing.github.io/) book series.

The code originally went up to the [Lights chapter](https://raytracing.github.io/books/RayTracingTheNextWeek.html#lights) of the second book. The renderer is then further extended with PBR material, which also works with the emmissive materials. The PBR rendering code implements Cook Torrance BRDF with GGX microfacet model for reflectance.

What exactly has been implemented ?
-----------------------------------
- Path tracing algorithm with Cook Torrance microfacet theory and GGX distribution model for reflectance / scattering
- Dynamic scene structure that allows loading and unloading cameras, lights and objects
- Point light, directional light
- Custom texture image usage for: albedo, roughness, metalness, normal
- GUI for the rendering and camera adjustment
- Multi-threaded rendering
- Headless render

What is planned ?
-------------------------------------
- Area light, spotlight (soon)
- Migrate manual scene declaration to load-able file (e.g json)
- Asset imports, potentially with Assimp
- Monte Carlo random walk sampling for the GGX distribution
- Denoising
- ~~Multithreading version~~
- GPU accelerated (CUDA)
- Eventually transition to Vulkan (?)
- and more.....

I intend this to be eventually a render engine


Building and Running
-------------------
The project uses CMake.

### To build
Below are some example ways to build the project:

- On Windows:
  - Using MSVC:
  ```shell
    cmake -S . -B build && cmake --build build
    cmake -S . -B build && cmake --build build --config Release # For release binary
    cmake -S . -B build && cmake --build build --config Debug   # For debug binary
  ```

  - Using Clang with MSVC variant:

  ```shell
    cmake -B build/windows -G Ninja `
      -DCMAKE_C_COMPILER="clang-cl" `
      -DCMAKE_CXX_COMPILER="clang-cl" `
      -DCMAKE_BUILD_TYPE="RelWithDebInfo" `
    && cmake --build build/windows
  ```

- On Linux:
  - Using GCC:
  ```shell
    cmake -S . -B build && cmake --build build
    cmake -S . -B build/Release -DCMAKE_BUILD_TYPE=Release && cmake --build build/Release
    cmake -S . -B build/Debug -DCMAKE_BUILD_TYPE=Debug && cmake --build build/Debug
  ```

  - Using Clang with GCC variant:
  ```shell
    cmake -B build/linux -G Ninja `
      -DCMAKE_C_COMPILER="clang" `
      -DCMAKE_CXX_COMPILER="clang++" `
      -DCMAKE_BUILD_TYPE="RelWithDebInfo" `
    && cmake --build build/linux
  ```

To run:


- On Windows:
  ```shell
    .\bin\Release\raytrace.exe <render_case> <output_file_path> # Or
    .\bin\Debug\raytrace.exe <render_case> <output_file_path>

    # Example:
    .\bin\Release\raytrace.exe 8 output\output5.ppm
  ```

- On Linux:
  ```shell
    ./bin/Release/raytrace <render_case> <output_file_path> # Or
    ./bin/Debug/raytrace <render_case> <output_file_path>

    # Example:
    ./bin/Release/raytrace 8 output/output5.ppm
  ```

- The built binary can be found in the **bin/** folder of the project directory

### ***As of this moment, only the last preset case 8 works, due to all the changes in the ray color calculation that no longer supports non pbr materials from the book***


An example render case of the project:
----------------

A render of a red sphere, a gold metal sphere, and a rusted iron sphere with white emissive light on top, surrounded by multiple walls / planes with reflective and partially metallic properties. Note that the video is sped up 2x.

![Main_render](output/showcase.gif)