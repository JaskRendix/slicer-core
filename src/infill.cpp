#include "infill.h"
#include <cmath>

// 2D helpers
struct Vec2 {
  double x, y;
};

static Vec2 to2(const v3 &p) { return {p.getX(), p.getY()}; }
static v3 to3(const Vec2 &p, double z) { return v3(p.x, p.y, z); }

static Vec2 rotate(const Vec2 &p, double c, double s) {
  return {p.x * c - p.y * s, p.x * s + p.y * c};
}

// Ray casting point-in-poly
static bool pointInPoly(const std::vector<Vec2> &poly, const Vec2 &pt) {
  bool inside = false;
  size_t n = poly.size();
  for (size_t i = 0, j = n - 1; i < n; j = i++) {
    const Vec2 &a = poly[i];
    const Vec2 &b = poly[j];
    bool intersect =
        ((a.y > pt.y) != (b.y > pt.y)) &&
        (pt.x < (b.x - a.x) * (pt.y - a.y) / (b.y - a.y + 1e-12) + a.x);
    if (intersect)
      inside = !inside;
  }
  return inside;
}

// Clip a line segment against polygon (outer minus holes)
static std::vector<Vec2> clipLine(const Vec2 &p0, const Vec2 &p1,
                                  const std::vector<Vec2> &outer,
                                  const std::vector<std::vector<Vec2>> &holes) {
  std::vector<Vec2> result;

  // Sample along the line (simple but works well)
  const int steps = 200;
  for (int i = 0; i <= steps; ++i) {
    double t = double(i) / steps;
    Vec2 p = {p0.x + (p1.x - p0.x) * t, p0.y + (p1.y - p0.y) * t};

    bool insideOuter = pointInPoly(outer, p);
    bool insideHole = false;
    for (auto &h : holes)
      if (pointInPoly(h, p))
        insideHole = true;

    if (insideOuter && !insideHole)
      result.push_back(p);
    else
      result.push_back({NAN, NAN});
  }

  // Convert runs of valid points into segments
  std::vector<Vec2> segments;
  bool inSeg = false;
  Vec2 start;

  for (auto &p : result) {
    if (!std::isnan(p.x)) {
      if (!inSeg) {
        start = p;
        inSeg = true;
      }
    } else {
      if (inSeg) {
        segments.push_back(start);
        segments.push_back(result[&p - &result[0] - 1]);
        inSeg = false;
      }
    }
  }

  return segments;
}

std::vector<InfillSegment>
generateLineInfill(const Island &island, double spacing, double angleDegrees) {
  std::vector<InfillSegment> out;

  // Convert island to 2D
  std::vector<Vec2> outer2;
  for (auto &p : island.outer.points)
    outer2.push_back(to2(p));

  std::vector<std::vector<Vec2>> holes2;
  for (auto &h : island.holes) {
    std::vector<Vec2> hh;
    for (auto &p : h.points)
      hh.push_back(to2(p));
    holes2.push_back(hh);
  }

  // Rotation
  double angle = angleDegrees * M_PI / 180.0;
  double c = std::cos(angle), s = std::sin(angle);

  for (auto &p : outer2)
    p = rotate(p, c, s);
  for (auto &h : holes2)
    for (auto &p : h)
      p = rotate(p, c, s);

  // Compute bounds
  double minY = 1e9, maxY = -1e9;
  for (auto &p : outer2) {
    minY = std::min(minY, p.y);
    maxY = std::max(maxY, p.y);
  }

  // Generate horizontal lines
  for (double y = minY; y <= maxY; y += spacing) {
    Vec2 p0 = {-1e6, y};
    Vec2 p1 = {1e6, y};

    auto clipped = clipLine(p0, p1, outer2, holes2);

    // Convert clipped segments back to 3D
    for (size_t i = 0; i + 1 < clipped.size(); i += 2) {
      Vec2 a = clipped[i];
      Vec2 b = clipped[i + 1];

      // Rotate back
      Vec2 ar = rotate(a, c, -s);
      Vec2 br = rotate(b, c, -s);

      double z = island.outer.points[0].getZ();
      out.push_back({to3(ar, z), to3(br, z)});
    }
  }

  return out;
}
