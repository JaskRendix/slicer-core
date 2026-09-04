#include "islands.h"
#include "winding.h"
#include <algorithm>
#include <cmath>
#include <limits>

struct AABB {
  double minX, minY, maxX, maxY;
  bool contains(double x, double y) const {
    return x >= minX && x <= maxX && y >= minY && y <= maxY;
  }
};

static AABB computeAABB(const SliceLayer::Polyline &poly) {
  double minx = std::numeric_limits<double>::infinity();
  double miny = std::numeric_limits<double>::infinity();
  double maxx = -std::numeric_limits<double>::infinity();
  double maxy = -std::numeric_limits<double>::infinity();
  for (const auto &pt : poly.points) {
    double x = pt.getX();
    double y = pt.getY();
    minx = std::min(minx, x);
    miny = std::min(miny, y);
    maxx = std::max(maxx, x);
    maxy = std::max(maxy, y);
  }
  return {minx, miny, maxx, maxy};
}

static bool pointOnSegment(double x, double y, double x1, double y1, double x2,
                           double y2) {
  const double EPS = 1e-9;

  double cross = (x - x1) * (y2 - y1) - (y - y1) * (x2 - x1);
  if (std::abs(cross) > EPS)
    return false;

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

    if ((yi > y) != (yj > y)) {
      double dy = yj - yi;
      if (std::abs(dy) > 1e-14) {
        double intersectX = (xj - xi) * (y - yi) / dy + xi;
        if (x < intersectX) {
          inside = !inside;
        }
      }
    }
  }
  return inside;
}

std::vector<Island>
buildIslands(const std::vector<SliceLayer::Polyline> &polys) {
  std::vector<SliceLayer::Polyline> outers;
  std::vector<SliceLayer::Polyline> holes;
  outers.reserve(polys.size());
  holes.reserve(polys.size());

  for (const auto &p : polys) {
    if (p.points.size() < 4)
      continue;

    double a = winding::signedArea(p);

    if (a > 0.0) {
      outers.push_back(p);
    } else if (a < 0.0) {
      holes.push_back(p);
    }
  }

  std::vector<Island> islands;
  islands.reserve(outers.size());
  std::vector<AABB> outerAABBs;
  outerAABBs.reserve(outers.size());

  for (const auto &o : outers) {
    islands.push_back(Island{o, {}});
    outerAABBs.push_back(computeAABB(o));
  }

  for (const auto &h : holes) {
    double cx = 0.0, cy = 0.0;
    size_t hSize = h.points.size();
    for (const auto &pt : h.points) {
      cx += pt.getX();
      cy += pt.getY();
    }
    if (hSize > 0) {
      cx /= static_cast<double>(hSize);
      cy /= static_cast<double>(hSize);
    }

    int best = -1;
    double bestArea = std::numeric_limits<double>::infinity();

    for (size_t i = 0; i < islands.size(); ++i) {
      if (!outerAABBs[i].contains(cx, cy))
        continue;

      const auto &outer = islands[i].outer;

      if (pointInPoly(outer, cx, cy)) {
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
