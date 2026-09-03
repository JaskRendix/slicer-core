#include <gtest/gtest.h>
#include "offset.h"
#include "sliceLayer.h"
#include "v3.h"
#include <algorithm>
#include <cmath>

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

static SliceLayer::Polyline reversed(const SliceLayer::Polyline &poly) {
    SliceLayer::Polyline out = poly;
    std::reverse(out.points.begin(), out.points.end());
    return out;
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

TEST(Offset, ExpandSquareCCW) {
    auto outList = offset::offsetPolyline(makeSquare(1.0), 0.1);
    ASSERT_GE(outList.size(), 1u);
    const auto &out = outList[0];

    ASSERT_GE(out.points.size(), 4u);

    double minX, maxX, minY, maxY;
    bbox(out, minX, maxX, minY, maxY);

    EXPECT_NEAR(minX, -1.1, 1e-4);
    EXPECT_NEAR(maxX,  1.1, 1e-4);
    EXPECT_NEAR(minY, -1.1, 1e-4);
    EXPECT_NEAR(maxY,  1.1, 1e-4);
}

TEST(Offset, ExpandSquareCW) {
    auto outList = offset::offsetPolyline(reversed(makeSquare(1.0)), 0.1);
    ASSERT_GE(outList.size(), 1u);
    const auto &out = outList[0];

    ASSERT_GE(out.points.size(), 4u);

    double minX, maxX, minY, maxY;
    bbox(out, minX, maxX, minY, maxY);

    EXPECT_NEAR(minX, -1.1, 1e-4);
    EXPECT_NEAR(maxX,  1.1, 1e-4);
    EXPECT_NEAR(minY, -1.1, 1e-4);
    EXPECT_NEAR(maxY,  1.1, 1e-4);
}

TEST(Offset, ShrinkSquare) {
    auto outList = offset::offsetPolyline(makeSquare(1.0), -0.2);
    ASSERT_GE(outList.size(), 1u);
    const auto &out = outList[0];

    ASSERT_GE(out.points.size(), 4u);

    double minX, maxX, minY, maxY;
    bbox(out, minX, maxX, minY, maxY);

    EXPECT_NEAR(minX, -0.8, 1e-4);
    EXPECT_NEAR(maxX,  0.8, 1e-4);
    EXPECT_NEAR(minY, -0.8, 1e-4);
    EXPECT_NEAR(maxY,  0.8, 1e-4);
}

TEST(Offset, ConcavePolygon) {
    SliceLayer::Polyline poly;
    poly.points = {
        v3(0,0,0), v3(2,0,0), v3(2,1,0),
        v3(1,1,0), v3(1,2,0), v3(0,2,0),
        v3(0,0,0)
    };

    auto outList = offset::offsetPolyline(poly, 0.1);
    ASSERT_GE(outList.size(), 1u);
    const auto &out = outList[0];

    ASSERT_GE(out.points.size(), 4u);

    double minX, maxX, minY, maxY;
    bbox(out, minX, maxX, minY, maxY);

    EXPECT_NEAR(minX, -0.1, 1e-4);
    EXPECT_NEAR(maxX,  2.1, 1e-4);
    EXPECT_NEAR(minY, -0.1, 1e-4);
    EXPECT_NEAR(maxY,  2.1, 1e-4);
}

TEST(Offset, CollapseToNothing) {
    auto out = offset::offsetPolyline(makeSquare(1.0), -2.0);
    EXPECT_TRUE(out.empty());
}

TEST(Offset, MultiPolyline) {
    SliceLayer::Polyline sq1;
    sq1.points = { v3(-2,-1,0), v3(-1,-1,0), v3(-1,0,0), v3(-2,0,0), v3(-2,-1,0) };

    SliceLayer::Polyline sq2;
    sq2.points = { v3(1,1,0), v3(2,1,0), v3(2,2,0), v3(1,2,0), v3(1,1,0) };

    std::vector<SliceLayer::Polyline> polys = { sq1, sq2 };
    auto out = offset::offsetLayerPolylines(polys, 0.1);

    ASSERT_EQ(out.size(), 2u);

    bool foundSq1 = false;
    bool foundSq2 = false;

    for (const auto &poly : out) {
        double minX, maxX, minY, maxY;
        bbox(poly, minX, maxX, minY, maxY);
        if (minX < 0.0) {
            EXPECT_NEAR(minX, -2.1, 1e-4);
            EXPECT_NEAR(maxX, -0.9, 1e-4);
            foundSq1 = true;
        } else {
            EXPECT_NEAR(minX,  0.9, 1e-4);
            EXPECT_NEAR(maxX,  2.1, 1e-4);
            foundSq2 = true;
        }
    }

    EXPECT_TRUE(foundSq1);
    EXPECT_TRUE(foundSq2);
}

TEST(Offset, DegenerateInput) {
    SliceLayer::Polyline p;
    p.points = { v3(0,0,0), v3(0,0,0), v3(0,0,0) };
    EXPECT_NO_THROW(offset::offsetPolyline(p, 0.1));
}

TEST(Offset, RotatedSquare) {
    SliceLayer::Polyline poly;
    double z = 0.0;

    poly.points = {
        v3( 1, 0, z),
        v3( 0, 1, z),
        v3(-1, 0, z),
        v3( 0,-1, z),
        v3( 1, 0, z)
    };

    auto outList = offset::offsetPolyline(poly, 0.2);
    ASSERT_GE(outList.size(), 1u);
    const auto &out = outList[0];

    double minX, maxX, minY, maxY;
    bbox(out, minX, maxX, minY, maxY);

    double expected = 1.0 + 0.2 * std::sqrt(2.0);

    EXPECT_NEAR(maxX,  expected, 3e-3);
    EXPECT_NEAR(minX, -expected, 3e-3);
    EXPECT_NEAR(maxY,  expected, 3e-3);
    EXPECT_NEAR(minY, -expected, 3e-3);
}

TEST(Offset, ZCoordinatePreserved) {
    auto poly = makeSquare(1.0, 5.0);
    auto outList = offset::offsetPolyline(poly, 0.1);
    ASSERT_GE(outList.size(), 1u);

    for (auto &p : outList[0].points)
        EXPECT_NEAR(p.getZ(), 5.0, 1e-9);
}

TEST(Offset, SelfIntersectingPolygon) {
    SliceLayer::Polyline poly;
    poly.points = {
        v3(0,0,0),
        v3(2,2,0),
        v3(0,2,0),
        v3(2,0,0),
        v3(0,0,0)
    };

    EXPECT_NO_THROW(offset::offsetPolyline(poly, 0.1));
}

TEST(Offset, LargeCoordinates) {
    SliceLayer::Polyline poly;
    poly.points = {
        v3(1e6, 1e6, 0),
        v3(1e6+1000, 1e6, 0),
        v3(1e6+1000, 1e6+1000, 0),
        v3(1e6, 1e6+1000, 0),
        v3(1e6, 1e6, 0)
    };

    auto outList = offset::offsetPolyline(poly, 100);
    ASSERT_GE(outList.size(), 1u);
    const auto &out = outList[0];

    double minX, maxX, minY, maxY;
    bbox(out, minX, maxX, minY, maxY);

    EXPECT_NEAR(minX, 1e6 - 100, 1e-3);
    EXPECT_NEAR(maxX, 1e6 + 1000 + 100, 1e-3);
}

TEST(Offset, ShapeFracturingIntoMultipleFragments) {
    SliceLayer::Polyline poly;
    poly.points = {
        v3(-3, -1, 0), v3(3, -1, 0), v3(3, 1, 0), v3(0, 0.2, 0), 
        v3(3, 3, 0), v3(3, 5, 0), v3(-3, 5, 0), v3(-3, 3, 0), 
        v3(0, 0.2, 0), v3(-3, 1, 0), v3(-3, -1, 0)
    };
    auto outList = offset::offsetPolyline(poly, -0.4);
    EXPECT_GT(outList.size(), 1u) << "Narrow neck should cause shape to fracture into multiple fragments";
}
