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

// Export a set of tagged polylines to a simple SVG file.
void export_svg(const std::string &filename,
                const std::vector<TaggedPolyline> &polylines,
                double stroke_width = 0.1);

// Export a set of tagged polylines to JSON (for tests / tooling).
void export_json(const std::string &filename,
                 const std::vector<TaggedPolyline> &polylines);

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
