#include "triangleMesh.h"
#include <cstring>
#include <fstream>
#include <iostream>

triangleMesh::triangleMesh() noexcept
    : bottomLeftVertex_(-1e6, -1e6, -1e6), topRightVertex_(1e6, 1e6, 1e6) {}

triangleMesh::triangleMesh(const std::string &stlFile, bool isBinary)
    : bottomLeftVertex_(-1e6, -1e6, -1e6), topRightVertex_(1e6, 1e6, 1e6) {
  if (isBinary)
    loadBinarySTL(stlFile);
  else
    loadAsciiSTL(stlFile);
}

void triangleMesh::push_back(const triangle &t) { mesh_.push_back(t); }

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
  char header[80];
  in.read(header, 80);

  while (in >> token) {
    if (token != "facet")
      break;

    std::string normalToken;
    double nx, ny, nz;
    in >> normalToken >> nx >> ny >> nz;

    in >> token >> token; // "outer loop"

    double v[9];
    for (int i = 0; i < 3; ++i) {
      in >> token >> v[i * 3] >> v[i * 3 + 1] >> v[i * 3 + 2];
    }

    in >> token; // endloop
    in >> token; // endfacet

    mesh_.emplace_back(v3(nx, ny, nz), v3(v[0], v[1], v[2]),
                       v3(v[3], v[4], v[5]), v3(v[6], v[7], v[8]));
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

    mesh_.emplace_back(v3(v[0], v[1], v[2]), v3(v[3], v[4], v[5]),
                       v3(v[6], v[7], v[8]), v3(v[9], v[10], v[11]));
  }
}
std::vector<lineSegment> triangleMesh::sliceAtZ(double z) const {
  std::vector<lineSegment> result;
  Plane pl(v3(0, 0, 1), z);

  for (const auto &tri : mesh_) {
    lineSegment seg;
    if (tri.intersectPlane(pl, seg) == 0) {
      result.push_back(seg);
    }
  }

  return result;
}
