//
//  LinAlg.h
//  8Beat (copy from applaudio)
//
//  Created by Rasmus Anthin on 2025-10-03.
//

#pragma once
#include <array>
#include <cmath>
#include <optional>


namespace beat::la
{
  enum Coord { X, Y, Z, W };
  enum M3Coord { m3XX, m3XY, m3XZ,
                 m3YX, m3YY, m3YZ,
                 m3ZX, m3ZY, m3ZZ };
  enum M4Coord { m4XX, m4XY, m4XZ, m4XW,
                 m4YX, m4YY, m4YZ, m4YW,
                 m4ZX, m4ZY, m4ZZ, m4ZW,
                 m4WX, m4WY, m4WZ, m4WW };

  class Vec3
  {
    std::array<float, 3> elem { 0.f, 0.f, 0.f };
    
  public:
    Vec3() = default;
    Vec3(float x, float y, float z)
      : elem({ x, y, z })
    {}
    Vec3(const std::array<float, 3>& xyz)
      : elem(xyz)
    {}
    
    float& operator[](int coord) { return elem[coord]; }
    const float& operator[](int coord) const { return elem[coord]; }
    
    inline float x() const noexcept { return elem[X]; }
    inline float y() const noexcept { return elem[Y]; }
    inline float z() const noexcept { return elem[Z]; }
    
    const std::array<float, 3>& get_arr() const { return elem; }
    
    Vec3 operator+() const
    {
      return *this;
    }
    
    Vec3 operator+(const Vec3& other) const
    {
      return { this->x() + other.x(),
               this->y() + other.y(),
               this->z() + other.z() };
    }
    
    const Vec3& operator+=(const Vec3& other)
    {
      *this = *this + other;
      return *this;
    }
    
    Vec3 operator-() const
    {
      return { -this->x(), -this->y(), -this->z() };
    }
    
    Vec3 operator-(const Vec3& other) const
    {
      return { this->x() - other.x(),
               this->y() - other.y(),
               this->z() - other.z() };
    }
    
    const Vec3& operator-=(const Vec3& other)
    {
      *this = *this - other;
      return *this;
    }
    
    Vec3 operator*(float s) const
    {
      return { this->x() * s, this->y() * s, this->z() * s };
    }
    
    const Vec3& operator*=(const float s)
    {
      *this = *this * s;
      return *this;
    }
    
    Vec3 operator/(float s) const
    {
      return { this->x() / s, this->y() / s, this->z() / s };
    }
    
    const Vec3& operator/=(const float s)
    {
      *this = *this / s;
      return *this;
    }
    
    float length_squared() const
    {
      auto px = x();
      auto py = y();
      auto pz = z();
      return px*px + py*py + pz*pz;
    }
    
    float length() const
    {
      return std::sqrt(length_squared());
    }
  };
  
  Vec3 Vec3_Zero { 0.f, 0.f, 0.f };
  
  Vec3 normalize(const Vec3& v)
  {
    auto l = v.length();
    if (std::abs(l) < 1e-6f)
      return Vec3_Zero;
    return v / l;
  }
  
  float dot(const Vec3& u, const Vec3& v)
  {
    return u.x() * v.x() + u.y() * v.y() + u.z() * v.z();
  }
  
  Vec3 cross(const Vec3& u, const Vec3& v)
  {
    return { u.y()*v.z() - u.z()*v.y(), u.z()*v.x() - u.x()*v.z(), u.x()*v.y() - u.y()*v.x() };
  }
  
  // /////////////////////////////////
  
  class Mtx3
  {
    std::array<float, 9> elem { 1.f, 0.f, 0.f,
                                0.f, 1.f, 0.f,
                                0.f, 0.f, 1.f };
    
  public:
    Mtx3() = default;
    Mtx3(float xx, float xy, float xz,
         float yx, float yy, float yz,
         float zx, float zy, float zz)
      : elem({ xx, xy, xz,
               yx, yy, yz,
               zx, zy, zz })
    {}
    Mtx3(const std::array<float, 9>& row_major_mat)
      : elem(row_major_mat)
    {}
    
    float& operator[](int idx) { return elem[idx]; }
    const float& operator[](int idx) const { return elem[idx]; }
    
