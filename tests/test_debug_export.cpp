#include <gtest/gtest.h>
#include <fstream>
#include <string>

#include "debug_export.h"
#include "v3.h"
#include "lineSegment.h"
#include "sliceLayer.h"
#include "islands.h"

using namespace mesh_slicing::debug;

// Utility: read file into string
static std::string read_file(const std::string &path) {
    std::ifstream f(path);
    return std::string((std::istreambuf_iterator<char>(f)),
                       std::istreambuf_iterator<char>());
}

TEST(DebugExport, FromV3Polylines) {
    std::vector<std::vector<v3>> loops = {
        { v3(0,0,0), v3(1,0,0), v3(1,1,0) },
        { v3(-1,-1,0), v3(-2,-1,0) }
    };

    auto out = from_v3_polylines(loops, "test");

    ASSERT_EQ(out.size(), 2);
    EXPECT_EQ(out[0].tag, "test");
    EXPECT_EQ(out[0].points.size(), 3);
    EXPECT_DOUBLE_EQ(out[0].points[1].x, 1.0);
    EXPECT_DOUBLE_EQ(out[0].points[1].y, 0.0);
}

TEST(DebugExport, FromIslands) {
    Island isl;
    isl.outer.points = { v3(0,0,0), v3(1,0,0), v3(1,1,0) };

    SliceLayer::Polyline hole;
    hole.points = { v3(0.2,0.2,0), v3(0.4,0.2,0), v3(0.4,0.4,0) };
    isl.holes.push_back(hole);

    std::vector<Island> islands = { isl };

    auto out = from_islands(islands);

    ASSERT_EQ(out.size(), 2);

    EXPECT_EQ(out[0].tag, "outer");
    EXPECT_EQ(out[0].points.size(), 3);

    EXPECT_EQ(out[1].tag, "hole");
    EXPECT_EQ(out[1].points.size(), 3);
}

TEST(DebugExport, FromPerimeters) {
    SliceLayer::Polyline p1;
    p1.points = { v3(0,0,0), v3(1,0,0) };

    SliceLayer::Polyline p2;
    p2.points = { v3(2,2,0), v3(3,2,0), v3(3,3,0) };

    std::vector<std::vector<SliceLayer::Polyline>> perims = {
        { p1 },
        { p2 }
    };

    auto out = from_perimeters(perims);

    ASSERT_EQ(out.size(), 2);
    EXPECT_EQ(out[0].tag, "perimeter");
    EXPECT_EQ(out[1].tag, "perimeter");
    EXPECT_EQ(out[1].points.size(), 3);
}

TEST(DebugExport, FromInfillSegments) {
    std::vector<lineSegment> segs = {
        lineSegment(v3(0,0,0), v3(1,1,0))
    };

    auto out = from_infill_segments(segs);

    ASSERT_EQ(out.size(), 1);
    EXPECT_EQ(out[0].tag, "infill");
    ASSERT_EQ(out[0].points.size(), 2);
    EXPECT_DOUBLE_EQ(out[0].points[1].x, 1.0);
    EXPECT_DOUBLE_EQ(out[0].points[1].y, 1.0);
}

TEST(DebugExport, ExportJSON) {
    std::vector<TaggedPolyline> polys = {
        { { {0,0}, {1,0} }, "outer" }
    };

    export_json("test_debug.json", polys);

    std::string s = read_file("test_debug.json");

    ASSERT_FALSE(s.empty());
    EXPECT_NE(s.find("\"outer\""), std::string::npos);
    EXPECT_NE(s.find("[0.000000, 0.000000]"), std::string::npos);
}

TEST(DebugExport, ExportSVG) {
    std::vector<TaggedPolyline> polys = {
        { { {0,0}, {1,0}, {1,1} }, "outer" }
    };

    export_svg("test_debug.svg", polys);

    std::string s = read_file("test_debug.svg");

    ASSERT_FALSE(s.empty());
    EXPECT_NE(s.find("<polyline"), std::string::npos);
    EXPECT_NE(s.find("#1f77b4"), std::string::npos); // outer color
    EXPECT_NE(s.find("0,0"), std::string::npos);
}
