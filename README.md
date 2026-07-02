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
- A beginning base GUI for the rendering and camera adjustment

What is planned ?
-------------------------------------
- Area light, spotlight (soon)
- Migrate manual scene declaration to load-able file (e.g json)
- Asset imports, potentially with Assimp
- Monte Carlo random walk sampling for the GGX distribution
- Denoising
- Multithreading version
- GPU accelerated (CUDA)
- Eventually transition to Vulkan (?)
- and more.....

I intend this to be eventually a render engine


Building and Running
-------------------
The project uses CMake.

To build:

- On Windows:
  ```shell
    cmake -S . -B build && cmake --build build
    cmake -S . -B build && cmake --build build --config Release # For release binary
    cmake -S . -B build && cmake --build build --config Debug   # For debug binary
  ```

- On Linux:

  ```shell
    cmake -S . -B build && cmake --build build
    cmake -S . -B build/Release -DCMAKE_BUILD_TYPE=Release && cmake --build build/Release
    cmake -S . -B build/Debug -DCMAKE_BUILD_TYPE=Debug && cmake --build build/Debug
  ```


To run:

- The built binary can be found in the **bin/** folder of the project directory

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

  As of this moment, only the last case 8 works, due to all the changes in the ray color calculation that no longer supports non pbr materials from the book


An example render case of the project:
----------------
![Main_render]

[Main_render]: output/MainRender.png


A render of a red sphere, a gold metal sphere, and a rusted iron sphere with white emissive light on top, surrounded by multiple walls / planes with reflective and partially metallic properties at 1000 samples per pixel