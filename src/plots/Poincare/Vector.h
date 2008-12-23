/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2004 Scientific Computing and Imaging Institute,
   University of Utah.

   
   Permission is hereby granted, free of charge, to any person obtaining a
   copy of this software and associated documentation files (the "Software"),
   to deal in the Software without restriction, including without limitation
   the rights to use, copy, modify, merge, publish, distribute, sublicense,
   and/or sell copies of the Software, and to permit persons to whom the
   Software is furnished to do so, subject to the following conditions:

   The above copyright notice and this permission notice shall be included
   in all copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
   OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
   THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
   FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
   DEALINGS IN THE SOFTWARE.
*/



/*
 *  Vector.h: ?
 *
 *  Written by:
 *   Author?
 *   Department of Computer Science
 *   University of Utah
 *   Date?
 *
 *  Copyright (C) 199? SCI Group
 */

#ifndef SLIVR_Vector_h
#define SLIVR_Vector_h

#undef SLIVRSHARE
#if defined(_WIN32)
#define SLIVRSHARE __declspec(dllexport)
#else
#define SLIVRSHARE
#endif


#include <string>
#include <iosfwd>
#include <math.h>
#include <assert.h>

#include "Utils.h"
//#include <slivr/share.h>

namespace SLIVR {
using std::string;


class Point;

class Vector {
  double x_,y_,z_;
public:
  inline explicit Vector(const Point&);
  inline Vector(double x, double y, double z): x_(x), y_(y), z_(z)
  { }
  inline Vector(const Vector&);
  inline Vector();
  inline explicit Vector(double init) : x_(init), y_(init), z_(init) {}
  inline double length() const;
  inline double length2() const;
  friend inline double Dot(const Vector&, const Vector&);
  friend inline double Dot(const Point&, const Vector&);
  friend inline double Dot(const Vector&, const Point&);
  inline Vector& operator=(const Vector&);
  inline Vector& operator=(const double&);
  inline Vector& operator=(const int&);
  
  //Note vector[0]=vector.x();vector[1]=vector.y();vector[2]=vector.z()
  inline double& operator[](int idx) {
    // Ugly, but works
    return (&x_)[idx];
  }

  //Note vector[0]=vector.x();vector[1]=vector.y();vector[2]=vector.z()
  inline double operator[](int idx) const {
    // Ugly, but works
    return (&x_)[idx];
  }

  // checks if one vector is exactly the same as another
  SLIVRSHARE int operator==(const Vector&) const;
  SLIVRSHARE int operator!=(const Vector&) const;

  inline Vector operator*(const double) const;
  inline Vector operator*(const Vector&) const;
  inline Vector& operator*=(const double);
  inline Vector& operator*=(const Vector&);
  inline Vector operator/(const double) const;
  inline Vector operator/(const Vector&) const;
  inline Vector& operator/=(const double);
  inline Vector operator+(const Vector&) const;
  inline Vector& operator+=(const Vector&);
  inline Vector operator-() const;
  inline Vector operator-(const Vector&) const;
  inline Vector operator-(const Point&) const;
  inline Vector& operator-=(const Vector&);
  inline double normalize();
  inline double safe_normalize();
  SLIVRSHARE Vector normal() const;
  SLIVRSHARE Vector safe_normal() const;
  friend inline Vector Cross(const Vector&, const Vector&);
  friend inline Vector Abs(const Vector&);
  inline void x(double);
  inline double x() const;
  inline void y(double);
  inline double y() const;
  inline void z(double);
  inline double z() const;

  inline void u(double);
  inline double u() const;
  inline void v(double);
  inline double v() const;
  inline void w(double);
  inline double w() const;

  void rotz90(const int);
    
  string get_string() const;


  friend class Point;
  friend class Transform;
    
  friend inline Vector Interpolate(const Vector&, const Vector&, double);
    
  SLIVRSHARE void find_orthogonal(Vector&, Vector&) const;
  SLIVRSHARE bool check_find_orthogonal(Vector&, Vector&) const;

  inline const Point &point() const;
  inline Point &asPoint() const;
  inline double minComponent() const {
    if(x_<y_){
      if(x_<z_)
        return x_;
      else
        return z_;
    } else {
      if(y_<z_)
        return y_;
      else
        return z_;
    }
  }
  inline double maxComponent() const {
    if(x_>y_){
      if(x_>z_)
        return x_;
      else
        return z_;
    } else {
      if(y_>z_)
        return y_;
      else
        return z_;
    }
  }

  inline void Set(double x, double y, double z)
  { 
    x_ = x;
    y_ = y;
    z_ = z;
  }
      
  SLIVRSHARE friend std::ostream& operator<<(std::ostream& os, const Vector& p);
  SLIVRSHARE friend std::istream& operator>>(std::istream& os, Vector& p);

}; // end class Vector

} // End namespace SLIVR

// This cannot be above due to circular dependencies
#include "Point.h"

