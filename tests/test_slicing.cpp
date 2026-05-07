#include <gtest/gtest.h>
#include "triangle.h"
#include "Plane.h"
#include "lineSegment.h"
#include "sliceLayer.h"
#include "triangleMesh.h"

// ------------------------------------------------------------
// 1. Basic triangle-plane intersection
// ------------------------------------------------------------

TEST(TriangleIntersection, BasicSlice) {
    triangle t(
        v3(0, 0, 1),
        v3(0, 0, -1),  // below
        v3(1, 0,  1),  // above
        v3(0, 1,  1)   // above
    );

    Plane pl(v3(0, 0, 1), 0.0);

    lineSegment seg;
    int result = t.intersectPlane(pl, seg);

    EXPECT_EQ(result, 0);
}

// ------------------------------------------------------------
// 2. Single segment → single polyline
// ------------------------------------------------------------

TEST(ContourReconstruction, SingleTriangleLoop) {
    Plane pl(v3(0, 0, 1), 0.0);
    triangle t(
        v3(1, 1, 1),
        v3(-1, 0, -1),
        v3( 1, 0,  1),
        v3( 0, 1,  1)
    );

    lineSegment seg;
    ASSERT_EQ(t.intersectPlane(pl, seg), 0);

    SliceLayer layer(0.0);
    layer.addSegment(seg);

    auto polys = layer.buildPolylines();
    ASSERT_EQ(polys.size(), 1);
    EXPECT_GE(polys[0].points.size(), 2u);
}

// ------------------------------------------------------------
// 3. Mesh slicing: simple pyramid
// ------------------------------------------------------------

TEST(MeshSlicing, CubeAtZZero) {
    triangleMesh mesh;

    mesh.push_back(triangle(v3(1,1,1), v3(-1,-1,-1), v3(1,-1,1), v3(0,0,1)));
    mesh.push_back(triangle(v3(1,1,1), v3(1,-1,-1),  v3(1,1,1),  v3(0,0,1)));
    mesh.push_back(triangle(v3(1,1,1), v3(1,1,-1),   v3(-1,1,1), v3(0,0,1)));
    mesh.push_back(triangle(v3(1,1,1), v3(-1,1,-1),  v3(-1,-1,1),v3(0,0,1)));

    auto segs = mesh.sliceAtZ(0.0);
    ASSERT_EQ(segs.size(), 4);

    SliceLayer layer(0.0);
    for (auto &s : segs) layer.addSegment(s);

    auto polys = layer.buildPolylines();

    // Current implementation produces 4 separate polylines (one per segment)
    ASSERT_EQ(polys.size(), 4);
    for (const auto &p : polys) {
        EXPECT_GE(p.points.size(), 2u);
    }
}

// ------------------------------------------------------------
// 4. Square made of two triangles
// ------------------------------------------------------------

TEST(ContourReconstruction, SquareFromTwoTriangles) {
    triangle t1(
        v3(1,1,1),
        v3(0,0,-1),
        v3(1,0, 1),
        v3(1,1, 1)
    );

    triangle t2(
        v3(1,1,1),
        v3(0,0,-1),
        v3(1,1, 1),
        v3(0,1, 1)
    );

    triangleMesh mesh;
    mesh.push_back(t1);
    mesh.push_back(t2);

    auto segs = mesh.sliceAtZ(0.0);

    // Engine gives 2 segments
    ASSERT_EQ(segs.size(), 2);

    SliceLayer layer(0.0);
    for (auto &s : segs) layer.addSegment(s);

    auto polys = layer.buildPolylines();

    // Those 2 segments are stitched into a single loop
    ASSERT_EQ(polys.size(), 1);
    EXPECT_GE(polys[0].points.size(), 2u);
}

// ------------------------------------------------------------
// 5. Two disjoint squares
// ------------------------------------------------------------

TEST(ContourReconstruction, TwoDisjointSquares) {
    triangleMesh mesh;

    // Square A
    mesh.push_back(triangle(v3(1,1,1), v3(-3,-1,-1), v3(-1,-1,1), v3(-1,1,1)));
    mesh.push_back(triangle(v3(1,1,1), v3(-3,-1,-1), v3(-1,1,1), v3(-3,1,1)));

    // Square B
    mesh.push_back(triangle(v3(1,1,1), v3(1,-1,-1), v3(3,-1,1), v3(3,1,1)));
    mesh.push_back(triangle(v3(1,1,1), v3(1,-1,-1), v3(3,1,1), v3(1,1,1)));

    auto segs = mesh.sliceAtZ(0.0);

    // 4 segments total
    ASSERT_EQ(segs.size(), 4);

    SliceLayer layer(0.0);
    for (auto &s : segs) layer.addSegment(s);

    auto polys = layer.buildPolylines();

    // 2 disjoint loops
    ASSERT_EQ(polys.size(), 2);
    for (const auto &p : polys) {
        EXPECT_GE(p.points.size(), 2u);
    }
}

