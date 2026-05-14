#include <gtest/gtest.h>
#include "triangle.h"
#include "Plane.h"
#include "lineSegment.h"
#include "v3.h"

// 1. No intersection
TEST(Triangle, NoIntersection) {
    triangle t(
        v3(0,0,5),
        v3(1,0,5),
        v3(0,1,5),
        v3(0,0,5)
    );

    Plane pl(v3(0,0,1), 0.0);
    lineSegment seg;
    EXPECT_EQ(t.intersectPlane(pl, seg), 1);
}

// 2. Single intersection segment
TEST(Triangle, BasicIntersection) {
    triangle t(
        v3(0,0,1),
        v3(0,0,-1),
        v3(1,0,1),
        v3(0,1,1)
    );

    Plane pl(v3(0,0,1), 0.0);
    lineSegment seg;
    EXPECT_EQ(t.intersectPlane(pl, seg), 0);
}

// 3. Coplanar triangle → no segment
TEST(Triangle, Coplanar) {
    triangle t(
        v3(0,0,0),
        v3(1,0,0),
        v3(0,1,0),
        v3(0,0,0)
    );

    Plane pl(v3(0,0,1), 0.0);
    lineSegment seg;
    EXPECT_EQ(t.intersectPlane(pl, seg), 1);
}

// 4. Degenerate triangle
TEST(Triangle, Degenerate) {
    triangle t(
        v3(1,1,1),
        v3(1,1,1),
        v3(1,1,1),
        v3(1,1,1)
    );

    Plane pl(v3(0,0,1), 0.0);
    lineSegment seg;
    EXPECT_EQ(t.intersectPlane(pl, seg), 1);
}

TEST(Triangle, VertexOnPlaneProducesSegment) {
    triangle t(
        v3(0,0,0),   // on plane
        v3(1,0,1),   // above
        v3(0,1,-1),  // below
        v3(0,0,0)
    );

    Plane pl(v3(0,0,1), 0.0);
    lineSegment seg;
    EXPECT_EQ(t.intersectPlane(pl, seg), 0);
}

TEST(Triangle, TwoVerticesOnPlaneProducesSegment) {
    triangle t(
        v3(0,0,0),   // on plane
        v3(1,0,0),   // on plane
        v3(0,1,1),   // above plane
        v3(0,0,0)
    );

    Plane pl(v3(0,0,1), 0.0);
    lineSegment seg;

    EXPECT_EQ(t.intersectPlane(pl, seg), 0);

    EXPECT_NEAR(seg.start().getZ(), 0.0, 1e-9);
    EXPECT_NEAR(seg.end().getZ(),   0.0, 1e-9);
}

TEST(Triangle, DuplicateIntersectionsCollapsed) {
    triangle t(
        v3(0,0,0),   // on plane
        v3(1,0,-1),  // below
        v3(0,1,-1),  // below
        v3(0,0,0)
    );

    Plane pl(v3(0,0,1), 0.0);
    lineSegment seg;
    EXPECT_EQ(t.intersectPlane(pl, seg), 1); // only one unique point
}
