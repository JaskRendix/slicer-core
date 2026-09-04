#include "debug_export.h"

#include <algorithm>
#include <cmath>
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
                const SvgExportOptions &options) {
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
      double px = p.x;
      double py = options.flip_y ? -p.y : p.y;
      min_x = std::min(min_x, px);
      max_x = std::max(max_x, px);
      min_y = std::min(min_y, py);
      max_y = std::max(max_y, py);
    }
  }

  const double padding = 5.0;
  const double width = (max_x - min_x) + 2 * padding;
  const double height = (max_y - min_y) + 2 * padding;

  std::ofstream out(filename);
  out << std::fixed << std::setprecision(6);
  out << "<svg xmlns=\"http://www.w3.org/2000/svg\" " << "width=\"" << width
      << "\" height=\"" << height << "\" " << "viewBox=\"" << (min_x - padding)
      << " " << (min_y - padding) << " " << width << " " << height << "\">\n";

  // Optional Grid Background
  if (options.draw_grid) {
    out << "<g stroke=\"#e5e5e5\" stroke-width=\""
        << (options.stroke_width * 0.5) << "\">\n";
    double step = 10.0; // Grid spacing unit
    double start_gx = std::floor(min_x / step) * step;
    double end_gx = std::ceil(max_x / step) * step;
    double start_gy = std::floor(min_y / step) * step;
    double end_gy = std::ceil(max_y / step) * step;

    for (double gx = start_gx; gx <= end_gx; gx += step) {
      out << "  <line x1=\"" << gx << "\" y1=\"" << (min_y - padding)
          << "\" x2=\"" << gx << "\" y2=\"" << (max_y + padding) << "\" />\n";
    }
    for (double gy = start_gy; gy <= end_gy; gy += step) {
      out << "  <line x1=\"" << (min_x - padding) << "\" y1=\"" << gy
          << "\" x2=\"" << (max_x + padding) << "\" y2=\"" << gy << "\" />\n";
    }
    out << "</g>\n";
  }

  out << "<g fill=\"none\">\n";

  for (const auto &pl : polylines) {
    if (pl.points.size() < 2)
      continue;

    const std::string color = color_for_tag(pl.tag);

    out << "<polyline stroke=\"" << color << "\" " << "stroke-width=\""
        << options.stroke_width << "\" " << "points=\"";

    for (const auto &p : pl.points) {
      double px = p.x;
      double py = options.flip_y ? -p.y : p.y;
      out << px << "," << py << " ";
    }

    out << "\" />\n";
  }

  out << "</g>\n";

  // Optional Scale Bar
  if (options.draw_scale_bar) {
    double bar_len = 10.0; // 10 units scale bar
    double bx = min_x;
    double by = max_y + padding - 2.0;
    out << "<g stroke=\"#333333\" stroke-width=\""
        << (options.stroke_width * 1.5) << "\">\n"
        << "  <line x1=\"" << bx << "\" y1=\"" << by << "\" x2=\""
        << (bx + bar_len) << "\" y2=\"" << by << "\" />\n"
        << "  <line x1=\"" << bx << "\" y1=\"" << (by - 1.0) << "\" x2=\"" << bx
        << "\" y2=\"" << (by + 1.0) << "\" />\n"
        << "  <line x1=\"" << (bx + bar_len) << "\" y1=\"" << (by - 1.0)
        << "\" x2=\"" << (bx + bar_len) << "\" y2=\"" << (by + 1.0) << "\" />\n"
        << "</g>\n"
        << "<text x=\"" << bx << "\" y=\"" << (by - 2.0)
        << "\" font-size=\"3\" fill=\"#333333\" font-family=\"sans-serif\">10 "
           "units</text>\n";
  }

  // Layer Z Text Annotation
  {
    std::ostringstream z_oss;
    z_oss << "Z = " << std::fixed << std::setprecision(2) << options.layer_z;
    out << "<text x=\"" << (min_x - padding + 2.0) << "\" y=\""
        << (min_y - padding + 5.0)
        << "\" font-size=\"3.5\" fill=\"#666666\" font-family=\"sans-serif\">"
        << z_oss.str() << "</text>\n";
  }

  out << "</svg>\n";
}

void export_svg(const std::string &filename,
                const std::vector<TaggedPolyline> &polylines,
                double stroke_width) {
  SvgExportOptions options;
  options.stroke_width = stroke_width;
  export_svg(filename, polylines, options);
}

void export_json(const std::string &filename,
                 const std::vector<TaggedPolyline> &polylines, double layer_z) {
  std::ofstream out(filename);
  out << std::fixed << std::setprecision(6);
  out << "{\n";
  out << "  \"layer_z\": " << layer_z << ",\n";
  out << "  \"polyline_count\": " << polylines.size() << ",\n";
  out << "  \"polylines\": [\n";

  for (std::size_t i = 0; i < polylines.size(); ++i) {
    const auto &pl = polylines[i];
    out << "    { \"tag\": \"" << pl.tag << "\", \"points\": [";

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

  out << "  ]\n";
  out << "}\n";
}

void export_layers_svg(const std::string &filename_prefix,
                       const std::vector<LayerData> &layers,
                       const SvgExportOptions &options) {
  for (std::size_t i = 0; i < layers.size(); ++i) {
    std::ostringstream oss;
    oss << filename_prefix << "_" << std::setw(3) << std::setfill('0') << i
        << ".svg";
    SvgExportOptions layer_opts = options;
    layer_opts.layer_z = layers[i].z;
    export_svg(oss.str(), layers[i].polylines, layer_opts);
  }
}

void export_layers_json(const std::string &filename,
                        const std::vector<LayerData> &layers) {
  std::ofstream out(filename);
  out << std::fixed << std::setprecision(6);
  out << "{\n  \"layer_count\": " << layers.size() << ",\n  \"layers\": [\n";

  for (std::size_t i = 0; i < layers.size(); ++i) {
    const auto &layer = layers[i];
    out << "    {\n";
    out << "      \"layer_index\": " << i << ",\n";
    out << "      \"layer_z\": " << layer.z << ",\n";
    out << "      \"polyline_count\": " << layer.polylines.size() << ",\n";
    out << "      \"polylines\": [\n";

    for (std::size_t j = 0; j < layer.polylines.size(); ++j) {
      const auto &pl = layer.polylines[j];
      out << "        { \"tag\": \"" << pl.tag << "\", \"points\": [";

      for (std::size_t k = 0; k < pl.points.size(); ++k) {
        const auto &p = pl.points[k];
        out << "[" << p.x << ", " << p.y << "]";
        if (k + 1 < pl.points.size())
          out << ", ";
      }

      out << "] }";
      if (j + 1 < layer.polylines.size())
        out << ",";
      out << "\n";
    }

    out << "      ]\n    }";
    if (i + 1 < layers.size())
      out << ",";
    out << "\n";
  }

  out << "  ]\n}\n";
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
