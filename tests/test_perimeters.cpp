#include <gtest/gtest.h>
#include "perimeters.h"
#include "sliceLayer.h"
#include "islands.h"
#include "v3.h"

static SliceLayer::Polyline makeSquare(double half, double z = 0.0) {
    SliceLayer::Polyline p;
    p.points = {
        v3(-half, -half, z),
        v3( half, -half, z),
        v3( half,  half, z),
        v3(-half,  half, z),
        v3(-half, -half, z)
    };
    return p;
}

static Island makeSquareIsland(double half, double z = 0.0) {
    Island isl;
    isl.outer = makeSquare(half, z);
    return isl;
}

static void bbox(const SliceLayer::Polyline &poly,
                 double &minX, double &maxX, double &minY, double &maxY) {
    minX = minY =  1e300;
    maxX = maxY = -1e300;
    for (const auto &p : poly.points) {
        minX = std::min(minX, p.getX()); maxX = std::max(maxX, p.getX());
        minY = std::min(minY, p.getY()); maxY = std::max(maxY, p.getY());
    }
}

TEST(Perimeters, ThreeShellsSquare) {
    auto shells = generatePerimeters(makeSquareIsland(2.0), 3, 0.5);

    ASSERT_EQ(shells.size(), 3u);

    double minX, maxX, minY, maxY;

    bbox(shells[0][0], minX, maxX, minY, maxY);
    EXPECT_NEAR(minX, -1.5, 1e-4); EXPECT_NEAR(maxX, 1.5, 1e-4);

    bbox(shells[1][0], minX, maxX, minY, maxY);
    EXPECT_NEAR(minX, -1.0, 1e-4); EXPECT_NEAR(maxX, 1.0, 1e-4);

    bbox(shells[2][0], minX, maxX, minY, maxY);
    EXPECT_NEAR(minX, -0.5, 1e-4); EXPECT_NEAR(maxX, 0.5, 1e-4);
}

TEST(Perimeters, CWInputNormalized) {
    Island isl = makeSquareIsland(2.0);
    std::reverse(isl.outer.points.begin(), isl.outer.points.end());

    auto shells = generatePerimeters(isl, 1, 0.5);

    ASSERT_EQ(shells.size(), 1u);

    double minX, maxX, minY, maxY;
    bbox(shells[0][0], minX, maxX, minY, maxY);
    EXPECT_NEAR(minX, -1.5, 1e-4);
    EXPECT_NEAR(maxX,  1.5, 1e-4);
}

TEST(Perimeters, CollapseStopsEarly) {
    auto shells = generatePerimeters(makeSquareIsland(1.0), 5, 0.6);

    ASSERT_EQ(shells.size(), 1u);

    double minX, maxX, minY, maxY;
    bbox(shells[0][0], minX, maxX, minY, maxY);
    EXPECT_NEAR(minX, -0.4, 1e-4);
    EXPECT_NEAR(maxX,  0.4, 1e-4);
}

TEST(Perimeters, ZeroCount) {
    auto shells = generatePerimeters(makeSquareIsland(2.0), 0, 0.5);
    EXPECT_TRUE(shells.empty());
}

TEST(Perimeters, NegativeWidthExpands) {
    auto shells = generatePerimeters(makeSquareIsland(1.0), 1, -0.5);

    ASSERT_EQ(shells.size(), 1u);

    double minX, maxX, minY, maxY;
    bbox(shells[0][0], minX, maxX, minY, maxY);
    EXPECT_NEAR(minX, -1.5, 1e-4);
    EXPECT_NEAR(maxX,  1.5, 1e-4);
}

TEST(Perimeters, ZPreserved) {
    auto shells = generatePerimeters(makeSquareIsland(2.0, 7.0), 1, 0.5);

    ASSERT_EQ(shells.size(), 1u);
    for (const auto &p : shells[0][0].points)
        EXPECT_NEAR(p.getZ(), 7.0, 1e-9);
}

TEST(Perimeters, DegenerateIsland) {
    Island isl;
    isl.outer.points = { v3(0,0,0), v3(0,0,0), v3(0,0,0) };

    auto shells = generatePerimeters(isl, 3, 0.5);
    EXPECT_TRUE(shells.empty());
}
