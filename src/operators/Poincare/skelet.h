// ===================================================================
//
// skelet.h
//
//
// Straight skeleton approximating the medial axis based on the
// following research by Mirela Tănase and Remco C. Veltkamp:

// A Straight Skeleton Approximating the Medial Axis.
// Mirela Tănase and Remco C. Veltkamp
// ALGORITHMS – ESA 2004
// Lecture Notes in Computer Science, 2004, Volume 3221/2004, 809-821,
// DOI: 10.1007/978-3-540-30140-0_71

// Straight line skeleton in linear time, topologically equivalent to
// the medial axis.
// Mirela Tănase and Remco C. Veltkamp
// Proceedings EWCG 2004, 20th European Workshop on Computational
// Geometry, pages 185–188, 2004.

// Copyright 1998, Mirela Tanase, Utrecht University

// This code may be freely used and modified for any purpose providing
// that this copyright notice is included with it.

// The author makes no warranty for this code, and cannot be held
// liable for any real or imagined damage resulting from its use.
// Users of this code must verify correctness for their application.

// Any usage of this code must acknowledge the above references.

// Contact information:
// Remco Veltkamp <R.C.Veltkamp@uu.nl>
// Department of Information and Computing Sciences, Utrecht University 
// Princetonplein 5, 3584 CC Utrecht, The Netherlands 

// ===================================================================

#ifndef __SKELET_H_INCLUDED
#define __SKELET_H_INCLUDED

#define NDEBUG 1

#include <math.h>
#include <assert.h>
#include <iostream>
#include <fstream>
#include <string>

#include <vector>
#include <deque>
#include <list>
#include <queue>
#include <functional>

namespace Skeleton {

/**********************************************************
 From prim.h
***********************************************************/

typedef double Number;
#ifndef M_PI
#define M_PI            3.14159265358979323846
#endif
#define MAXDOUBLE       1.7E+300
#ifndef INFINITY
#define INFINITY        MAXDOUBLE
#endif
#define MIN_DIFF        0.00005
#define MIN_ANGLE_DIFF  0.0000005

#define SIMILAR(a,b) ((a)-(b) < MIN_DIFF && (b)-(a) < MIN_DIFF)

#define ANGLE_SIMILAR(a,b) \
  (normalizedAngle (a) - normalizedAngle (b) < MIN_ANGLE_DIFF && \
   normalizedAngle (b) - normalizedAngle (a) < MIN_ANGLE_DIFF)

class Point
{
public:
  Point (Number X = 0, Number Y = 0) : x (X), y (Y) { }

  Number x, y;

  bool operator == (const Point &p) const 
  { 
    return SIMILAR (x, p.x) && SIMILAR (y, p.y); 
  }

  Point operator * (const Number &n) const 
  { 
    return Point (n*x, n*y); 
  }

  int isInfinity (void) 
  { 
    return *this == Point (INFINITY, INFINITY); 
  }

  void normalize( )
  {
    double mag = sqrt(x*x + y*y);

    if (mag != 0) 
    { 
      x /= mag;
      y /= mag;
    }
  }
};


inline bool operator != (const Point &p1, const Point &p2) 
{ 
  return !(p1 == p2);
}

  
inline std::ostream &operator << (std::ostream &os, const Point &p) 
{ 
  os << '[' << p.x << "; " << p.y << ']'; 
  return os; 
}


class Ray
{
public:
  Ray (const Point &p = Point (0, 0), const Point &q = Point (0, 0));
  Ray (const Point &p, const Number &a) : origin (p), angle (a) { };
  Ray opaque (void) const 
  { 
    return Ray (origin, angle + M_PI); 
  }
   
  Point origin;
  Number angle;
};

inline std::ostream &operator << (std::ostream &os, const Ray &l)
{ 
  os << '[' << l.origin.x << "; "
     << l.origin.y << "; "
     << 180*l.angle/M_PI << ']'; 

  return os; 
}

struct Segment
{
  Segment (const Point &p = Point (),
           const Point &q = Point ()) : a (p), b (q) { };
  Segment (Number x1, Number y1,
           Number x2, Number y2) : a (x1, y1), b (x2, y2) { }
  
