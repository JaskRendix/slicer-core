# mesh-slicing

A C++20 library that slices 3D triangle meshes (STL) into 2D layers, handles contour offsetting, perimeters, and generates infill patterns.

**Features**

* Multi-threaded triangle-plane intersection (`std::thread`)
* Line-based ASCII and binary STL loaders
* Adaptive epsilon tolerance (scales with mesh bounding box)
* Winding detection and normalization (CCW outer loops, CW holes)
* Island building (outer contours + inner holes)
* Clipper2-based offsetting with multi-fragment handling
* Perimeters (concurrent outer/hole shelling)
* Infill patterns: line, grid, hex
* Debug export to SVG and JSON

**Dependencies**

* C++20 compiler
* CMake 3.16+
* Clipper2 (fetched automatically via CMake)

**Build**

```bash
mkdir build
cd build
cmake ..
make -j
```

**Testing**

```bash
cd build
ctest --output-on-failure
```

**Project Structure**

```text
include/        - Public headers (triangleMesh.h, sliceLayer.h, offset.h, infill.h, etc.)
src/            - Implementation files
examples/       - Usage examples (example.cpp)
tests/          - Unit test suite (GoogleTest / CTest)
```

**Core Implementation Notes**

* **Intersection:** Slices triangles at a target Z height, filtering fully coplanar triangles and handling on-plane vertices. Duplicates are collapsed using an adaptive epsilon (`max_dim * 1e-7`).
* **Threading:** `sliceAtZ` splits mesh triangles into chunks processed across hardware concurrency threads.
* **Path Handling:** Loaders accept standard `std::filesystem::path` inputs.

---

**Purpose**
Demonstrates geometric reasoning, slicing algorithms, contour reconstruction, offsetting, infill generation, modular C++ design, and test-driven development. Acts as a reference implementation for building slicing engines or toolpath planners.
