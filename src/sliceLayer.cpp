#include "sliceLayer.h"
#include "lineSegment.h"
#include "v3.h"
#include <algorithm>
#include <cmath>
#include <deque>
#include <unordered_map>
#include <vector>

namespace {

struct QuantizedPoint {
  int64_t x, y, z;

  bool operator==(const QuantizedPoint &other) const {
    return x == other.x && y == other.y && z == other.z;
  }
};

struct PointHash {
  std::size_t operator()(const QuantizedPoint &p) const noexcept {
    std::size_t h1 = std::hash<int64_t>{}(p.x);
    std::size_t h2 = std::hash<int64_t>{}(p.y);
    return h1 ^ (h2 << 1);
  }
};

inline QuantizedPoint quantize(const v3 &v, double eps) {
  return {static_cast<int64_t>(std::round(v.getX() / eps)),
          static_cast<int64_t>(std::round(v.getY() / eps)), 0};
}

std::vector<v3> removeCollinearPoints(const std::vector<v3> &points,
                                      double eps) {
  if (points.size() <= 2)
    return points;

  std::vector<v3> cleaned;
  cleaned.reserve(points.size());
  cleaned.push_back(points[0]);

  double epsSq = eps * eps;

  for (size_t i = 1; i < points.size() - 1; ++i) {
    const auto &a = cleaned.back();
    const auto &b = points[i];
    const auto &c = points[i + 1];

    v3 ab = b - a;
    v3 bc = c - b;

    v3 cross{ab.getY() * bc.getZ() - ab.getZ() * bc.getY(),
             ab.getZ() * bc.getX() - ab.getX() * bc.getZ(),
             ab.getX() * bc.getY() - ab.getY() * bc.getX()};

    double crossLenSq = cross.getX() * cross.getX() +
                        cross.getY() * cross.getY() +
                        cross.getZ() * cross.getZ();

    if (crossLenSq > epsSq) {
      cleaned.push_back(b);
    }
  }

  cleaned.push_back(points.back());
  return cleaned;
}

} // namespace

void SliceLayer::addSegment(const lineSegment &seg) {
  segments_.push_back(seg);
}

std::vector<SliceLayer::Polyline>
SliceLayer::buildPolylines(double epsilon) const {
  if (segments_.empty())
    return {};

  std::unordered_map<QuantizedPoint, std::vector<size_t>, PointHash> adjMap;
  adjMap.reserve(segments_.size() * 2);

  for (size_t i = 0; i < segments_.size(); ++i) {
    adjMap[quantize(segments_[i].start(), epsilon)].push_back(i);
    adjMap[quantize(segments_[i].end(), epsilon)].push_back(i);
  }

  std::vector<Polyline> result;
  std::vector<bool> used(segments_.size(), false);

  for (size_t i = 0; i < segments_.size(); ++i) {
    if (used[i])
      continue;

    std::deque<v3> dq;
    dq.push_back(segments_[i].start());
    dq.push_back(segments_[i].end());
    used[i] = true;

    // Extend backward from front
    bool extended = true;
    while (extended) {
      extended = false;
      auto front_q = quantize(dq.front(), epsilon);

      auto it = adjMap.find(front_q);
      if (it == adjMap.end() || it->second.size() > 2)
        break;

      for (size_t segIdx : it->second) {
        if (used[segIdx])
          continue;

        const auto &s = segments_[segIdx].start();
        const auto &e = segments_[segIdx].end();

        auto sq = quantize(s, epsilon);
        auto eq = quantize(e, epsilon);

        if (front_q == eq) {
          dq.push_front(s);
          used[segIdx] = true;
          extended = true;
          break;
        } else if (front_q == sq) {
          dq.push_front(e);
          used[segIdx] = true;
          extended = true;
          break;
        }
      }
    }

    // Extend forward from back
    extended = true;
    while (extended) {
      extended = false;
      auto back_q = quantize(dq.back(), epsilon);
      auto front_q = quantize(dq.front(), epsilon);

      if (back_q == front_q && dq.size() > 2) {
        break;
      }

      auto it = adjMap.find(back_q);
      if (it == adjMap.end() || it->second.size() > 2)
        break;

      for (size_t segIdx : it->second) {
        if (used[segIdx])
          continue;

        const auto &s = segments_[segIdx].start();
        const auto &e = segments_[segIdx].end();

        auto sq = quantize(s, epsilon);
        auto eq = quantize(e, epsilon);

        if (back_q == sq) {
          dq.push_back(e);
          used[segIdx] = true;
          extended = true;
          break;
        } else if (back_q == eq) {
          dq.push_back(s);
          used[segIdx] = true;
          extended = true;
          break;
        }
      }
    }

    std::vector<v3> raw_points(dq.begin(), dq.end());
    bool is_closed = false;

    if (raw_points.size() > 2) {
      auto fq = quantize(raw_points.front(), epsilon);
      auto bq = quantize(raw_points.back(), epsilon);
      if (fq == bq) {
        is_closed = true;
        raw_points.pop_back(); // Drop duplicate closing endpoint
      }
    }

    std::vector<v3> cleaned_points = removeCollinearPoints(raw_points, epsilon);

    result.push_back(Polyline{std::move(cleaned_points), is_closed});
  }

  return result;
}
