#include "islands.h"
#include "winding.h"
#include <cmath>
#include <limits>

static bool pointOnSegment(double x, double y, double x1, double y1, double x2,
                           double y2) {
  const double EPS = 1e-9;

  // Collinearity check
  double cross = (x - x1) * (y2 - y1) - (y - y1) * (x2 - x1);
  if (std::abs(cross) > EPS)
    return false;

  // Dot product check (within segment bounds)
  double dot = (x - x1) * (x2 - x1) + (y - y1) * (y2 - y1);
  if (dot < -EPS)
    return false;

  double len2 = (x2 - x1) * (x2 - x1) + (y2 - y1) * (y2 - y1);
  if (dot > len2 + EPS)
    return false;

  return true;
}

static bool pointInPoly(const SliceLayer::Polyline &poly, double x, double y) {
  bool inside = false;
  const auto &pts = poly.points;
  size_t n = pts.size();
  if (n < 3)
    return false;

  for (size_t i = 0, j = n - 1; i < n; j = i++) {
    double xi = pts[i].getX(), yi = pts[i].getY();
    double xj = pts[j].getX(), yj = pts[j].getY();

    if (pointOnSegment(x, y, xi, yi, xj, yj))
      return true;

    bool intersect = ((yi > y) != (yj > y)) &&
                     (x < (xj - xi) * (y - yi) / (yj - yi + 1e-12) + xi);

    if (intersect)
      inside = !inside;
  }
  return inside;
}

std::vector<Island>
buildIslands(const std::vector<SliceLayer::Polyline> &polys) {

  // Normalize: CCW = outer, CW = hole
  std::vector<SliceLayer::Polyline> outers;
  std::vector<SliceLayer::Polyline> holes;

  for (auto p : polys) {
    if (p.points.size() < 4)
      continue;

    double a = winding::signedArea(p);

    if (a > 0.0) {
      // CCW outer
      outers.push_back(p);
    } else if (a < 0.0) {
      // CW hole
      holes.push_back(p);
    }
  }

  // Create islands
  std::vector<Island> islands;
  islands.reserve(outers.size());
  for (const auto &o : outers)
    islands.push_back(Island{o, {}});

  // Assign holes to smallest containing outer
  for (const auto &h : holes) {
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
          best = static_cast<int>(i);
        }
      }
    }

    if (best >= 0)
      islands[best].holes.push_back(h);
  }

  return islands;
}
