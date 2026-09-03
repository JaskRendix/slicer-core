#include <gtest/gtest.h>
#include "triangle.h"
#include "Plane.h"
#include "lineSegment.h"
#include "v3.h"

TEST(Triangle, NoIntersection) {
    triangle t(
        v3(0,0,5),
        v3(1,0,5),
        v3(0,1,5),
        v3(0,0,5)
    );

    Plane pl(v3(0,0,1), 0.0);
    auto res = t.intersectPlane(pl);
    EXPECT_FALSE(res.has_value());
}

TEST(Triangle, BasicIntersection) {
    triangle t(
        v3(0,0,1),
        v3(0,0,-1),
        v3(1,0,1),
        v3(0,1,1)
    );

    Plane pl(v3(0,0,1), 0.0);
    auto res = t.intersectPlane(pl);
    EXPECT_TRUE(res.has_value());
}

TEST(Triangle, CoplanarTriangleNoSegment) {
    triangle t(
        v3(0,0,0),
        v3(1,0,0),
        v3(0,1,0),
        v3(0,0,0)
    );

    Plane pl(v3(0,0,1), 0.0);
    auto res = t.intersectPlane(pl);
    EXPECT_FALSE(res.has_value());
}

TEST(Triangle, DegenerateTriangleNoSegment) {
    triangle t(
        v3(1,1,1),
        v3(1,1,1),
        v3(1,1,1),
        v3(1,1,1)
    );

    Plane pl(v3(0,0,1), 0.0);
    auto res = t.intersectPlane(pl);
    EXPECT_FALSE(res.has_value());
}

TEST(Triangle, VertexOnPlaneProducesSegment) {
    triangle t(
        v3(0,0,0),   // on plane
        v3(1,0,1),   // above
        v3(0,1,-1),  // below
        v3(0,0,0)
    );

    Plane pl(v3(0,0,1), 0.0);
    auto res = t.intersectPlane(pl);
    EXPECT_TRUE(res.has_value());
}

TEST(Triangle, TwoVerticesOnPlaneProducesSegment) {
    triangle t(
        v3(0,0,0),   // on plane
        v3(1,0,0),   // on plane
        v3(0,1,1),   // above
        v3(0,0,0)
    );

    Plane pl(v3(0,0,1), 0.0);
    auto res = t.intersectPlane(pl);

    ASSERT_TRUE(res.has_value());
    const auto &seg = *res;

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
    auto res = t.intersectPlane(pl);

    // Only one unique intersection → no segment
    EXPECT_FALSE(res.has_value());
}

TEST(Triangle, CoplanarEdgeButTriangleIntersects) {
    triangle t(
        v3(0,0,0),   // on plane
        v3(1,0,0),   // on plane (coplanar edge)
        v3(0,1,-1),  // below
        v3(0,0,0)
    );

    Plane pl(v3(0,0,1), 0.0);
    auto res = t.intersectPlane(pl);

    EXPECT_TRUE(res.has_value());
}

TEST(Triangle, TwoProperCrossings) {
    triangle t(
        v3(0,0,-1),  // below
        v3(2,0,1),   // above
        v3(0,2,-1),  // below
        v3(0,0,-1)
    );

    Plane pl(v3(0,0,1), 0.0);
    auto res = t.intersectPlane(pl);
    EXPECT_TRUE(res.has_value());
}

TEST(Triangle, OneAboveTwoBelow) {
    triangle t(
        v3(0,0,1),   // above
        v3(1,0,-1),  // below
        v3(0,1,-1),  // below
        v3(0,0,1)
    );

    Plane pl(v3(0,0,1), 0.0);
    auto res = t.intersectPlane(pl);
    EXPECT_TRUE(res.has_value());
}

TEST(Triangle, OneBelowTwoAbove) {
    triangle t(
        v3(0,0,-1),  // below
        v3(1,0,1),   // above
        v3(0,1,1),   // above
        v3(0,0,-1)
    );

    Plane pl(v3(0,0,1), 0.0);
    auto res = t.intersectPlane(pl);
    EXPECT_TRUE(res.has_value());
}

TEST(Triangle, TinyTriangleIntersection) {
    triangle t(
        v3(0,0,-1e-12),
        v3(1e-12,0,1e-12),
        v3(0,1e-12,1e-12),
        v3(0,0,-1e-12)
    );

    Plane pl(v3(0,0,1), 0.0);
    auto res = t.intersectPlane(pl, 1e-15);
    EXPECT_TRUE(res.has_value());
}

TEST(Triangle, HugeTriangleIntersection) {
    triangle t(
        v3(0,0,-1e9),
        v3(1e9,0,1e9),
        v3(0,1e9,1e9),
        v3(0,0,-1e9)
    );

    Plane pl(v3(0,0,1), 0.0);
    auto res = t.intersectPlane(pl);
    EXPECT_TRUE(res.has_value());
}
