#include <gtest/gtest.h>
#include "offset.h"
#include "sliceLayer.h"
#include "v3.h"
#include <algorithm>
#include <cmath>

// ------------------------------------------------------------
// Helpers
// ------------------------------------------------------------

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

static double dist2D(const v3 &a, const v3 &b) {
    double dx = a.getX() - b.getX();
    double dy = a.getY() - b.getY();
    return std::sqrt(dx*dx + dy*dy);
}

// Axis-aligned bounding box of a polyline
static void bbox(const SliceLayer::Polyline &poly,
                 double &minX, double &maxX, double &minY, double &maxY) {
    minX = minY =  1e300;
    maxX = maxY = -1e300;
    for (const auto &p : poly.points) {
        minX = std::min(minX, p.getX()); maxX = std::max(maxX, p.getX());
        minY = std::min(minY, p.getY()); maxY = std::max(maxY, p.getY());
    }
}

// Check that a corner exists somewhere in the output
static bool hasCorner(const SliceLayer::Polyline &poly, const v3 &corner, double tol = 1e-4) {
    for (const auto &p : poly.points)
        if (dist2D(p, corner) < tol) return true;
    return false;
}

// ------------------------------------------------------------
// 1. Expanding a CCW square — check bbox grows by offset
// ------------------------------------------------------------

TEST(Offset, ExpandSquareCCW) {
    auto out = offset::offsetPolyline(makeSquare(1.0), 0.1);

    ASSERT_GE(out.points.size(), 4u);

    double minX, maxX, minY, maxY;
    bbox(out, minX, maxX, minY, maxY);

    EXPECT_NEAR(minX, -1.1, 1e-4);
    EXPECT_NEAR(maxX,  1.1, 1e-4);
    EXPECT_NEAR(minY, -1.1, 1e-4);
    EXPECT_NEAR(maxY,  1.1, 1e-4);
}

// ------------------------------------------------------------
// 2. Expanding a CW square — same bbox as CCW
// ------------------------------------------------------------

TEST(Offset, ExpandSquareCW) {
    auto out = offset::offsetPolyline(reversed(makeSquare(1.0)), 0.1);

    ASSERT_GE(out.points.size(), 4u);

    double minX, maxX, minY, maxY;
    bbox(out, minX, maxX, minY, maxY);

    EXPECT_NEAR(minX, -1.1, 1e-4);
    EXPECT_NEAR(maxX,  1.1, 1e-4);
    EXPECT_NEAR(minY, -1.1, 1e-4);
    EXPECT_NEAR(maxY,  1.1, 1e-4);
}

// ------------------------------------------------------------
// 3. Shrinking a square — bbox shrinks by offset
// ------------------------------------------------------------

TEST(Offset, ShrinkSquare) {
    auto out = offset::offsetPolyline(makeSquare(1.0), -0.2);

    ASSERT_GE(out.points.size(), 4u);

    double minX, maxX, minY, maxY;
    bbox(out, minX, maxX, minY, maxY);

    EXPECT_NEAR(minX, -0.8, 1e-4);
    EXPECT_NEAR(maxX,  0.8, 1e-4);
    EXPECT_NEAR(minY, -0.8, 1e-4);
    EXPECT_NEAR(maxY,  0.8, 1e-4);
}

// ------------------------------------------------------------
// 4. Concave polygon (L-shape) — bbox is correct
// ------------------------------------------------------------

TEST(Offset, ConcavePolygon) {
    SliceLayer::Polyline poly;
    poly.points = {
        v3(0,0,0), v3(2,0,0), v3(2,1,0),
        v3(1,1,0), v3(1,2,0), v3(0,2,0),
        v3(0,0,0)
    };

    auto out = offset::offsetPolyline(poly, 0.1);

    ASSERT_GE(out.points.size(), 4u);

    double minX, maxX, minY, maxY;
    bbox(out, minX, maxX, minY, maxY);

    // Outer extent grows by 0.1
    EXPECT_NEAR(minX, -0.1, 1e-4);
    EXPECT_NEAR(maxX,  2.1, 1e-4);
    EXPECT_NEAR(minY, -0.1, 1e-4);
    EXPECT_NEAR(maxY,  2.1, 1e-4);
}

// ------------------------------------------------------------
// 5. Collapse: shrinking too far → empty or degenerate, no crash
// ------------------------------------------------------------

TEST(Offset, CollapseToNothing) {
    auto out = offset::offsetPolyline(makeSquare(1.0), -2.0);
    // Clipper2 returns empty when shape collapses — just assert no crash
    SUCCEED();
}

// ------------------------------------------------------------
// 6. Multi-polyline offset — both bboxes grow
// ------------------------------------------------------------

TEST(Offset, MultiPolyline) {
    std::vector<SliceLayer::Polyline> polys = { makeSquare(1.0), makeSquare(0.5) };
    auto out = offset::offsetLayerPolylines(polys, 0.1);

    ASSERT_EQ(out.size(), 2u);

    double minX, maxX, minY, maxY;

    bbox(out[0], minX, maxX, minY, maxY);
    EXPECT_NEAR(minX, -1.1, 1e-4);
    EXPECT_NEAR(maxX,  1.1, 1e-4);

    bbox(out[1], minX, maxX, minY, maxY);
    EXPECT_NEAR(minX, -0.6, 1e-4);
    EXPECT_NEAR(maxX,  0.6, 1e-4);
}

// ------------------------------------------------------------
// 7. Degenerate input → no crash, returns something
// ------------------------------------------------------------

TEST(Offset, DegenerateInput) {
    SliceLayer::Polyline p;
    p.points = { v3(0,0,0), v3(0,0,0), v3(0,0,0) };
    EXPECT_NO_THROW(offset::offsetPolyline(p, 0.1));
}