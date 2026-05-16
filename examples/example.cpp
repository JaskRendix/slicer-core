#include <iostream>
#include "triangleMesh.h"
#include "sliceLayer.h"
#include "triangle.h"
#include "Plane.h"
#include "offset.h"
#include "multiSlice.h"
#include "winding.h"

// NEW:
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

    double offsetDist = 0.1;

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

        // Print original polylines
        std::cout << "Original polylines: " << layer.polylines.size() << "\n";
        for (std::size_t i = 0; i < layer.polylines.size(); ++i) {
            std::cout << "  Polyline " << i
                      << "  winding=" << (winding::isCCW(layer.polylines[i]) ? "CCW" : "CW")
                      << "\n";

            for (auto &p : layer.polylines[i].points) {
                std::cout << "    (" << p.getX()
                          << ", " << p.getY()
                          << ", " << p.getZ() << ")\n";
            }
        }

        // Compute offsets
        auto offsetPolys = offset::offsetLayerPolylines(layer.polylines, offsetDist);

        // Print offset polylines
        std::cout << "Offset polylines: " << offsetPolys.size() << "\n";
        for (std::size_t i = 0; i < offsetPolys.size(); ++i) {
            std::cout << "  Offset polyline " << i << ":\n";
            for (auto &p : offsetPolys[i].points) {
                std::cout << "    (" << p.getX()
                          << ", " << p.getY()
                          << ", " << p.getZ() << ")\n";
            }
        }

        //
        // === NEW: DEBUG VISUALIZATION EXPORT ===
        //
        std::vector<mesh_slicing::debug::TaggedPolyline> dbg;

        // Original layer polylines
        {
            std::vector<std::vector<v3>> loops;
            loops.reserve(layer.polylines.size());
            for (const auto &pl : layer.polylines)
                loops.push_back(pl.points);

            auto tagged = mesh_slicing::debug::from_v3_polylines(loops, "original");
            dbg.insert(dbg.end(), tagged.begin(), tagged.end());
        }

        // Offset polylines
        {
            std::vector<std::vector<v3>> loops;
            loops.reserve(offsetPolys.size());
            for (const auto &pl : offsetPolys)
                loops.push_back(pl.points);

            auto tagged = mesh_slicing::debug::from_v3_polylines(loops, "offset");
            dbg.insert(dbg.end(), tagged.begin(), tagged.end());
        }

        // Export SVG + JSON for this layer
        {
            std::string svgName  = "layer_" + std::to_string(layerIndex) + ".svg";
            std::string jsonName = "layer_" + std::to_string(layerIndex) + ".json";

            mesh_slicing::debug::export_svg(svgName, dbg);
            mesh_slicing::debug::export_json(jsonName, dbg);

            std::cout << "Exported debug: " << svgName << " and " << jsonName << "\n";
        }

        ++layerIndex;
    }

    return 0;
}