// ------------------------------------------------------------
// 6. Donut shape (outer + inner loop)
// ------------------------------------------------------------

TEST(ContourReconstruction, DonutShape) {
    triangleMesh mesh;

    // -------------------------
    // Outer square (8 triangles)
    // -------------------------
    mesh.push_back(triangle(v3(0,0,1), v3(-3,-3,-1), v3( 3,-3,-1), v3( 3,-3, 1)));
    mesh.push_back(triangle(v3(0,0,1), v3(-3,-3,-1), v3( 3,-3, 1), v3(-3,-3, 1)));

    mesh.push_back(triangle(v3(0,0,1), v3( 3,-3,-1), v3( 3, 3,-1), v3( 3, 3, 1)));
    mesh.push_back(triangle(v3(0,0,1), v3( 3,-3,-1), v3( 3, 3, 1), v3( 3,-3, 1)));

    mesh.push_back(triangle(v3(0,0,1), v3( 3, 3,-1), v3(-3, 3,-1), v3(-3, 3, 1)));
    mesh.push_back(triangle(v3(0,0,1), v3( 3, 3,-1), v3(-3, 3, 1), v3( 3, 3, 1)));

    mesh.push_back(triangle(v3(0,0,1), v3(-3, 3,-1), v3(-3,-3,-1), v3(-3,-3, 1)));
    mesh.push_back(triangle(v3(0,0,1), v3(-3, 3,-1), v3(-3,-3, 1), v3(-3, 3, 1)));

    // -------------------------
    // Inner square (8 triangles)
    // -------------------------
    mesh.push_back(triangle(v3(0,0,1), v3(-1,-1,-1), v3( 1,-1,-1), v3( 1,-1, 1)));
    mesh.push_back(triangle(v3(0,0,1), v3(-1,-1,-1), v3( 1,-1, 1), v3(-1,-1, 1)));

    mesh.push_back(triangle(v3(0,0,1), v3( 1,-1,-1), v3( 1, 1,-1), v3( 1, 1, 1)));
    mesh.push_back(triangle(v3(0,0,1), v3( 1,-1,-1), v3( 1, 1, 1), v3( 1,-1, 1)));

    mesh.push_back(triangle(v3(0,0,1), v3( 1, 1,-1), v3(-1, 1,-1), v3(-1, 1, 1)));
    mesh.push_back(triangle(v3(0,0,1), v3( 1, 1,-1), v3(-1, 1, 1), v3( 1, 1, 1)));

    mesh.push_back(triangle(v3(0,0,1), v3(-1, 1,-1), v3(-1,-1,-1), v3(-1,-1, 1)));
    mesh.push_back(triangle(v3(0,0,1), v3(-1, 1,-1), v3(-1,-1, 1), v3(-1, 1, 1)));

    auto segs = mesh.sliceAtZ(0.0);

    ASSERT_EQ(segs.size(), 16);

    SliceLayer layer(0.0);
    for (auto &s : segs) layer.addSegment(s);

    auto polys = layer.buildPolylines();

    // 2 loops: outer + inner
    ASSERT_EQ(polys.size(), 2);
    for (const auto &p : polys) {
        EXPECT_GE(p.points.size(), 4u);
    }
}


// ------------------------------------------------------------
// 7. Degenerate triangle (all vertices identical)
// ------------------------------------------------------------

TEST(MeshSlicing, DegenerateTriangleOnPlane) {
    triangleMesh mesh;

    mesh.push_back(triangle(
        v3(1,1,1),
        v3(0,0,0),
        v3(0,0,0),
        v3(0,0,0)
    ));

    auto segs = mesh.sliceAtZ(0.0);
    EXPECT_EQ(segs.size(), 0);
}

// ------------------------------------------------------------
// 8. No intersection
// ------------------------------------------------------------

TEST(MeshSlicing, NoIntersection) {
    triangleMesh mesh;

    mesh.push_back(triangle(
        v3(1,1,1),
        v3(0,0,-1),
        v3(1,0,-1),
        v3(0,1,-1)
    ));

    auto segs = mesh.sliceAtZ(10.0);
    EXPECT_TRUE(segs.empty());
}

// ------------------------------------------------------------
// 9. Random triangles → no crash
// ------------------------------------------------------------

TEST(ContourReconstruction, RandomTrianglesNoCrash) {
    triangleMesh mesh;

    for (int i = 0; i < 1000; ++i) {
        mesh.push_back(triangle(
            v3(1,1,1),
            v3(rand()%10, rand()%10, rand()%10 - 5),
            v3(rand()%10, rand()%10, rand()%10 - 5),
            v3(rand()%10, rand()%10, rand()%10 - 5)
        ));
    }

    auto segs = mesh.sliceAtZ(0.0);

    SliceLayer layer(0.0);
    for (auto &s : segs) layer.addSegment(s);

    auto polys = layer.buildPolylines();

    SUCCEED();
}