  Point a, b;
};

inline std::ostream &operator << (std::ostream &os, const Segment &l)
{
  os << "line (" << l.a << ", " << l.b << ")\n";
  return os;
}

// vrati osu uhlu bac prochazejici bodem a (returns the angle between
// b and c axis and passing through)
Ray angleAxis (const Point &a, const Point &b, const Point &c);

Point intersection (const Ray& a, const Ray &b);
Point intersectionAnywhere (const Ray& a, const Ray &b);
Number dist (const Point &p, const Point &q);
Number dist (const Point &p, const Ray &l);
Number dist (const Point &p, const Segment &l);
Number &normalizeAngle (Number &angle);

inline Number normalizedAngle (Number angle) 
{ 
  Number temp = angle; 
  normalizeAngle (temp); 
  return temp; 
}
  
inline int pointOnRay (const Point &p, const Ray &r) 
{ 
  return p == r.origin || ANGLE_SIMILAR (Ray (r.origin, p).angle, r.angle); 
}
  
inline int facingTowards (const Ray &a, const Ray &b) 
{ 
  return pointOnRay (a.origin, b) && pointOnRay (b.origin, a) && !(a.origin == b.origin); 
}
  
inline Number dist (const Ray &l, const Point &p)
{
  return dist (p, l);
}

inline Number dist (const Segment &l, const Point &p)
{
  return dist (p, l);
}

int colinear (const Ray &a, const Ray &b);
  
int colinearOpDir (const Ray &a, const Ray &b);


/******************************************************************************
 Definice trid  (Definitions of Classes)
******************************************************************************/

struct SkeletonLine;

struct VertexSkelet
{
  VertexSkelet (void) : ID (-1) { };  // Parameterless constructor using STL

  VertexSkelet (const Point &p,
                const Point &prev = Point (),
                const Point &next = Point ())
    : point (p), axis (angleAxis (p, prev, next)),
      leftLine (p, prev), rightLine (p, next),
      leftVertex (NULL), rightVertex (NULL),
      nextVertex (NULL), prevVertex (NULL), higher (NULL),
      done (false), ID (-1),
      leftSkeletonLine (NULL), rightSkeletonLine (NULL),
      advancingSkeletonLine (NULL) { }

  VertexSkelet (const Point &p, VertexSkelet &left, VertexSkelet &right);

  VertexSkelet *highest (void) { 
    return higher ? higher -> highest () : this; }

  bool atContour (void) const { 
    return leftVertex == this && rightVertex == this; }

  bool operator == (const VertexSkelet &v) const { 
    return point == v.point; }

  bool operator < (const VertexSkelet &v) const { 
    assert 
      (false); 
    return false; } 
  // data
  Point point;             // vertex coordinates

  Ray axis;                // bi-sector

  Ray leftLine, rightLine; // left and right boundary line, axis is
                           // the axis (je jejich osou?)

  VertexSkelet *leftVertex, *rightVertex;  // 2 vrcholy, jejich
                                           // zaniknutim vzikl tento
                                           // (two peaks, ...)

  VertexSkelet *nextVertex, *prevVertex;   // vrchol sousedici v LAV
                                           // (adjacent vertices in
                                           // the contiguous LAV)
  VertexSkelet *higher;  // vrchol vznikly pri zaniknuti tohoto (peak
                         // arose at the expiration of this)

  bool done;  // flag activities

  int ID;  // cislo automaticky pridelovane pri vkladani do LAV
           // (allotment of number automatically inserting the LAV)

  //Pouzivano pri konstrukci kostry z okridlenych hran (Used at
  //construction of the skeleton of the winged edge)
  SkeletonLine *leftSkeletonLine, *rightSkeletonLine, *advancingSkeletonLine;
};

struct Intersection
{
  Intersection (void) { };          
  Intersection (VertexSkelet &v);         
  Point poi;                         
  VertexSkelet *leftVertex, *rightVertex; 
  Number height;                     
  enum Type { CONVEX, NONCONVEX } type;
   
  bool operator < (const Intersection &i) const 
  {
    return ((height < i.height) || (( height == i.height ) && (type > i.type)));
  }  

