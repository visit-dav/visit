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
 *  Point.h: ?
 *
 *  Written by:
 *   Author?
 *   Department of Computer Science
 *   University of Utah
 *   Date?
 *
 *  Copyright (C) 199? SCI Group
 */

#ifndef SLIVR_Point_h
#define SLIVR_Point_h

#undef SLIVRSHARE
#if defined(_WIN32)
#define SLIVRSHARE __declspec(dllexport)
#else
#define SLIVRSHARE
#endif

//#include <slivr/share.h>
#include "Utils.h"

#include <string>
#include <iosfwd>

namespace SLIVR {

using std::string;
    
class Vector;

class Point {
  double x_,y_,z_;
public:
  inline explicit Point(const Vector& v);
  inline Point(double x, double y, double z): x_(x), y_(y), z_(z) {}
  SLIVRSHARE Point(double, double, double, double);
  inline Point(const Point&);
  inline Point();
  SLIVRSHARE int operator==(const Point&) const;
  SLIVRSHARE int operator!=(const Point&) const;
  inline Point& operator=(const Point&);
  inline Vector operator+(const Point&) const;
  inline Vector operator-(const Point&) const;
  inline Point operator+(const Vector&) const;
  inline Point operator-(const Vector&) const;
  inline Point operator*(double) const;
  inline Point& operator*=(const double);
  inline Point& operator+=(const Vector&);
  inline Point& operator-=(const Vector&);
  inline Point& operator+=(const Point&);
  inline Point& operator-=(const Point&);
  inline Point& operator/=(const double);
  inline Point operator/(const double) const;
  inline Point operator-() const;
  inline double& operator()(int idx);
  inline double operator()(int idx) const;
  inline void addscaled(const Point& p, const double scale);  // this += p * w;
  inline void x(const double);
  inline double x() const;
  inline void y(const double);
  inline double y() const;
  inline void z(const double);
  inline double z() const;
  inline const Vector &vector() const;
  inline Vector &asVector() const;
    
  SLIVRSHARE string get_string() const;

  friend class Vector;
  friend inline double Dot(const Point&, const Point&);
  friend inline double Dot(const Vector&, const Point&);
  friend inline double Dot(const Point&, const Vector&);

  friend inline Point Min(const Point&, const Point&);
  friend inline Point Max(const Point&, const Point&);

  SLIVRSHARE friend Point AffineCombination(const Point&, double,
                                           const Point&, double,
                                           const Point&, double,
                                           const Point&, double);
  SLIVRSHARE friend Point AffineCombination(const Point&, double,
                                           const Point&, double,
                                           const Point&, double);
  SLIVRSHARE friend Point AffineCombination(const Point&, double,
                                           const Point&, double);

  friend inline Point Interpolate(const Point&, const Point&, double);

  // is one point within a small interval of another?

  SLIVRSHARE int Overlap( double a, double b, double e );
  SLIVRSHARE int InInterval( Point a, double epsilon );
    
  SLIVRSHARE friend std::ostream& operator<<(std::ostream& os, const Point& p);
  SLIVRSHARE friend std::istream& operator>>(std::istream& os, Point& p);

}; // end class Point


// Actual declarations of these functions (as 'friend' above doesn't
// (depending on the compiler) actually declare them.
SLIVRSHARE Point AffineCombination(const Point&, double, const Point&, double,
                                  const Point&, double, const Point&, double);
SLIVRSHARE Point AffineCombination(const Point&, double, const Point&, double, 
                                  const Point&, double);
SLIVRSHARE Point AffineCombination(const Point&, double, const Point&, double);

inline 
Point operator*(double d, const Point &p) {
  return p*d;
}
inline 
Point operator+(const Vector &v, const Point &p) {
  return p+v;
}

SLIVRSHARE std::ostream& operator<<(std::ostream& os, const Point& p);
SLIVRSHARE std::istream& operator>>(std::istream& os, Point& p);

} // End namespace SLIVR

// This cannot be above due to circular dependencies
#include "Vector.h"

