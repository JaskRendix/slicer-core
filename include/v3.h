#pragma once
#include <cmath>

class v3 {
  double x_, y_, z_;

public:
  v3() : x_(0), y_(0), z_(0) {}
  v3(double x, double y, double z) : x_(x), y_(y), z_(z) {}

  double getX() const { return x_; }
  double getY() const { return y_; }
  double getZ() const { return z_; }

  // Basic arithmetic
  v3 operator+(const v3 &o) const {
    return v3(x_ + o.x_, y_ + o.y_, z_ + o.z_);
  }
  v3 operator-(const v3 &o) const {
    return v3(x_ - o.x_, y_ - o.y_, z_ - o.z_);
  }
  v3 operator*(double s) const { return v3(x_ * s, y_ * s, z_ * s); }
  v3 operator/(double s) const { return v3(x_ / s, y_ / s, z_ / s); }

  // Compound assignment
  v3 &operator+=(const v3 &o) {
    x_ += o.x_;
    y_ += o.y_;
    z_ += o.z_;
    return *this;
  }

  v3 &operator-=(const v3 &o) {
    x_ -= o.x_;
    y_ -= o.y_;
    z_ -= o.z_;
    return *this;
  }

  // Dot and cross
  double dot(const v3 &o) const { return x_ * o.x_ + y_ * o.y_ + z_ * o.z_; }

  v3 cross(const v3 &o) const {
    return v3(y_ * o.z_ - z_ * o.y_, z_ * o.x_ - x_ * o.z_,
              x_ * o.y_ - y_ * o.x_);
  }
};
