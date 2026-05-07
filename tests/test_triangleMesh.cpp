#include <gtest/gtest.h>
#include "triangleMesh.h"
#include "triangle.h"
#include "lineSegment.h"
#include "v3.h"
#include <fstream>

static triangle makeTri(double z0, double z1, double z2) {
    return triangle(
        v3(0,0,z0),
        v3(1,0,z1),
        v3(0,1,z2),
        v3(0,0,z0)
    );
}

TEST(TriangleMesh, EmptyMeshSlice) {
    triangleMesh mesh;
    auto segs = mesh.sliceAtZ(0.0);
    EXPECT_TRUE(segs.empty());
}

TEST(TriangleMesh, PushBackStoresTriangles) {
    triangleMesh mesh;
    mesh.push_back(makeTri(0,1,2));
    mesh.push_back(makeTri(3,4,5));

    EXPECT_EQ(mesh.size(), 2u);
}

TEST(TriangleMesh, SliceAtZBasic) {
    triangleMesh mesh;

    // One triangle crossing z=0 → 1 segment
    mesh.push_back(makeTri(-1, 1, 1));

    auto segs = mesh.sliceAtZ(0.0);
    EXPECT_EQ(segs.size(), 1u);
}

TEST(TriangleMesh, SliceAtZNoIntersection) {
    triangleMesh mesh;

    mesh.push_back(makeTri(5,5,5));   // all above
    mesh.push_back(makeTri(-5,-5,-5)); // all below

    auto segs = mesh.sliceAtZ(0.0);
    EXPECT_TRUE(segs.empty());
}

TEST(TriangleMesh, DegenerateTriangle) {
    triangleMesh mesh;

    mesh.push_back(triangle(
        v3(1,1,1),
        v3(1,1,1),
        v3(1,1,1),
        v3(1,1,1)
    ));

    auto segs = mesh.sliceAtZ(0.0);
    EXPECT_TRUE(segs.empty());
}

TEST(TriangleMesh, AABBSize) {
    triangleMesh mesh;

    mesh.push_back(triangle(
        v3(-2,-3,-4),
        v3( 5, 7, 9),
        v3( 1, 2, 3),
        v3(-2,-3,-4)
    ));

    v3 size = mesh.meshAABBSize();

    EXPECT_NEAR(size.getX(), 7.0, 1e-9);
    EXPECT_NEAR(size.getY(), 10.0, 1e-9);
    EXPECT_NEAR(size.getZ(), 13.0, 1e-9);
}



TEST(TriangleMesh, NormalizeCentersMesh) {
    triangleMesh mesh;

    mesh.push_back(triangle(
        v3(10,10,10),
        v3(20,10,10),
        v3(10,20,10),
        v3(10,10,10)
    ));

    mesh.normalize();

    // After normalization, AABB should be symmetric around origin
    v3 size = mesh.meshAABBSize();

    EXPECT_NEAR(size.getX(), 10.0, 1e-9);
    EXPECT_NEAR(size.getY(), 10.0, 1e-9);
    EXPECT_NEAR(size.getZ(), 0.0, 1e-9);
}

TEST(TriangleMesh, LoadAsciiSTL) {
    const char* path = "test_ascii.stl";

    std::ofstream out(path);
    out << "solid test\n";
    out << "facet normal 0 0 1\n";
    out << "outer loop\n";
    out << "vertex 0 0 0\n";
    out << "vertex 1 0 0\n";
    out << "vertex 0 1 0\n";
    out << "endloop\n";
    out << "endfacet\n";
    out << "endsolid\n";
    out.close();

    triangleMesh mesh(path, false);

    EXPECT_EQ(mesh.size(), 1u);

    std::remove(path);
}

TEST(TriangleMesh, LoadBinarySTL) {
    const char* path = "test_bin.stl";

    std::ofstream out(path, std::ios::binary);

    char header[80] = {};
    out.write(header, 80);

    uint32_t nFaces = 1;
    out.write(reinterpret_cast<char*>(&nFaces), sizeof(uint32_t));

    float data[12] = {
        0,0,1,   // normal
        0,0,0,   // v0
        1,0,0,   // v1
        0,1,0    // v2
    };
    out.write(reinterpret_cast<char*>(data), sizeof(float)*12);

    uint16_t attr = 0;
    out.write(reinterpret_cast<char*>(&attr), sizeof(uint16_t));

    out.close();

    triangleMesh mesh(path, true);

    EXPECT_EQ(mesh.size(), 1u);

    std::remove(path);
}
