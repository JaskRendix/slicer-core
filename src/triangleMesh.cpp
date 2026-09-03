#include "triangleMesh.h"
#include <algorithm>
#include <cmath>
#include <cstring>
#include <fstream>
#include <iostream>
#include <sstream>
#include <thread>
#include <vector>

triangleMesh::triangleMesh() noexcept
    : bottomLeftVertex_(1e9, 1e9, 1e9), topRightVertex_(-1e9, -1e9, -1e9) {}

triangleMesh::triangleMesh(const std::filesystem::path &stlFile, bool isBinary)
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
// Robust line-based ASCII STL loader
//
void triangleMesh::loadAsciiSTL(const std::filesystem::path &path) {
  std::ifstream in(path);
  if (!in) {
    std::cerr << "Invalid STL file: " << path << "\n";
    return;
  }

  std::string line;
  v3 normal(0, 0, 0);
  std::vector<v3> current_facet_verts;
  current_facet_verts.reserve(3);

  while (std::getline(in, line)) {
    std::istringstream iss(line);
    std::string token;
    if (!(iss >> token))
      continue;

    if (token == "facet") {
      std::string normalToken;
      double nx, ny, nz;
      if (iss >> normalToken >> nx >> ny >> nz) {
        normal = v3(nx, ny, nz);
      }
      current_facet_verts.clear();
    } else if (token == "vertex") {
      double x, y, z;
      if (iss >> x >> y >> z) {
        current_facet_verts.emplace_back(x, y, z);
      }
    } else if (token == "endfacet") {
      if (current_facet_verts.size() == 3) {
        push_back(triangle(normal, current_facet_verts[0],
                           current_facet_verts[1], current_facet_verts[2]));
      }
      current_facet_verts.clear();
    }
  }
}

//
// Binary STL loader with pre-allocation
//
void triangleMesh::loadBinarySTL(const std::filesystem::path &path) {
  std::ifstream in(path, std::ios::binary);
  if (!in) {
    std::cerr << "Invalid STL file: " << path << "\n";
    return;
  }

  char header[80];
  in.read(header, 80);

  uint32_t nFaces = 0;
  in.read(reinterpret_cast<char *>(&nFaces), sizeof(uint32_t));

  mesh_.reserve(nFaces);

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
// sliceAtZ — robust multi-threaded slicer with adaptive epsilon, safe chunking,
// and deduplication
//
std::vector<lineSegment> triangleMesh::sliceAtZ(double z) const {
  v3 aabb = meshAABBSize();
  double max_dim = std::max({aabb.getX(), aabb.getY(), aabb.getZ()});
  const double eps = std::max(1e-15, max_dim * 1e-7);

  const std::size_t num_triangles = mesh_.size();
  if (num_triangles == 0)
    return {};

  unsigned int hardware_threads = std::thread::hardware_concurrency();
  std::size_t num_threads =
      std::max(1U, hardware_threads == 0 ? 4U : hardware_threads);

  if (num_triangles < 1000)
    num_threads = 1;

  std::vector<std::vector<lineSegment>> thread_results(num_threads);
  std::vector<std::thread> threads;
  threads.reserve(num_threads);

  std::size_t chunk_size = (num_triangles + num_threads - 1) / num_threads;

  for (std::size_t i = 0; i < num_threads; ++i) {
    std::size_t start = i * chunk_size;
    if (start >= num_triangles)
      break;
    std::size_t end = std::min(num_triangles, start + chunk_size);

    threads.emplace_back(
        [this, start, end, z, eps, &res = thread_results[i]]() {
          for (std::size_t idx = start; idx < end; ++idx) {
            const auto &tri = mesh_[idx];
            v3 p0 = tri.p0();
            v3 p1 = tri.p1();
            v3 p2 = tri.p2();

            double z0 = p0.getZ();
            double z1 = p1.getZ();
            double z2 = p2.getZ();

            if ((z0 < z - eps && z1 < z - eps && z2 < z - eps) ||
                (z0 > z + eps && z1 > z + eps && z2 > z + eps))
              continue;

            v3 raw_pts[3];
            int count = 0;

            auto intersectEdge = [&](const v3 &a, const v3 &b, v3 &outPoint,
                                     bool &hasPoint) {
              hasPoint = false;
              double za = a.getZ(), zb = b.getZ();

              if (std::fabs(za - z) < eps && std::fabs(zb - z) < eps)
                return;

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
                raw_pts[count++] = p;
              }
            };

            addIf(p0, p1);
            addIf(p1, p2);
            addIf(p2, p0);

            // Deduplicate intersection points
            std::vector<v3> pts;
            pts.reserve(count);
            for (int c = 0; c < count; ++c) {
              bool duplicate = false;
              for (const auto &pt : pts) {
                double dx = raw_pts[c].getX() - pt.getX();
                double dy = raw_pts[c].getY() - pt.getY();
                double dz = raw_pts[c].getZ() - pt.getZ();
                if (dx * dx + dy * dy + dz * dz < eps * eps) {
                  duplicate = true;
                  break;
                }
              }
              if (!duplicate) {
                pts.push_back(raw_pts[c]);
              }
            }

            if (pts.size() == 2) {
              double dx = pts[0].getX() - pts[1].getX();
              double dy = pts[0].getY() - pts[1].getY();
              double dz = pts[0].getZ() - pts[1].getZ();
              double dist2 = dx * dx + dy * dy + dz * dz;

              if (dist2 > eps * eps) {
                // Check if this segment forms a coplanar edge of the triangle
                auto isOriginalEdge = [&](const v3 &a, const v3 &b,
                                          const v3 &vA, const v3 &vB) {
                  double d1 =
                      std::hypot(a.getX() - vA.getX(), a.getY() - vA.getY());
                  double d2 =
                      std::hypot(b.getX() - vB.getX(), b.getY() - vB.getY());
                  double d3 =
                      std::hypot(a.getX() - vB.getX(), a.getY() - vB.getY());
                  double d4 =
                      std::hypot(b.getX() - vA.getX(), b.getY() - vA.getY());
                  return (d1 < eps && d2 < eps) || (d3 < eps && d4 < eps);
                };

                bool is_coplanar_edge = false;
                if (std::fabs(p0.getZ() - z) < eps &&
                    std::fabs(p1.getZ() - z) < eps &&
                    isOriginalEdge(pts[0], pts[1], p0, p1))
                  is_coplanar_edge = true;
                else if (std::fabs(p1.getZ() - z) < eps &&
                         std::fabs(p2.getZ() - z) < eps &&
                         isOriginalEdge(pts[0], pts[1], p1, p2))
                  is_coplanar_edge = true;
                else if (std::fabs(p2.getZ() - z) < eps &&
                         std::fabs(p0.getZ() - z) < eps &&
                         isOriginalEdge(pts[0], pts[1], p2, p0))
                  is_coplanar_edge = true;

                if (!is_coplanar_edge) {
                  res.emplace_back(pts[0], pts[1]);
                }
              }
            }
          }
        });
  }

  for (auto &t : threads) {
    if (t.joinable())
      t.join();
  }

  std::size_t total_segments = 0;
  for (const auto &res : thread_results)
    total_segments += res.size();

  std::vector<lineSegment> out;
  out.reserve(total_segments);

  for (auto &res : thread_results) {
    out.insert(out.end(), std::make_move_iterator(res.begin()),
               std::make_move_iterator(res.end()));
  }

  return out;
}
