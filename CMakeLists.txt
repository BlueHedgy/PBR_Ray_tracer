cmake_minimum_required(VERSION 3.21)

# This is your project statement. You should always list languages;
# Listing the version is nice here since it sets lots of useful variables
project(
  raytrace
  VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  add_compile_options(-fcolor-diagnostics -fansi-escape-codes)

  if (MSVC)
    add_compile_options(
      /W4 /permissive-
      /clang:-Wno-unused-parameter
      /clang:-Wno-sign-compare
      /clang:-Wno-unused-variable
    )
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin/windows)

  elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    add_compile_options(
    -Wall
    -Wno-changes-meaning
    -Wno-unused-parameter
    -Wno-unused-variable
    )
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin/linux)

  endif()

elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  add_compile_options(-Wall -Wno-changes-meaning)
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin/linux)

elseif (MSVC)
  add_compile_options(/W4 /permissive-)
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin/windows)

endif()



# set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")

set(SOURCES
main.cpp
miscs/stb_init.cpp
)

set(IMGUI_SOURCE
  external/imgui/imgui.cpp
  external/imgui/imgui_demo.cpp
  external/imgui/imgui_draw.cpp
  external/imgui/imgui_tables.cpp
  external/imgui/imgui_widgets.cpp
  external/imgui/misc/cpp/imgui_stdlib.cpp
)

set(IMGUI_BACKENDS
  external/imgui/backends/imgui_impl_sdl3.cpp
  external/imgui/backends/imgui_impl_opengl3.cpp
)


add_executable(raytrace ${SOURCES} ${IMGUI_SOURCE} ${IMGUI_BACKENDS})

add_subdirectory( external/SDL EXCLUDE_FROM_ALL )

target_include_directories(raytrace PRIVATE
  ${CMAKE_SOURCE_DIR}
  ${CMAKE_SOURCE_DIR}/material
  ${CMAKE_SOURCE_DIR}/external
  ${CMAKE_SOURCE_DIR}/external/stb
  ${CMAKE_SOURCE_DIR}/external/SDL/include
  ${CMAKE_SOURCE_DIR}/external/imgui
  ${CMAKE_SOURCE_DIR}/external/imgui/backends
  ${CMAKE_SOURCE_DIR}/external/imgui/misc/cpp/
  ${CMAKE_SOURCE_DIR}/noises
  ${CMAKE_SOURCE_DIR}/primitives
  ${CMAKE_SOURCE_DIR}/gui
)

find_package(OpenGL REQUIRED)
target_link_libraries(raytrace PRIVATE SDL3::SDL3 OpenGL::GL)