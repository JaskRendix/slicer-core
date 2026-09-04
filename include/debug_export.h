#pragma once

#include <string>
#include <vector>

#include "infill.h"
#include "islands.h"
#include "lineSegment.h"
#include "perimeters.h"
#include "sliceLayer.h"
#include "v3.h"

namespace mesh_slicing::debug {

// Simple 2D point for export
struct Point2 {
  double x;
  double y;
};

// Polyline with a semantic tag (outer, hole, perimeter, infill, etc.)
struct TaggedPolyline {
  std::vector<Point2> points;
  std::string tag; // "outer", "hole", "perimeter", "infill", ...
};

// Configuration options for SVG export
struct SvgExportOptions {
  double stroke_width = 0.1;
  bool flip_y = false;
  bool draw_grid = false;
  bool draw_scale_bar = false;
  double layer_z = 0.0;
};

// Multi-layer container for batch exports
struct LayerData {
  double z;
  std::vector<TaggedPolyline> polylines;
};

// Export a set of tagged polylines to an SVG file with options.
void export_svg(const std::string &filename,
                const std::vector<TaggedPolyline> &polylines,
                const SvgExportOptions &options = {});

// Backward compatibility overload for stroke width
void export_svg(const std::string &filename,
                const std::vector<TaggedPolyline> &polylines,
                double stroke_width);

// Export a set of tagged polylines to JSON with layer metadata.
void export_json(const std::string &filename,
                 const std::vector<TaggedPolyline> &polylines,
                 double layer_z = 0.0);

// Batch export multiple slices to individual SVG files (e.g., prefix_000.svg)
void export_layers_svg(const std::string &filename_prefix,
                       const std::vector<LayerData> &layers,
                       const SvgExportOptions &options = {});

// Batch export multiple slices to a single JSON file
void export_layers_json(const std::string &filename,
                        const std::vector<LayerData> &layers);

// From a generic polyline of v3 (assuming z is constant / ignored)
std::vector<TaggedPolyline>
from_v3_polylines(const std::vector<std::vector<v3>> &loops,
                  const std::string &tag);

// From islands (outer + holes)
std::vector<TaggedPolyline> from_islands(const std::vector<Island> &islands);

// From perimeters: vector< vector<SliceLayer::Polyline> >
std::vector<TaggedPolyline> from_perimeters(
    const std::vector<std::vector<SliceLayer::Polyline>> &perimeters);

// From infill segments
std::vector<TaggedPolyline>
from_infill_segments(const std::vector<lineSegment> &segments);

} // namespace mesh_slicing::debug
