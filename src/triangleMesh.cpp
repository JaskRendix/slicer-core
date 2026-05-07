#include "triangleMesh.h"
#include <algorithm>
#include <cmath>
#include <cstring>
#include <fstream>
#include <iostream>

triangleMesh::triangleMesh() noexcept
    : bottomLeftVertex_(1e9, 1e9, 1e9), topRightVertex_(-1e9, -1e9, -1e9) {}

triangleMesh::triangleMesh(const std::string &stlFile, bool isBinary)
    : bottomLeftVertex_(1e9, 1e9, 1e9), topRightVertex_(-1e9, -1e9, -1e9) {
  if (isBinary)
    loadBinarySTL(stlFile);
  else
    loadAsciiSTL(stlFile);
}

void triangleMesh::push_back(const triangle &t) {
  mesh_.push_back(t);

  auto update = [&](const v3 &p) {
    bottomLeftVertex_.setX(std::min(bottomLeftVertex_.getX(), p.getX()));
    bottomLeftVertex_.setY(std::min(bottomLeftVertex_.getY(), p.getY()));
    bottomLeftVertex_.setZ(std::min(bottomLeftVertex_.getZ(), p.getZ()));

    topRightVertex_.setX(std::max(topRightVertex_.getX(), p.getX()));
    topRightVertex_.setY(std::max(topRightVertex_.getY(), p.getY()));
    topRightVertex_.setZ(std::max(topRightVertex_.getZ(), p.getZ()));
  };

  update(t.p0());
  update(t.p1());
  update(t.p2());
}

v3 triangleMesh::meshAABBSize() const noexcept {
  return v3(topRightVertex_.getX() - bottomLeftVertex_.getX(),
            topRightVertex_.getY() - bottomLeftVertex_.getY(),
            topRightVertex_.getZ() - bottomLeftVertex_.getZ());
}

void triangleMesh::normalize() noexcept {
  v3 halfbox = (topRightVertex_ - bottomLeftVertex_) / 2.0;
  v3 center = bottomLeftVertex_ + halfbox;

  for (auto &tri : mesh_)
    tri -= center;

  bottomLeftVertex_ = halfbox * -1.0;
  topRightVertex_ = halfbox;
}

//
// ASCII STL loader
//
void triangleMesh::loadAsciiSTL(const std::string &path) {
  std::ifstream in(path);
  if (!in) {
    std::cerr << "Invalid STL file: " << path << "\n";
    return;
  }

  std::string token;
  while (in >> token) {
    if (token != "facet")
      continue;

    std::string normalToken;
    double nx, ny, nz;
    in >> normalToken >> nx >> ny >> nz;

    in >> token >> token; // "outer loop"

    double v[9];
    for (int i = 0; i < 3; ++i)
      in >> token >> v[i * 3] >> v[i * 3 + 1] >> v[i * 3 + 2];

    in >> token; // endloop
    in >> token; // endfacet

    push_back(triangle(v3(nx, ny, nz), v3(v[0], v[1], v[2]),
                       v3(v[3], v[4], v[5]), v3(v[6], v[7], v[8])));
  }
}

//
// Binary STL loader
//
void triangleMesh::loadBinarySTL(const std::string &path) {
  std::ifstream in(path, std::ios::binary);
  if (!in) {
    std::cerr << "Invalid STL file: " << path << "\n";
    return;
  }

  char header[80];
  in.read(header, 80);

  uint32_t nFaces = 0;
  in.read(reinterpret_cast<char *>(&nFaces), sizeof(uint32_t));

  for (uint32_t i = 0; i < nFaces; ++i) {
    float v[12];
    in.read(reinterpret_cast<char *>(v), sizeof(float) * 12);

    uint16_t attr;
    in.read(reinterpret_cast<char *>(&attr), sizeof(uint16_t));

    push_back(triangle(v3(v[0], v[1], v[2]), v3(v[3], v[4], v[5]),
                       v3(v[6], v[7], v[8]), v3(v[9], v[10], v[11])));
  }
}

//
// sliceAtZ — watertight slicer implementation without std::optional
//
std::vector<lineSegment> triangleMesh::sliceAtZ(double z) const {
  const double eps = 1e-9;
  std::vector<lineSegment> out;

  for (const auto &tri : mesh_) {

    v3 p0 = tri.p0();
    v3 p1 = tri.p1();
    v3 p2 = tri.p2();

    double z0 = p0.getZ();
    double z1 = p1.getZ();
    double z2 = p2.getZ();

    // Skip triangles fully above or below
    if ((z0 < z - eps && z1 < z - eps && z2 < z - eps) ||
        (z0 > z + eps && z1 > z + eps && z2 > z + eps))
      continue;

    v3 pts[3];
    int count = 0;

    auto intersectEdge = [&](const v3 &a, const v3 &b, v3 &outPoint,
                             bool &hasPoint) {
      hasPoint = false;
      double za = a.getZ(), zb = b.getZ();

      // Edge lies on plane → ignore (prevents apex artifacts)
      if (std::fabs(za - z) < eps && std::fabs(zb - z) < eps)
        return;

      // One endpoint on plane
      if (std::fabs(za - z) < eps) {
        outPoint = a;
        hasPoint = true;
        return;
      }
      if (std::fabs(zb - z) < eps) {
        outPoint = b;
        hasPoint = true;
        return;
      }

      // Proper crossing
      if ((za < z && zb > z) || (za > z && zb < z)) {
        double t = (z - za) / (zb - za);
        outPoint = v3(a.getX() + t * (b.getX() - a.getX()),
                      a.getY() + t * (b.getY() - a.getY()), z);
        hasPoint = true;
      }
    };

    auto addIf = [&](const v3 &a, const v3 &b) {
      v3 p;
      bool has = false;
      intersectEdge(a, b, p, has);
      if (has && count < 3) {
        pts[count++] = p;
      }
    };

    addIf(p0, p1);
    addIf(p1, p2);
    addIf(p2, p0);

    // Only valid if exactly 2 distinct points
    if (count == 2) {
      double dx = pts[0].getX() - pts[1].getX();
      double dy = pts[0].getY() - pts[1].getY();
      double dz = pts[0].getZ() - pts[1].getZ();
      double dist2 = dx * dx + dy * dy + dz * dz;

      if (dist2 > eps * eps)
        out.emplace_back(pts[0], pts[1]);
    }
  }

  return out;
}
