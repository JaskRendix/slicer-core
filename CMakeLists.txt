cmake_minimum_required(VERSION 3.16)
project(mesh_slicing LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Include headers
include_directories(include)

# Library sources (header-only classes are NOT listed here)
add_library(mesh_slicing
    src/triangle.cpp
    src/triangleMesh.cpp
    src/sliceLayer.cpp
    src/multiSlice.cpp
    src/offset.cpp
    src/islands.cpp
    src/infill.cpp
    src/winding_normalize.cpp
    src/perimeters.cpp
    src/infill_grid.cpp
)

# Example executable
add_executable(example examples/example.cpp)
target_link_libraries(example mesh_slicing)

# Tests directory
enable_testing()
add_subdirectory(tests)
