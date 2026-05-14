#include <gtest/gtest.h>
#include "sliceLayer.h"
#include "lineSegment.h"
#include "v3.h"

static lineSegment seg(double x1, double y1, double x2, double y2, double z=0.0) {
    return lineSegment(v3(x1,y1,z), v3(x2,y2,z));
}

static SliceLayer::Polyline makePolyline(const std::vector<v3> &pts) {
    SliceLayer::Polyline p;
    p.points = pts;
    return p;
}

TEST(SliceLayer, SingleSegment) {
    SliceLayer layer(0.0);
    layer.addSegment(seg(0,0, 1,0));

    auto polys = layer.buildPolylines();

    ASSERT_EQ(polys.size(), 1u);
    ASSERT_EQ(polys[0].points.size(), 2u);
}

TEST(SliceLayer, TwoConnectedSegments) {
    SliceLayer layer(0.0);
    layer.addSegment(seg(0,0, 1,0));
    layer.addSegment(seg(1,0, 2,0));

    auto polys = layer.buildPolylines();

    ASSERT_EQ(polys.size(), 1u);
    ASSERT_EQ(polys[0].points.size(), 3u);
}

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

TEST(SliceLayer, EpsilonMatching) {
    SliceLayer layer(0.0);

    layer.addSegment(seg(0,0, 1,0));
    layer.addSegment(seg(1 + 1e-7, 0, 2,0)); // tiny mismatch

    auto polys = layer.buildPolylines(1e-6);

    ASSERT_EQ(polys.size(), 1u);
    ASSERT_EQ(polys[0].points.size(), 3u);
}

TEST(SliceLayer, NoSegments) {
    SliceLayer layer(0.0);
    auto polys = layer.buildPolylines();
    EXPECT_TRUE(polys.empty());
}

TEST(SliceLayer, ZeroLengthSegment) {
    SliceLayer layer(0.0);
    layer.addSegment(seg(1,1, 1,1));

    auto polys = layer.buildPolylines();

    ASSERT_EQ(polys.size(), 1u);
    ASSERT_EQ(polys[0].points.size(), 2u); // start + end identical
}

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

TEST(SliceLayer, LoopClosureWithEpsilon) {
    SliceLayer layer(0.0);

    layer.addSegment(seg(0,0, 1,0));
    layer.addSegment(seg(1,0, 1,1));
    layer.addSegment(seg(1,1, 0,1));
    layer.addSegment(seg(0,1, 0,0 + 1e-7)); // slightly off

    auto polys = layer.buildPolylines(1e-6);

    ASSERT_EQ(polys.size(), 1u);
    ASSERT_EQ(polys[0].points.size(), 5u);
}

TEST(SliceLayer, NoInfiniteExtensionOnBadInput) {
    SliceLayer layer(0.0);

    // Two segments that cannot connect
    layer.addSegment(seg(0,0, 1,0));
    layer.addSegment(seg(5,5, 6,5));

    auto polys = layer.buildPolylines();

    ASSERT_EQ(polys.size(), 2u);
}
