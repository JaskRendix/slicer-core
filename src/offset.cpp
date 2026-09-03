#include "offset.h"

namespace offset {

std::vector<SliceLayer::Polyline>
offsetPolyline(const SliceLayer::Polyline &poly, double distance,
               Clipper2Lib::JoinType joinType, double miterLimit) {
  return offsetLayerPolylines({poly}, distance, joinType, miterLimit);
}

std::vector<SliceLayer::Polyline>
offsetLayerPolylines(const std::vector<SliceLayer::Polyline> &polys,
                     double distance, Clipper2Lib::JoinType joinType,
                     double miterLimit) {
  if (polys.empty())
    return {};

  double z = 0.0;
  for (const auto &p : polys) {
    if (!p.points.empty()) {
      z = p.points[0].getZ();
      break;
    }
  }

  Clipper2Lib::PathsD input;
  input.reserve(polys.size());
  for (const auto &poly : polys) {
    if (poly.points.size() < 3)
      continue;
    Clipper2Lib::PathD path;
    path.reserve(poly.points.size());
    for (const auto &p : poly.points) {
      path.push_back({p.getX(), p.getY()});
    }
    input.push_back(std::move(path));
  }

  if (input.empty())
    return {};

  auto result = Clipper2Lib::InflatePaths(
      input, distance, joinType, Clipper2Lib::EndType::Polygon, miterLimit);

  std::vector<SliceLayer::Polyline> out;
  out.reserve(result.size());
  for (const auto &path : result) {
    if (path.empty())
      continue;

    SliceLayer::Polyline p_out;
    p_out.points.reserve(path.size() + 1);
    for (const auto &pt : path) {
      p_out.points.emplace_back(pt.x, pt.y, z);
    }

    const auto &f = p_out.points.front();
    const auto &b = p_out.points.back();
    if (f.getX() != b.getX() || f.getY() != b.getY() || f.getZ() != b.getZ()) {
      p_out.points.push_back(f);
    }
    out.push_back(std::move(p_out));
  }

  return out;
}

} // namespace offset