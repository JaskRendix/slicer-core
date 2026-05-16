#include "debug_export.h"

#include <algorithm>
#include <fstream>
#include <iomanip>
#include <sstream>

namespace mesh_slicing::debug {

namespace {

std::string color_for_tag(const std::string &tag) {
  if (tag == "outer")
    return "#1f77b4";
  if (tag == "hole")
    return "#ff7f0e";
  if (tag == "perimeter")
    return "#2ca02c";
  if (tag == "infill")
    return "#d62728";
  if (tag == "support")
    return "#9467bd";
  return "#7f7f7f";
}

} // namespace

void export_svg(const std::string &filename,
                const std::vector<TaggedPolyline> &polylines,
                double stroke_width) {
  if (polylines.empty()) {
    std::ofstream empty_out(filename);
    empty_out << "<svg xmlns=\"http://www.w3.org/2000/svg\"></svg>\n";
    return;
  }

  double min_x = polylines[0].points[0].x;
  double max_x = min_x;
  double min_y = polylines[0].points[0].y;
  double max_y = min_y;

  for (const auto &pl : polylines) {
    for (const auto &p : pl.points) {
      min_x = std::min(min_x, p.x);
      max_x = std::max(max_x, p.x);
      min_y = std::min(min_y, p.y);
      max_y = std::max(max_y, p.y);
    }
  }

  const double padding = 5.0;
  const double width = (max_x - min_x) + 2 * padding;
  const double height = (max_y - min_y) + 2 * padding;

  std::ofstream out(filename);
  out << "<svg xmlns=\"http://www.w3.org/2000/svg\" " << "width=\"" << width
      << "\" height=\"" << height << "\" " << "viewBox=\"" << (min_x - padding)
      << " " << (min_y - padding) << " " << width << " " << height << "\">\n";

  out << "<g fill=\"none\">\n";

  for (const auto &pl : polylines) {
    if (pl.points.size() < 2)
      continue;

    const std::string color = color_for_tag(pl.tag);

    out << "<polyline stroke=\"" << color << "\" " << "stroke-width=\""
        << stroke_width << "\" " << "points=\"";

    for (const auto &p : pl.points) {
      // Flip Y for SVG if you like; here we keep coordinates as-is
      out << p.x << "," << p.y << " ";
    }

    out << "\" />\n";
  }

  out << "</g>\n</svg>\n";
}

void export_json(const std::string &filename,
                 const std::vector<TaggedPolyline> &polylines) {
  std::ofstream out(filename);
  out << std::fixed << std::setprecision(6);
  out << "{ \"polylines\": [\n";

  for (std::size_t i = 0; i < polylines.size(); ++i) {
    const auto &pl = polylines[i];
    out << "  { \"tag\": \"" << pl.tag << "\", \"points\": [";

    for (std::size_t j = 0; j < pl.points.size(); ++j) {
      const auto &p = pl.points[j];
      out << "[" << p.x << ", " << p.y << "]";
      if (j + 1 < pl.points.size())
        out << ", ";
    }

    out << "] }";
    if (i + 1 < polylines.size())
      out << ",";
    out << "\n";
  }

  out << "] }\n";
}

std::vector<TaggedPolyline>
from_v3_polylines(const std::vector<std::vector<v3>> &loops,
                  const std::string &tag) {
  std::vector<TaggedPolyline> out;
  out.reserve(loops.size());

  for (const auto &loop : loops) {
    TaggedPolyline pl;
    pl.tag = tag;
    pl.points.reserve(loop.size());
    for (const auto &v : loop) {
      pl.points.push_back(Point2{v.getX(), v.getY()});
    }
    out.push_back(std::move(pl));
  }

  return out;
}

std::vector<TaggedPolyline> from_islands(const std::vector<Island> &islands) {
  std::vector<TaggedPolyline> out;

  for (const auto &isl : islands) {
    // outer
    {
      TaggedPolyline outer_pl;
      outer_pl.tag = "outer";
      outer_pl.points.reserve(isl.outer.points.size());
      for (const auto &v : isl.outer.points) {
        outer_pl.points.push_back(Point2{v.getX(), v.getY()});
      }
      out.push_back(std::move(outer_pl));
    }

    // holes
    for (const auto &hole : isl.holes) {
      TaggedPolyline hole_pl;
      hole_pl.tag = "hole";
      hole_pl.points.reserve(hole.points.size());
      for (const auto &v : hole.points) {
        hole_pl.points.push_back(Point2{v.getX(), v.getY()});
      }
      out.push_back(std::move(hole_pl));
    }
  }

  return out;
}

std::vector<TaggedPolyline> from_perimeters(
    const std::vector<std::vector<SliceLayer::Polyline>> &perimeters) {
  std::vector<TaggedPolyline> out;

  for (const auto &ring : perimeters) {
    for (const auto &poly : ring) {
      TaggedPolyline pl;
      pl.tag = "perimeter";
      pl.points.reserve(poly.points.size());
      for (const auto &v : poly.points) {
        pl.points.push_back(Point2{v.getX(), v.getY()});
      }
      out.push_back(std::move(pl));
    }
  }

  return out;
}

std::vector<TaggedPolyline>
from_infill_segments(const std::vector<lineSegment> &segments) {
  std::vector<TaggedPolyline> out;
  out.reserve(segments.size());

  for (const auto &seg : segments) {
    TaggedPolyline pl;
    pl.tag = "infill";
    const v3 &a = seg.a();
    const v3 &b = seg.b();
    pl.points.push_back(Point2{a.getX(), a.getY()});
    pl.points.push_back(Point2{b.getX(), b.getY()});
    out.push_back(std::move(pl));
  }

  return out;
}

} // namespace mesh_slicing::debug