    float& operator()(int r, int c) { return elem[r*3 + c]; }
    const float& operator()(int r, int c) const { return elem[r*3 + c]; }
    
    inline float xx() const noexcept { return elem[m3XX]; }
    inline float xy() const noexcept { return elem[m3XY]; }
    inline float xz() const noexcept { return elem[m3XZ]; }
    inline float yx() const noexcept { return elem[m3YX]; }
    inline float yy() const noexcept { return elem[m3YY]; }
    inline float yz() const noexcept { return elem[m3YZ]; }
    inline float zx() const noexcept { return elem[m3ZX]; }
    inline float zy() const noexcept { return elem[m3ZY]; }
    inline float zz() const noexcept { return elem[m3ZZ]; }
    
    const std::array<float, 9>& get_arr() const { return elem; }
    
    Vec3 transform_vec(const Vec3& local_vec) const
    {
      auto lv_x = local_vec.x();
      auto lv_y = local_vec.y();
      auto lv_z = local_vec.z();
      return { xx() * lv_x + xy() * lv_y + xz() * lv_z,
               yx() * lv_x + yy() * lv_y + yz() * lv_z,
               zx() * lv_x + zy() * lv_y + zz() * lv_z };
    }
    
    bool get_column_vec(int col, Vec3& col_vec) const
    {
      if (0 <= col && col < 4)
      {
        col_vec = { (*this)(X, col), (*this)(Y, col), (*this)(Z, col) };
        return true;
      }
      return false;
    }
    
    bool set_column_vec(int col, const Vec3& col_vec)
    {
      if (0 <= col && col < 4)
      {
        (*this)(X, col) = col_vec[X];
        (*this)(Y, col) = col_vec[Y];
        (*this)(Z, col) = col_vec[Z];
        return true;
      }
      return false;
    }
  };
  
  Mtx3 Mtx3_Identity { 1.f, 0.f, 0.f,
                       0.f, 1.f, 0.f,
                       0.f, 0.f, 1.f };
  
  // /////////////////////////////////
  
  class Mtx4
  {
    std::array<float, 16> elem { 1.f, 0.f, 0.f, 0.f,
                                 0.f, 1.f, 0.f, 0.f,
                                 0.f, 0.f, 1.f, 0.f,
                                 0.f, 0.f, 0.f, 1.f };
    
  public:
    Mtx4() = default;
    Mtx4(float xx, float xy, float xz, float xw,
         float yx, float yy, float yz, float yw,
         float zx, float zy, float zz, float zw,
         float wx, float wy, float wz, float ww)
      : elem({ xx, xy, xz, xw,
               yx, yy, yz, yw,
               zx, zy, zz, zw,
               wx, wy, wz, ww })
    {}
    Mtx4(const std::array<float, 16>& row_major_mat)
      : elem(row_major_mat)
    {}
    
    float& operator[](int idx) { return elem[idx]; }
    const float& operator[](int idx) const { return elem[idx]; }
    
    float& operator()(int r, int c) { return elem[r*4 + c]; }
    const float& operator()(int r, int c) const { return elem[r*4 + c]; }
    
    inline float xx() const noexcept { return elem[m4XX]; }
    inline float xy() const noexcept { return elem[m4XY]; }
    inline float xz() const noexcept { return elem[m4XZ]; }
    inline float xw() const noexcept { return elem[m4XW]; }
    inline float yx() const noexcept { return elem[m4YX]; }
    inline float yy() const noexcept { return elem[m4YY]; }
    inline float yz() const noexcept { return elem[m4YZ]; }
    inline float yw() const noexcept { return elem[m4YW]; }
    inline float zx() const noexcept { return elem[m4ZX]; }
    inline float zy() const noexcept { return elem[m4ZY]; }
    inline float zz() const noexcept { return elem[m4ZZ]; }
    inline float zw() const noexcept { return elem[m4ZW]; }
    inline float wx() const noexcept { return elem[m4WX]; }
    inline float wy() const noexcept { return elem[m4WY]; }
    inline float wz() const noexcept { return elem[m4WZ]; }
    inline float ww() const noexcept { return elem[m4WW]; }
    
    const std::array<float, 16>& get_arr() const { return elem; }
    