namespace SLIVR {


inline Vector::Vector(const Point& p)
  : x_(p.x_), y_(p.y_), z_(p.z_)
{
}

inline Vector::Vector()
{
}

inline Vector::Vector(const Vector& p)
{
  x_=p.x_;
  y_=p.y_;
  z_=p.z_;
}

inline double Vector::length2() const
{
  return x_*x_+y_*y_+z_*z_;
}

inline Vector& Vector::operator=(const Vector& v)
{
  x_=v.x_;
  y_=v.y_;
  z_=v.z_;
  return *this;
}

// for initializing in dynamic code
// one often want template<class T> T val = 0.0;

inline Vector& Vector::operator=(const double& d)
{
  x_ = d;
  y_ = d;
  z_ = d;
  return *this;
}

inline Vector& Vector::operator=(const int& d)
{
  x_ = static_cast<int>(d);
  y_ = static_cast<int>(d);
  z_ = static_cast<int>(d);
  return *this;
}

inline bool operator<(Vector v1, Vector v2)
{
  return(v1.length()<v2.length());
}

inline bool operator<=(Vector v1, Vector v2)
{
  return(v1.length()<=v2.length());
}

inline bool operator>(Vector v1, Vector v2)
{
  return(v1.length()>v2.length());
}

inline bool operator>=(Vector v1, Vector v2)
{
  return(v1.length()>=v2.length());
}



inline Vector Vector::operator*(const double s) const
{
  return Vector(x_*s, y_*s, z_*s);
}

inline Vector& Vector::operator*=(const Vector& v)
{
  x_ *= v.x_;
  y_ *= v.y_;
  z_ *= v.z_;
  return *this;
}

// Allows for double * Vector so that everything doesn't have to be
// Vector * double
inline Vector operator*(const double s, const Vector& v) {
  return v*s;
}

inline Vector Vector::operator/(const double d) const
{
  return Vector(x_/d, y_/d, z_/d);
}

inline Vector Vector::operator/(const Vector& v2) const
{
  return Vector(x_/v2.x_, y_/v2.y_, z_/v2.z_);
}

inline Vector Vector::operator+(const Vector& v2) const
{
  return Vector(x_+v2.x_, y_+v2.y_, z_+v2.z_);
}

inline Vector Vector::operator*(const Vector& v2) const
{
  return Vector(x_*v2.x_, y_*v2.y_, z_*v2.z_);
}

inline Vector Vector::operator-(const Vector& v2) const
{
  return Vector(x_-v2.x_, y_-v2.y_, z_-v2.z_);
}

inline Vector Vector::operator-(const Point& v2) const
{
  return Vector(x_-v2.x_, y_-v2.y_, z_-v2.z_);
}

inline Vector& Vector::operator+=(const Vector& v2)
{
  x_+=v2.x_;
  y_+=v2.y_;
  z_+=v2.z_;
  return *this;
}

inline Vector& Vector::operator-=(const Vector& v2)
{
  x_-=v2.x_;
  y_-=v2.y_;
  z_-=v2.z_;
  return *this;
}

inline Vector Vector::operator-() const
{
  return Vector(-x_,-y_,-z_);
}

inline double Vector::length() const
{
  return sqrt(x_*x_+y_*y_+z_*z_);
}

inline Vector Abs(const Vector& v)
{
  double x=v.x_<0?-v.x_:v.x_;
  double y=v.y_<0?-v.y_:v.y_;
  double z=v.z_<0?-v.z_:v.z_;
  return Vector(x,y,z);
}

inline Vector Cross(const Vector& v1, const Vector& v2)
{
  return Vector(v1.y_*v2.z_-v1.z_*v2.y_,
                v1.z_*v2.x_-v1.x_*v2.z_,
                v1.x_*v2.y_-v1.y_*v2.x_);
}

inline Vector Interpolate(const Vector& v1, const Vector& v2,
                          double weight)
{
  double weight1=1.0-weight;
  return Vector(v2.x_*weight+v1.x_*weight1,
                v2.y_*weight+v1.y_*weight1,
                v2.z_*weight+v1.z_*weight1);
}

inline Vector& Vector::operator*=(const double d)
{
  x_*=d;
  y_*=d;
  z_*=d;
  return *this;
}

inline Vector& Vector::operator/=(const double d)
{
  x_/=d;
  y_/=d;
  z_/=d;
  return *this;
}

inline void Vector::x(double d)
{
  x_=d;
}

inline double Vector::x() const
{
  return x_;
}

inline void Vector::y(double d)
{
  y_=d;
}

inline double Vector::y() const
{
  return y_;
}

inline void Vector::z(double d)
{
  z_=d;
}

inline double Vector::z() const
{
  return z_;
}



inline void Vector::u(double d)
{
  x_=d;
}

inline double Vector::u() const
{
  return x_;
}

inline void Vector::v(double d)
{
  y_=d;
}

inline double Vector::v() const
{
  return y_;
}

inline void Vector::w(double d)
{
  z_=d;
}

inline double Vector::w() const
{
  return z_;
}

inline double Dot(const Vector& v1, const Vector& v2)
{
  return v1.x_*v2.x_+v1.y_*v2.y_+v1.z_*v2.z_;
}

inline double Dot(const Vector& v, const Point& p)
{
  return v.x_*p.x_+v.y_*p.y_+v.z_*p.z_;
}

inline
double Vector::normalize()
{
  double l=sqrt(x_*x_ + y_*y_ + z_*z_);
  if (l > 0.0)
  {
    x_/=l;
    y_/=l;
    z_/=l;
  }
  return l;
}

inline
double Vector::safe_normalize()
{
  double l=sqrt(x_*x_ + y_*y_ + z_*z_);
  if (l > 0.0)
  {
    x_/=l;
    y_/=l;
    z_/=l;
  }
  return l;
}


inline const Point &Vector::point() const {
  return (const Point &)(*this);
}

inline Point &Vector::asPoint() const {
  return (Point &)(*this);
}

inline Vector Min(const Vector &v1, const Vector &v2)
{
  return Vector(Min(v1.x(), v2.x()),
                Min(v1.y(), v2.y()),
                Min(v1.z(), v2.z()));
}

inline Vector Max(const Vector &v1, const Vector &v2)
{
  return Vector(Max(v1.x(), v2.x()),
                Max(v1.y(), v2.y()),
                Max(v1.z(), v2.z()));
}

} // End namespace SLIVR


#endif
