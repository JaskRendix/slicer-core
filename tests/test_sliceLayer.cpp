#include <gtest/gtest.h>
#include "sliceLayer.h"
#include "lineSegment.h"
#include "v3.h"

static lineSegment seg(double x1, double y1, double x2, double y2, double z=0.0) {
    return lineSegment(v3(x1,y1,z), v3(x2,y2,z));
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
    layer.addSegment(seg(1,0, 2,0)); // Collinear intermediate point (1,0) is filtered out

    auto polys = layer.buildPolylines();

    ASSERT_EQ(polys.size(), 1u);
    ASSERT_EQ(polys[0].points.size(), 2u); // Cleaned down to endpoints (0,0) and (2,0)
}

TEST(SliceLayer, ClosedLoop) {
    SliceLayer layer(0.0);

    layer.addSegment(seg(0,0, 1,0));
    layer.addSegment(seg(1,0, 1,1));
    layer.addSegment(seg(1,1, 0,1));
    layer.addSegment(seg(0,1, 0,0));

    auto polys = layer.buildPolylines();

    ASSERT_EQ(polys.size(), 1u);
    EXPECT_TRUE(polys[0].is_closed);

    // Duplicate closing endpoint is popped; 4 unique corners remain
    ASSERT_EQ(polys[0].points.size(), 4u);
}

TEST(SliceLayer, TwoDisjointLoops) {
    SliceLayer layer(0.0);

    layer.addSegment(seg(0,0, 1,0));
    layer.addSegment(seg(1,0, 1,1));
    layer.addSegment(seg(1,1, 0,1));
    layer.addSegment(seg(0,1, 0,0));

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
    EXPECT_TRUE(polys[0].is_closed);
    ASSERT_EQ(polys[0].points.size(), 4u);
}

TEST(SliceLayer, EpsilonMatching) {
    SliceLayer layer(0.0);

    layer.addSegment(seg(0,0, 1,0));
    layer.addSegment(seg(1 + 1e-7, 0, 2,0));

    auto polys = layer.buildPolylines(1e-6);

    ASSERT_EQ(polys.size(), 1u);
    ASSERT_EQ(polys[0].points.size(), 2u);
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
    ASSERT_EQ(polys[0].points.size(), 2u);
}

TEST(SliceLayer, MixedOpenClosed) {
    SliceLayer layer(0.0);

    // Closed square
    layer.addSegment(seg(0,0, 1,0));
    layer.addSegment(seg(1,0, 1,1));
    layer.addSegment(seg(1,1, 0,1));
    layer.addSegment(seg(0,1, 0,0));

    // Open polyline with a bend (non-collinear intermediate point)
    layer.addSegment(seg(5,5, 6,5));
    layer.addSegment(seg(6,5, 6,6));

    auto polys = layer.buildPolylines();

    ASSERT_EQ(polys.size(), 2u);

    bool foundClosed = false;
    bool foundOpen = false;

    for (auto &p : polys) {
        if (p.is_closed && p.points.size() == 4) foundClosed = true;
        if (!p.is_closed && p.points.size() == 3) foundOpen = true;
    }

    EXPECT_TRUE(foundClosed);
    EXPECT_TRUE(foundOpen);
}

TEST(SliceLayer, LoopClosureWithEpsilon) {
    SliceLayer layer(0.0);

    layer.addSegment(seg(0,0, 1,0));
    layer.addSegment(seg(1,0, 1,1));
    layer.addSegment(seg(1,1, 0,1));
    layer.addSegment(seg(0,1, 0,0 + 1e-7));

    auto polys = layer.buildPolylines(1e-6);

    ASSERT_EQ(polys.size(), 1u);
    EXPECT_TRUE(polys[0].is_closed);
    ASSERT_EQ(polys[0].points.size(), 4u);
}

TEST(SliceLayer, NoInfiniteExtensionOnBadInput) {
    SliceLayer layer(0.0);

    layer.addSegment(seg(0,0, 1,0));
    layer.addSegment(seg(5,5, 6,5));

    auto polys = layer.buildPolylines();

    ASSERT_EQ(polys.size(), 2u);
}
