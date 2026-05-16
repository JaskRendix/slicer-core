cmake_minimum_required(VERSION 3.16)
project(mesh_slicing LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
FetchContent_Declare(clipper2
    GIT_REPOSITORY https://github.com/AngusJohnson/Clipper2.git
    GIT_TAG main)
set(CLIPPER2_TESTS    OFF CACHE BOOL "" FORCE)
set(CLIPPER2_EXAMPLES OFF CACHE BOOL "" FORCE)
set(CLIPPER2_UTILS    OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(clipper2)
add_subdirectory(${clipper2_SOURCE_DIR}/CPP ${clipper2_BINARY_DIR}/CPP)

include_directories(include)

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
    src/debug_export.cpp
)

target_link_libraries(mesh_slicing Clipper2)
target_include_directories(mesh_slicing PRIVATE
    ${clipper2_SOURCE_DIR}/CPP/Clipper2Lib/include)

add_executable(example examples/example.cpp)
target_link_libraries(example mesh_slicing)

enable_testing()
add_subdirectory(tests)