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
 *  Plane.cc: Uniform Plane containing triangular elements
 *
 *  Written by:
 *   David Weinstein
 *   Department of Computer Science
 *   University of Utah
 *   October 1994
 *
 */


#include <slivr/Plane.h>
#include <slivr/Point.h>
#include <slivr/Vector.h>
#include <iostream>

namespace SLIVR {

Plane::Plane()
  : n(Vector(0,0,1)), d(0)
{
}

Plane::Plane(double a, double b, double c, double d) : n(Vector(a,b,c)), d(d) {
  double l=n.length();
  d/=l;
  n.normalize();
}



Plane::Plane(const Point &p, const Vector &normal)
  : n(normal), d(-Dot(p, normal))
{
}

Plane::Plane(const Plane &copy)
  : n(copy.n), d(copy.d)
{
}

Plane::Plane(const Point &p1, const Point &p2, const Point &p3) {
  Vector v1(p2-p1);
  Vector v2(p2-p3);
  n=Cross(v2,v1);
  n.normalize();
  d=-Dot(p1, n);
}

Plane::~Plane() {
}

void Plane::flip() {
  n.x(-n.x());
  n.y(-n.y());
  n.z(-n.z());
  d=-d;
}

double Plane::eval_point(const Point &p) const {
  return Dot(p, n)+d;
}

Point Plane::project(const Point& p) const
{
  return p-n*(d+Dot(p,n));
}

Vector Plane::project(const Vector& v) const
{
  return v-n*Dot(v,n);
}

Vector Plane::normal() const
{
  return n;
}

void
Plane::ChangePlane(const Point &p1, const Point &p2, const Point &p3) {
  Vector v1(p2-p1);
  Vector v2(p2-p3);
  n=Cross(v2,v1);
  n.normalize();
  d=-Dot(p1, n);
}


void
Plane::ChangePlane(const Point &P, const Vector &N) {
  n = N;
  n.safe_normalize();
  d = -Dot(P,n);
}



int
Plane::Intersect( Point s, Vector v, Point& hit ) const
{
  Point origin( 0., 0., 0. );
  Point ptOnPlane = origin - n * d;
  double tmp = Dot( n, v );

  if( tmp > -1.e-6 && tmp < 1.e-6 ) // Vector v is parallel to plane
  {
    // vector from origin of line to point on plane
      
    Vector temp = s - ptOnPlane;

    if ( temp.length() < 1.e-5 )
    {
      // the origin of plane and the origin of line are almost
      // the same
	  
      hit = ptOnPlane;
      return 1;
    }
    else
    {
      // point s and d are not the same, but maybe s lies
      // in the plane anyways
	  
      tmp = Dot( temp, n );

      if(tmp > -1.e-6 && tmp < 1.e-6)
      {
	hit = s;
	return 1;
      }
      else
	return 0;
    }
  }

  tmp = - ( ( d + Dot( s, n ) ) / Dot( v, n ) );

  hit = s + v * tmp;

  return 1;
}


#if 0
int
Plane::Intersect( Point s, Vector v, double &t ) const 
{
  Point origin( 0., 0., 0. );
  Point ptOnPlane = origin - n * d;
  double tmp = Dot( n, v );

  if( tmp > -1.e-6 && tmp < 1.e-6 ) // Vector v is parallel to plane
  {
    // vector from origin of line to point on plane
      
    Vector temp = s - ptOnPlane;

    if ( temp.length() < 1.e-5 )
    {
      // the origin of plane and the origin of line are almost
      // the same
	  
      t = 0.0;
      return 1;
    }
    else
    {
      // point s and d are not the same, but maybe s lies
      // in the plane anyways
	  
      tmp = Dot( temp, n );

      if(tmp > -1.e-6 && tmp < 1.e-6)
      {
	t = 0.0;
	return 1;
      }
      else
	return 0;
    }
  }

  t = - ( ( d + Dot( s, n ) ) / Dot( v, n ) );

  return 1;
}

#else
int
Plane::Intersect(Point s, Vector v, double &t) const
{
  double tmp = Dot( n, v );
  if(tmp > -1.e-6 && tmp < 1.e-6) // Vector v is parallel to plane
  {
    // vector from origin of line to point on plane
    Vector temp = (s + n*d).asVector();
    if (temp.length() < 1.e-5)
    {
      // origin of plane and origin of line are almost the same
      t = 0.0;
      return 1;
    }
    else
    {
      // point s and d are not the same, but maybe s lies
      // in the plane anyways
      tmp = Dot(temp, n);
      if (tmp > -1.e-6 && tmp < 1.e-6)
      {
	t = 0.0;
	return 1;
      }
      else
	return 0;
    }
  }
  
  t = -((d + Dot(s, n)) / Dot(v, n));
  return 1;
}
#endif



void
Plane::get(double (&abcd)[4]) const
{
  abcd[0] = n.x();
  abcd[1] = n.y();
  abcd[2] = n.z();
  abcd[3] = d;
}


bool
Plane::operator==(const Plane &rhs) const { 
  double cosine = Dot(this->n, rhs.n);
  double d1 = this->eval_point(Point(0,0,0));
  double d2 = rhs.eval_point(Point(0,0,0));
  return (fabs(double(d1-d2)) < 0.000001) && (fabs(double(cosine-1.0)) < 0.00001); 
}


} // End namespace SLIVR