    Vec3 transform_pos(const Vec3& local_pos) const
    {
      auto lp_x = local_pos.x();
      auto lp_y = local_pos.y();
      auto lp_z = local_pos.z();
      return { xx() * lp_x + xy() * lp_y + xz() * lp_z + xw(),
               yx() * lp_x + yy() * lp_y + yz() * lp_z + yw(),
               zx() * lp_x + zy() * lp_y + zz() * lp_z + zw() };
    }
    
    Vec3 transform_vec(const Vec3& local_vec) const
    {
      auto lv_x = local_vec.x();
      auto lv_y = local_vec.y();
      auto lv_z = local_vec.z();
      return { xx() * lv_x + xy() * lv_y + xz() * lv_z,
               yx() * lv_x + yy() * lv_y + yz() * lv_z,
               zx() * lv_x + zy() * lv_y + zz() * lv_z };
    }
    
    bool get_column_vec(int col, Vec3& col_vec, float& w) const
    {
      if (0 <= col && col < 4)
      {
        col_vec = { (*this)(X, col), (*this)(Y, col), (*this)(Z, col) };
        w = (*this)(W, col);
        return true;
      }
      return false;
    }
    
    bool get_column_vec(int col, Vec3& col_vec) const
    {
      if (0 <= col && col < 4)
      {
        col_vec = { (*this)(X, col), (*this)(Y, col), (*this)(Z, col) };
        return true;
      }
      return false;
    }
    
    Mtx3 get_rot_matrix() const
    {
      Mtx3 rot_mtx;
      rot_mtx(X, X) = (*this)(X, X);
      rot_mtx(X, Y) = (*this)(X, Y);
      rot_mtx(X, Z) = (*this)(X, Z);
      
      rot_mtx(Y, X) = (*this)(Y, X);
      rot_mtx(Y, Y) = (*this)(Y, Y);
      rot_mtx(Y, Z) = (*this)(Y, Z);
      
      rot_mtx(Z, X) = (*this)(Z, X);
      rot_mtx(Z, Y) = (*this)(Z, Y);
      rot_mtx(Z, Z) = (*this)(Z, Z);
      
      return rot_mtx;
    }
    
    void set_rot_matrix(const Mtx3& rot_mtx)
    {
      (*this)(X, X) = rot_mtx(X, X);
      (*this)(X, Y) = rot_mtx(X, Y);
      (*this)(X, Z) = rot_mtx(X, Z);
      
      (*this)(Y, X) = rot_mtx(Y, X);
      (*this)(Y, Y) = rot_mtx(Y, Y);
      (*this)(Y, Z) = rot_mtx(Y, Z);
      
      (*this)(Z, X) = rot_mtx(Z, X);
      (*this)(Z, Y) = rot_mtx(Z, Y);
      (*this)(Z, Z) = rot_mtx(Z, Z);
    }
    
    bool set_column_vec(int col, const Vec3& col_vec, std::optional<float> w = std::nullopt)
    {
      if (0 <= col && col < 4)
      {
        (*this)(X, col) = col_vec[X];
        (*this)(Y, col) = col_vec[Y];
        (*this)(Z, col) = col_vec[Z];
        if (w.has_value())
          (*this)(W, col) = w.value();
        return true;
      }
      return false;
    }
  };
  
  Mtx4 Mtx4_Identity { 1.f, 0.f, 0.f, 0.f,
                       0.f, 1.f, 0.f, 0.f,
                       0.f, 0.f, 1.f, 0.f,
                       0.f, 0.f, 0.f, 1.f };
                       
  Mtx4 look_at(const Vec3& location_pos, const Vec3& look_at_pos, const Vec3& up_dir)
  {
    auto F = look_at_pos - location_pos;
    auto f = normalize(F);
    auto up = normalize(up_dir);
    auto s = cross(f, up);
    auto u = cross(normalize(s), f);
    Mtx4 trf_lookat;
    trf_lookat.set_column_vec(X, s);
    trf_lookat.set_column_vec(Y, u);
    trf_lookat.set_column_vec(Z, -f);
    trf_lookat.set_column_vec(W, location_pos);
    return trf_lookat;
  }

}
