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

TEST(DebugExport, ExportSVG_FlipY) {
    std::vector<TaggedPolyline> polys = {
        { { {0,0}, {0,10} }, "outer" }
    };

    SvgExportOptions opt;
    opt.flip_y = true;

    export_svg("test_flip_y.svg", polys, opt);
    std::string s = read_file("test_flip_y.svg");

    ASSERT_FALSE(s.empty());
    // Y should be negated
    EXPECT_NE(s.find("0,-10"), std::string::npos);
}

TEST(DebugExport, ExportSVG_DrawGrid) {
    std::vector<TaggedPolyline> polys = {
        { { {0,0}, {10,0}, {10,10} }, "outer" }
    };

    SvgExportOptions opt;
    opt.draw_grid = true;

    export_svg("test_grid.svg", polys, opt);
    std::string s = read_file("test_grid.svg");

    ASSERT_FALSE(s.empty());
    EXPECT_NE(s.find("<line"), std::string::npos);   // grid lines exist
    EXPECT_NE(s.find("#e5e5e5"), std::string::npos); // grid color
}

TEST(DebugExport, ExportSVG_ScaleBar) {
    std::vector<TaggedPolyline> polys = {
        { { {0,0}, {10,0} }, "outer" }
    };

    SvgExportOptions opt;
    opt.draw_scale_bar = true;

    export_svg("test_scale_bar.svg", polys, opt);
    std::string s = read_file("test_scale_bar.svg");

    ASSERT_FALSE(s.empty());
    EXPECT_NE(s.find("10 units"), std::string::npos);
}

TEST(DebugExport, ExportSVG_LayerZAnnotation) {
    std::vector<TaggedPolyline> polys = {
        { { {0,0}, {1,0} }, "outer" }
    };

    SvgExportOptions opt;
    opt.layer_z = 42.5;

    export_svg("test_layer_z.svg", polys, opt);
    std::string s = read_file("test_layer_z.svg");

    ASSERT_FALSE(s.empty());
    EXPECT_NE(s.find("Z = 42.50"), std::string::npos);
}

TEST(DebugExport, ExportLayersSVG) {
    LayerData L0;
    L0.z = 0.0;
    L0.polylines = { { { {0,0}, {1,0} }, "outer" } };

    LayerData L1;
    L1.z = 1.0;
    L1.polylines = { { { {0,1}, {1,1} }, "outer" } };

    std::vector<LayerData> layers = { L0, L1 };

    SvgExportOptions opt;
    export_layers_svg("test_layers", layers, opt);

    std::string s0 = read_file("test_layers_000.svg");
    std::string s1 = read_file("test_layers_001.svg");

    ASSERT_FALSE(s0.empty());
    ASSERT_FALSE(s1.empty());

    EXPECT_NE(s0.find("Z = 0.00"), std::string::npos);
    EXPECT_NE(s1.find("Z = 1.00"), std::string::npos);
}

TEST(DebugExport, ExportLayersJSON) {
    LayerData L0;
    L0.z = 0.0;
    L0.polylines = { { { {0,0}, {1,0} }, "outer" } };

    LayerData L1;
    L1.z = 1.0;
    L1.polylines = { { { {0,1}, {1,1} }, "outer" } };

    std::vector<LayerData> layers = { L0, L1 };

    export_layers_json("test_layers.json", layers);
    std::string s = read_file("test_layers.json");

    ASSERT_FALSE(s.empty());
    EXPECT_NE(s.find("\"layer_count\": 2"), std::string::npos);
    EXPECT_NE(s.find("\"layer_z\": 1.000000"), std::string::npos);
}

TEST(DebugExport, ExportSVG_EmptyPolylines) {
    std::vector<TaggedPolyline> polys;
    export_svg("test_empty.svg", polys);

    std::string s = read_file("test_empty.svg");
    ASSERT_NE(s.find("<svg"), std::string::npos);
    ASSERT_NE(s.find("</svg>"), std::string::npos);
}

TEST(DebugExport, ExportJSON_EmptyPolylines) {
    std::vector<TaggedPolyline> polys;
    export_json("test_empty.json", polys);

    std::string s = read_file("test_empty.json");
    ASSERT_NE(s.find("\"polyline_count\": 0"), std::string::npos);
}

