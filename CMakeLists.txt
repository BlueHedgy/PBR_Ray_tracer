cmake_minimum_required(VERSION 3.21)

# This is your project statement. You should always list languages;
# Listing the version is nice here since it sets lots of useful variables
project(
  raytrace
  VERSION 1.0 LANGUAGES CXX)
  
set(CMAKE_BUILD_TYPE "Debug")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (CMAKE_<LANG>_COMPILER_ID STREQUAL "GNU")
  set(CMAKE_CXX_FLAGS "-Wall -Wno-changes-meaning")
endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
include_directories(
  material
  external
)

set(SOURCES
  main.cpp
)
add_executable(raytrace ${SOURCES})
