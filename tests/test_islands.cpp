#include <gtest/gtest.h>
#include "islands.h"
#include "sliceLayer.h"
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

static SliceLayer::Polyline makeReversed(const SliceLayer::Polyline &poly) {
    SliceLayer::Polyline out = poly;
    std::reverse(out.points.begin(), out.points.end());
    return out;
}

TEST(Islands, SingleOuter) {
    auto outer = makeSquare(1.0);
    std::vector<SliceLayer::Polyline> polys = {outer};

    auto islands = buildIslands(polys);

    ASSERT_EQ(islands.size(), 1u);
    EXPECT_EQ(islands[0].holes.size(), 0u);
}

TEST(Islands, OuterWithHole) {
    auto outer = makeSquare(2.0);
    auto hole  = makeReversed(makeSquare(0.5)); // CW hole

    std::vector<SliceLayer::Polyline> polys = {outer, hole};

    auto islands = buildIslands(polys);

    ASSERT_EQ(islands.size(), 1u);
    ASSERT_EQ(islands[0].holes.size(), 1u);
}

TEST(Islands, TwoSeparateIslands) {
    auto A = makeSquare(1.0);
    auto B = makeSquare(1.0);

    // Shift B to the right
    for (auto &p : B.points) p = v3(p.getX() + 5.0, p.getY(), p.getZ());

    std::vector<SliceLayer::Polyline> polys = {A, B};

    auto islands = buildIslands(polys);

    ASSERT_EQ(islands.size(), 2u);
    EXPECT_EQ(islands[0].holes.size(), 0u);
    EXPECT_EQ(islands[1].holes.size(), 0u);
}

TEST(Islands, NestedHoles) {
    auto outer = makeSquare(3.0);
    auto hole1 = makeReversed(makeSquare(2.0));
    auto hole2 = makeReversed(makeSquare(1.0)); // hole inside hole

    std::vector<SliceLayer::Polyline> polys = {outer, hole1, hole2};

    auto islands = buildIslands(polys);

    ASSERT_EQ(islands.size(), 1u);
    ASSERT_EQ(islands[0].holes.size(), 2u);
}

TEST(Islands, DegenerateIgnored) {
    SliceLayer::Polyline deg;
    deg.points = {v3(0,0,0), v3(0,0,0), v3(0,0,0)};

    auto outer = makeSquare(1.0);

    std::vector<SliceLayer::Polyline> polys = {outer, deg};

    auto islands = buildIslands(polys);

    ASSERT_EQ(islands.size(), 1u);
    EXPECT_EQ(islands[0].holes.size(), 0u);
}

TEST(Islands, MixedWinding) {
    auto outer = makeReversed(makeSquare(2.0)); // CW outer
    auto hole  = makeSquare(0.5);               // CCW hole

    std::vector<SliceLayer::Polyline> polys = {outer, hole};

    auto islands = buildIslands(polys);

    ASSERT_EQ(islands.size(), 1u);
    ASSERT_EQ(islands[0].holes.size(), 1u);
}
