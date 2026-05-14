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

TEST(Perimeters, HoleIgnoredForOuterShells) {
    // Outer square 2x2
    Island isl;
    isl.outer = makeSquare(2.0);

    // Inner square hole 1x1 (CW)
    SliceLayer::Polyline hole = makeSquare(1.0);
    std::reverse(hole.points.begin(), hole.points.end());
    isl.holes.push_back(hole);

    auto shells = generatePerimeters(isl, 2, 0.5);

    ASSERT_EQ(shells.size(), 2u);

    double minX, maxX, minY, maxY;

    // First shell
    bbox(shells[0][0], minX, maxX, minY, maxY);
    EXPECT_NEAR(minX, -1.5, 1e-4);
    EXPECT_NEAR(maxX,  1.5, 1e-4);

    // Hole must remain unchanged
    double hminX, hmaxX, hminY, hmaxY;
    bbox(isl.holes[0], hminX, hmaxX, hminY, hmaxY);
    EXPECT_NEAR(hminX, -1.0, 1e-4);
    EXPECT_NEAR(hmaxX,  1.0, 1e-4);
}

TEST(Perimeters, MultiIslandIndependentShells) {
    Island isl1 = makeSquareIsland(2.0);
    Island isl2 = makeSquareIsland(1.0);

    auto shells1 = generatePerimeters(isl1, 1, 0.5);
    auto shells2 = generatePerimeters(isl2, 1, 0.5);

    ASSERT_EQ(shells1.size(), 1u);
    ASSERT_EQ(shells2.size(), 1u);

    double minX, maxX, minY, maxY;

    // Island 1 shrinks from 2.0 → 1.5
    bbox(shells1[0][0], minX, maxX, minY, maxY);
    EXPECT_NEAR(minX, -1.5, 1e-4);
    EXPECT_NEAR(maxX,  1.5, 1e-4);

    // Island 2 shrinks from 1.0 → 0.5
    bbox(shells2[0][0], minX, maxX, minY, maxY);
    EXPECT_NEAR(minX, -0.5, 1e-4);
    EXPECT_NEAR(maxX,  0.5, 1e-4);
}

TEST(Perimeters, RotatedSquareShell) {
    Island isl;

    // 45° rotated square (diamond)
    isl.outer.points = {
        v3( 1, 0, 0),
        v3( 0, 1, 0),
        v3(-1, 0, 0),
        v3( 0,-1, 0),
        v3( 1, 0, 0)
    };

    auto shells = generatePerimeters(isl, 1, 0.2);
    ASSERT_EQ(shells.size(), 1u);

    double minX, maxX, minY, maxY;
    bbox(shells[0][0], minX, maxX, minY, maxY);

    // Expected shrink: 1 - 0.2*sqrt(2)
    double expected = 1.0 - 0.2 * std::sqrt(2.0);

    EXPECT_NEAR(maxX,  expected, 3e-3);
    EXPECT_NEAR(minX, -expected, 3e-3);
}

TEST(Perimeters, ConcavePolygonShell) {
    Island isl;

    isl.outer.points = {
        v3(0,0,0), v3(3,0,0), v3(3,1,0),
        v3(1,1,0), v3(1,3,0), v3(0,3,0),
        v3(0,0,0)
    };

    auto shells = generatePerimeters(isl, 1, 0.2);
    ASSERT_EQ(shells.size(), 1u);

    double minX, maxX, minY, maxY;
    bbox(shells[0][0], minX, maxX, minY, maxY);

    EXPECT_NEAR(minX, 0.2, 1e-3);
    EXPECT_NEAR(maxX, 2.8, 1e-3);
    EXPECT_NEAR(minY, 0.2, 1e-3);
    EXPECT_NEAR(maxY, 2.8, 1e-3);
}
