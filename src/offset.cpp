#include "offset.h"
#include "clipper2/clipper.h"

namespace offset {

SliceLayer::Polyline offsetPolyline(const SliceLayer::Polyline &poly,
                                    double distance) {
  SliceLayer::Polyline out;

  if (poly.points.size() < 3)
    return poly;

  double z = poly.points[0].getZ();

  Clipper2Lib::PathsD input(1);
  for (const auto &p : poly.points)
    input[0].push_back({p.getX(), p.getY()});

  auto result =
      Clipper2Lib::InflatePaths(input, distance, Clipper2Lib::JoinType::Miter,
                                Clipper2Lib::EndType::Polygon);

  if (result.empty())
    return out;

  for (const auto &pt : result[0])
    out.points.emplace_back(pt.x, pt.y, z);

  // Close the polygon if Clipper2 didn't
  if (!out.points.empty()) {
    const auto &f = out.points.front();
    const auto &b = out.points.back();
    if (f.getX() != b.getX() || f.getY() != b.getY() || f.getZ() != b.getZ())
      out.points.push_back(f);
  }

  return out;
}

std::vector<SliceLayer::Polyline>
offsetLayerPolylines(const std::vector<SliceLayer::Polyline> &polys,
                     double distance) {
  std::vector<SliceLayer::Polyline> out;
  out.reserve(polys.size());
  for (const auto &p : polys)
    out.push_back(offsetPolyline(p, distance));
  return out;
}

} // namespace offset