#ifndef TRIANGLEMESH_H
#define TRIANGLEMESH_H

#include "triangle.h"
#include <array>
#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

class triangleMesh {
public:
  triangleMesh() noexcept;
  explicit triangleMesh(const std::string &stlFile, bool isBinary);

  ~triangleMesh() = default;

  [[nodiscard]] std::size_t size() const noexcept { return mesh_.size(); }

  void normalize() noexcept;
  void push_back(const triangle &tri);

  [[nodiscard]] v3 meshAABBSize() const noexcept;

  [[nodiscard]] std::vector<triangle> &getMesh() noexcept { return mesh_; }
  [[nodiscard]] const std::vector<triangle> &getMesh() const noexcept {
    return mesh_;
  }

  [[nodiscard]] v3 getBottomLeftVertex() const noexcept {
    return bottomLeftVertex_;
  }
  [[nodiscard]] v3 getTopRightVertex() const noexcept {
    return topRightVertex_;
  }
  [[nodiscard]] std::vector<lineSegment> sliceAtZ(double z) const;

private:
  void loadAsciiSTL(const std::string &path);
  void loadBinarySTL(const std::string &path);

private:
  v3 bottomLeftVertex_;
  v3 topRightVertex_;
  std::vector<triangle> mesh_;
};

#endif // TRIANGLEMESH_H
