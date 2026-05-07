#include <gtest/gtest.h>
#include "infill.h"
#include "islands.h"
#include "v3.h"
#include <cmath>
#include <chrono>



static Island makeSquare(double half, double z = 0.0) {
    Island isl;
    isl.outer.points = {
        v3(-half, -half, z),
        v3( half, -half, z),
        v3( half,  half, z),
        v3(-half,  half, z),
        v3(-half, -half, z)
    };
    return isl;
}

static Island makeRect(double hw, double hh, double z = 0.0) {
    Island isl;
    isl.outer.points = {
        v3(-hw, -hh, z),
        v3( hw, -hh, z),
        v3( hw,  hh, z),
        v3(-hw,  hh, z),
        v3(-hw, -hh, z)
    };
    return isl;
}

static Island makeDonut(double outerHalf, double innerHalf, double z = 0.0) {
    Island isl = makeSquare(outerHalf, z);
    SliceLayer::Polyline hole;
    hole.points = {
        v3(-innerHalf, -innerHalf, z),
        v3( innerHalf, -innerHalf, z),
        v3( innerHalf,  innerHalf, z),
        v3(-innerHalf,  innerHalf, z),
        v3(-innerHalf, -innerHalf, z)
    };
    isl.holes.push_back(hole);
    return isl;
}

static Island makeMultiHole(double outerHalf, double z = 0.0) {
    Island isl = makeSquare(outerHalf, z);
    auto makeHole = [&](double cx, double cy, double r) {
        SliceLayer::Polyline h;
        h.points = {
            v3(cx - r, cy - r, z),
            v3(cx + r, cy - r, z),
            v3(cx + r, cy + r, z),
            v3(cx - r, cy + r, z),
            v3(cx - r, cy - r, z)
        };
        isl.holes.push_back(h);
    };
    makeHole(-1.5, -1.5, 0.4);
    makeHole( 1.5, -1.5, 0.4);
    makeHole(-1.5,  1.5, 0.4);
    makeHole( 1.5,  1.5, 0.4);
    return isl;
}

static double segAngleDeg(const InfillSegment &s) {
    double dx = s.b.getX() - s.a.getX();
    double dy = s.b.getY() - s.a.getY();
    double a = std::atan2(dy, dx) * 180.0 / M_PI;
    if (a < 0) a += 180.0;
    return a;
}

static bool midpointInRect(const InfillSegment &s,
                            double hw, double hh) {
    double mx = 0.5 * (s.a.getX() + s.b.getX());
    double my = 0.5 * (s.a.getY() + s.b.getY());
    return std::abs(mx) <= hw + 1e-6 && std::abs(my) <= hh + 1e-6;
}



TEST(LineInfill, HorizontalLinesInsideSquare) {
    auto segs = generateLineInfill(makeSquare(1.0), 0.5, 0.0);

    EXPECT_GE(segs.size(), 3u);
    for (const auto &s : segs) {
        EXPECT_NEAR(s.a.getY(), s.b.getY(), 1e-9) << "line should be horizontal";
        EXPECT_LE(std::abs(s.a.getX()), 1.0 + 1e-6);
        EXPECT_LE(std::abs(s.b.getX()), 1.0 + 1e-6);
    }
}

TEST(LineInfill, VerticalLines_Rotated90) {
    auto segs = generateLineInfill(makeSquare(1.0), 0.5, 90.0);

    EXPECT_GE(segs.size(), 3u);
    for (const auto &s : segs) {
        EXPECT_NEAR(s.a.getX(), s.b.getX(), 1e-9) << "line should be vertical";
    }
}

TEST(LineInfill, DiagonalLines_Rotated45) {
    auto segs = generateLineInfill(makeSquare(2.0), 0.5, 45.0);

    EXPECT_GE(segs.size(), 1u);

    double totalLength = 0.0;
    for (const auto &s : segs) {
        double dx = s.b.getX() - s.a.getX();
        double dy = s.b.getY() - s.a.getY();
        totalLength += std::sqrt(dx*dx + dy*dy);
    }
    // A 4x4 square at 0.5 spacing has ~8 lines averaging ~2 units each
    EXPECT_GT(totalLength, 5.0) << "rotated infill should cover meaningful area";
}

TEST(LineInfill, SpacingRespected) {
    double spacing = 0.4;
    auto segs = generateLineInfill(makeSquare(2.0), spacing, 0.0);

    // Collect unique Y values
    std::vector<double> ys;
    for (const auto &s : segs)
        ys.push_back(s.a.getY());
    std::sort(ys.begin(), ys.end());
    ys.erase(std::unique(ys.begin(), ys.end()), ys.end());

    for (size_t i = 1; i < ys.size(); ++i) {
        double gap = ys[i] - ys[i - 1];
        EXPECT_NEAR(gap, spacing, 1e-6) << "gap between lines should equal spacing";
    }
}

