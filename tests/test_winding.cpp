#include <gtest/gtest.h>
#include "winding.h"
#include "sliceLayer.h"
#include "v3.h"
#include <algorithm>

static SliceLayer::Polyline makeSquareCCW() {
    SliceLayer::Polyline p;
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
    return p;
}

TEST(Winding, DetectCCW) {
    auto poly = makeSquareCCW();
    EXPECT_TRUE(winding::isCCW(poly));
    EXPECT_FALSE(winding::isCW(poly));
}

TEST(Winding, DetectCW) {
    auto poly = makeSquareCW();
    EXPECT_TRUE(winding::isCW(poly));
    EXPECT_FALSE(winding::isCCW(poly));
}

TEST(Winding, DegenerateIsNeither) {
    SliceLayer::Polyline p;
    p.points = {v3(0,0,0), v3(0,0,0), v3(0,0,0)};
    EXPECT_FALSE(winding::isCW(p));
    EXPECT_FALSE(winding::isCCW(p));
}
