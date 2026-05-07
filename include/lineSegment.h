#ifndef LINESEGMENT_H
#define LINESEGMENT_H

#include "v3.h"
#include <array>
#include <cstddef>

class lineSegment {
public:
  lineSegment() noexcept : vertices_{v3{}, v3{}} {}

  lineSegment(const v3 &p0, const v3 &p1) noexcept : vertices_{p0, p1} {}

  ~lineSegment() = default;

  [[nodiscard]] constexpr const v3 &getVertex(std::size_t i) const noexcept {
    return vertices_[i];
  }

  [[nodiscard]] constexpr v3 &getVertex(std::size_t i) noexcept {
    return vertices_[i];
  }

  constexpr void setVertex(const v3 &v, std::size_t i) noexcept {
    vertices_[i] = v;
  }

  [[nodiscard]] constexpr const v3 &a() const noexcept { return vertices_[0]; }
  [[nodiscard]] constexpr const v3 &b() const noexcept { return vertices_[1]; }

  // 🔹 Compatibility with old code (sliceLayer.cpp)
  [[nodiscard]] constexpr const v3 &start() const noexcept { return a(); }
  [[nodiscard]] constexpr const v3 &end() const noexcept { return b(); }

private:
  std::array<v3, 2> vertices_;
};

#endif // LINESEGMENT_H