namespace SLIVR {

inline Point::Point(const Vector& v)
  : x_(v.x_), y_(v.y_), z_(v.z_)
{
}

inline Point::Point(const Point& p)
{
  x_=p.x_;
  y_=p.y_;
  z_=p.z_;
}

inline Point::Point()
{
}


inline Point& Point::operator=(const Point& p)
{
  x_=p.x_;
  y_=p.y_;
  z_=p.z_;
  return *this;
}

inline Vector Point::operator+(const Point& p) const
{
  return Vector(x_+p.x_, y_+p.y_, z_+p.z_);
}

inline Vector Point::operator-(const Point& p) const
{
  return Vector(x_-p.x_, y_-p.y_, z_-p.z_);
}

inline Point Point::operator+(const Vector& v) const
{
  return Point(x_+v.x_, y_+v.y_, z_+v.z_);
}

inline Point Point::operator-(const Vector& v) const
{
  return Point(x_-v.x_, y_-v.y_, z_-v.z_);
}

inline Point& Point::operator+=(const Vector& v)
{
  x_+=v.x_;
  y_+=v.y_;
  z_+=v.z_;
  return *this;
}

inline Point& Point::operator-=(const Vector& v)
{
  x_-=v.x_;
  y_-=v.y_;
  z_-=v.z_;
  return *this;
}

inline Point& Point::operator+=(const Point& v)
{
  x_+=v.x_;
  y_+=v.y_;
  z_+=v.z_;
  return *this;
}

inline Point& Point::operator-=(const Point& v)
{
  x_-=v.x_;
  y_-=v.y_;
  z_-=v.z_;
  return *this;
}


inline Point& Point::operator*=(const double d)
{
  x_*=d;
  y_*=d;
  z_*=d;
  return *this;
}

inline Point& Point::operator/=(const double d)
{
  x_/=d;
  y_/=d;
  z_/=d;
  return *this;
}

inline Point Point::operator-() const
{
  return Point(-x_, -y_, -z_);
}

inline Point Point::operator*(double d) const
{
  return Point(x_*d, y_*d, z_*d);
}

inline Point Point::operator/(const double d) const
{
  return Point(x_/d,y_/d,z_/d);
}

inline double& Point::operator()(int idx) 
{
  return (&x_)[idx];
}

inline double Point::operator()(int idx) const 
{
  return (&x_)[idx];
}

inline Point Interpolate(const Point& v1, const Point& v2,
                         double weight)
{
  double weight1 = 1.0 - weight;
  return Point(v2.x_*weight+v1.x_*weight1,
               v2.y_*weight+v1.y_*weight1,
               v2.z_*weight+v1.z_*weight1);
}

inline void Point::addscaled(const Point& p, const double scale) 
{
  // this += p * w;
  x_ += p.x_ * scale;
  y_ += p.y_ * scale;
  z_ += p.z_ * scale;
}

inline void Point::x(const double d)
{
  x_=d;
}

inline double Point::x() const
{
  return x_;
}

inline void Point::y(const double d)
{
  y_=d;
}

inline double Point::y() const
{
  return y_;
}

inline void Point::z(const double d)
{
  z_=d;
}

inline double Point::z() const
{
  return z_;
}

inline const Vector &Point::vector() const
{
  return (const Vector &)(*this);
}

inline Vector &Point::asVector() const
{
  return (Vector &)(*this);
}

inline Point Min(const Point& p1, const Point& p2)
{
  double x=Min(p1.x_, p2.x_);
  double y=Min(p1.y_, p2.y_);
  double z=Min(p1.z_, p2.z_);
  return Point(x,y,z);
}

inline Point Max(const Point& p1, const Point& p2)
{

  double x=Max(p1.x_, p2.x_);
  double y=Max(p1.y_, p2.y_);
  double z=Max(p1.z_, p2.z_);
  return Point(x,y,z);
}

inline double Dot(const Point& p, const Vector& v)
{
  return p.x_*v.x_+p.y_*v.y_+p.z_*v.z_;
}

inline double Dot(const Point& p1, const Point& p2)
{
  return p1.x_*p2.x_ + p1.y_*p2.y_ + p1.z_*p2.z_;
}

} // End namespace SLIVR

#endif //ifndef SLIVR_Point_h
