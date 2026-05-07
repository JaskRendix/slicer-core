include(FetchContent)

FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)

set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

add_executable(test_slicing test_slicing.cpp)
target_link_libraries(test_slicing PRIVATE mesh_slicing gtest gtest_main)

include(GoogleTest)
gtest_discover_tests(test_slicing)