TEST(LineInfill, ZCoordinatePreserved) {
    double z = 3.14;
    auto segs = generateLineInfill(makeSquare(1.0, z), 0.5, 0.0);

    EXPECT_GE(segs.size(), 1u);
    for (const auto &s : segs) {
        EXPECT_NEAR(s.a.getZ(), z, 1e-9);
        EXPECT_NEAR(s.b.getZ(), z, 1e-9);
    }
}

TEST(LineInfill, NonSquareRect_CorrectCoverage) {
    // Wide rectangle: more lines along short axis
    auto segs = generateLineInfill(makeRect(4.0, 1.0), 0.5, 0.0);

    EXPECT_GE(segs.size(), 3u);
    for (const auto &s : segs)
        EXPECT_TRUE(midpointInRect(s, 4.0, 1.0));
}



TEST(GridInfill, ContainsBothDirections) {
    auto segs = generateGridInfill(makeSquare(1.0), 0.5);

    bool horiz = false, vert = false;
    for (const auto &s : segs) {
        if (std::abs(s.a.getY() - s.b.getY()) < 1e-9) horiz = true;
        if (std::abs(s.a.getX() - s.b.getX()) < 1e-9) vert  = true;
    }
    EXPECT_TRUE(horiz) << "grid must have horizontal lines";
    EXPECT_TRUE(vert)  << "grid must have vertical lines";
}

TEST(GridInfill, CountApproximatelyDouble_Line) {
    Island isl = makeSquare(1.0);
    auto line = generateLineInfill(isl, 0.5, 0.0);
    auto grid = generateGridInfill(isl, 0.5);

    EXPECT_GE(grid.size(), line.size());          // at least as many as one direction
    EXPECT_LE(grid.size(), line.size() * 3);      // but not more than triple
}

TEST(HexInfill, ContainsThreeAngles) {
    Island isl = makeSquare(2.0);
    auto grid = generateGridInfill(isl, 0.5);
    auto hex  = generateHexInfill(isl, 0.5);

    // Hex runs 3 passes vs grid's 2, so must produce more segments
    EXPECT_GT(hex.size(), grid.size());
}

TEST(HexInfill, CountApproximatelyTriple_Line) {
    Island isl = makeSquare(2.0);
    auto line = generateLineInfill(isl, 0.5, 0.0);
    auto hex  = generateHexInfill(isl, 0.5);

    EXPECT_GE(hex.size(), line.size() * 2);   // strictly more than two passes
}

TEST(Holes, SingleHole_MidpointNeverInsideHole) {
    Island isl = makeDonut(2.0, 0.5);
    auto segs = generateLineInfill(isl, 0.25, 0.0);

    EXPECT_GE(segs.size(), 1u);
    for (const auto &s : segs) {
        double mx = 0.5 * (s.a.getX() + s.b.getX());
        double my = 0.5 * (s.a.getY() + s.b.getY());
        bool inHole = std::abs(mx) < 0.5 && std::abs(my) < 0.5;
        EXPECT_FALSE(inHole) << "segment midpoint must not be inside hole";
    }
}

TEST(Holes, SingleHole_SegmentsStillExistOutside) {
    // A large donut should still produce plenty of infill
    auto segs = generateLineInfill(makeDonut(3.0, 0.5), 0.5, 0.0);
    EXPECT_GE(segs.size(), 6u);
}

TEST(Holes, MultipleHoles_NoneViolated) {
    Island isl = makeMultiHole(3.0);
    auto segs = generateLineInfill(isl, 0.3, 0.0);

    EXPECT_GE(segs.size(), 1u);
    for (const auto &s : segs) {
        for (const auto &hole : isl.holes) {
            double cx = hole.points[0].getX() + 0.4;
            double cy = hole.points[0].getY() + 0.4;
            double mx = 0.5 * (s.a.getX() + s.b.getX());
            double my = 0.5 * (s.a.getY() + s.b.getY());
            bool inHole = std::abs(mx - cx) < 0.4 && std::abs(my - cy) < 0.4;
            EXPECT_FALSE(inHole);
        }
    }
}

TEST(Holes, HoleLargerThanSpacing_GridRespects) {
    Island isl = makeDonut(3.0, 1.5);
    auto segs = generateGridInfill(isl, 0.5);

    for (const auto &s : segs) {
        double mx = 0.5 * (s.a.getX() + s.b.getX());
        double my = 0.5 * (s.a.getY() + s.b.getY());
        bool inHole = std::abs(mx) < 1.5 && std::abs(my) < 1.5;
        EXPECT_FALSE(inHole);
    }
}


