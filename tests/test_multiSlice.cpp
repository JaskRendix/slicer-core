#include <gtest/gtest.h>
#include "multiSlice.h"
#include "triangleMesh.h"
#include "triangle.h"
#include "v3.h"

static triangleMesh makePyramid() {
    triangleMesh mesh;

    mesh.push_back(triangle(v3(0,0,1), v3(-1,-1,-1), v3(1,-1,-1), v3(0,0,1)));
    mesh.push_back(triangle(v3(0,0,1), v3(1,-1,-1),  v3(1,1,-1),  v3(0,0,1)));
    mesh.push_back(triangle(v3(0,0,1), v3(1,1,-1),   v3(-1,1,-1), v3(0,0,1)));
    mesh.push_back(triangle(v3(0,0,1), v3(-1,1,-1),  v3(-1,-1,-1),v3(0,0,1)));

    return mesh;
}

TEST(MultiSlice, EmptyMesh) {
    triangleMesh mesh;
    auto layers = sliceMeshMultiLayer(mesh, 0.25);
    EXPECT_TRUE(layers.empty());
}

TEST(MultiSlice, InvalidLayerHeight) {
    auto mesh = makePyramid();

    EXPECT_TRUE(sliceMeshMultiLayer(mesh, 0.0).empty());
    EXPECT_TRUE(sliceMeshMultiLayer(mesh, -1.0).empty());
}

TEST(MultiSlice, PyramidLayerCount) {
    auto mesh = makePyramid();

    auto layers = sliceMeshMultiLayer(mesh, 0.25);

    // Expect 9 layers from Z=-1 to Z=1 inclusive
    EXPECT_EQ(layers.size(), 9u);

    // Z values should be monotonic increasing
    for (size_t i = 1; i < layers.size(); ++i) {
        EXPECT_LT(layers[i-1].z, layers[i].z);
    }
}

TEST(MultiSlice, PyramidPolylineCounts) {
    auto mesh = makePyramid();
    auto layers = sliceMeshMultiLayer(mesh, 0.25);

    for (auto &layer : layers) {
        EXPECT_LE(layer.polylines.size(), 1u);
        if (!layer.polylines.empty())
            EXPECT_GE(layer.polylines[0].points.size(), 3u);
    }
}

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

TEST(MultiSlice, EpsilonLastLayerIncluded) {
    triangleMesh mesh;

    mesh.push_back(triangle(
        v3(0,0,0), v3(1,0,0), v3(0,1,0), v3(0,0,0)
    ));

    mesh.push_back(triangle(
        v3(0,0,1), v3(1,0,1), v3(0,1,1), v3(0,0,1)
    ));

    auto layers = sliceMeshMultiLayer(mesh, 0.3);

    bool hasTop = false;
    for (auto &l : layers)
        if (std::abs(l.z - 1.0) < 1e-9)
            hasTop = true;

    EXPECT_TRUE(hasTop);
}
