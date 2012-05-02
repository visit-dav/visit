// ===================================================================
//
// skelet.C
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

#include "skelet.h"

#include <math.h>
#include <assert.h>
#include <iostream>
#include <fstream>
#include <string>
#include <algorithm>

#include <vector>
#include <deque>
#include <list>
#include <queue>

#define SCREEN_WIDTH  640
#define SCREEN_HEIGHT 480


namespace Skeleton {

#ifdef __WATCOMC__
void line (const Segment &line, int color = 6);
void cls (void);
#endif

#ifdef EPS         
// std::ostream *epsStream;

// void initEPS (const char *filename)
// {
//   epsStream = new std::ofstream (filename);
//   *epsStream << "%!PS-Adobe-2.0 EPSF-2.0"                   << std::endl
//           << "%%BoundingBox: 0 0 " << SCREEN_WIDTH << ' '<< SCREEN_HEIGHT << std::endl
//           << "%%Title: Kostra"                           << std::endl
//           << "%%CreationDate: "  << std::endl;
// }

// void doneEPS (void)
// {
//   *epsStream << "stroke showpage end" << std::endl;
//   epsStream -> flush ();
//   delete epsStream;
// }
#endif



/**************************************************************************
   From prim.C
***************************************************************************/
Ray::Ray (const Point &p, const Point &q)
  : origin (p)
{
  angle = atan2 (q.y - p.y, q.x - p.x);
}

// returns the angle abc axis passing through b
Ray angleAxis (const Point &b, const Point &a, const Point &c)
{
  Ray ba (b, a);
  Ray bc (b, c);
  if (ba.angle > bc.angle) 
    ba.angle -= 2*M_PI;
  return Ray (b, (ba.angle + bc.angle) / 2);
}

Point intersection (const Ray& a, const Ray &b)
{
  if (a.origin == b.origin) 
    return a.origin;
  if (pointOnRay (b.origin, a) && pointOnRay (a.origin, b))
    return Point ((a.origin.x + b.origin.x)/2, (a.origin.y + b.origin.y)/2);
  if (pointOnRay (b.origin, a)) 
    return b.origin;
  if (pointOnRay (a.origin, b)) 
    return a.origin;
  if (colinear (a, b)) 
    return Point (INFINITY, INFINITY);

  Number sa = sin (a.angle);
  Number sb = sin (b.angle);
  Number ca = cos (a.angle);
  Number cb = cos (b.angle);
  Number x = sb*ca - sa*cb;
  if (SIMILAR (x, 0)) 
    return Point (INFINITY, INFINITY);
  Number u = (cb*(a.origin.y - b.origin.y) - sb*(a.origin.x - b.origin.x))/x;
  if (!SIMILAR (u, 0) && u < 0) 
    return Point (INFINITY, INFINITY);
  if ((ca*(b.origin.y - a.origin.y) - sa*(b.origin.x - a.origin.x))/x > 0) 
    return Point (INFINITY, INFINITY);
  return Point (a.origin.x + u*ca, a.origin.y + u*sa);
};


Point intersectionAnywhere (const Ray& a, const Ray &b)
{
  if (a.origin == b.origin) 
    return a.origin;
  if (pointOnRay (b.origin, a) && pointOnRay (a.origin, b))
    return Point ((a.origin.x + b.origin.x)/2, (a.origin.y + b.origin.y)/2);
  if (pointOnRay (b.origin, a)) 
    return b.origin;
  if (pointOnRay (a.origin, b)) 
    return a.origin;
  if (pointOnRay (b.origin, a.opaque ()) && pointOnRay (a.origin, b.opaque ()))
    return Point ((a.origin.x + b.origin.x)/2, (a.origin.y + b.origin.y)/2);

  if (colinear (a, b)) 
    return Point (INFINITY, INFINITY);

  Number sa = sin (a.angle);
  Number sb = sin (b.angle);
  Number ca = cos (a.angle);
  Number cb = cos (b.angle);
  Number x = sb*ca - sa*cb;

  if (x == 0) 
    return Point (INFINITY, INFINITY);

  Number u = (cb*(a.origin.y - b.origin.y) - sb*(a.origin.x - b.origin.x))/x;

  return Point (a.origin.x + u*ca, a.origin.y + u*sa);
};

Number dist (const Point &p, const Point &q)
{
  return sqrt ((p.x-q.x)*(p.x-q.x) + (p.y - q.y)*(p.y - q.y));
}

Number dist (const Point &p, const Ray &l)
{
  Number a = l.angle - Ray (l.origin, p).angle;
  Number d = sin (a) * dist (l.origin, p);

  if (d < 0) 
    return -d;
  else
    return d;
}

Number dist (const Point &p, const Segment &l)
{
  Ray hl1 (l.a, l.b);
  Ray hl2 (p, hl1.angle + M_PI/2);
  Point i = intersectionAnywhere (hl1, hl2);

  if (i.x < l.a.x + MIN_DIFF && i.x > l.b.x - MIN_DIFF) 
    return dist (i, p);

  if (i.x > l.a.x - MIN_DIFF && i.x < l.b.x + MIN_DIFF) 
    return dist (i, p);

  return std::min (dist (p, l.a), dist (p, l.b));
}

Number &normalizeAngle (Number &angle)
{
  if (angle >=  M_PI) 
  { 
    angle -= 2*M_PI; 
    return normalizeAngle (angle); 
  }
  if (angle < -M_PI) 
  {
    angle += 2*M_PI; 
    return normalizeAngle (angle); 
  }

  return angle;
}

int colinear (const Ray &a, const Ray &b)
{
  Number aa = a.angle;
  Number ba = b.angle;
  Number aa2 = a.angle + M_PI;
  normalizeAngle (aa);
  normalizeAngle (ba);
  normalizeAngle (aa2);

  return ANGLE_SIMILAR (ba, aa) || ANGLE_SIMILAR (ba, aa2);
}

int colinearOpDir (const Ray &a, const Ray &b)
{
  Number aa = a.angle;
  Number ba = b.angle;
  normalizeAngle (aa);
  Number aa2;
  if (!(aa == -M_PI ))
    aa2 = a.angle + M_PI;
  else 
    aa2 = a.angle;  
  normalizeAngle (ba);
  normalizeAngle (aa2);
  return ANGLE_SIMILAR (ba, aa2);
}

/******************************************************************************
 Operace pro vystup  (Operations for output)
 ******************************************************************************/
inline std::ostream &operator << (std::ostream &os, const VertexSkelet &v)
{
  os << v.ID<<" > ";
  if (v.advancingSkeletonLine) 
    os << v.higher -> ID;
  else 
    os << -1;
  return os;
}

inline std::ostream &operator << (std::ostream &os, const VertexList &v)
{
  VertexList :: const_iterator i = v.begin ();
  os << v.size () << std::endl;
  while (i != v.end ()) { 
    os << *i ; 
    i++; }
  return os;
}

inline std::ostream &operator << (std::ostream &os, const SkeletonLine &sl)
{
  os << sl.lower.vertexID () << ' '  << sl.lower.vertex->point<<' ' 
     << sl.higher.vertexID () << ' '<< sl.higher.vertex->point<<std::endl;

  int IDH1, IDH2, IDL1, IDR1, IDL2, IDR2, IDP1, IDN1, IDP2, IDN2;

  if ((sl.lower.vertex) -> higher)
    IDH1 = (sl.lower.vertex) -> higher -> ID;
  else IDH1=-1;

  if ((sl.lower.vertex) -> leftVertex)
    IDL1 = (sl.lower.vertex) -> leftVertex ->ID;
  else IDL1=-1;

  if ((sl.lower.vertex) -> rightVertex)
    IDR1 = (sl.lower.vertex) -> rightVertex ->ID;
  else IDR1=-1;

  if ((sl.lower.vertex) -> prevVertex)
    IDP1 = (sl.lower.vertex) -> prevVertex ->ID;
  else IDP1=-1;

  if ((sl.lower.vertex) -> nextVertex)
    IDN1 = (sl.lower.vertex) -> nextVertex ->ID;
  else IDN1=-1;

  if ((sl.higher.vertex) -> higher)
    IDH2 = (sl.higher.vertex) -> higher -> ID;
  else IDH2=-1;

  if ((sl.higher.vertex) -> leftVertex)
    IDL2 = (sl.higher.vertex) -> leftVertex ->ID;
  else IDL2=-1; 

  if ((sl.higher.vertex) -> rightVertex)
    IDR2 = (sl.higher.vertex) -> rightVertex ->ID;
  else IDR2=-1; 

  if ((sl.higher.vertex) -> prevVertex)
    IDP2 = (sl.higher.vertex) -> prevVertex ->ID;
  else IDP2=-1; 

  if ((sl.higher.vertex) -> nextVertex)
    IDN2 = (sl.higher.vertex) -> nextVertex ->ID;
  else IDN2=-1; 

  os<<sl.lower.vertexID () <<"-> " << IDH1<<" l:"<< IDL1<<" r:"<<IDR1<<" p: "<<IDP1<<" n:"<<IDN1<<std::endl;
  os<<sl.higher.vertexID ()<<"-> " << IDH2 <<" l:"<< IDL2<<" r:"<<IDR2<<" p: "<<IDP2<<" n:"<<IDN2<<std::endl;

  return os;
}

inline std::ostream &operator << (std::ostream &os, const Skeleton &s)
{
  os << s.size () << std::endl;
  for (Skeleton :: const_iterator si = s.begin (); si != s.end (); si++)
    os << *si;
  return os;
}

/******************************************************************************
 Global variables
 ******************************************************************************/

IntersectionQueue iq;          // prioritni fronta pruseciku setridena
                               // podle vzdalenosti od nositelky
                               // (tj. podle vysky ve strese/ i.e. the
                               // height of the roof)
VertexList        vl;          // SLAV, jednotlive cykly (LAV) jsou
                               // udrzovany pomoci ukazatelu
                               // nextVertex, prevVertex (... cycles
                               // are conserved with indicators...)
Skeleton skeleton;             // output structure containing the skeleton
// std::fstream out_file;
int NrCP;
std::fstream OutFile;



/******************************************************************************
 Function algorithms
 ******************************************************************************/

VertexSkelet :: VertexSkelet (const Point &p,
                              VertexSkelet &left,
                              VertexSkelet &right)  // tvorba vrcholu
                                                    // na miste
                                                    // pruseciku (p)
                                                    // (formation on
                                                    // top of the site
                                                    // of intersection
                                                    // p)
  : point (p), done (false), higher (NULL), ID (-1),
    leftSkeletonLine (NULL), rightSkeletonLine (NULL),
    advancingSkeletonLine (NULL)
{
  this -> leftLine = left.leftLine;  // hrany puvodni kontury, jejichz
                                     // osou bude vytvareny bisektor
                                     // vedouci z tohoto vrcholu
                                     // (edges of the original
                                     // contours, whose axis will be
                                     // generated bisector leading
                                     // from the top)
  this -> rightLine = right.rightLine;
  this -> leftVertex = &left;
  this -> rightVertex = &right;
   
  assert 
    (SIMILAR (dist (point, leftLine),
              dist (point, rightLine))); // vytvareny vchol musi byt
                                         // stejne daleko od obou hran
                                         // (must be created with same
                                         // distance from both edges)
  Point i = intersection (leftLine, rightLine);  // pro urceni smeru
                                                 // bisektoru je
                                                 // potreba znat
                                                 // souradnice (to
                                                 // determine the
                                                 // direction you need
                                                 // to know the
                                                 // coordinates of
                                                 // bisectors)
  // prusecik neni smerem dopredu (intersection point is not forward)
  // // jeste jednoho bodu, vhodnym adeptem je prusecik nositelek (one
  // more point, adept is a suitable intersection point wearers)
  if (i.x == INFINITY)
  {   // hran puvodni kontury (edges of the original contours)
    assert (i.y == INFINITY);

    i = intersectionAnywhere (leftLine, rightLine);  // Anywhere =>
                                                     // prusecik
                                                     // primek a ne
                                                     // poloprimek
                                                     // (anywhere =>
                                                     // meeting point
                                                     // and no rays)
    if (i.x == INFINITY)  // rovnobezne hrany (compatibility edges)
    {
      assert (i.y == INFINITY);
      axis = Ray (point, leftLine.angle);  // tvorba bisektoru pri
                                           // rovnobeznych hranach =>
                                           // || s nimi (creation of
                                           // parallel edges in
                                           // bisector => s them)
    }
    else // prusecik smerem dozadu (intersection point of the rear)
    {
      axis = Ray (point, i);
      axis.angle += M_PI;  // tvorba bisektoru (formation of bisector)
    }
  }
  else  // prusecik smerem dopredu (intersection point forward)
  {
    axis = Ray (point, i);  // tvorba bisektoru (formation of bisector)
  }
}



//bool intersectionFromLeft (const Ray &l, const VertexSkelet &v) // pouze pro ladeni
//{
//  if (!(intersection (l, v.axis) == Point (INFINITY, INFINITY))) 
//    return false;
//  if (v.rightVertex == &v) 
//    return false;
//  if (!(intersection (l, v.rightVertex -> axis) == Point (INFINITY, INFINITY))) 
//    return true;
//  return intersectionFromLeft (l, *v.rightVertex);
//}
//
//bool intersectionFromRight (const Ray &l, const VertexSkelet &v) // pouze pro ladeni
//{
//  if (!(intersection (l, v.axis) == Point (INFINITY, INFINITY))) 
//    return false;
//  if (v.leftVertex == &v) 
//    return false;
//  if (!(intersection (l, v.leftVertex -> axis) == Point (INFINITY, INFINITY))) 
//    return true;
//  return intersectionFromRight (l, *v.leftVertex);
//}

// vrati souradnice pruseciku (returns the coordinates of the intersections)
Point coordinatesOfAnyIntersectionOfTypeB (const VertexSkelet &v,
                                           const VertexSkelet &left,
                                           const VertexSkelet &right)
{
  Point p1 = intersectionAnywhere (v.rightLine, right.leftLine);
  Point p2 = intersectionAnywhere (v.leftLine, left.rightLine);
  Point poi;
  if (!(p1 == Point (INFINITY, INFINITY)) && !(p2 == Point (INFINITY, INFINITY)))
  {
    if (pointOnRay (p1, v.rightLine)) 
      return Point (INFINITY, INFINITY);
    if (pointOnRay (p2, v.leftLine))  
      return Point (INFINITY, INFINITY);
    poi = intersectionAnywhere (angleAxis (p1, p2, v.point), v.axis); 
    // out_file<<" in funct. coordanyintB "<<poi<<" ";
  }
  else //if (p1 != Point (INFINITY, INFINITY))
  // specialni pripad rovnobeznosti (special case of parallelism)
  {
    poi = intersectionAnywhere (left.rightLine, v.axis);
    poi.x = (poi.x + v.point.x) / 2;  // to je pak prusecik na
                                      // polovicni vzdalenosti (the
                                      // intersection point at half
                                      // the distance)
    poi.y = (poi.y + v.point.y) / 2;  // mezi vrcholem V a protilehlou
                                      // hranou (in between the top
                                      // and the opposite edge)
  }

  return poi;
}

// vrati souradnice pruseciku jen pokud je platny (lezi ve vyseci)
// (returns the coordinates of the intersection only if it is valid
// (lies in the arc))
Point intersectionOfTypeB (const VertexSkelet &v,
                           const VertexSkelet &left,
                           const VertexSkelet &right)
{
  assert 
    (v.prevVertex == NULL || facingTowards (v.leftLine, v.prevVertex -> rightLine));
 
  assert 
    (v.nextVertex == NULL || facingTowards (v.rightLine, v.nextVertex -> leftLine));
  assert 
    (left.prevVertex == NULL || facingTowards (left.leftLine, left.prevVertex -> rightLine));
  assert 
    (left.nextVertex == NULL || facingTowards (left.rightLine, left.nextVertex -> leftLine));
  assert 
    (right.prevVertex == NULL || facingTowards (right.leftLine, right.prevVertex -> rightLine));
  assert 
    (right.nextVertex == NULL || facingTowards (right.rightLine, right.nextVertex -> leftLine));
  
  Point pl (intersection (v.axis, left.rightLine));  // test protne-li
                                                     // bisektor danou
                                                     // hranu (test to
                                                     // intersect the
                                                     // edge of the
                                                     // bisector)
  Point pr (intersection (v.axis, right.leftLine));
  if (pl == Point (INFINITY, INFINITY) && pr == Point (INFINITY, INFINITY))
    return Point (INFINITY, INFINITY);  // lezi-li hrana "za"
                                        // bisektorem, inidkuje se
                                        // neuspech (if the edge lies
                                        // in "za", return failure)
   
  Point p;
  if (!(pl == Point (INFINITY, INFINITY))) 
    p = pl;
  if (!(pr == Point (INFINITY, INFINITY))) 
    p = pr;
  assert 
    (p != Point (INFINITY, INFINITY));

  assert 
    (pl == Point (INFINITY, INFINITY) || pr == Point (INFINITY, INFINITY) || pl == pr);
   
  Point poi = coordinatesOfAnyIntersectionOfTypeB (v, left, right);
  // out_file<<" any int of type B "<<v.ID<<":"<<poi<<" "<<std::endl;

  if (pointOnRay(poi, left.axis) || pointOnRay(poi, right.axis))
  { // cout<<v.ID<<":"<<poi<<" ";
    return poi;  // VERTEX - VERTEX COLLISION
  }
  else
  {
    Number al = left.axis.angle - left.rightLine.angle;                  
    Number ar = right.axis.angle - right.leftLine.angle;                
   
    Number alp = Ray (left.point, poi).angle - left.rightLine.angle;     
    Number arp = Ray (right.point, poi).angle - right.leftLine.angle;   
   
    normalizeAngle (al); 
    normalizeAngle (ar); 
    normalizeAngle (alp); 
    normalizeAngle (arp);
    assert 
      (al <= 0 || ANGLE_SIMILAR (al, 0));
    assert 
      (ar >= 0 || ANGLE_SIMILAR (ar, -M_PI) || ANGLE_SIMILAR (ar, 0));
         
    // out_file<<al<<" "<<alp<<" "<<ar<<" "<<arp<<" "<<std::endl;

    if ((alp > 0 || alp < al) && !ANGLE_SIMILAR (alp, 0) && !ANGLE_SIMILAR (alp, al)) 
    {
      // out_file<<" first if .... ";
      return Point (INFINITY, INFINITY);   
    }                                          
    
    if ((arp < 0 || arp > ar ) && !ANGLE_SIMILAR (arp, 0) && !ANGLE_SIMILAR (arp, ar)) 
    {
      // out_file<<" second if .... ";
      return Point (INFINITY, INFINITY);  
    }
    
    // cout<<v.ID<<":"<<poi<<" ";
    return poi;  // VERTEX - EDGE COLLISION
  }
}

// vrati nejblizsi z pruseciku typu B (returns the closest intersection of B)
Number nearestIntersection (const VertexSkelet &v,
                            VertexSkelet **left,
                            VertexSkelet **right,
                            Point &p)
{
  Number minDist = INFINITY;  // neplatna hodnota (invalid value)
  VertexList :: iterator minI = vl.end ();  // neplatny iterator
                                            // (invalid iterator)
  VertexList::iterator i;

  for (i = vl.begin (); i != vl.end (); i++)  // iterace pres vsechny
                                              // LAV (iterate over all
                                              // LAV)
  {
    if ((*i).done) 
      continue;  // vynechani jiz zpracovanych vrcholu (leaving out
                 // the top of the already processed one)
    if ((*i).nextVertex == NULL || (*i).prevVertex == NULL) 
      continue;  // osamely vrchol na spicce - neni v LAV (solitary
                 // peak at the top - not the LAV)
    if (&*i == &v || (*i).nextVertex == &v) 
      continue;  // ignorovani hran vychazejicich z V (disrespecting
                 // edges based on V)
    assert 
      ((*i).rightVertex != NULL);
    assert 
      ((*i).leftVertex != NULL);

    Point poi = intersectionOfTypeB (v,
                                     (*i),
                                     *(*i).nextVertex); // zjisteni
                                                        // souradnic
                                                        // potencialniho
                                                        // bodu B
                                                        // (find
                                                        // coordinates
                                                        // of
                                                        // potential
                                                        // B)
    if (poi == Point (INFINITY, INFINITY)) 
      continue;  // vetsinou - nelezi-li bod ve spravne vyseci (mostly
                 // - do not go to a point in the correct arc)
    Number d = dist (poi, v.point);  // zjisteni vzdalenosti od
                                     // vrcholu (findings from a
                                     // distance from the top)
    if (d < minDist)
    { 
      minDist = d; 
      minI = i;
    }  // a vyber nejblizsiho (choose the nearest)
  }
  if (minDist == INFINITY)
  {
    // *left = NULL; *right = NULL;
    p = Point(INFINITY,INFINITY);
    return INFINITY; 
  }  // nenalezen zadny vhodny bod B (found no suitable point for B)
   
  i = minI;
  Point poi = coordinatesOfAnyIntersectionOfTypeB (v, (*i), *(*i).nextVertex);
   
  Number d = dist (poi, v.leftLine);  // zjisteni vzdalenosti vrcholu
                                      // V od nositelky (vyska ve
                                      // strese) (findings from a
                                      // distance from the top of the
                                      // wearer)
  assert 
    (SIMILAR (d, dist (poi, v.rightLine)));
  assert 
    (SIMILAR (d, dist (poi, (*i).rightLine)));
  assert 
    (SIMILAR (d, dist (poi, (*i).nextVertex -> leftLine)));
   
  p = poi;  // nastaveni navracenych hodnot (setting value for return)
  *left = (VertexSkelet *) &*i;
  *right = (*i).nextVertex;

  return d;  // vysledkem je vzdalenost od nositelky (the result is
             // the distance from the wearer)
}


bool invalidIntersection (const VertexSkelet &v,
                          const Intersection &is,
                          VertexSkelet **left,
                          VertexSkelet **right)        
{
  for (VertexList :: iterator i = vl.begin (); i != vl.end (); i++)
  {
    if ((*i).done) 
      continue;
    if ((*i).nextVertex == NULL || (*i).prevVertex == NULL) 
      continue; // osamely vrchol na spicce (solitary peak to peak)
    Point poi = intersection (v.axis, (*i).axis);
    if (poi == Point (INFINITY, INFINITY)) 
      continue;
    if (&*i == is.leftVertex || &*i == is.rightVertex) 
      continue;
      
    Number dv = dist (poi, v.leftLine);
    Number dvx = dist (poi, v.rightLine);
    //        assert (SIMILAR (dv, dist (poi, v.rightLine)));
    assert 
      (SIMILAR (dv, dvx));
    if (dv > is.height) 
      continue;
      
    Number di = dist (poi, (*i).leftLine);
    assert 
      (SIMILAR (di, dist (poi, (*i).rightLine)));
    if (di > dv + MIN_DIFF) 
      continue;
    if (dv > di + MIN_DIFF)
      continue;

    if (((*i).point == v.point) && (!(&*i == *right)) && (!(&*i == *left)))
      continue;
         
    if ((&*i == v.prevVertex) || (&*i == v.nextVertex))
      continue;

    if (&*i == *right)
    {
      Ray VLeft = Ray(is.poi, v.leftLine.angle );
      Ray ERight = Ray(is.poi, (*right) -> rightLine.angle);
      Point inter1 = intersection(VLeft, ERight);
      Ray VRight = Ray(is.poi, v.rightLine.angle );
      Ray ELeft = Ray(is.poi, (*right) -> leftLine.angle);
      Point inter2 = intersection(VRight, ELeft);

      if (!(inter1 == Point(INFINITY, INFINITY)))
      {
        if (colinear(v.leftLine, (*right) -> rightLine)) 
          continue;
        else
        {
          Point int_edges1 = intersectionAnywhere(v.leftLine ,(*right) -> rightLine);
          if (pointOnRay(int_edges1, (v.leftLine).opaque()) && pointOnRay(int_edges1, ((*right) -> rightLine).opaque()))
            continue;
        }
      }

      if (!(inter2 == Point(INFINITY, INFINITY)))
      {
        if (colinear(v.rightLine, (*right) -> leftLine)) 
          continue;
        else
        {
          Point int_edges2 = intersectionAnywhere(v.rightLine ,(*right) -> leftLine); 
          if (pointOnRay(int_edges2,(v.rightLine).opaque()) && pointOnRay(int_edges2, ((*right) -> leftLine).opaque()))
            continue; 
        }
      }
    }

    if  (&*i == *left)
    {
      Ray VLeft = Ray(is.poi, v.leftLine.angle );
      Ray ERight = Ray(is.poi, (*left) -> rightLine.angle);
      Point inter1 = intersection(VLeft, ERight);
      Ray VRight = Ray(is.poi, v.rightLine.angle );
      Ray ELeft = Ray(is.poi, (*left) -> leftLine.angle);
      Point inter2 = intersection(VRight, ELeft);

      if (!(inter1 == Point(INFINITY, INFINITY)))
      {
        if (colinear(v.leftLine, (*left) -> rightLine)) 
          continue;
        else
        {
          Point int_edges1 = intersectionAnywhere(v.leftLine ,(*left) -> rightLine);
          if (pointOnRay(int_edges1, (v.leftLine).opaque()) && pointOnRay(int_edges1,((*left) -> rightLine).opaque()))
                continue;
        }
      }

      if (!(inter2 == Point(INFINITY, INFINITY)))
      {
        if (colinear(v.rightLine, (*left) -> leftLine)) 
          continue;
        else
        {
          Point int_edges2 = intersectionAnywhere(v.rightLine ,(*left) -> leftLine); 
          if (pointOnRay(int_edges2, (v.rightLine).opaque()) && pointOnRay(int_edges2,((*left) -> leftLine).opaque()))
            continue; 
        }
      }
    }

    return true;
  }

  return false;
}


// spocitani nejnizsiho pruseciku pro vrchol v (calculate the lowest
// peak in the intersection)
Intersection :: Intersection (VertexSkelet &v)
{ 
  assert 
    (v.prevVertex == NULL || facingTowards (v.leftLine, v.prevVertex -> rightLine));
  assert 
    (v.nextVertex == NULL || facingTowards (v.rightLine, v.nextVertex -> leftLine));
   
  VertexSkelet &l = *v.prevVertex;  // sousedi v LAV (neighbors in the LAV)
  VertexSkelet &r = *v.nextVertex;
   
  assert 
    (v.leftLine.angle == v.leftVertex -> leftLine.angle);
  assert 
    (v.rightLine.angle == v.rightVertex -> rightLine.angle);
   
  Number al = v.axis.angle - l.axis.angle;
  normalizeAngle (al);
  Number ar = v.axis.angle - r.axis.angle;
  normalizeAngle (ar);
  Point i1 = (facingTowards (v.axis, l.axis) ?
              Point (INFINITY, INFINITY) :
              intersection (v.axis, l.axis));  // pruseciky se
                                               // sousedn. (intersections
                                               // with the neighbors)
  Point i2 = (facingTowards (v.axis, r.axis) ?
              Point (INFINITY, INFINITY) :
              intersection (v.axis, r.axis));  // vrcholy - edge event
                                               // (peaks - edge event)

  Number d1, d2;
  if (i1 == Point(INFINITY, INFINITY))
    d1 = INFINITY;
  else 
    d1 = dist (v.point, i1);
  // d1 - vzdalenost od nositelky pruseciku bisektoru s bisektorem
  // leveho souseda v LAV
  if  (i2 == Point(INFINITY, INFINITY))
    d2 = INFINITY;
  else
    d2 = dist(v.point, i2);
  // d2 - vzdalenost od nositelky pruseciku bisektoru s bisektorem
  // praveho souseda v LAV
        
  VertexSkelet *leftPointer, *rightPointer;
  Point p;
  Number d3 = INFINITY;  // d3 - u nekonvexnich vrcholu vzdalenost
                         // bodu B od nositelky (at the top of
                         // nonconvex distance B from the wearer)
  Number av = v.leftLine.angle - v.rightLine.angle;  // test na
                                                     // otevrenost
                                                     // uhlu u vrcholu
                                                     // V =>
                                                     // konvexni/nekonvexni
                                                     // (openness to
                                                     // the angle at
                                                     // the top =>
                                                     // convex/nonconvex)
  normalizeAngle (av);

  if (av > 0  && !ANGLE_SIMILAR (av, 0) &&
      (intersection (v.leftLine, v.rightLine) == v.point ||
       intersection (v.leftLine, v.rightLine) == Point (INFINITY, INFINITY)))
  {
    d3 = nearestIntersection(v, &leftPointer,
                               &rightPointer, p);  // zjisteni
                                                   // vzdalenosti bodu
                                                   // B od nositelky
                                                   // (findings from a
                                                   // distance B from
                                                   // the wearer)
  } 

  if (SIMILAR(d1, INFINITY) && SIMILAR(d2, INFINITY) && SIMILAR(d3, INFINITY))
  {
    height = INFINITY;
  }
  else
  {  
    if (d1 <= d2 && d1 <= d3)
    { 
      leftVertex = &l; 
      rightVertex = &v; 
      poi = i1; 
      type = CONVEX; 
      height = dist (v.leftLine, i1);
    }
    
    if (d2 <= d1 && d2 <= d3)
    {
      leftVertex = &v; 
      rightVertex = &r; 
      poi = i2; 
      type = CONVEX; 
      height = dist (v.rightLine, i2);
    }
    
    if (d3 < d1 - MIN_DIFF && d3 < d2 - MIN_DIFF)
    { 
      poi = p; 
      leftVertex = rightVertex = &v; 
      type = NONCONVEX; 
      height = d3;
    }
      
    if (poi == Point (INFINITY, INFINITY)) 
      height = INFINITY;
    if (type == NONCONVEX && invalidIntersection (v, *this, &leftPointer, &rightPointer)) 
      height = INFINITY;
  }
}



/******************************************************************************
 HANDLING SPLIT EVENTS 
 ******************************************************************************/
void applyNonconvexIntersection (Intersection &i)  
{
  assert 
    (i.leftVertex == i.rightVertex);          
   
  VertexSkelet *leftPointer, *rightPointer;              
  Point p;                                         
  Number d3 = INFINITY;

  d3 = nearestIntersection (*i.leftVertex, &leftPointer, &rightPointer, p);

  if (d3 == INFINITY) 
    return;                                             
                                                   
  if (!(p == i.poi))
  { 
    Intersection i1 (*i.leftVertex);
    if (!SIMILAR(i1.height, INFINITY))
    {
      iq.push (i1);             
    }
    return; 
  }                               
                                        

  // RECONSTRUCT LOCCALLY THE FRONT TO DIFFERENTIATE BETWEEN THE EVENTS
  int collision_type;
  Ray SEright(i.poi, (*leftPointer).rightLine.angle);
  Point Edge_right = intersection(SEright, (*rightPointer).axis);
  Ray SEleft(i.poi, (*rightPointer).leftLine.angle);
  Point Edge_left = intersection(SEleft, (*leftPointer).axis);

  VertexSkelet *vertexPtr;

  VertexSkelet *v1left, *v1right, *v2left, *v2right;
  v1right = i.rightVertex;
  v2left = i.leftVertex;

  if (Edge_right == i.poi)
  {
    collision_type = 11;     // VERTEX-VERTEX COLLISION
    vertexPtr = rightPointer;
    v1left = v2right = rightPointer;
    rightPointer = leftPointer -> nextVertex -> nextVertex;
  }
  else  if (Edge_left == i.poi)
  {
    collision_type = 11;     // VERTEX-VERTEX COLLISION
    vertexPtr = leftPointer;
    v1left = v2right = leftPointer;
    leftPointer = rightPointer -> prevVertex -> prevVertex;
  }
  else
  {
    v1left = rightPointer;
    v2right = leftPointer;
    collision_type = 12;    // VERTEX-EDGE COLLISION
        
  }

  VertexSkelet v1 (p, *v1left, *v1right);    
  VertexSkelet v2 (p, *v2left, *v2right);

  assert 
    (v1.point != Point (INFINITY, INFINITY));
  assert 
    (v2.point != Point (INFINITY, INFINITY));
   
  i.leftVertex -> done = true;                   
  // i.rightVertex -> done = true;
   
  VertexSkelet *newNext1 = i.rightVertex -> nextVertex;  
  VertexSkelet *newPrev1 = leftPointer -> highest ();
  v1.prevVertex = newPrev1;
  v1.nextVertex = newNext1;
  vl.push_back (v1);
  VertexSkelet *v1Pointer = &vl.back ();
  newPrev1 -> nextVertex = v1Pointer;
  newNext1 -> prevVertex = v1Pointer;
  i.rightVertex -> higher = v1Pointer;

  
  VertexSkelet *newNext2 = rightPointer -> highest ();   
  VertexSkelet *newPrev2 = i.leftVertex -> prevVertex;
  v2.prevVertex = newPrev2;
  v2.nextVertex = newNext2;
  vl.push_back (v2);
  VertexSkelet *v2Pointer = &vl.back ();
  newPrev2 -> nextVertex = v2Pointer;
  newNext2 -> prevVertex = v2Pointer;
  i.leftVertex  -> higher = v2Pointer;

   
  skeleton.push_back (SkeletonLine (*i.rightVertex, *v1Pointer)); 
  SkeletonLine *linePtr = &skeleton.back ();
  skeleton.push_back (SkeletonLine (*v1Pointer, *v2Pointer));     
  SkeletonLine *auxLine1Ptr = &skeleton.back ();
  skeleton.push_back (SkeletonLine (*v2Pointer, *v1Pointer));     
  SkeletonLine *auxLine2Ptr = &skeleton.back ();
  SkeletonLine *SeclinePtr;

  if (collision_type == 11)
  {
    vertexPtr -> done = true;
    skeleton.push_back (SkeletonLine (*vertexPtr, *v2Pointer)); 
    SeclinePtr = &skeleton.back ();
    SeclinePtr -> lower.right = vertexPtr ->leftSkeletonLine;
    SeclinePtr -> lower.left = vertexPtr -> rightSkeletonLine;
  }
   
  linePtr -> lower.right = i.leftVertex -> leftSkeletonLine;    
  linePtr -> lower.left = i.leftVertex -> rightSkeletonLine;
   
  if (collision_type == 11)
  {
    v2Pointer -> leftSkeletonLine = SeclinePtr;
    v1Pointer -> rightSkeletonLine = linePtr;
  }
  else
    v1Pointer -> rightSkeletonLine = v2Pointer -> leftSkeletonLine = linePtr;

  v1Pointer -> leftSkeletonLine = auxLine1Ptr;
  v2Pointer -> rightSkeletonLine = auxLine2Ptr;
   
  auxLine1Ptr -> lower.right = auxLine2Ptr;
  auxLine2Ptr -> lower.left = auxLine1Ptr;
   
  if (i.leftVertex -> leftSkeletonLine) 
    i.leftVertex -> leftSkeletonLine -> higher.left = linePtr;
  if (i.leftVertex -> rightSkeletonLine) 
    i.leftVertex -> rightSkeletonLine -> higher.right = linePtr;
  i.leftVertex -> advancingSkeletonLine = linePtr;

  if (collision_type == 11)
  {
    vertexPtr -> higher = v2Pointer;
    if (vertexPtr -> leftSkeletonLine) 
      vertexPtr -> leftSkeletonLine -> higher.left = SeclinePtr;
    if (vertexPtr -> rightSkeletonLine) 
      vertexPtr -> rightSkeletonLine -> higher.right = SeclinePtr;
    vertexPtr -> advancingSkeletonLine = SeclinePtr;
  }
   
  if (newNext1 == newPrev1)                                
  {
    v1Pointer -> done = true;                            
    newNext1 -> done = true;
    skeleton.push_back (SkeletonLine (*v1Pointer, *newNext1)); 
    SkeletonLine *linePtr = &skeleton.back ();
    linePtr -> lower.right  = v1Pointer -> leftSkeletonLine;
    linePtr -> lower.left   = v1Pointer -> rightSkeletonLine;
    linePtr -> higher.right = newNext1 -> leftSkeletonLine;
    linePtr -> higher.left  = newNext1 -> rightSkeletonLine;
      
    if (v1Pointer -> leftSkeletonLine)  
      v1Pointer -> leftSkeletonLine  -> higher.left  = linePtr;
    if (v1Pointer -> rightSkeletonLine) 
      v1Pointer -> rightSkeletonLine -> higher.right = linePtr;
    if (newNext1 -> leftSkeletonLine)   
      newNext1  -> leftSkeletonLine  -> higher.left  = linePtr;
    if (newNext1 -> rightSkeletonLine)  
      newNext1  -> rightSkeletonLine -> higher.right = linePtr;
  }
  else
  {
    Intersection i1 (*v1Pointer);                        
    if (!SIMILAR(i1.height, INFINITY))
    {
      iq.push (i1);  // a ulozeni do fronty (stored in a queue)
    }
  }

  if (newNext2 == newPrev2)                                
  {
    v2Pointer -> done = true;                           
    newNext2 -> done = true;
    skeleton.push_back (SkeletonLine (*v2Pointer, *newNext2));
    SkeletonLine *linePtr = &skeleton.back ();
    linePtr -> lower.right  = v2Pointer -> leftSkeletonLine;
    linePtr -> lower.left   = v2Pointer -> rightSkeletonLine;
    linePtr -> higher.right = newNext2 -> leftSkeletonLine;
    linePtr -> higher.left  = newNext2 -> rightSkeletonLine;
      
    if (v2Pointer -> leftSkeletonLine)  
      v2Pointer -> leftSkeletonLine  -> higher.left  = linePtr;
    if (v2Pointer -> rightSkeletonLine) 
      v2Pointer -> rightSkeletonLine -> higher.right = linePtr;
    if (newNext2 -> leftSkeletonLine)   
      newNext2  -> leftSkeletonLine  -> higher.left  = linePtr;
    if (newNext2 -> rightSkeletonLine)  
      newNext2  -> rightSkeletonLine -> higher.right = linePtr;
  }
  else
  {
    Intersection i2 (*v2Pointer);                       
    if (!SIMILAR(i2.height, INFINITY)) 
      {
        iq.push (i2);  // a ulozeni do fronty (stored in a queue)
      }
  }

   // ARS - Skeleton vertex

  if (collision_type == 11)
  {
    // IDENTIFY THE EDGES OF THE VERTICES INVOLVED

    // left edge of reflex vertex and the edge that becomes incident to

    VertexSkelet *EdgeLeft = v2.prevVertex;
    while ((EdgeLeft ->ID) > NrCP)
      EdgeLeft = EdgeLeft -> rightVertex;

    VertexSkelet *EdgeRight = i.rightVertex;
    while ((EdgeRight ->ID) > NrCP)
      EdgeRight = EdgeRight -> leftVertex;

    EdgeLeft = vertexPtr;
    while ((EdgeLeft ->ID) > NrCP)
      EdgeLeft = EdgeLeft -> rightVertex;

    EdgeRight = v2.nextVertex;
    while ((EdgeRight ->ID) > NrCP)
      EdgeRight = EdgeRight -> leftVertex;


    // right edge of reflex vertex and the edge that becomes incident to

    EdgeLeft = i.rightVertex;
    while ((EdgeLeft ->ID) > NrCP)
      EdgeLeft = EdgeLeft -> rightVertex;

    EdgeRight = v1.nextVertex;
    while ((EdgeRight ->ID) > NrCP)
      EdgeRight = EdgeRight -> leftVertex;

    EdgeLeft = v1.prevVertex;
    while ((EdgeLeft ->ID) > NrCP)
      EdgeLeft = EdgeLeft -> rightVertex;

    EdgeRight = vertexPtr;
    while ((EdgeRight ->ID) > NrCP)
      EdgeRight = EdgeRight -> leftVertex;

  }
  else 
  {
    // IDENTIFY THE EDGES OF THE REFLEX VERTEX

    VertexSkelet* EdgeLeft = v2.prevVertex;
    while ((EdgeLeft ->ID) > NrCP)
      EdgeLeft = EdgeLeft -> rightVertex;

    VertexSkelet* EdgeRight = i.rightVertex;
    while ((EdgeRight ->ID) > NrCP)
      EdgeRight = EdgeRight -> leftVertex;

    EdgeLeft = i.rightVertex;
    while ((EdgeLeft ->ID) > NrCP)
      EdgeLeft = EdgeLeft -> rightVertex;

    EdgeRight = v1.nextVertex;
    while ((EdgeRight ->ID) > NrCP)
      EdgeRight = EdgeRight -> leftVertex;


    // IDENTIFY THE EDGE THAT IS SPLIT

    EdgeLeft = leftPointer;
    while ((EdgeLeft ->ID) > NrCP)
      EdgeLeft = EdgeLeft -> rightVertex;

    EdgeRight = rightPointer;
    while ((EdgeRight ->ID) > NrCP)
      EdgeRight = EdgeRight -> leftVertex;
  }
}


/******************************************************************************
 HANDLING EDGE EVENTS 
******************************************************************************/

void applyConvexIntersection (const Intersection &i)      
{
  VertexSkelet vtx (i.poi, *i.leftVertex, *i.rightVertex);
       
  assert 
    (vtx.point != Point (INFINITY, INFINITY));

  // RECONSTRUCT LOCALLY THE FRONT TO DIFFERENTIATE BETWEEN THE EVENTS

  int event_type=0;

  int IDleft =  (i.leftVertex) -> ID;
  int IDright = (i.rightVertex) -> ID;
                 
  Point P1, P2, P3, P4;

  double t1, t2;

  if ((IDleft <= NrCP ) && (IDright <= NrCP ))
  {
    P2 = (i.leftVertex) -> point;
    P3 = (i.rightVertex) -> point;

    if (((i.leftVertex) -> prevVertex -> ID) > NrCP)
      P1 = intersectionAnywhere((i.leftVertex) -> prevVertex -> axis, (i.leftVertex)->leftLine ); 
    else P1 = (i.leftVertex) -> prevVertex -> point;

    if (((i.rightVertex) -> nextVertex -> ID) > NrCP )
        P4 = intersectionAnywhere((i.rightVertex) -> nextVertex -> axis, (i.rightVertex)->rightLine );
    P4 = (i.rightVertex) -> nextVertex -> point;   
  }
  else
  {
    if (IDleft > IDright)
    {
      P2 = (i.leftVertex) -> point;
      Ray R1 (P2,(i.leftVertex) -> leftLine.angle);
      P1 = intersection(R1, (i.leftVertex) -> prevVertex -> axis);
      if (P1 == Point(INFINITY, INFINITY))
        P1 = intersectionAnywhere(R1, (i.leftVertex) -> prevVertex -> axis);
      Ray R3(P2,(i.leftVertex) -> rightLine.angle);
      P3 = intersection(R3,  (i.rightVertex) -> axis);
      Ray R4 (P3, (i.rightVertex) -> rightLine.angle);
      P4 = intersection(R4, (i.rightVertex) -> nextVertex -> axis);
      if (P4 == Point(INFINITY, INFINITY))
        P4 = intersectionAnywhere(R4, (i.rightVertex) -> nextVertex -> axis);
    }
    else 
    { 
      P3 = (i.rightVertex) -> point;
      Ray R4(P3, (i.rightVertex) -> rightLine.angle);
      P4 = intersection(R4, (i.rightVertex) -> nextVertex -> axis);
      if (P4 == Point(INFINITY, INFINITY))
        P4 = intersectionAnywhere(R4, (i.rightVertex) -> nextVertex -> axis);
      Ray R2(P3, (i.rightVertex) -> leftLine.angle);
      P2 = intersection(R2, (i.leftVertex) ->axis);
      Ray R1 (P2, (i.leftVertex) -> leftLine.angle);
      P1 = intersection(R1,(i.leftVertex) -> prevVertex -> axis );
      if (P1 == Point(INFINITY, INFINITY))
        P1 = intersectionAnywhere(R1,(i.leftVertex) -> prevVertex -> axis );
    }
  }

  Ray R12(P1, P2); Ray R23(P2,P3); Ray R34(P3, P4);
                

  if ((!(P2 == P3)) ||
      ((P2 == P3) && !(P1 == P4)) ||
      ((P2 == P3) && (P1 == P4) && !(P1 == P2)))
  {
    if ((P1 == P2) || (P1 == P3))
      t1=0;
    else  t1 = ((P2.x-P1.x)*(P1.y-P3.y)-(P2.y-P1.y)*(P1.x-P3.x));

    if ((P2 == P3) || (P2 == P4))
      t2=0;
    else t2 = ((P3.x-P2.x)*(P2.y-P4.y)-(P3.y-P2.y)*(P2.x-P4.x));
    
    if (( t1 < 0 ) || ( t2 < 0 )) 
      event_type=22;     // reflex edge event
    else event_type=21;  // convex edge event
  }
  else
    event_type = 20;

   
  VertexSkelet *newNext = i.rightVertex -> nextVertex;           
  VertexSkelet *newPrev = i.leftVertex -> prevVertex;
  vtx.prevVertex = newPrev;
  vtx.nextVertex = newNext;
  vl.push_back (vtx);
  VertexSkelet *vtxPointer = &vl.back ();
  newPrev -> nextVertex = vtxPointer;
  newNext -> prevVertex = vtxPointer;
  i.leftVertex  -> higher = vtxPointer;
  i.rightVertex -> higher = vtxPointer;
   
  i.leftVertex  -> done = true;                            
  i.rightVertex -> done = true;
   
  Intersection newI (*vtxPointer);                          
  if (!SIMILAR(newI.height, INFINITY)) 
  {
    iq.push (newI);
  }
  
  skeleton.push_back (SkeletonLine (*i.leftVertex, *vtxPointer));  
  SkeletonLine *lLinePtr = &skeleton.back ();
  skeleton.push_back (SkeletonLine (*i.rightVertex, *vtxPointer));
  SkeletonLine *rLinePtr = &skeleton.back ();
   
  lLinePtr -> lower.right = i.leftVertex -> leftSkeletonLine;     
  lLinePtr -> lower.left = i.leftVertex -> rightSkeletonLine;
  lLinePtr -> higher.right = rLinePtr;
  rLinePtr -> lower.right = i.rightVertex -> leftSkeletonLine;
  rLinePtr -> lower.left = i.rightVertex -> rightSkeletonLine;
  rLinePtr -> higher.left = lLinePtr;
   
  if (i.leftVertex -> leftSkeletonLine) 
    i.leftVertex -> leftSkeletonLine -> higher.left = lLinePtr;
  if (i.leftVertex -> rightSkeletonLine) 
    i.leftVertex -> rightSkeletonLine -> higher.right = lLinePtr;
   
  if (i.rightVertex -> leftSkeletonLine) 
    i.rightVertex -> leftSkeletonLine -> higher.left = rLinePtr;
  if (i.rightVertex -> rightSkeletonLine) 
    i.rightVertex -> rightSkeletonLine -> higher.right = rLinePtr;
   
  vtxPointer -> leftSkeletonLine = lLinePtr;
  vtxPointer -> rightSkeletonLine = rLinePtr;
   
  i.leftVertex -> advancingSkeletonLine = lLinePtr;
  i.rightVertex -> advancingSkeletonLine = rLinePtr;

  // ARS - Skeleton vertex
   
  // IDENTIFY THE EDGE THAT DISSAPEARS IN THE EVENT
  VertexSkelet *Edge_left = i.leftVertex; 
  VertexSkelet *Edge_right = i.rightVertex;
  while ((Edge_left -> ID) > NrCP)
    Edge_left = Edge_left -> rightVertex; 
  while ((Edge_right -> ID) > NrCP)
    Edge_right = Edge_right -> leftVertex;


  // IDENTIFY THE EDGES THAT BECOME ADJACENT
  Edge_left = vtx.prevVertex;
  Edge_right = vtx.leftVertex;
  while ((Edge_left -> ID) > NrCP)
    Edge_left = Edge_left -> rightVertex;  
  while ((Edge_right -> ID) > NrCP)
    Edge_right = Edge_right -> leftVertex;



  Edge_left = vtx.rightVertex;
  Edge_right = vtx.nextVertex;
  while ((Edge_left -> ID) > NrCP)
    Edge_left = Edge_left -> rightVertex;  
  while ((Edge_right -> ID) > NrCP)
    Edge_right = Edge_right -> leftVertex;


  if ( event_type == 22 )
  {
  }
}



void applyLast3 (const Intersection &i)                            
{
  assert 
    (i.leftVertex  -> nextVertex == i.rightVertex);           
  assert 
    (i.rightVertex -> prevVertex == i.leftVertex);
  assert 
    (i.leftVertex  -> prevVertex -> prevVertex == i.rightVertex);
  assert 
    (i.rightVertex -> nextVertex -> nextVertex == i.leftVertex);
   
  VertexSkelet &v1 = *i.leftVertex;
  VertexSkelet &v2 = *i.rightVertex;
  VertexSkelet &v3 = *i.leftVertex -> prevVertex;
  v1.done = true;                                                  
  v2.done = true;
  v3.done = true;
   
  Point is1 = facingTowards (v1.axis, v2.axis) ? Point (INFINITY, INFINITY) : intersection (v1.axis, v2.axis);
  Point is2 = facingTowards (v2.axis, v3.axis) ? Point (INFINITY, INFINITY) : intersection (v2.axis, v3.axis);
  Point is3 = facingTowards (v3.axis, v1.axis) ? Point (INFINITY, INFINITY) : intersection (v3.axis, v1.axis);
   
  Point is = i.poi;                                               
  assert 
    (is == is1 || is1 == Point (INFINITY, INFINITY));
  assert 
    (is == is2 || is2 == Point (INFINITY, INFINITY));
  assert 
    (is == is3 || is3 == Point (INFINITY, INFINITY));
   
  VertexSkelet v (is);                                                   
   
  v.done = true;                                                   
  vl.push_back (v);
  VertexSkelet *vtxPointer = &vl.back ();
  skeleton.push_back (SkeletonLine (v1, *vtxPointer));             
  SkeletonLine *line1Ptr = &skeleton.back ();
  skeleton.push_back (SkeletonLine (v2, *vtxPointer));
  SkeletonLine *line2Ptr = &skeleton.back ();
  skeleton.push_back (SkeletonLine (v3, *vtxPointer));
  SkeletonLine *line3Ptr = &skeleton.back ();
   
  line1Ptr -> higher.right = line2Ptr;                           
  line2Ptr -> higher.right = line3Ptr;
  line3Ptr -> higher.right = line1Ptr;
   
  line1Ptr -> higher.left = line3Ptr;
  line2Ptr -> higher.left = line1Ptr;
  line3Ptr -> higher.left = line2Ptr;
   
  line1Ptr -> lower.left = v1.rightSkeletonLine;
  line1Ptr -> lower.right = v1.leftSkeletonLine;
   
  line2Ptr -> lower.left = v2.rightSkeletonLine;
  line2Ptr -> lower.right = v2.leftSkeletonLine;
   
  line3Ptr -> lower.left = v3.rightSkeletonLine;
  line3Ptr -> lower.right = v3.leftSkeletonLine;
   
  if (v1.leftSkeletonLine) 
    v1.leftSkeletonLine -> higher.left = line1Ptr;
  if (v1.rightSkeletonLine) 
    v1.rightSkeletonLine -> higher.right = line1Ptr;
   
  if (v2.leftSkeletonLine) 
    v2.leftSkeletonLine -> higher.left = line2Ptr;
  if (v2.rightSkeletonLine) 
    v2.rightSkeletonLine -> higher.right = line2Ptr;
   
  if (v3.leftSkeletonLine) 
    v3.leftSkeletonLine -> higher.left = line3Ptr;
  if (v3.rightSkeletonLine) 
    v3.rightSkeletonLine -> higher.right = line3Ptr;
   
  v1.advancingSkeletonLine = line1Ptr;
  v2.advancingSkeletonLine = line2Ptr;
  v3.advancingSkeletonLine = line3Ptr;

  int  event_type = 20;

  // IDENTIFY THE EDGES THAT COLLAPSE

  VertexSkelet *Edge1_left = i.leftVertex; 
  VertexSkelet *Edge1_right = i.rightVertex;
  while ((Edge1_left -> ID) > NrCP)
    Edge1_left = Edge1_left -> rightVertex; 
  while ((Edge1_right -> ID) > NrCP)
    Edge1_right = Edge1_right -> leftVertex;


  VertexSkelet* Edge2_left =  i.rightVertex;
  VertexSkelet* Edge2_right = i.rightVertex -> nextVertex;
  while((Edge2_left -> ID) > NrCP)
    Edge2_left = Edge2_left -> rightVertex;
  while ((Edge2_right -> ID) > NrCP)
    Edge2_right = Edge2_right -> leftVertex;


  VertexSkelet* Edge3_left =  i.rightVertex -> nextVertex;
  VertexSkelet* Edge3_right = i.leftVertex;
  while((Edge3_left -> ID) > NrCP)
    Edge3_left = Edge3_left -> rightVertex;
  while ((Edge3_right -> ID) > NrCP)
    Edge3_right = Edge3_right -> leftVertex;

}



/******************************************************************************
 ******************************************************************************/

Skeleton &makeSkeleton (PointVectorVector &contours)          
{
  while (iq.size ()) 
    iq.pop ();                              
  vl.erase (vl.begin (), vl.end ());                         
  skeleton.erase (skeleton.begin (), skeleton.end ());
   
  //std::cerr << "First loop..." << std::endl;
        
  for (int ci = 0; ci < contours.size (); ci ++)             
  {
    PointVector &points (contours[ci]);                 
    PointVector :: iterator first = points.begin();
    if (first == points.end ()) 
      break;                   
    PointVector :: iterator next = first;
                
    while (++next != points.end ())
    {
      if (*first == *next) 
        points.erase (next);
      else 
        first = next;
      next = first;
    }
    
        
    int s = points.size ();

    for (int f = 0; f < s; f++)
    {
      vl.push_back (VertexSkelet (points [f], points [(s+f-1)%s], points [(s+f+1)%s]));  // zaroven spocita bisektory
    }
  }
   
  if (vl.size () < 3) 
    return skeleton;  
   
  //std::cerr << "Second loop..." << std::endl;
  
  VertexList :: iterator i;
  int vn = 0, cn = 0;
  VertexList :: iterator contourBegin;

  for (i = vl.begin (); i != vl.end (); i++)
  {
    (*i).prevVertex = &*vl.prev (i);         
    (*i).nextVertex = &*vl.next (i);         
    (*i).leftVertex = &*i;
    (*i).rightVertex = &*i;
    if (vn == 0) 
      contourBegin = i;
    if (vn == contours [cn].size () - 1)
    {
      (*i).nextVertex = &*contourBegin;
      (*contourBegin).prevVertex = &*i;
      vn = 0;
      cn ++;
    }
    else 
      vn ++;
  }
  
  //std::cerr << "Third loop..." << std::endl;
  
  NrCP = -1;
  int type_ev = 0;

  for (i=vl.begin(); i!=vl.end(); i++)  
  {
    double h=0;
    NrCP++;
  }

#ifdef EPS
//  *epsStream << "%Hranice" << std::endl;               
//  for (i = vl.begin (); i != vl.end (); i++)
//  {
//    *epsStream << (*i).point.x << ' ' << (*i).point.y << " moveto ";
//    *epsStream << (*i).nextVertex -> point.x << ' ' << (*i).nextVertex -> point.y << " lineto\n";
//   }
//   *epsStream << "%Vnitrek" << std::endl;
#endif
     
  //std::cerr << "Fourth loop..." << std::endl;
        
  for (i = vl.begin (); i != vl.end (); i++)
  {
    Intersection is (*i);
    if (!SIMILAR(is.height, INFINITY)) 
    {       
      iq.push (is);
    }
  }
  
  //std::cerr << "Fifth loop..." << std::endl;
  
  //Possible dead loop below (Phi_BT 1350 (1.51054 0 0.0428917) )!!!
  int counter = 0;
  
  int NrCP2=NrCP;
  while (iq.size ())                             
    {
      // To avoid infinite loop
      counter ++;
      if (counter > 100000)
      {
        std::cerr << "Infinite loop! Exiting ..." << std::endl;
        break;
        //skeleton.erase (skeleton.begin (), skeleton.end ()); return skeleton;
      }
      
      Intersection i = iq.top ();             
      iq.pop ();  
      NrCP2++; 
      if (SIMILAR(i.height, INFINITY)) 
        return skeleton;
      
      if (i.leftVertex -> done && i.rightVertex -> done) 
      { 
        continue;
      }
      
      if ((i.leftVertex -> done) || (i.rightVertex -> done))            
      {  
        if (!i.leftVertex -> done) 
        {         
          Intersection is(*i.leftVertex);
          iq.push (is); 
        }
        
        if (!i.rightVertex -> done) 
        {       
          Intersection is(*i.rightVertex);
          iq.push (is); 
        }    
        continue;                                                 
      }
      
      assert (i.leftVertex -> prevVertex != i.rightVertex);
      assert (i.rightVertex -> nextVertex != i.leftVertex);
      
      if (i.type == Intersection :: CONVEX)
      {
        if (i.leftVertex -> prevVertex -> prevVertex == i.rightVertex ||
            i.rightVertex -> nextVertex -> nextVertex == i.leftVertex)
        { 
          applyLast3 (i); 
        }                    // FRONT ANNIHILATION
    
        else 
        {  
          applyConvexIntersection (i);
        }

        VertexList::iterator vnew;

        for (vnew = vl.begin (); vnew != vl.end (); vnew++)
        {
          Number va = (*vnew).leftLine.angle - (*vnew).rightLine.angle;
          normalizeAngle(va);
          if (va > 0 && !ANGLE_SIMILAR(va, 0) &&
              (intersection((*vnew).leftLine,(*vnew).rightLine) ==
               (*vnew).point ||
               intersection((*vnew).leftLine,(*vnew).rightLine) ==
               Point(INFINITY, INFINITY)))
          {
            Intersection is (*vnew);
            if (!SIMILAR(is.height, INFINITY) &&
                (is.type == Intersection :: NONCONVEX))
            {       
              iq.push (is);
            }
          }
        }
      }
      
      // CONVEX INTERSECTIONS

      if (i.type == Intersection :: NONCONVEX)
      {
        std::cerr << "Call non-convex intersection ..." << std::endl;
        
        applyNonconvexIntersection (i);
      } // NON-CONVEX INTERSECTIONS 
      
#ifdef __WATCOMC__
      if (kbhit ()) { getch (); return skeleton;
    }
#endif
  }

  //std::cerr << "Skeleton Done!" << std::endl;
  
  return skeleton;    

  // vraci se odkaz na vytvorenou kostru (returns the reference to the
  // created skeleton)
}

// Expects a clockwise polygon. Duplicate vertices are not allowed. 
// In particular, the first vertex may not be the same as the last vertex.

// zkratka pokud je tvorena kostra pro jedinou konturu bez der
// (abbreviation if it is made up of a framework for a single contour
// without holes)
Skeleton &makeSkeleton (PointVector &points)
{
  PointVectorVector vv;
  vv.push_back (points);
  return makeSkeleton (vv);
}

}
