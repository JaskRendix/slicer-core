#include <gtest/gtest.h>
#include "multiSlice.h"
#include "triangleMesh.h"
#include "triangle.h"
#include "v3.h"

// ------------------------------------------------------------
// Helpers
// ------------------------------------------------------------

static triangleMesh makePyramid() {
    triangleMesh mesh;

    mesh.push_back(triangle(v3(0,0,1), v3(-1,-1,-1), v3(1,-1,-1), v3(0,0,1)));
    mesh.push_back(triangle(v3(0,0,1), v3(1,-1,-1),  v3(1,1,-1),  v3(0,0,1)));
    mesh.push_back(triangle(v3(0,0,1), v3(1,1,-1),   v3(-1,1,-1), v3(0,0,1)));
    mesh.push_back(triangle(v3(0,0,1), v3(-1,1,-1),  v3(-1,-1,-1),v3(0,0,1)));

    return mesh;
}

// ------------------------------------------------------------
// 1. Empty mesh → no layers
// ------------------------------------------------------------

TEST(MultiSlice, EmptyMesh) {
    triangleMesh mesh;
    auto layers = sliceMeshMultiLayer(mesh, 0.25);
    EXPECT_TRUE(layers.empty());
}

// ------------------------------------------------------------
// 2. Zero or negative layer height → no layers
// ------------------------------------------------------------

TEST(MultiSlice, InvalidLayerHeight) {
    auto mesh = makePyramid();

    EXPECT_TRUE(sliceMeshMultiLayer(mesh, 0.0).empty());
    EXPECT_TRUE(sliceMeshMultiLayer(mesh, -1.0).empty());
}

// ------------------------------------------------------------
// 3. Pyramid sliced at 0.25 → 8 layers (7 valid + 1 apex degenerate)
// ------------------------------------------------------------

TEST(MultiSlice, PyramidLayerCount) {
    auto mesh = makePyramid();

    auto layers = sliceMeshMultiLayer(mesh, 0.25);

    // Expect 8 layers from Z=-1 to Z=1 inclusive
    EXPECT_EQ(layers.size(), 8u);

    // Z values should be monotonic increasing
    for (size_t i = 1; i < layers.size(); ++i) {
        EXPECT_LT(layers[i-1].z, layers[i].z);
    }
}

// ------------------------------------------------------------
// 4. Each non-degenerate layer should have exactly 1 square polyline
// ------------------------------------------------------------

TEST(MultiSlice, PyramidPolylineCounts) {
    auto mesh = makePyramid();
    auto layers = sliceMeshMultiLayer(mesh, 0.25);

    for (auto &layer : layers) {
        // Apex layer produces no valid polyline
        if (std::abs(layer.z - 1.0) < 1e-9) {
            EXPECT_TRUE(layer.polylines.empty());
            continue;
        }

        ASSERT_EQ(layer.polylines.size(), 1u);
        EXPECT_GE(layer.polylines[0].points.size(), 4u);
    }
}

// ------------------------------------------------------------
// 5. Non-uniform Z-range mesh → correct min/max detection
// ------------------------------------------------------------

TEST(MultiSlice, NonUniformZBounds) {
    triangleMesh mesh;

    mesh.push_back(triangle(
        v3(0,0,5), v3(1,0,2), v3(0,1,3), v3(0,0,5)
    ));

    mesh.push_back(triangle(
        v3(0,0,-7), v3(1,0,-6), v3(0,1,-6), v3(0,0,-7)
    ));

    auto layers = sliceMeshMultiLayer(mesh, 1.0);

    // Z should range from -7 to +5 inclusive → 13 layers
    EXPECT_EQ(layers.size(), 13u);

    EXPECT_NEAR(layers.front().z, -7.0, 1e-9);
    EXPECT_NEAR(layers.back().z, 5.0, 1e-9);
}

// ------------------------------------------------------------
// 6. Floating-point epsilon ensures last layer is included
// ------------------------------------------------------------

TEST(MultiSlice, EpsilonLastLayerIncluded) {
    triangleMesh mesh;

    mesh.push_back(triangle(
        v3(0,0,0), v3(1,0,0), v3(0,1,0), v3(0,0,0)
    ));

    mesh.push_back(triangle(
        v3(0,0,1), v3(1,0,1), v3(0,1,1), v3(0,0,1)
    ));

    auto layers = sliceMeshMultiLayer(mesh, 0.3);

    // Without epsilon, 1.0 might be skipped due to floating drift
    bool hasTop = false;
    for (auto &l : layers)
        if (std::abs(l.z - 1.0) < 1e-9)
            hasTop = true;

    EXPECT_TRUE(hasTop);
}
