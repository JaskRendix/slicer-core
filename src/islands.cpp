#include "islands.h"
#include "winding.h"
#include <limits>

// Point-in-polygon test (ray casting)
static bool pointInPoly(const SliceLayer::Polyline &poly, double x, double y) {
  bool inside = false;
  const auto &pts = poly.points;
  size_t n = pts.size();

  for (size_t i = 0, j = n - 1; i < n; j = i++) {
    double xi = pts[i].getX(), yi = pts[i].getY();
    double xj = pts[j].getX(), yj = pts[j].getY();

    bool intersect = ((yi > y) != (yj > y)) &&
                     (x < (xj - xi) * (y - yi) / (yj - yi + 1e-12) + xi);

    if (intersect)
      inside = !inside;
  }
  return inside;
}

std::vector<Island>
buildIslands(const std::vector<SliceLayer::Polyline> &polys) {
  std::vector<SliceLayer::Polyline> outers;
  std::vector<SliceLayer::Polyline> holes;

  // Separate by winding
  for (const auto &p : polys) {
    if (winding::isCCW(p))
      outers.push_back(p);
    else
      holes.push_back(p);
  }

  // Create islands for each outer
  std::vector<Island> islands;
  islands.reserve(outers.size());
  for (auto &o : outers)
    islands.push_back(Island{o, {}});

  // Assign each hole to the smallest containing outer
  for (const auto &h : holes) {
    // pick a representative point of the hole
    const auto &hp = h.points[0];
    double hx = hp.getX(), hy = hp.getY();

    int best = -1;
    double bestArea = std::numeric_limits<double>::infinity();

    for (size_t i = 0; i < islands.size(); ++i) {
      const auto &outer = islands[i].outer;

      if (pointInPoly(outer, hx, hy)) {
        double area = std::abs(winding::signedArea(outer));
        if (area < bestArea) {
          bestArea = area;
          best = (int)i;
        }
      }
    }

    if (best >= 0)
      islands[best].holes.push_back(h);
  }

  return islands;
}
