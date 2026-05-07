#include "infill.h"
#include <cmath>
#include <limits>

// 2D helpers
struct Vec2 {
  double x, y;
};

static Vec2 to2(const v3 &p) { return {p.getX(), p.getY()}; }
static v3 to3(const Vec2 &p, double z) { return v3(p.x, p.y, z); }

static Vec2 rotate(const Vec2 &p, double c, double s) {
  return {p.x * c - p.y * s, p.x * s + p.y * c};
}

static bool pointInPoly(const std::vector<Vec2> &poly, const Vec2 &pt) {
  bool inside = false;
  size_t n = poly.size();
  if (n < 3)
    return false;
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

static std::vector<Vec2> clipLine(const Vec2 &p0, const Vec2 &p1,
                                  const std::vector<Vec2> &outer,
                                  const std::vector<std::vector<Vec2>> &holes) {
  const int steps = 200;
  std::vector<Vec2> samples;
  samples.reserve(steps + 1);

  for (int i = 0; i <= steps; ++i) {
    double t = double(i) / steps;
    Vec2 p{p0.x + (p1.x - p0.x) * t, p0.y + (p1.y - p0.y) * t};
    bool insideOuter = pointInPoly(outer, p);
    bool insideHole = false;
    for (const auto &h : holes)
      if (pointInPoly(h, p)) {
        insideHole = true;
        break;
      }
    samples.push_back((insideOuter && !insideHole) ? p : Vec2{NAN, NAN});
  }

  std::vector<Vec2> segments;
  bool inSeg = false;
  Vec2 start{};
  for (std::size_t i = 0; i < samples.size(); ++i) {
    const auto &p = samples[i];
    if (!std::isnan(p.x)) {
      if (!inSeg) {
        start = p;
        inSeg = true;
      }
    } else if (inSeg) {
      segments.push_back(start);
      segments.push_back(samples[i - 1]);
      inSeg = false;
    }
  }
  if (inSeg) {
    segments.push_back(start);
    segments.push_back(samples.back());
  }
  return segments;
}

std::vector<InfillSegment>
generateLineInfill(const Island &island, double spacing, double angleDegrees) {
  std::vector<InfillSegment> out;
  if (island.outer.points.size() < 4)
    return out;
  if (spacing <= 0.0) // ← moved here, was buried inside the loop
    return out;

  std::vector<Vec2> outer2;
  outer2.reserve(island.outer.points.size());
  for (const auto &p : island.outer.points)
    outer2.push_back(to2(p));

  std::vector<std::vector<Vec2>> holes2;
  holes2.reserve(island.holes.size());
  for (const auto &h : island.holes) {
    std::vector<Vec2> hh;
    hh.reserve(h.points.size());
    for (const auto &p : h.points)
      hh.push_back(to2(p));
    holes2.push_back(std::move(hh));
  }

  double minX = std::numeric_limits<double>::infinity(),
         maxX = -std::numeric_limits<double>::infinity();
  double minY = std::numeric_limits<double>::infinity(),
         maxY = -std::numeric_limits<double>::infinity();
  for (const auto &p : outer2) {
    minX = std::min(minX, p.x);
    maxX = std::max(maxX, p.x);
    minY = std::min(minY, p.y);
    maxY = std::max(maxY, p.y);
  }
  if ((maxX - minX) < spacing || (maxY - minY) < spacing)
    return out;

  double angle = angleDegrees * M_PI / 180.0;
  double c = std::cos(angle), s = std::sin(angle);

  for (auto &p : outer2)
    p = rotate(p, c, s);
  for (auto &h : holes2)
    for (auto &p : h)
      p = rotate(p, c, s);

  // ← compute rotated bounds on both axes
  double minXrot = std::numeric_limits<double>::infinity(),
         maxXrot = -std::numeric_limits<double>::infinity();
  double minYrot = std::numeric_limits<double>::infinity(),
         maxYrot = -std::numeric_limits<double>::infinity();
  for (const auto &p : outer2) {
    minXrot = std::min(minXrot, p.x);
    maxXrot = std::max(maxXrot, p.x);
    minYrot = std::min(minYrot, p.y);
    maxYrot = std::max(maxYrot, p.y);
  }

  double z = island.outer.points[0].getZ();
  for (double y = minYrot; y <= maxYrot + 1e-9; y += spacing) {
    // ← tight X bounds instead of ±1e6
    auto clipped =
        clipLine({minXrot - 1.0, y}, {maxXrot + 1.0, y}, outer2, holes2);
    for (std::size_t i = 0; i + 1 < clipped.size(); i += 2) {
      Vec2 ar = rotate(clipped[i], c, -s);
      Vec2 br = rotate(clipped[i + 1], c, -s);
      out.push_back({to3(ar, z), to3(br, z)});
    }
  }
  return out;
}

std::vector<InfillSegment> generateGridInfill(const Island &island,
                                              double spacing) {
  auto a = generateLineInfill(island, spacing, 0.0);
  auto b = generateLineInfill(island, spacing, 90.0);
  a.insert(a.end(), b.begin(), b.end());
  return a;
}

std::vector<InfillSegment> generateHexInfill(const Island &island,
                                             double spacing) {
  auto a = generateLineInfill(island, spacing, 0.0);
  auto b = generateLineInfill(island, spacing, 60.0);
  auto c = generateLineInfill(island, spacing, 120.0);
  a.insert(a.end(), b.begin(), b.end());
  a.insert(a.end(), c.begin(), c.end());
  return a;
}