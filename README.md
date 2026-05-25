Physically based ray tracer
==================

Introduction
------------------
This project is an expansion upon [Peter Shirley](https://www.petershirley.com/)'s [Ray tracing in one weekend](https://raytracing.github.io/) book series.

The code goes up to the [Lights chapter](https://raytracing.github.io/books/RayTracingTheNextWeek.html#lights) of the second book. The renderer is then further equiped with PBR material, which also works with the emmissive materials. The PBR rendering code implements Cook Torrance BRDF model with both Blinn-Phong and GGX methods.

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
    .\bin\Release\raytrace.exe 9 output\output5.ppm
  ```

- On Linux:
  ```shell
    ./bin/Release/raytrace <render_case> <output_file_path> # Or
    ./bin/Debug/raytrace <render_case> <output_file_path>

    # Example:
    ./bin/Release/raytrace 9 output/output5.ppm
  ```


An example render case of the project:
----------------
![Main_render]

[Main_render]: output/MainRender.png
