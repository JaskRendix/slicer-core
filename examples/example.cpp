#include <iostream>
#include <algorithm>
#include "triangleMesh.h"
#include "sliceLayer.h"
#include "triangle.h"
#include "Plane.h"
#include "offset.h"
#include "multiSlice.h"
#include "winding.h"
#include "islands.h"
#include "perimeters.h"
#include "infill.h"
#include "debug_export.h"

// Proper degeneracy check: area == 0 or too few points
static bool isDegenerate(const SliceLayer::Polyline &poly) {
    if (poly.points.size() < 4)  // 3 + repeated first
        return true;

    double area = 0.0;
    for (size_t i = 0; i + 1 < poly.points.size(); ++i) {
        const auto &p = poly.points[i];
        const auto &q = poly.points[i + 1];
        area += p.getX() * q.getY() - q.getX() * p.getY();
    }

    return std::abs(area) < 1e-9;
}

int main() {
    triangleMesh mesh;

    // Simple pyramid
    mesh.push_back(triangle(v3(0,0,1), v3(-1,-1,-1), v3(1,-1,-1), v3(0,0,1)));
    mesh.push_back(triangle(v3(0,0,1), v3(1,-1,-1),  v3(1,1,-1),  v3(0,0,1)));
    mesh.push_back(triangle(v3(0,0,1), v3(1,1,-1),   v3(-1,1,-1), v3(0,0,1)));
    mesh.push_back(triangle(v3(0,0,1), v3(-1,1,-1),  v3(-1,-1,-1),v3(0,0,1)));

    double layerHeight = 0.25;
    auto layers = sliceMeshMultiLayer(mesh, layerHeight);

    std::cout << "Layers: " << layers.size() << "\n";

    size_t layerIndex = 0;
    for (const auto &layer : layers) {
        std::cout << "\n=== Layer Z = " << layer.z << " ===\n";

        // Skip degenerate apex layers
        bool allDegenerate = true;
        for (const auto &p : layer.polylines)
            if (!isDegenerate(p)) allDegenerate = false;

        if (allDegenerate) {
            std::cout << "(degenerate apex layer, skipping)\n";
            ++layerIndex;
            continue;
        }

        // Normalize winding to CCW for island building if needed
        auto normalizedPolys = layer.polylines;
        for (auto &poly : normalizedPolys) {
            if (!winding::isCCW(poly)) {
                std::reverse(poly.points.begin(), poly.points.end());
            }
        }

        // 1. Build Islands (Outer + Holes with AABB pruning)
        auto islands = buildIslands(normalizedPolys);
        std::cout << "Islands built: " << islands.size() << "\n";

        // 2. Generate Perimeters & Infill per island
        std::vector<std::vector<SliceLayer::Polyline>> allPerims;
        std::vector<InfillSegment> allInfill;

        for (const auto &island : islands) {
            auto perims = generatePerimeters(island, 1, 0.1);
            for (const auto &ring : perims) {
                allPerims.push_back(ring);
            }

            auto infillSegs = generateGridInfill(island, 0.2);
            allInfill.insert(allInfill.end(), infillSegs.begin(), infillSegs.end());
        }

        // 3. Assemble Debug Tagged Polylines for Visualization
        std::vector<mesh_slicing::debug::TaggedPolyline> dbg;

        auto islandPolys = mesh_slicing::debug::from_islands(islands);
        dbg.insert(dbg.end(), islandPolys.begin(), islandPolys.end());

        auto perimeterPolys = mesh_slicing::debug::from_perimeters(allPerims);
        dbg.insert(dbg.end(), perimeterPolys.begin(), perimeterPolys.end());

        for (const auto &seg : allInfill) {
            mesh_slicing::debug::TaggedPolyline pl;
            pl.tag = "infill";
            pl.points.push_back({seg.a.getX(), seg.a.getY()});
            pl.points.push_back({seg.b.getX(), seg.b.getY()});
            dbg.push_back(pl);
        }

        // 4. Configure Advanced SVG Export Options
        mesh_slicing::debug::SvgExportOptions options;
        options.stroke_width = 0.05;
        options.draw_grid = true;
        options.draw_scale_bar = true;
        options.layer_z = layer.z;

        // Export SVG + JSON with metadata for this layer (inside build/ directory)
        std::string svgName  = "layer_" + std::to_string(layerIndex) + ".svg";
        std::string jsonName = "layer_" + std::to_string(layerIndex) + ".json";

        mesh_slicing::debug::export_svg(svgName, dbg, options);
        mesh_slicing::debug::export_json(jsonName, dbg, layer.z);

        std::cout << "Exported composited debug: " << svgName << " and " << jsonName << "\n";

        ++layerIndex;
    }

    return 0;
}
