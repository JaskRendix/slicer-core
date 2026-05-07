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
