include(FetchContent)

FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)

set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# Collect all test files
file(GLOB TEST_SOURCES
    test_helpers.h
    test_infill.cpp
    test_islands.cpp
    test_multiSlice.cpp
    test_offset.cpp
    test_perimeters.cpp
    test_sliceLayer.cpp
    test_triangle.cpp
    test_triangleMesh.cpp
    test_winding_normalize.cpp
    test_winding.cpp
)

add_executable(mesh_slicing_tests ${TEST_SOURCES})

target_link_libraries(mesh_slicing_tests
    PRIVATE
        mesh_slicing
        gtest
        gtest_main
)

include(GoogleTest)
gtest_discover_tests(mesh_slicing_tests)
