#include <gtest/gtest.h>
#include "winding_normalize.h"
#include "winding.h"
#include "sliceLayer.h"
#include "v3.h"
#include <algorithm>

static SliceLayer::Polyline makeSquareCCW() {
    SliceLayer::Polyline p;
    p.is_closed = true;
    p.points = {
        v3(-1,-1,0),
        v3( 1,-1,0),
        v3( 1, 1,0),
        v3(-1, 1,0),
        v3(-1,-1,0)
    };
    return p;
}

static SliceLayer::Polyline makeSquareCW() {
    auto p = makeSquareCCW();
    std::reverse(p.points.begin(), p.points.end());
    p.is_closed = true;
    return p;
}

TEST(WindingNormalize, ToCCW) {
    auto poly = makeSquareCW();
    winding_normalize::toCCW(poly);
    EXPECT_TRUE(winding::isCCW(poly));
}

TEST(WindingNormalize, ToCW) {
    auto poly = makeSquareCCW();
    winding_normalize::toCW(poly);
    EXPECT_TRUE(winding::isCW(poly));
}

TEST(WindingNormalize, DegenerateNoChange) {
    SliceLayer::Polyline p;
    p.is_closed = true;
    p.points = {v3(0,0,0), v3(0,0,0), v3(0,0,0)};
    auto beforeSize = p.points.size();
    winding_normalize::toCCW(p);
    EXPECT_EQ(p.points.size(), beforeSize);
}

TEST(WindingNormalize, CanonicalRotationLexicographic) {
    SliceLayer::Polyline p;
    p.is_closed = true;
    p.points = {
        v3(5,5,0),
        v3(1,1,0),  // smallest lexicographically
        v3(3,3,0),
        v3(4,4,0)
    };

    winding_normalize::normalizeCanonical(p);

    EXPECT_NEAR(p.points.front().getX(), 1.0, 1e-9);
    EXPECT_NEAR(p.points.front().getY(), 1.0, 1e-9);
}

TEST(WindingNormalize, CanonicalPreservesWinding) {
    auto poly = makeSquareCCW();
    poly.is_closed = true;

    winding_normalize::normalizeCanonical(poly);

    EXPECT_TRUE(winding::isCCW(poly));
}

TEST(WindingNormalize, NormalizeAllAppliesCCWAndCanonical) {
    std::vector<SliceLayer::Polyline> polys;

    auto p = makeSquareCW();   // reversed orientation
    p.is_closed = true;
    polys.push_back(p);

    winding_normalize::normalizeAll(polys);

    ASSERT_EQ(polys.size(), 1u);
    EXPECT_TRUE(winding::isCCW(polys[0]));

    // Smallest lexicographic point is (-1,-1)
    EXPECT_NEAR(polys[0].points.front().getX(), -1.0, 1e-9);
    EXPECT_NEAR(polys[0].points.front().getY(), -1.0, 1e-9);
}

TEST(WindingNormalize, OpenPolylineNoChange) {
    SliceLayer::Polyline p;
    p.is_closed = false;
    p.points = { v3(2,2,0), v3(1,1,0), v3(0,0,0) };

    std::vector<v3> before = p.points;

    winding_normalize::toCCW(p);
    winding_normalize::normalizeCanonical(p);

    ASSERT_EQ(p.points.size(), before.size());
    for (size_t i = 0; i < p.points.size(); ++i) {
        EXPECT_NEAR(p.points[i].getX(), before[i].getX(), 1e-9);
        EXPECT_NEAR(p.points[i].getY(), before[i].getY(), 1e-9);
    }
}

TEST(WindingNormalize, DegenerateClosedNoChange) {
    SliceLayer::Polyline p;
    p.is_closed = true;
    p.points = { v3(0,0,0), v3(1,1,0) }; // only 2 points

    std::vector<v3> before = p.points;

    winding_normalize::toCCW(p);
    winding_normalize::normalizeCanonical(p);

    ASSERT_EQ(p.points.size(), before.size());
    for (size_t i = 0; i < p.points.size(); ++i) {
        EXPECT_NEAR(p.points[i].getX(), before[i].getX(), 1e-9);
        EXPECT_NEAR(p.points[i].getY(), before[i].getY(), 1e-9);
    }
}

TEST(WindingNormalize, SignedAreaCorrectness) {
    auto ccw = makeSquareCCW();
    ccw.is_closed = true;

    auto cw = makeSquareCW();
    cw.is_closed = true;

    double areaCCW = winding_normalize::computeSignedArea(ccw);
    double areaCW  = winding_normalize::computeSignedArea(cw);

    EXPECT_GT(areaCCW, 0.0);
    EXPECT_LT(areaCW, 0.0);
}