TEST(DebugExport, ExportSVG_SinglePointPolyline) {
    std::vector<TaggedPolyline> polys = {
        { { {0,0} }, "outer" }
    };

    export_svg("test_single_point.svg", polys);
    std::string s = read_file("test_single_point.svg");

    // Should not crash, but also not produce a polyline
    EXPECT_EQ(s.find("<polyline"), std::string::npos);
}

TEST(DebugExport, FromPerimeters_Degenerate) {
    SliceLayer::Polyline p;
    p.points = { v3(0,0,0) }; // degenerate

    std::vector<std::vector<SliceLayer::Polyline>> perims = { { p } };
    auto out = from_perimeters(perims);

    ASSERT_EQ(out.size(), 1);
    EXPECT_EQ(out[0].points.size(), 1);
}

TEST(DebugExport, ExportSVG_MultipleTagsColors) {
    std::vector<TaggedPolyline> polys = {
        { { {0,0}, {1,0} }, "outer" },
        { { {2,0}, {3,0} }, "hole" },
        { { {4,0}, {5,0} }, "perimeter" },
        { { {6,0}, {7,0} }, "infill" }
    };

    export_svg("test_colors.svg", polys);
    std::string s = read_file("test_colors.svg");

    EXPECT_NE(s.find("#1f77b4"), std::string::npos); // outer
    EXPECT_NE(s.find("#ff7f0e"), std::string::npos); // hole
    EXPECT_NE(s.find("#2ca02c"), std::string::npos); // perimeter
    EXPECT_NE(s.find("#d62728"), std::string::npos); // infill
}

TEST(DebugExport, ExportSVG_AllOptionsCombined) {
    std::vector<TaggedPolyline> polys = {
        { { {0,0}, {10,0}, {10,10} }, "outer" }
    };

    SvgExportOptions opt;
    opt.flip_y = true;
    opt.draw_grid = true;
    opt.draw_scale_bar = true;
    opt.layer_z = 12.34;

    export_svg("test_all_options.svg", polys, opt);
    std::string s = read_file("test_all_options.svg");

    EXPECT_NE(s.find("Z = 12.34"), std::string::npos);
    EXPECT_NE(s.find("#e5e5e5"), std::string::npos); // grid
    EXPECT_NE(s.find("10 units"), std::string::npos); // scale bar
    EXPECT_NE(s.find("0,-10"), std::string::npos); // flip_y
}

TEST(DebugExport, ExportLayersSVG_MixedTags) {
    LayerData L0;
    L0.z = 0.0;
    L0.polylines = {
        { { {0,0}, {1,0} }, "outer" }
    };

    LayerData L1;
    L1.z = 1.0;
    L1.polylines = {
        { { {0,1}, {1,1} }, "infill" }
    };

    std::vector<LayerData> layers = { L0, L1 };

    SvgExportOptions opt;
    export_layers_svg("test_layers_mixed", layers, opt);

    std::string s0 = read_file("test_layers_mixed_000.svg");
    std::string s1 = read_file("test_layers_mixed_001.svg");

    EXPECT_NE(s0.find("#1f77b4"), std::string::npos); // outer color
    EXPECT_NE(s1.find("#d62728"), std::string::npos); // infill color
}

TEST(DebugExport, ExportLayersJSON_MixedTags) {
    LayerData L0;
    L0.z = 0.0;
    L0.polylines = {
        { { {0,0}, {1,0} }, "outer" }
    };

    LayerData L1;
    L1.z = 1.0;
    L1.polylines = {
        { { {0,1}, {1,1} }, "hole" }
    };

    std::vector<LayerData> layers = { L0, L1 };

    export_layers_json("test_layers_mixed.json", layers);
    std::string s = read_file("test_layers_mixed.json");

    EXPECT_NE(s.find("\"tag\": \"outer\""), std::string::npos);
    EXPECT_NE(s.find("\"tag\": \"hole\""), std::string::npos);
}

TEST(DebugExport, ExportSVG_StressManyPolylines) {
    std::vector<TaggedPolyline> polys;

    for (int i = 0; i < 500; ++i) {
        TaggedPolyline pl;
        pl.tag = "outer";
        pl.points = { {double(i),0}, {double(i),1} };
        polys.push_back(pl);
    }

    export_svg("test_stress.svg", polys);
    std::string s = read_file("test_stress.svg");

    ASSERT_FALSE(s.empty());
    EXPECT_NE(s.find("<polyline"), std::string::npos);
}
