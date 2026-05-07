#include <gtest/gtest.h>
#include "winding_normalize.h"
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
    p.points = {v3(0,0,0), v3(0,0,0), v3(0,0,0)};
    auto before = p.points;
    winding_normalize::toCCW(p);
    EXPECT_EQ(p.points.size(), before.size());
}
