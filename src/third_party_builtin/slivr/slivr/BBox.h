/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2008 Scientific Computing and Imaging Institute,
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
 *  BBox.h: Bounding box class
 *
 *  Written by:
 *   Steven G. Parker
 *   Department of Computer Science
 *   University of Utah
 *   1994
 *
 */

#ifndef SLIVR_BBox_h
#define SLIVR_BBox_h

#include <slivr/Point.h>
#include <slivr/share.h>
#include <ostream>

#ifdef _WIN32
#  undef min
#  undef max
#endif

namespace SLIVR {

class Vector;

class SLIVRSHARE BBox {
    
public:

  BBox() 
  : is_valid_(false) {}
    
  ~BBox() 
  {}
  
  BBox(const BBox& copy) 
  : cmin_(copy.cmin_), cmax_(copy.cmax_), is_valid_(copy.is_valid_) {}
  
  BBox& operator=(const BBox& copy)
  {
    is_valid_ = copy.is_valid_;
    cmin_ = copy.cmin_;
    cmax_ = copy.cmax_;
    return *this;
  }
    
  BBox(const Point& min, const Point& max)
    : cmin_(min), cmax_(max), is_valid_(true) {}
    
  inline int valid() const { return is_valid_; }

  inline void set_valid(bool v) { is_valid_ = v; }
  inline void reset() { is_valid_ = false; }

  //! Expand the bounding box to include point p
  inline void extend(const Point& p)
  {
    if(is_valid_)
    {
      cmin_=Min(p, cmin_);
      cmax_=Max(p, cmax_);
    } 
    else 
    {
      cmin_=p;
      cmax_=p;
      is_valid_ = true;
    }
  }

  //! Extend the bounding box on all sides by a margin
  //! For example to expand it by a certain epsilon to make
  //! sure that a lookup will be inside the bounding box
  inline void extend(double val)
  {
    if (is_valid_)
    {
      cmin_.x(cmin_.x()-val); 
      cmin_.y(cmin_.y()-val); 
      cmin_.z(cmin_.z()-val); 
      cmax_.x(cmax_.x()+val); 
      cmax_.y(cmax_.y()+val); 
      cmax_.z(cmax_.z()+val);     
    }
  }

  //! Expand the bounding box to include a sphere of radius radius
  //! and centered at point p
  inline void extend(const Point& p, double radius)
  {
    Vector r(radius,radius,radius);
    if(is_valid_)
    {
      cmin_=Min(p-r, cmin_);
      cmax_=Max(p+r, cmax_);
    } 
    else 
    {
      cmin_=p-r;
      cmax_=p+r;
      is_valid_ = true;
    }
  }

  //! Expand the bounding box to include bounding box b
  inline void extend(const BBox& b)
  {
    if(b.valid())
    {
      extend(b.min());
      extend(b.max());
    }
  }
  
  //! Expand the bounding box to include a disk centered at cen,
  //! with normal normal, and radius r.
  void extend_disk(const Point& cen, const Vector& normal, double r);

  inline Point center() const  
    { assert(is_valid_); Vector d = diagonal(); return cmin_ + (d * 0.5); }
  
  inline double longest_edge() const
  {
    assert(is_valid_);
    Vector diagonal(cmax_-cmin_);
    return Max(diagonal.x(), diagonal.y(), diagonal.z());
  }

  inline double shortest_edge() const
  {
    assert(is_valid_);
    Vector diagonal(cmax_-cmin_);
    return Min(diagonal.x(), diagonal.y(), diagonal.z());
  }

  //! Move the bounding box 
  void translate(const Vector &v);

  //! Scale the bounding box by s, centered around o
  void scale(double s, const Vector &o);

  inline Point min() const
    { return cmin_; }
  
  inline Point max() const
    { return cmax_; }

  inline Vector diagonal() const
    { assert(is_valid_); return cmax_-cmin_; }

  inline bool inside(const Point &p) const 
  {
    return (is_valid_ && p.x() >= cmin_.x() && 
	    p.y() >= cmin_.y() && p.z() >= cmin_.z() && 
	    p.x() <= cmax_.x() && p.y() <= cmax_.y() && 
	    p.z() <= cmax_.z());
  }

  //! bbox's that share a face overlap
  bool overlaps(const BBox& bb) const;
  //! bbox's that share a face do not overlap_inside
  bool overlaps_inside(const BBox& bb) const;

  //! returns true if the ray hit the bbox and returns the hit point
  //! in hitNear
  bool intersect(const Point& e, const Vector& v, Point& hitNear);

  friend std::ostream& operator<<(std::ostream& out, const BBox& b);

private:
  Point cmin_;
  Point cmax_;
  bool is_valid_;
};

} // End namespace SLIVR

#endif
