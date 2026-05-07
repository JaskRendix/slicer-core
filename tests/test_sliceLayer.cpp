#include <gtest/gtest.h>
#include "sliceLayer.h"
#include "lineSegment.h"
#include "v3.h"

// ------------------------------------------------------------
// Helpers
// ------------------------------------------------------------

static lineSegment seg(double x1, double y1, double x2, double y2, double z=0.0) {
    return lineSegment(v3(x1,y1,z), v3(x2,y2,z));
}

static SliceLayer::Polyline makePolyline(const std::vector<v3> &pts) {
    SliceLayer::Polyline p;
    p.points = pts;
    return p;
}

// ------------------------------------------------------------
// 1. Single segment → one polyline with 2 points
// ------------------------------------------------------------

TEST(SliceLayer, SingleSegment) {
    SliceLayer layer(0.0);
    layer.addSegment(seg(0,0, 1,0));

    auto polys = layer.buildPolylines();

    ASSERT_EQ(polys.size(), 1u);
    ASSERT_EQ(polys[0].points.size(), 2u);
}

// ------------------------------------------------------------
// 2. Two connected segments → one polyline with 3 points
// ------------------------------------------------------------

TEST(SliceLayer, TwoConnectedSegments) {
    SliceLayer layer(0.0);
    layer.addSegment(seg(0,0, 1,0));
    layer.addSegment(seg(1,0, 2,0));

    auto polys = layer.buildPolylines();

    ASSERT_EQ(polys.size(), 1u);
    ASSERT_EQ(polys[0].points.size(), 3u);
}

// ------------------------------------------------------------
// 3. Closed loop → stops extending when front == back
// ------------------------------------------------------------

TEST(SliceLayer, ClosedLoop) {
    SliceLayer layer(0.0);

    layer.addSegment(seg(0,0, 1,0));
    layer.addSegment(seg(1,0, 1,1));
    layer.addSegment(seg(1,1, 0,1));
    layer.addSegment(seg(0,1, 0,0)); // closes loop

    auto polys = layer.buildPolylines();

    ASSERT_EQ(polys.size(), 1u);

    // Should contain 5 points: 4 corners + repeated first
    ASSERT_EQ(polys[0].points.size(), 5u);

    EXPECT_NEAR(polys[0].points.front().getX(), polys[0].points.back().getX(), 1e-9);
    EXPECT_NEAR(polys[0].points.front().getY(), polys[0].points.back().getY(), 1e-9);
}

// ------------------------------------------------------------
// 4. Two disjoint loops → two polylines
// ------------------------------------------------------------

TEST(SliceLayer, TwoDisjointLoops) {
    SliceLayer layer(0.0);

    // Loop A
    layer.addSegment(seg(0,0, 1,0));
    layer.addSegment(seg(1,0, 1,1));
    layer.addSegment(seg(1,1, 0,1));
    layer.addSegment(seg(0,1, 0,0));

    // Loop B (shifted)
    layer.addSegment(seg(5,5, 6,5));
    layer.addSegment(seg(6,5, 6,6));
    layer.addSegment(seg(6,6, 5,6));
    layer.addSegment(seg(5,6, 5,5));

    auto polys = layer.buildPolylines();

    ASSERT_EQ(polys.size(), 2u);
}

// ------------------------------------------------------------
// 5. Unordered segments → still stitched correctly
// ------------------------------------------------------------

TEST(SliceLayer, UnorderedSegments) {
    SliceLayer layer(0.0);

    layer.addSegment(seg(1,0, 1,1));
    layer.addSegment(seg(0,1, 0,0));
    layer.addSegment(seg(1,1, 0,1));
    layer.addSegment(seg(0,0, 1,0));

    auto polys = layer.buildPolylines();

    ASSERT_EQ(polys.size(), 1u);
    ASSERT_EQ(polys[0].points.size(), 5u);
}

// ------------------------------------------------------------
// 6. Epsilon matching: endpoints within tolerance should connect
// ------------------------------------------------------------

TEST(SliceLayer, EpsilonMatching) {
    SliceLayer layer(0.0);

    layer.addSegment(seg(0,0, 1,0));
    layer.addSegment(seg(1 + 1e-7, 0, 2,0)); // tiny mismatch

    auto polys = layer.buildPolylines(1e-6);

    ASSERT_EQ(polys.size(), 1u);
    ASSERT_EQ(polys[0].points.size(), 3u);
}

// ------------------------------------------------------------
// 7. Degenerate: no segments → no polylines
// ------------------------------------------------------------

TEST(SliceLayer, NoSegments) {
    SliceLayer layer(0.0);
    auto polys = layer.buildPolylines();
    EXPECT_TRUE(polys.empty());
}

// ------------------------------------------------------------
// 8. Degenerate: segment with identical endpoints
// ------------------------------------------------------------

TEST(SliceLayer, ZeroLengthSegment) {
    SliceLayer layer(0.0);
    layer.addSegment(seg(1,1, 1,1));

    auto polys = layer.buildPolylines();

    ASSERT_EQ(polys.size(), 1u);
    ASSERT_EQ(polys[0].points.size(), 2u); // start + end identical
}

// ------------------------------------------------------------
// 9. Mixed open + closed shapes
// ------------------------------------------------------------

TEST(SliceLayer, MixedOpenClosed) {
    SliceLayer layer(0.0);

    // Closed square
    layer.addSegment(seg(0,0, 1,0));
    layer.addSegment(seg(1,0, 1,1));
    layer.addSegment(seg(1,1, 0,1));
    layer.addSegment(seg(0,1, 0,0));

    // Open polyline
    layer.addSegment(seg(5,5, 6,5));
    layer.addSegment(seg(6,5, 7,5));

    auto polys = layer.buildPolylines();

    ASSERT_EQ(polys.size(), 2u);

    // Identify which is which
    bool foundClosed = false;
    bool foundOpen = false;

    for (auto &p : polys) {
        if (p.points.size() == 5) foundClosed = true;
        if (p.points.size() == 3) foundOpen = true;
    }

    EXPECT_TRUE(foundClosed);
    EXPECT_TRUE(foundOpen);
}
