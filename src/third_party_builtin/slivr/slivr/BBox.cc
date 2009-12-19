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
 *  BBox.cc: Bounding box class
 *
 *  Written by:
 *   Steven G. Parker
 *   Department of Computer Science
 *   University of Utah
 *   1994
 *
 */

#include <slivr/BBox.h>
#include <slivr/Vector.h>
#include <slivr/Utils.h>
#include <assert.h>
#include <math.h>


using namespace SLIVR;
using namespace std;
    
void 
BBox::extend_disk(const Point& cen, const Vector& normal, double r)
{
  if (normal.length2() < 1.e-12) { extend(cen); return; }
  Vector n(normal);
  n.safe_normalize();
  double x=sqrt(1-n.x())*r;
  double y=sqrt(1-n.y())*r;
  double z=sqrt(1-n.z())*r;
  extend(cen+Vector(x,y,z));
  extend(cen-Vector(x,y,z));
}

void 
BBox::translate(const Vector &v)
{
  cmin_+=v;
  cmax_+=v;
}

void 
BBox::scale(double s, const Vector&o)
{
  cmin_-=o;
  cmax_-=o;
  cmin_*=s;
  cmax_*=s;
  cmin_+=o;
  cmax_+=o;
}

bool 
BBox::overlaps(const BBox & bb) const
{
  if( bb.cmin_.x() > cmax_.x() || bb.cmax_.x() < cmin_.x())
    return false;
  else if( bb.cmin_.y() > cmax_.y() || bb.cmax_.y() < cmin_.y())
    return false;
  else if( bb.cmin_.z() > cmax_.z() || bb.cmax_.z() < cmin_.z())
    return false;

  return true;
}

bool 
BBox::overlaps_inside(const BBox & bb) const
{
  if( bb.cmin_.x() >= cmax_.x() || bb.cmax_.x() <= cmin_.x())
    return false;
  else if( bb.cmin_.y() >= cmax_.y() || bb.cmax_.y() <= cmin_.y())
    return false;
  else if( bb.cmin_.z() >= cmax_.z() || bb.cmax_.z() <= cmin_.z())
    return false;

  return true;
}

bool 
BBox::intersect(const Point& origin, const Vector& dir,
                Point& hitPoint)
{
  Vector t1 = (cmin_ - origin) / dir;
  Vector t2 = (cmax_ - origin) / dir;
  Vector tn = Min(t1, t2);
  Vector tf = Max(t1, t2);
  double tnear = tn.maxComponent();
  double tfar = tf.minComponent();
  if(tnear <= tfar){
    hitPoint = origin + dir*tnear;
    return true;
  } else {
    return false;
  }
}