  bool operator > (const Intersection &i) const
  {
    if (i.height < 10e300) 
      return (((height - i.height) > 10e-7) ||
              ((fabs(height - i.height)<10e-7) && (type > i.type)));
    else return false;
  }

  bool operator == (const Intersection &i) const { 
    return ((height == i.height) && ( poi == i.poi)); }
};

#if (defined __WATCOMC__) 
typedef priority_queue <vector <Intersection>, greater <Intersection> > IntersectionQueue;
#else
  typedef std::priority_queue <
  Intersection, 
  std::vector<Intersection>,
  std::greater<Intersection> > IntersectionQueue;
#endif

class VertexList : public std::list <VertexSkelet>
{
public:
  VertexList (void) { }

  iterator prev (const iterator &i)
  { 
    iterator tmp (i); 
    if (tmp == begin ()) 
      tmp = end (); 
    tmp --; 
    return tmp;
  } // => cyklicky

  iterator next (const iterator &i)
  { 
    iterator tmp (i); 
    tmp ++; 
    if (tmp == end ()) 
      tmp = begin (); 
    return tmp;
  } // => cyklicky

  void push_back (const VertexSkelet& x)
  {
    assert 
      (x.prevVertex == NULL || facingTowards (x.leftLine, x.prevVertex -> rightLine));
    assert 
      (x.nextVertex == NULL || facingTowards (x.rightLine, x.nextVertex -> leftLine));

    ((VertexSkelet &)x).ID = (int)size ();       // automatic numbering

    std::list <VertexSkelet> :: push_back (x);
  }
};

struct SkeletonLine
{
  SkeletonLine (void) : ID (-1) { }

  SkeletonLine (const VertexSkelet &l, const VertexSkelet &h) : lower (l), higher (h), ID (-1) { };
  
  operator Segment (void) { 
    return Segment (lower.vertex -> point,
                    higher.vertex -> point); } // Only for debugging
   
  struct SkeletonPoint
  {
    SkeletonPoint (const VertexSkelet &v = VertexSkelet (),
                   SkeletonLine *l = NULL, SkeletonLine *r = NULL)
      : vertex (&v), left (l), right (r)
    {
    }

    const VertexSkelet *vertex;  // pointer to the vertex (contains coordinates)

    SkeletonLine *left, *right;  // left and right neighboring vertices

    int leftID (void) const
    { 
      if (!left) 
        return -1; 
      return left -> ID;
    }

    int rightID (void) const
    { 
      if (!right) 
        return -1; 
      return right -> ID;
    }

    int vertexID (void) const
    { 
      if (!vertex) 
        return -1; 
      return vertex -> ID;
    }
  } lower, higher;  // two points of type SkeletonPoint (dva body typu
                    // SkeletonPoint)

  bool operator == (const SkeletonLine &s) const
  { 
    return higher.vertex -> ID == s.higher.vertex -> ID &&
      lower.vertex -> ID  == s.lower.vertex -> ID;
  }

  bool operator < (const SkeletonLine &s) const
  { 
    assert 
      (false); 
    return false;
  } // kvuli STL, jinak se nepouziva (using STL, otherwise not use?)
  
  int ID;  // Cislo automaticky pridelovane pri vkladani do kostry
           // (The index will automatically be assigned when inserting
           // into the skeleton)
};

class Skeleton : public std::list <SkeletonLine>
{
public:
  void push_back (const SkeletonLine &x)
  {
#ifdef EPS
//  *epsStream << x.lower.vertex -> point.x << ' ' << x.lower.vertex -> point.y << " moveto ";
//  *epsStream << x.higher.vertex -> point.x << ' ' << x.higher.vertex -> point.y << " lineto\n";
#endif

    ((SkeletonLine &)x).ID = (int)size ();     // automatic numbering 
    std::list <SkeletonLine> :: push_back (x);
  }
};

typedef std::vector <Point>       PointVector;
typedef std::vector <PointVector> PointVectorVector;

Skeleton &makeSkeleton (PointVector &points);

} // end namespace Skeleton

#endif