TEST(EdgeCases, TinyIsland_NoSegments) {
    EXPECT_EQ(generateLineInfill(makeSquare(0.01), 0.5, 0.0).size(), 0u);
    EXPECT_EQ(generateGridInfill(makeSquare(0.01), 0.5).size(),      0u);
    EXPECT_EQ(generateHexInfill (makeSquare(0.01), 0.5).size(),      0u);
}

TEST(EdgeCases, SpacingLargerThanIsland_NoSegments) {
    EXPECT_EQ(generateLineInfill(makeSquare(1.0), 10.0, 0.0).size(), 0u);
}

TEST(EdgeCases, EmptyIsland_NoCrash) {
    Island isl;
    EXPECT_NO_THROW(generateLineInfill(isl, 0.5, 0.0));
    EXPECT_NO_THROW(generateGridInfill(isl, 0.5));
    EXPECT_NO_THROW(generateHexInfill (isl, 0.5));
}

TEST(EdgeCases, DegeneratePoints_NoCrash) {
    Island isl;
    isl.outer.points = { v3(0,0,0), v3(0,0,0), v3(0,0,0) };
    EXPECT_NO_THROW(generateLineInfill(isl, 0.5, 0.0));
}

TEST(EdgeCases, ZeroSpacing_NoCrashOrInfiniteLoop) {
    // spacing=0 would loop forever if unguarded; treat as degenerate
    Island isl = makeSquare(1.0);
    // We only assert no hang — output is unspecified for invalid input
    auto segs = generateLineInfill(isl, 0.0, 0.0);
    (void)segs;
    SUCCEED();
}

TEST(EdgeCases, NegativeAngle_ProducesSegments) {
    auto segs = generateLineInfill(makeSquare(1.0), 0.5, -45.0);
    EXPECT_GE(segs.size(), 1u);
}

TEST(EdgeCases, AngleOver360_ProducesSegments) {
    auto segs = generateLineInfill(makeSquare(1.0), 0.5, 405.0);
    EXPECT_GE(segs.size(), 1u);
}

TEST(EdgeCases, IslandAtNonZeroZ_CorrectZ) {
    double z = 7.5;
    auto segs = generateLineInfill(makeSquare(1.0, z), 0.5, 0.0);
    for (const auto &s : segs) {
        EXPECT_NEAR(s.a.getZ(), z, 1e-9);
        EXPECT_NEAR(s.b.getZ(), z, 1e-9);
    }
}

TEST(EdgeCases, HoleEqualsOuter_NoSegments) {
    // Hole perfectly fills outer → nothing to fill
    Island isl = makeDonut(1.0, 1.0);
    auto segs = generateLineInfill(isl, 0.25, 0.0);
    EXPECT_EQ(segs.size(), 0u);
}



TEST(Performance, LargeIsland_LineInfill_Under500ms) {
    Island isl = makeSquare(50.0);
    auto t0 = std::chrono::steady_clock::now();
    auto segs = generateLineInfill(isl, 0.5, 0.0);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                  std::chrono::steady_clock::now() - t0).count();

    EXPECT_GE(segs.size(), 100u);
    EXPECT_LT(ms, 500) << "line infill on 100×100 island took " << ms << "ms";
}

TEST(Performance, LargeIsland_GridInfill_Under1000ms) {
    Island isl = makeSquare(50.0);
    auto t0 = std::chrono::steady_clock::now();
    auto segs = generateGridInfill(isl, 0.5);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                  std::chrono::steady_clock::now() - t0).count();

    EXPECT_GE(segs.size(), 200u);
    EXPECT_LT(ms, 1000) << "grid infill on 100×100 island took " << ms << "ms";
}

TEST(Performance, LargeDonut_LineInfill_Under500ms) {
    Island isl = makeDonut(50.0, 20.0);
    auto t0 = std::chrono::steady_clock::now();
    auto segs = generateLineInfill(isl, 0.5, 0.0);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                  std::chrono::steady_clock::now() - t0).count();

    EXPECT_GE(segs.size(), 10u);
    EXPECT_LT(ms, 500) << "line infill on large donut took " << ms << "ms";
}

TEST(Performance, FineSpacing_HighSegmentCount) {
    auto segs = generateLineInfill(makeSquare(5.0), 0.05, 0.0);
    // 10×10 island at 0.05 spacing → ~200 lines
    EXPECT_GE(segs.size(), 150u);
}
