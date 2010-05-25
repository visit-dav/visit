/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2009 Scientific Computing and Imaging Institute,
   University of Utah.

   License for the specific language governing rights and limitations under
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
 *  StreamlineAnalyzerLib.cc:
 *
 *  Written by:
 *   Allen Sanderson
 *   SCI Institute
 *   University of Utah
 *   September 2006
 *
 */

#include <StreamlineAnalyzerLib.h>

#include <algorithm>
#include <iostream>
#include <map>

#include <float.h>

#ifdef STRAIGHTLINE_SKELETON
#include "skelet.h"
#endif

using namespace std;

namespace FusionPSE {

#define SIGN(x) ((x) < 0.0 ? (int) -1 : (int) 1)



//===================================================================
// Copyright 2001, softSurfer (www.softsurfer.com)
// This code may be freely used and modified for any purpose
// providing that this copyright notice is included with it.
// SoftSurfer makes no warranty for this code, and cannot be held
// liable for any real or imagined damage resulting from its use.
// Users of this code must verify correctness for their application.

//===================================================================
// isLeft(): tests if a point is Left|On|Right of an infinite line.
//    Input:  three points P0, P1, and P2
//    Return: >0 for P2 left of the line through P0 and P1
//            =0 for P2 on the line
//            <0 for P2 right of the line
//    See: the January 2001 Algorithm on Area of Triangles
inline float
isLeft( Point P0, Point P1, Point P2 )
{
    return (P1.x - P0.x)*(P2.z - P0.z) - (P2.x - P0.x)*(P1.z - P0.z);
}

//===================================================================
// ptcmp(): 
//     Input:  two points to compare
//
//     Return: 1 if point 0 is "greater than" point 1
//            -1 if point 0 is "less than" point 1
//             0 if point 0 is "equal to" point 1
inline int ptcmp( const void* v0, const void* v1 )
{
  Point *p0 = (Point *) v0;
  Point *p1 = (Point *) v1;

  if( p0->x > p1->x ) return 1;
  else if( p0->x < p1->x ) return -1;
  else if( p0->y > p1->y ) return 1;
  else if( p0->y < p1->y ) return -1;
  else return 0;
}

//===================================================================
// chainHull_2D(): Andrew's monotone chain 2D convex hull algorithm
//     Input:  pts = an array of 2D points 
//
//     Output: hullPts = an array of the convex hull vertices
//                       in a clockwise order
//     Return: the number of points in hullPts
int chainHull_2D( vector< pair< Point, unsigned int > > &pts,
                  vector< pair< Point, unsigned int > > &hullPts,
                  int direction  )
{
    //  Presorted by increasing x- and y-coordinates
    qsort( &(pts[0]), pts.size(),
           sizeof( pair< Point, unsigned int > ), ptcmp );

    int n = pts.size();

    hullPts.resize( n );

    // the output array hullPts[] will be used as the stack
    int    bot=0, top=(-1);  // indices for bottom and top of the stack
    int    i;                // array scan index

    // Get the indices of points with min x-coord and min|max y-coord
    int minmin = 0, minmax;
    float xmin = pts[0].first.x;
    for (i=1; i<n; i++)
        if (pts[i].first.x != xmin) break;
    minmax = i-1;
    if (minmax == n-1) {       // degenerate case: all x-coords == xmin
        hullPts[++top] = pts[minmin];
        if (pts[minmax].first.z != pts[minmin].first.z) // a nontrivial segment
            hullPts[++top] = pts[minmax];
        hullPts[++top] = pts[minmin];           // add polygon endpoint
        return top+1;
    }

    // Get the indices of points with max x-coord and min|max y-coord
    int maxmin, maxmax = n-1;
    float xmax = pts[n-1].first.x;
    for (i=n-2; i>=0; i--)
        if (pts[i].first.x != xmax) break;
    maxmin = i+1;

    // Compute the lower hull on the stack H
    hullPts[++top] = pts[minmin];      // push minmin point onto stack
    i = minmax;
    while (++i <= maxmin)
    {
        // the lower line joins pts[minmin] with pts[maxmin]
        if (isLeft( pts[minmin].first,
                    pts[maxmin].first, pts[i].first) >= 0 && i < maxmin)
            continue;          // ignore pts[i] above or on the lower line

        while (top > 0)        // there are at least 2 points on the stack
        {
            // test if pts[i] is left of the line at the stack top
            if (isLeft( hullPts[top-1].first,
                        hullPts[top].first, pts[i].first) > 0)
                break;         // pts[i] is a new hull vertex
            else
                top--;         // pop top point off stack
        }
        hullPts[++top] = pts[i];       // push pts[i] onto stack
    }

    // Next, compute the upper hull on the stack H above the bottom hull
    if (maxmax != maxmin)      // if distinct xmax points
        hullPts[++top] = pts[maxmax];  // push maxmax point onto stack
    bot = top;                 // the bottom point of the upper hull stack
    i = maxmin;
    while (--i >= minmax)
    {
        // the upper line joins pts[maxmax] with pts[minmax]
        if (isLeft( pts[maxmax].first,
                    pts[minmax].first, pts[i].first) >= 0 && i > minmax)
            continue;          // ignore pts[i] below or on the upper line

        while (top > bot)    // at least 2 points on the upper stack
        {
            // test if pts[i] is left of the line at the stack top
            if (isLeft( hullPts[top-1].first,
                        hullPts[top].first, pts[i].first) > 0)
                break;         // pts[i] is a new hull vertex
            else
                top--;         // pop top point off stack
        }
        hullPts[++top] = pts[i];       // push pts[i] onto stack
    }
    if (minmax != minmin)
        hullPts[++top] = pts[minmin];  // push joining endpoint onto stack

    hullPts.resize( top+1 );

    return hullPts.size();
}

//===================================================================


Point FieldlineLib::interpert( Point lastPt, Point currPt, double t ) {

  return Point( Vector( lastPt ) + Vector( currPt - lastPt ) * t );
}


int FieldlineLib::ccw( Vector v0, Vector v1 ) {
    
  if( v0.x * v1.z - v0.z * v1.x > FLT_MIN ) return  1;    // CCW
  if( v0.z * v1.x - v0.x * v1.z > FLT_MIN ) return -1;    // CW
  if( v0.x * v1.x < 0.0 || v0.z * v1.z < 0.0 ) return -1; // CW
    
  if( v0.x*v0.x+v0.z*v0.z >=
      v1.x*v1.x+v1.z*v1.z ) return 0;                     //  ON LINE
    
  return 1;                                               //  CCW
}


int FieldlineLib::intersect( Point l0_p0, Point l0_p1,
                             Point l1_p0, Point l1_p1 )
{
  //  See if the lines intersect.    
  if( ( ccw( Vector(l0_p1-l0_p0), Vector(l1_p0-l0_p0)) *
        ccw( Vector(l0_p1-l0_p0), Vector(l1_p1-l0_p0) ) <= 0 ) &&
      ( ccw( Vector(l1_p1-l1_p0), Vector(l0_p0-l1_p0)) *
        ccw( Vector(l1_p1-l1_p0), Vector(l0_p1-l1_p0) ) <= 0 ) ) {
        
    //  See if there is not a shared point.
    if( l0_p0 != l1_p0 && l0_p0 != l1_p1 &&
        l0_p1 != l1_p0 && l0_p1 != l1_p1 )
      return 1;
        
    //  See if there is a shared point.
    else if( l0_p0 == l1_p0 || l0_p0 == l1_p1 ||
             l0_p1 == l1_p0 || l0_p1 == l1_p1 )
      return 2;
        
    //  There must be a point that is on the other line.
    else
      return 3;
  }
    
  //  Lines do not intersect.
  return 0;
}


void FieldlineLib::convexHull( vector< pair< Point, unsigned int > > &hullPts,
                               unsigned int &m,
                               unsigned int npts,
                               int dir ) {

  if( npts - m < 3 ) {
    m = npts;
    return;
  }

  unsigned int min = m;

  // Find the point with the minimum z value.
  for( unsigned int i=m; i<npts; i++ ) {
    if( hullPts[ min ].first.z > hullPts[i].first.z )
      min = i;
  }

  // Store the minimum point so that we know when we are done.
  hullPts[npts] = hullPts[min];

  do {

    // Make the point with the smallest z value first.
    if( min != m ) {
      pair< Point, unsigned int > tmpPt = hullPts[m];
      hullPts[m] = hullPts[min];
      hullPts[min] = tmpPt;

//       unsigned tmpIndex = ordering[m];
//       ordering[m] = ordering[min];
//       ordering[min] = tmpIndex;
    }

    m++;
    min = m;

    // Find the next point that is the farthest to the right of all others.
    for( unsigned int i=min+1; i<npts+1; i++ ) {

      Vector v0 = (Vector) hullPts[m-1].first - (Vector) hullPts[min].first;
      Vector v1 = (Vector) hullPts[i  ].first - (Vector) hullPts[min].first;

      int CCW = ccw( v0, v1 );

      // The next point that is the farthest to the right of all
      // others will be in a clockwise direction (i.e. the convex
      // hull is in the counter clockwise direction.
      if( CCW == dir )
        min = i;

      // In this case the points are co-linear so take the point
      // that is closest to the starting point.
      else if( hullPts[i].second != hullPts[m-1].second && CCW == 0 ) {
        v1 = (Vector) hullPts[m-1].first - (Vector) hullPts[i].first;

        if( v0.length2() > v1.length2() )
          min = i;
      }
    }

    // Stop when the first point is found again.
  } while( min != npts );

  for( unsigned int i=0; i<m; ++i ) {
    cerr << hullPts[i].second << endl;
  }
  cerr << endl;
}


bool FieldlineLib::hullCheck( vector< Point > &points, int &direction )
{

  // If one, two, or three points the ordering makes no difference and
  // it is convex.
  if( points.size() <= 2 ) {
      return true;
  }
  else if( points.size() == 3 ) {
    return ccw( points[0] - points[1], points[2] - points[1] );
  }

  vector< pair < Point, unsigned int > > pts;
  vector< pair < Point, unsigned int > > hullPts;

  // Store the points and their original order in a temporary vector.
  for( unsigned int i=0; i<points.size(); i++ )
    pts.push_back( pair< Point, unsigned int >( points[i], i ) );

  unsigned int npts = chainHull_2D( pts, hullPts, 0 );

  unsigned int cc = 0;

  for( unsigned int i=0; i<npts-1; i++ )
  {
    // The hull points are returned in a clockwise order. So if the
    // point ordering is increasing then the point ordering is also in
    // clockwise.
    if( hullPts[i].second < hullPts[i+1].second+1)
      ++cc;
  }
   
  // The hull is found in a clockwise direction if the point count is
  // increasing.
  direction = ((cc > npts/2) ? 1 : -1);

  return (npts == points.size());
}


// Find the great comon denominator.
unsigned int FieldlineLib::GCD( unsigned int a, unsigned int b )
{
  if( a < b )
    { unsigned int tmp = a; a = b; b = tmp; }

  if( a % b )
    return GCD(b, a % b);
  else
    return b;
}


Point FieldlineLib::circle(Point &pt1, Point &pt2, Point &pt3)
{
  if (!IsPerpendicular(pt1, pt2, pt3) )
    return CalcCircle(pt1, pt2, pt3);   
  else if (!IsPerpendicular(pt1, pt3, pt2) )
    return CalcCircle(pt1, pt3, pt2);   
  else if (!IsPerpendicular(pt2, pt1, pt3) )
    return CalcCircle(pt2, pt1, pt3);   
  else if (!IsPerpendicular(pt2, pt3, pt1) )
    return CalcCircle(pt2, pt3, pt1);   
  else if (!IsPerpendicular(pt3, pt2, pt1) )
    return CalcCircle(pt3, pt2, pt1);   
  else if (!IsPerpendicular(pt3, pt1, pt2) )
    return CalcCircle(pt3, pt1, pt2);   
  else
    return Point(-1,-1,-1);
}


// Check the given point are perpendicular to x or y axis 
bool FieldlineLib::IsPerpendicular(Point &pt1, Point &pt2, Point &pt3)
{
  double d21z = pt2.z - pt1.z;
  double d21x = pt2.x - pt1.x;
  double d32z = pt3.z - pt2.z;
  double d32x = pt3.x - pt2.x;
        
  // checking whether the line of the two pts are vertical
  if (fabs(d21x) <= FLT_MIN && fabs(d32z) <= FLT_MIN)
    return false;
    
  else if (fabs(d21z) < FLT_MIN ||
           fabs(d32z) < FLT_MIN ||
           fabs(d21x) < FLT_MIN ||
           fabs(d32x)<= FLT_MIN )
    return true;

  else
    return false;
}

Point FieldlineLib::CalcCircle(Point &pt1, Point &pt2, Point &pt3)
{
  double d21z = pt2.z - pt1.z;
  double d21x = pt2.x - pt1.x;
  double d32z = pt3.z - pt2.z;
  double d32x = pt3.x - pt2.x;
        
  if (fabs(d21x) < FLT_MIN && fabs(d32z ) < FLT_MIN ) {

    return Point( ( 0.5*(pt2.x + pt3.x) ),
                  ( pt1.y ),
                  ( 0.5*(pt1.z + pt2.z) ) );
  }
        
  // IsPerpendicular() assure that xDelta(s) are not zero
  double aSlope = d21z / d21x;
  double bSlope = d32z / d32x;

  // checking whether the given points are colinear.    
  if (fabs(aSlope-bSlope) > FLT_MIN) {
    
    // calc center
    double x = (aSlope*bSlope*(pt1.z - pt3.z) +
                bSlope*(pt1.x + pt2.x) -
                aSlope*(pt2.x + pt3.x) ) / (2.0* (bSlope-aSlope) );

    return Point( x,
                  pt1.y,
                  -(x - (pt1.x+pt2.x)/2.0) / aSlope + (pt1.z+pt2.z)/2.0 );
  }

  return Point(0,0,0);
}


bool
FieldlineLib::
IntersectCheck( vector< Point >& points, unsigned int nbins ) {

  for( unsigned int i=0, j=nbins; i<nbins && j<points.size(); i++, j++ ) {
    Point l0_p0 = points[i];
    Point l0_p1 = points[j];

    for( unsigned int k=i+1, l=j+1; k<nbins && l<points.size(); k++, l++ ) {
      Point l1_p0 = points[k];
      Point l1_p1 = points[l];

//       cerr << nbins
//         << "   " << i << "  " << j << "  " << k << "  " << l << endl;

      if( j != k && intersect( l0_p0, l0_p1, l1_p0, l1_p1 ) == 1)
        return false;
    }
  }

  return true;
}


unsigned int FieldlineLib::
Blankinship( unsigned int toroidalWinding,
             unsigned int poloidalWinding,
             unsigned int offset /* = 1 */ )
{

  unsigned int skip;

  if( toroidalWinding > 1 && poloidalWinding != 0 ) {
    //  To find the skip find the mutual primes via the
    //  Blankinship Algorithm.
    for( skip=1; skip<toroidalWinding; skip++ )
      if( (skip * poloidalWinding) % toroidalWinding == offset )
        break;
    
    if( skip == toroidalWinding )
      skip = 1;
    
  } else {
    skip = 0;
  }

  return skip;
}


int
compareWindingSet( const pair < pair<unsigned int, unsigned int >, double > s0,
                   const pair < pair<unsigned int, unsigned int >, double > s1
                   )
{
  return (s0.second > s1.second );
}


void FieldlineLib::
poloidalWindingCheck( vector< unsigned int > &poloidalWindingCounts,
                      vector< pair < pair<unsigned int,
                                          unsigned int >,
                                     double > > &windingSetList )
{
  windingSetList.clear();

  unsigned int nsets = poloidalWindingCounts.size();

  float safetyfactor = (float) nsets / (float) poloidalWindingCounts[nsets-1];

  if( verboseFlag )
    cerr << "Base safetyfactor " << safetyfactor << endl;

  // The premise is that for a given toroidal winding the poloidal
  // winding should be consistent between each Nth punction point,
  // where N is the toroidal winding. For instance, if the toroidal
  // winding is 5 and the poloidal winding is 2. Then the pattern
  // should be:

  // 0 1 1 1 2 - 2 3 3 3 4 - 4 5 5 5 6

  // In this case the different between every 5th value (the toroidal
  // winding) should be 2 (the poloidal winding).

  for( unsigned int toroidalWinding=1;
       toroidalWinding<nsets/2;
       ++toroidalWinding )
  {
    // Get the average value of the poloidal winding.
    double poloidalWindingAve = 0;

    for( unsigned int i=0; i<nsets-toroidalWinding; ++i)
      poloidalWindingAve += (poloidalWindingCounts[i+toroidalWinding] -
                             poloidalWindingCounts[i]);

    poloidalWindingAve =
      (float) poloidalWindingAve / (float) (nsets-toroidalWinding);
      
    if( poloidalWindingAve < 0.5 )
      continue;

    // Round the average value to the nearest integer value.
    unsigned int poloidalWinding = (poloidalWindingAve + 0.5);

    // Count the number of times the poloidal winding matches the
    // windings between puncture points (i.e. the poloidal winding
    // set).
    unsigned int nMatches = 0;

    for( unsigned int i=0; i<nsets-toroidalWinding; ++i)
    {
      if( poloidalWinding ==
          poloidalWindingCounts[i+toroidalWinding] - poloidalWindingCounts[i] )
        ++nMatches;
    }

    double confidence = (double) nMatches / (double) (nsets-toroidalWinding);

    unsigned int t = toroidalWinding;
    unsigned int p = poloidalWinding;

    // If the toroidalWinding and poloidalWinding have a common
    // denominator find the greatest denominator and remove it.
//    if( t != p )
    {
      for( unsigned int d=p; d>1; --d) {
        if( t % d == 0 && p % d == 0 ) {
          t /= d;
          p /= d;
          
          d = p;
        }
      }
    }

    bool lowOrder = false;

    // Keep the low order toroidalWinding / poloidalWinding
    if( t != toroidalWinding && p != poloidalWinding )
    {
      for( unsigned int i=0; i<windingSetList.size(); ++i )
      {
        if( windingSetList[i].first.first == t &&
            windingSetList[i].first.second == p )
        {
          lowOrder = true;

          // If the confidence happens to be higher for the higher
          // order keep it instead. Typically the lower order math is
          // better.
          if( windingSetList[i].second < confidence )
            windingSetList[i].second = confidence;
          
          break;
        }
      }
    }

    // Did not fina a lower order match so record the set.
    if( ! lowOrder )
    {
      pair < pair<unsigned int, unsigned int >, double >
        windingSetMatch( pair<unsigned int, unsigned int >(t, p),
                         confidence );

      windingSetList.push_back( windingSetMatch );
    }
  }

  // Now sort the results.
  sort( windingSetList.begin(), windingSetList.end(),
        compareWindingSet );
}


double FieldlineLib::
calculateSumOfSquares( vector< Point >& points,
                       unsigned int period,
                       int checkType )
{
  // Find the sum of squares for each of the periods. If the period is
  // correct the sum of squares should be a small compared to a wrong
  // value.

  double sumofsquares = 0;
  double tSamples = 0;
    
  for( unsigned int i=0; i<period; ++i )
  {
    // Find the centroid of the points based on the period.
    Vector centroid(0,0,0);
    double length = 0;
    double nSamples = 0;
    
    for( unsigned int j=i; j<points.size(); j+=period )
    {
      if( checkType == 0 || checkType == 1 )
      {
        centroid += points[j];
        ++nSamples;
      }

      else if( checkType == 2 && j>period )
      {
        length += (points[j]-points[j-period]).length();
        ++nSamples;
      }
    }
    
    if( checkType == 0 || checkType == 1 )
      centroid /= (double) nSamples;

    else if( checkType == 2 )
      length   /= (double) nSamples;

    if( nSamples > 1 )
    {
      // Get the sum of squares for each bin.
      double tmpsumofsquares = 0;
      
      for( unsigned int j=i; j<points.size(); j+=period )
      {
        // Centroid difference
        if( checkType == 0 )
        {
          Vector diff = points[j] - centroid;
          tmpsumofsquares += diff.length2();
          ++tSamples;
        }
        // Z difference
        else if( checkType == 1 )
        {
          double diff = points[j].z - centroid.z;
          tmpsumofsquares += (diff * diff);
          ++tSamples;
        }
        // Length difference
        else if( checkType == 2 && j>period )
        {
          double diff = (points[j]-points[j-period]).length() - length;
          tmpsumofsquares += (diff * diff);
          ++tSamples;
        }
      }

      sumofsquares += tmpsumofsquares;
    }
  }

  return sumofsquares / tSamples;
}

int
compareSecond( const pair< unsigned int, double > s0,
               const pair< unsigned int, double > s1 )
{
  return ( s0.second < s1.second );
}

unsigned int FieldlineLib::
periodicityStats( vector< Point >& points,
                  vector< pair< unsigned int, double > >& stats )
{
  stats.clear();

 // Find the base period variance.
  unsigned int best_period = points.size();
  double best_var = 1.0e9;

  double base_var = calculateSumOfSquares( points, 1, 1 );

//   if( verboseFlag )
//     cerr << "Base variance  " << base_var << endl;
  
  // Find the period with the best variance.
  for( unsigned int i=2; i<points.size()*4/7; ++i ) 
  {
    double test_var = 1.0 -
      (base_var - calculateSumOfSquares( points, i, 1 )) / base_var;

    stats.push_back( pair< unsigned int, double > (i, test_var ) );

//    if( verboseFlag && (i == 72 || i == 180) )
//     if( verboseFlag )
//       cerr << "Test  " << i << "  "
//         << calculateSumOfSquares( points, i, 1 ) << "  "
//         << test_var << "  "
//       << endl;

    if( best_var > test_var ) 
    {
      best_var = test_var;
      best_period = i;
    }
  }

  if( verboseFlag )
    cerr << "Best period " << best_period << "  "
         << "variance  " << best_var << endl;
  
  if( stats.size() == 0 )
    stats.push_back( pair< unsigned int, double > (best_period, best_var ) );

  // Now sort the results.
  sort( stats.begin(), stats.end(), compareSecond );

  // Find the greatest common denominator
  unsigned int cc = 0, gcd;

  for( unsigned int i=0; i<stats.size(); ++i, ++cc )
  {
    if( stats[i].second > 2.0 * stats[0].second )
    {
      if( cc == 1 )
      {
        if( verboseFlag )
        {
          cerr << "tested period  " << stats[i].first << "  "
               << "normalized variance  " << stats[i].second << "  "
               << endl;

          if( i+1<stats.size() )
            cerr << "tested period  " << stats[i+1].first << "  "
                 << "normalized variance  " << stats[i+1].second << "  **"
                 << endl;
        }

        stats.resize(i+1);

      }
      else
        stats.resize(i);
      
      break;
    }

    if( verboseFlag )
      cerr << "tested period  " << stats[i].first << "  "
           << "normalized variance  " << stats[i].second << "  "
           << endl;
  }

  if( cc == 1 )
  {
    gcd = stats[0].first;
  }
  else
  {
    map< int, int > GCDCount;
    
    map<int, int>::iterator ic;
    
    for( unsigned int i=0; i<cc; ++i )
    {
      for( unsigned int j=i+1; j<cc; ++j )
      {
        gcd = GCD( stats[i].first, stats[j].first );
        
        // Find the GCD excluding 1, 2 and 3.
        if( gcd > 3 )
        {
          ic = GCDCount.find( gcd );
          
          if( ic == GCDCount.end() )
            GCDCount.insert( pair<int, int>( gcd, 1) );
          else (*ic).second++;
        }
      }
    }
    
    ic = GCDCount.begin();
    
    cc = 0;

    gcd = stats[0].first;

    // Find the most frequent GCD excluding 1, 2 and 3.
    while( ic != GCDCount.end() )
    {
      if( cc == (*ic).second && gcd < (*ic).first )
      {
        gcd = (*ic).first;
      }
      else if( cc < (*ic).second )
      {
        gcd = (*ic).first;
        
        cc = (*ic).second;
      }
      
      ++ic;
    }
  }
  
  return gcd;
}


unsigned int FieldlineLib::
periodicityChecks( vector< Point >& points,
                   vector< pair< unsigned int, double > >& stats,
                   double &consistency,
                   bool useBest )
{
  bool tmpVF = verboseFlag;

  unsigned int gcd, period, testPeriod;

  // Get the baseline period.
  period = periodicityStats( points, stats );

  if( useBest )
    period = stats[0].first;

  if( verboseFlag )
    cerr << "period = " << period << endl;

  // Make sure there are enough points to do the consistency analysis.
  if( points.size() < period * 1.75 )
    return period;

  // Check for consistancy
  vector< Point > tmp_pts = points;

  unsigned int cc=0, count=0;

  pair< unsigned int, double > minStats;
  unsigned int minSize;

  minStats.second = 1.0e9;

  // Check the consistancy in the period until the number of points is
  // reduced to 1.75% the period.
  vector< pair< unsigned int, double > > tmpStats;

  for( int i=points.size()-1; i>period * 1.75; --i )
  {
    tmp_pts.resize( i );
    
//     if( verboseFlag )
//       cerr << "nPoints " << i << "  ";
    
    verboseFlag = false;
    testPeriod = periodicityStats( tmp_pts, tmpStats );
    verboseFlag = tmpVF;

    if( useBest )
      testPeriod = tmpStats[0].first;

    if( testPeriod % period == 0 )
      ++count;

    if( minStats.second > tmpStats[0].second )
    {
      minStats = tmpStats[0];
      minSize = i;
    }

//     if( verboseFlag )
//       cerr << "period  " << testPeriod << "  "
//         << "variance  " << tmpStats[0].second << endl;

    ++cc;
  }

  if( cc )
    consistency = (double) count / (double) cc;
  else
    consistency = 0;

  return period;
}


bool FieldlineLib::
rationalCheck( vector< Point >& points,
               unsigned int toroidalWinding,
               unsigned int &nnodes,
               float delta)
{
  // Look at the distance between the centroid of each toroidal group
  // and the points that are in it. If it is smaller then the distance
  // between the points that make up the fieldine is assumed to be on
  // a rational surface.

  // Note: this distance, delta, is defined indirectly based on the
  // integration step size.

  for( unsigned int i=0; i<toroidalWinding; i++ ) {

    // Get the local centroid for the toroidal group.
    Vector localCentroid(0,0,0);

    unsigned int npts = 0;

    for( unsigned int j=i; j<points.size(); j+=toroidalWinding, ++npts )
      localCentroid += (Vector) points[j];

    localCentroid /= (float) npts;

    for( unsigned int j=i; j<points.size(); j+=toroidalWinding ) {

      Vector vec = (Vector) points[j] - localCentroid;

      if( vec.length() > delta ) {
        return false;
      }
    }
  }

  nnodes = 1;

  return true;
}


bool FieldlineLib::
islandChecks( vector< Point >& points,
              unsigned int toroidalWinding,
              unsigned int &islands,
              unsigned int &nnodes,
              bool &complete )
{
  islands = 0;
  nnodes = 0;
  complete = 0;

  unsigned int npts;

  // Get the centroid for all of the points.
  Vector globalCentroid(0,0,0);

  for( unsigned int i=0; i<points.size(); i++ )
    globalCentroid += (Vector) points[i];

  globalCentroid /= (float) points.size();

  if( toroidalWinding == 1 ) {

    // Check to see if it overlaps itself - it should or more points
    // need to be obtained.
    npts = points.size() / toroidalWinding;

    for( unsigned int j=toroidalWinding, k=j+toroidalWinding;
         k<points.size();
         j+=toroidalWinding, k+=toroidalWinding ) {
      
      // See if the test point is between the first secton.
      Vector v0 = (Vector) points[0] - (Vector) points[k];
      Vector v1 = (Vector) points[1] - (Vector) points[k];
        
      if( Dot( v0, v1 ) < 0.0 ) {
        npts = k / toroidalWinding;
        complete = true;
        break;
      }

      // See if the first point is between the test section.
      v0 = (Vector) points[0] - (Vector) points[j];
      v1 = (Vector) points[0] - (Vector) points[k];
      
      if( Dot( v0, v1 ) < 0.0) {
        npts = k / toroidalWinding;
        complete = true;
        break;
      }
    }

  } else {

    npts = toroidalWinding;
  }

  vector< Point > tmp_points;
  int direction;

  tmp_points.resize( npts );

  for( unsigned int j=toroidalWinding; j<npts; ++j )
    tmp_points[j] = points[j];

  bool convex = hullCheck( tmp_points, direction );

  // A convex hull for a toroidalWinding of 1 is defined as not
  // being an island.
  if( toroidalWinding == 1 && convex )
  {
//  cerr << "toroidalWinding of 1 and convex hull" << endl;
    return false;
  }

//  cerr << "Convex  " << convex << endl;

  // Check for islands. Islands will exists if there is a change in
  // direction of the connected points relative to a base point. If
  // the hull is convex the base point may the centroid of all of the
  // points or based upon a point that is perpendicular to the
  // principal axis of the group of points.
  vector< unsigned int > nodes(toroidalWinding);

  for( unsigned int i=0; i<toroidalWinding; i++ )
  {
    Vector baseCentroid;

    // If the hull is convex the global centroid can be used because
    // all of the islands will surround it in a radial manner.
    if( convex )
    {
      baseCentroid = globalCentroid;
    }
    // Otherwise use an offset from the local centroid of each group.
    else // !concave
    {
      // Get the local centroid for the group.
      Vector localCentroid(0,0,0);

      unsigned int npts = 0;

      for( unsigned int j=i; j<points.size(); j+=toroidalWinding, npts++ )
        localCentroid += (Vector) points[j];

      localCentroid /= (float) npts;

//      cerr << "localCentroid  " << localCentroid << endl;

      // Get the principal axes of the points.
      float Ixx = 0.0;
      float Ixz = 0.0;
      float Izz = 0.0;

      float maxDist = 0;

      // Get the moments of intertial for each point. It assumed that
      // everything is in the Y plane as such the moments of intertial
      // along the Y axis are zero.
      for( unsigned int j=i; j<points.size(); j+=toroidalWinding )
      {

        Vector vec = (Vector) points[j] - localCentroid;

        if( maxDist < vec.length() )
          maxDist = vec.length();

        Ixx += vec.z*vec.z;
        Ixz -= vec.x*vec.z;
        Izz += vec.x*vec.x;
      }

      // Short cut to the principal axes because the Y moments of
      // intertial are zero.
      float alpha = atan( 2.0 * Ixz / (Ixx - Izz) ) / 2.0;

      float Pxx = Ixx + Ixz * sin(alpha       )/cos(alpha       );
      float Pzz = Izz + Ixz * cos(alpha+M_PI/2)/sin(alpha+M_PI/2);

//     cerr << "Principal axes of intertial "
//       << Pxx << "  " << Pzz << "  " << Pxx/Pzz << "  " << Pzz/Pxx << endl;

      // For a toroidalWinding of 1 if the moment of interia of one axis is
      // similar to the other axis then use the centroid directly.
      if( toroidalWinding == 1 && Pxx / Pzz < 5.0 && Pzz / Pxx < 5.0 )
      {
//      cerr << "Using local centroid " << endl;
        baseCentroid = localCentroid;
      }
      else
      {
        // Find the approximate center if points were projected onto a
        // circle.
        unsigned int npts = 0;
        Vector center(0,0,0);
 
        for( unsigned int j=i+toroidalWinding;
             j<points.size()-toroidalWinding;
             j+=toroidalWinding ) {
          
          unsigned int j_1 = j - toroidalWinding;
          unsigned int j1  = j + toroidalWinding;


          center += (Vector) circle( points[j_1],
                                     points[j  ],
                                     points[j1 ] );

          ++npts;
        }

        center /= (float) npts;

//      cerr << "center  " << center << endl;

        // Use the principal axes to get an offset from the local
        // centroid which gives a point outside the island.

        // The direction along the axis is determined by where the
        // center is located.

//      cerr << "localCentroid  " << localCentroid << endl;

        if( Pxx > Pzz )
        {
          if( Dot( center - localCentroid, 
                   Vector( cos(alpha), 0, sin(alpha) ) ) )
            baseCentroid = localCentroid +
              Vector(  cos(alpha), 0, sin(alpha) ) * maxDist;
          else
            baseCentroid = localCentroid -
              Vector(  cos(alpha), 0, sin(alpha) ) * maxDist;
        }
        else
        {
          if( Dot( center - localCentroid, 
                   Vector( -sin(alpha), 0, cos(alpha) ) ) )
            baseCentroid = localCentroid +
              Vector( -sin(alpha), 0, cos(alpha) ) * maxDist;
          else
            baseCentroid = localCentroid -
              Vector( -sin(alpha), 0, cos(alpha) ) * maxDist;
        }
      }
    }

//    cerr << "baseCentroid  " << baseCentroid << endl;

    unsigned int turns = 0;
    unsigned int firstTurn = 0;
    unsigned int   midTurn = 0;
    unsigned int  lastTurn = 0;

    // Get the direction based on the first two points.
    Vector v0 = (Vector) points[i                ] - baseCentroid;
    Vector v1 = (Vector) points[i+toroidalWinding] - baseCentroid;

    bool localCCW, lastCCW = (ccw( v0, v1 ) == 1);

    // Get the direction based on the remaining points.
    for( unsigned int j=i+2*toroidalWinding;
         j<points.size();
         j+=toroidalWinding )
    {
      v0 = v1;
      v1 = (Vector) points[j] - baseCentroid;

      // Points are parallel - this result should not occur as the
      // base centroid point should be on the concave side of the
      // island.
      if (ccw( v0, v1 ) == 0 )
        localCCW = lastCCW;
      else
        localCCW = (ccw( v0, v1 ) == 1);

      // A change in direction indicates that an island is present.
      if( localCCW != lastCCW )
      {
        lastCCW = localCCW;

        // Record the number of turns. Ideally three turns will be
        // found which indicates that the island is complete. Although
        // the island could be complete with only two turns.
        ++turns;

        if( turns == 1 )
          ++islands;

        if( turns == 1 )     firstTurn = j - toroidalWinding;
        else if( turns == 2 )  midTurn = j - toroidalWinding;
        else if( turns == 3 ) lastTurn = j - toroidalWinding;

        if( turns == 3 )
          break;
      }
    }

    unsigned int overlap = 0;

    // Approximate number of points in the islands. 
    nodes[i] = points.size() / toroidalWinding;

    // Check to see if the island overlaps itself. Only use this if
    // there are two or more turns. Otherwise the it can appear that
    // it overlaps itself earlier.
    if( turns >= 2 )
    {
      // Okay initial approximatation. It really depending on where
      // the turns are relative to each other/
      if( turns == 3 )
        nodes[i] = (lastTurn - firstTurn) / toroidalWinding + 1;

      // Look for the overlap between the ends of the island which are
      // flatter.
      unsigned int base, start;      
      unsigned int quarter_dist =
        ((midTurn - firstTurn) / 4 / toroidalWinding) * toroidalWinding;

      // If the start is in the middle section use it.
      if( quarter_dist <= firstTurn && firstTurn <= quarter_dist * 3 )
      {
        base = i;
        start = midTurn;
      }

      // If the start is before the middle section use the first
      // middle section.
      else if( firstTurn >= quarter_dist * 2 )
      {
        base = firstTurn - quarter_dist * 2;
        start = midTurn;
      }

      // If the start is after the middle section use the next middle
      // secction.
      else
      {
        base = firstTurn + quarter_dist;

        if( lastTurn )
          start = lastTurn;
        else
          start = midTurn;
      }

      if( turns && verboseFlag )
        cerr << "Island " << i << " - "
             << " nodes " << nodes[i]
             << " turns " << turns
             << " first " << firstTurn
             << " mid " << midTurn
             << " last " << lastTurn
             << " base " << base
             << " start " << start
             << endl;

      for( unsigned int j=start, k=j+toroidalWinding;
           k<points.size();
           j+=toroidalWinding, k+=toroidalWinding ) {

        // See if the test point is between the first secton.
        v0 = ((Vector) points[base                ] -
              (Vector) points[k                   ]);
        v1 = ((Vector) points[base+toroidalWinding] -
              (Vector) points[k                   ]);
        
        if( Dot( v0, v1 ) < 0.0 ) {
          overlap = k;
          nodes[i] = (overlap-base) / toroidalWinding;
          turns = 3;
          complete = true;
          break;
        }

        // See if the first point is between the test section.
        v0 = (Vector) points[base] - (Vector) points[j];
        v1 = (Vector) points[base] - (Vector) points[k];
        
        if( Dot( v0, v1 ) < 0.0 ) {
          overlap = k;
          nodes[i] = (overlap-base) / toroidalWinding;
          turns = 3;
          complete = true;
          break;
        }
      }

      // Do a distance search based on the overlaping sections having
      // two points in close proximity while at the same time the
      // overlaping sections have approximately the same length.
      if( 1 || !complete )
      {
        unsigned int stop;

        // Because the curve may overlap the start multiple times
        if( lastTurn )
        {
          stop = lastTurn +
            ((midTurn - firstTurn) / 4 / toroidalWinding) * toroidalWinding;
        }
        else
        {
          stop = points.size()-toroidalWinding;
        }

        // Get the maximum distance from the first point so that the
        // search is not conducted near neighbor points.
        double max_gap = 0;
        
        for( unsigned int j=i+toroidalWinding;
             j<points.size()-toroidalWinding;
             j+=toroidalWinding )
        {
          double tmp_gap = (points[i] - points[j]).length();

          if( max_gap < tmp_gap )
            max_gap = tmp_gap;
        }

        max_gap *= 0.5;

        double min_gap = 1.0e9;  // Distance between points.
        double min_gap_index;

        double min_len = 1.0e9;  // Difference in segments.
        double min_len_index;

        double min_dist = 1.0e9; // Minimum distance.
        double min_dist_index;

        double min_dist2 = 1.0e9;// Second minimum distance.
        double min_dist2_index;

        bool past_max_gap = false;

        // Base length of the first segment.
        double len = (points[i] - points[i+toroidalWinding]).length();

        for( unsigned int j=i+toroidalWinding;
             j<points.size()-toroidalWinding && j<stop;
             j+=toroidalWinding )
        {
          double tmp_gap = (points[i] - points[j]).length();

          if( tmp_gap > max_gap )
            past_max_gap = true;

          double tmp_len = (points[j] - points[j+toroidalWinding]).length();
          double dist = sqrt(tmp_gap*tmp_gap + (tmp_len-len)*(tmp_len-len));

          if( past_max_gap )
          {
            if( min_gap > tmp_gap )
            {
              min_gap = tmp_gap;
              min_gap_index = j;
            }

            if( min_len > tmp_len )
            {
             min_len = tmp_len;
             min_len_index = j;
            }
          
            if( min_dist > dist )
            {
//          cerr << j << "  "
//               << tmp_gap << "  "
//               << fabs(tmp_len - len) << "  " 
//               << dist << "  "
//               << endl;

              min_dist2 = min_dist;
              min_dist2_index = min_dist_index;

              min_dist = dist;
              min_dist_index = j;
            }

            else if( min_dist2 > dist )
            {
//          cerr << j << "  "
//               << tmp_gap << "  "
//               << fabs(tmp_len - len) << "  " 
//               << dist << "  "
//               << endl;

              min_dist2 = dist;
              min_dist2_index = j;
            }
          }
        }

        if( !complete )
        {
          overlap = min_dist_index;
          nodes[i] = (min_dist_index-i)/toroidalWinding;
          turns = 3;
          complete = true;
        }

        cerr << "gap " << min_gap_index << "  "
             << "len " << min_len_index << "  "
             << "dist " << min_dist_index << "  "
             << "dist2 " << min_dist2_index << "  "
             << "nodes " << (min_dist_index-i)/toroidalWinding << endl;
      }
    }
    
    if( turns && verboseFlag )
      cerr << "Island " << i << " - "
           << " nodes " << nodes[i]
           << " turns " << turns 
           << " first " << firstTurn
           << " mid " << midTurn
           << " last " << lastTurn
           << " overlap " << overlap
           << " complete " << complete
           << endl;

    nnodes += nodes[i];

    // Started on a surface but jumpped to an island - i.e. a
    // separtrice
    if( !complete && lastTurn &&
        ((int) (firstTurn / toroidalWinding) - (int) (nodes[i]/2)) > 2 )
    {
      --islands;

      if( turns && verboseFlag )
        cerr << "  !!!!!!!!!!!!!!! Separatrice !!!!!!!!!!!!!!!" << endl;
    }
  }

  // Get the average number of nodes.
  nnodes /= (float) toroidalWinding;

  if( islands ) {

    unsigned int cc = 0;
    unsigned int index = 0;
    unsigned int new_nnodes = 0;

    for( unsigned int i=0; i<toroidalWinding; i++ )
    {
      if( nodes[i] < nnodes - 1 || nnodes + 1 < nodes[i] )
      {
        cc++;
        index = i;
      }
      else
        new_nnodes += nodes[i];
    }

    if( cc == 1 )
    {
      nnodes = new_nnodes / (float) (toroidalWinding-cc);
    }

    for( unsigned int i=0; i<toroidalWinding; i++ )
    {
      if( nodes[i] < nnodes - 1 || nnodes + 1 < nodes[i] )
      {
        if( verboseFlag )
          cerr << "Appears to be islands but not self consistent, "
               << "average number of nodes  " << nnodes << "   (  ";
          
        for( unsigned int i=0; i<toroidalWinding; i++ )
            if( verboseFlag )
              cerr << nodes[i] << "  ";

        cerr << ")" << endl;

        break;
      }
    }  
  }

  return (bool) islands;
}


FieldlineInfo
FieldlineLib::fieldlineProperties( vector< Point > &ptList,
                                   unsigned int override,
                                   unsigned int maxToroidalWinding,
                                   double windingPairConfidence,
                                   double periodicityConsistency,
                                   bool findIslandCenters )
{
  vector< Point > poloidal_puncture_pts;
  vector< Point > toroidal_puncture_pts;
  vector< Point > ridgeline_points;

  poloidal_puncture_pts.clear();
  toroidal_puncture_pts.clear();
  ridgeline_points.clear();

  vector< unsigned int > poloidalWindingCounts;
  poloidalWindingCounts.clear();

  bool haveFirstIntersection = false;
  double maxZ = 0;

  float delta = 0.0;

  Point lastPt, currPt = ptList[0];
  Vector planePt( 0, 0, 0 );

  cerr << "-----------------------------------------------------------------" << endl;

  if( verboseFlag )
    cerr << "Analyzing  " << ptList[0] << "  "
         << "with  " << ptList.size() << " fieldline points"
         << endl;

  // Set up the Y plane equation as the base analysis takes place in
  // the X-Z plane.
  Vector planeNY( 0, 1, 0 );
  double planeY[4];

  planeY[0] = planeNY.x;
  planeY[1] = planeNY.y;
  planeY[2] = planeNY.z;
  planeY[3] = Dot( planePt, planeNY );
      
  double lastDistY, currDistY = Dot(planeNY, currPt) - planeY[3];

  Vector planeNZ(0, 0, 1);
  double planeZ[4];
      
  planeZ[0] = planeNZ.x;
  planeZ[1] = planeNZ.y;
  planeZ[2] = planeNZ.z;
  planeZ[3] = Dot(planePt, planeNZ);

  double lastDistZ, currDistZ = Dot(planeNZ, currPt) - planeZ[3];

  // Now collect the points.
  unsigned int npts = 0;

  for( unsigned int i=1; i<ptList.size(); ++i)
  {
    lastPt = currPt;
    currPt = ptList[i];

    // Save the distance between points to use for finding periodic
    // fieldlines (i.e. rational surfaces and re-connection).
    Vector vec = (Vector) lastPt - (Vector) currPt;

    delta += vec.length();
    ++npts;

    // Poloidal plane distances.
    lastDistY = currDistY;
    currDistY = Dot( planeNY, currPt ) - planeY[3];

    // First look at only points that intersect the poloidal plane.
    if( SIGN(lastDistY) != SIGN(currDistY) ) 
    {
      Vector dir(currPt-lastPt);
      
      double dot = Dot(planeNY, dir);
      
      // If the segment is in the same direction as the poloidal plane
      // then find where it intersects the plane.
      if( dot > 0.0 )
      {
        Vector w = (Vector) lastPt - planePt;
        
        double t = -Dot(planeNY, w ) / dot;
        
        Point point = Point(lastPt + dir * t);
        
        poloidal_puncture_pts.push_back( point );

        poloidalWindingCounts.push_back( toroidal_puncture_pts.size() );
      }
    }

    // Poloidal plane distances.
    lastDistZ = currDistZ;
    currDistZ = Dot( planeNZ, currPt ) - planeZ[3];
    
    // Find the positive zero crossings which indicate a poloidal
    // winding. Do this check after the first toroidal puncture.
    if( !poloidal_puncture_pts.empty() )
    {
      // First look at only points that intersect the toroiadal plane.
      if( SIGN(lastDistZ) != SIGN(currDistZ) ) 
      {
        Vector dir(currPt-lastPt);
      
        double dot = Dot(planeNZ, dir);
      
        // If the segment is in the same direction as the toroidal plane
        // then find where it intersects the plane.
        if( dot > 0.0 )
        {
          Vector w = (Vector) lastPt - planePt;
        
          double t = -Dot(planeNZ, w ) / dot;
        
          Point point = Point(lastPt + dir * t);
        
          toroidal_puncture_pts.push_back( point );

          if( haveFirstIntersection )
            ridgeline_points.push_back( Point( (float) ridgeline_points.size(),
                                               0,
                                               maxZ) );
          else
            haveFirstIntersection = true;

          maxZ = 0;
        }
      }
    }

    if( maxZ < currPt.z )
      maxZ = currPt.z;
  }

  if( ptList.empty() ||
      poloidal_puncture_pts.empty() ||
      toroidal_puncture_pts.empty() ||
      ridgeline_points.empty() )
  {
    FieldlineInfo fi;

    fi.type = FieldlineInfo::UNKNOWN;

    fi.toroidalWinding = 0;
    fi.poloidalWinding = 0;
    fi.windingGroupOffset = 0;
    fi.islands = 0;
    fi.nnodes  = 0;

    fi.confidence        = 0;
    fi.analysisStatus    = FieldlineInfo::UNKNOWN;
    fi.nPuncturesNeeded  = 0;
    fi.toroidalPeriod    = 0;
    fi.poloidalPeriod    = 0;
    fi.ridgelineVariance = 0;

    return fi;
  }


  // Find the mean value and variance of the ridgeline curve.
  double ridgeline_dc = 0;
  for( unsigned int i=0; i<ridgeline_points.size(); ++i ) 
    ridgeline_dc += ridgeline_points[i].z;
  
  ridgeline_dc /= (float) ridgeline_points.size();
  
  double ridgeline_dc_var = 0;
  for( unsigned int i=0; i<ridgeline_points.size(); ++i ) 
    ridgeline_dc_var += ((ridgeline_points[i].z - ridgeline_dc) *
                         (ridgeline_points[i].z - ridgeline_dc));
  
  // At this point all of the poloidal and toroidal puncture points
  // have been found.
    if( verboseFlag )
      cerr << poloidal_puncture_pts.size() << " poloidal puncture pts, "
           << toroidal_puncture_pts.size() << " toroidal puncture pts, and "
           << ridgeline_points.size() << " ridgeline pts " << endl;

  // Get the average distance between puncture points for finding
  // periodic fieldlines (i.e. rational surfaces and re-connection).
  delta  /= (float) npts;


  // Start the analysis.
  FieldlineInfo::FieldlineType type = FieldlineInfo::UNKNOWN;

  double safetyFactor;

  unsigned int toroidalWinding = 0, poloidalWinding = 0;
  unsigned int windingGroupOffset = 0, islands = 0;
  unsigned int nnodes = 0;
  float confidence = 0, ridgelineVariance = 0;

  bool complete = 0;
  unsigned int nPuncturesNeeded = 0;
  unsigned int toroidalPeriod = 0;
  unsigned int poloidalPeriod = 0;
  unsigned int ridgelineGCD = 0;

  unsigned int windingNumberMatchIndex = -1;
  unsigned int toroidalMatchIndex = -1;
  unsigned int poloidalMatchIndex = -1;

  vector< pair < pair<unsigned int, unsigned int >, double > > windingNumberList;
  vector< Point > islandCenters;

  // The user has set the toroidal winding get the poloidal winding
  // based on the data found.
  if( override ) 
  {
    toroidalWinding = override;

    // Get the average value of the poloidal winding.
    double poloidalWindingAve = 0;

    unsigned int npts = poloidalWindingCounts.size();

    for( unsigned int i=0; i<npts-toroidalWinding; ++i)
      poloidalWindingAve += (poloidalWindingCounts[i+toroidalWinding] -
                             poloidalWindingCounts[i]);

    poloidalWindingAve =
      (float) poloidalWindingAve / (float) (npts-toroidalWinding);
      
    // Round the average value to the nearest integer value.
    poloidalWinding = (poloidalWindingAve + 0.5);

    // Count the number of times the poloidal winding matches the
    // windings between puncture points (i.e. the poloidal winding
    // set).
    unsigned int nMatches = 0;

    for( unsigned int i=0; i<npts-toroidalWinding; ++i)
    {
      if( poloidalWinding ==
          poloidalWindingCounts[i+toroidalWinding] - poloidalWindingCounts[i] )
        ++nMatches;
    }

    double confidence = (double) nMatches / (double) (npts-toroidalWinding);

    windingGroupOffset = Blankinship( toroidalWinding, poloidalWinding );

    if( verboseFlag )
      cerr << "final toroidalWinding/poloidal  "
           << toroidalWinding << "  "
           << poloidalWinding << "  ("
           << (double)toroidalWinding/(double)poloidalWinding << ")  "
           << "consistency "
           << 100.0 * confidence
           << " percent" << endl;
  } 
  else
  {
    safetyFactor = (float) poloidalWindingCounts.size() /
      (float) poloidalWindingCounts[poloidalWindingCounts.size()-1];

    // Check the consistency of the poloidal winding counts. 
    poloidalWindingCheck( poloidalWindingCounts, windingNumberList );

    // Get the first set that passes the intersection test and passes
    // the user setable match limit. Default is 0.90 (90%)
    for( unsigned int i=0; i<windingNumberList.size(); ++i )
    {
      if( (maxToroidalWinding == 0 ||
           windingNumberList[i].first.first <= maxToroidalWinding) &&
          windingNumberList[i].second >= windingPairConfidence &&
          IntersectCheck( poloidal_puncture_pts,
                          windingNumberList[i].first.first ) )
      {
        windingNumberMatchIndex = i;

        toroidalWinding = windingNumberList[i].first.first;
        poloidalWinding = windingNumberList[i].first.second;

        // Base value from the periodicity check. This check is quite
        // accurate for stable systems.
        confidence = 0.40;

        // If the poloidal winding check was really good bump up the
        // confidence.
        if( windingNumberList[i].second > 0.98 )
          confidence += 0.10;

        if( verboseFlag )
          cerr << "*** using  toroidal/poloidal Winding  "
               << windingNumberList[i].first.first << "/"
               << windingNumberList[i].first.second << "  ("
               << ((double) windingNumberList[i].first.first /
                   (double) windingNumberList[i].first.second) << ")  "
               << "consistency "
               << 100.0 * windingNumberList[i].second
               << "%" << endl;
        
        break;
      }
      else
      {
        // Debug info
        if( verboseFlag )
          cerr << "  overlaps toroidal/poloidal Winding  "
               << windingNumberList[i].first.first << "/"
               << windingNumberList[i].first.second << "  ("
               << ((double) windingNumberList[i].first.first /
                   (double) windingNumberList[i].first.second) << ")  "
               << "consistency " << 100.0 * windingNumberList[i].second
               << "%"
               << endl;

        if( windingNumberList[i].second < windingPairConfidence )
          break;
      }
    }

    // Match consistency is less than the user set value. Run more
    // expensive tests to identify the fieldline.
    if( windingNumberMatchIndex == -1 )
    {
      if( verboseFlag )
        cerr << "Poor consistency - probably chaotic" << endl;


      FieldlineInfo fi;

      fi.type = FieldlineInfo::CHAOTIC;

      fi.toroidalWinding = 0;
      fi.poloidalWinding = 0;
      fi.windingGroupOffset = 0;
      fi.islands = 0;
      fi.nnodes  = 0;

      fi.confidence        = 0;
      fi.analysisStatus    = 0;

      fi.nPuncturesNeeded  = 0;
      fi.toroidalPeriod    = 0;
      fi.poloidalPeriod    = 0;
      fi.ridgelineVariance = 0;

      return fi;
    }
  }

  // Report the winding number pairs.
  vector< pair < pair<unsigned int, unsigned int >, double > >::iterator
    iter = windingNumberList.begin();
  
  unsigned int i = 0;

  // Iterator past the windingNumberMatchIndex as those have already been
  // reported.
  while( i < windingNumberMatchIndex+1 && i < windingNumberList.size() )
  {
    ++i; ++iter;
  }
    
  // Debug info
  if( verboseFlag )
  {
    double cutoff = windingNumberList[windingNumberMatchIndex].second;
      
    if( windingNumberList[windingNumberMatchIndex].second >= 0.9 )
      cutoff -= 0.05;
    else if( windingNumberList[windingNumberMatchIndex].second >= 0.8 )
      cutoff -= 0.025;
    
    while( i < windingNumberList.size() &&
           windingNumberList[i].second >= cutoff )
    {
      cerr << (IntersectCheck( poloidal_puncture_pts,
                               windingNumberList[i].first.first) ?
               "  possible toroidal/poloidal Winding  " :
               "  overlaps toroidal/poloidal Winding  ")
           << windingNumberList[i].first.first << "/"
           << windingNumberList[i].first.second << "  ("
             << ((double) windingNumberList[i].first.first /
                 (double) windingNumberList[i].first.second) << ")  "
               << "consistency " << 100.0 * windingNumberList[i].second
             << "%"
             << endl;

      ++i; ++iter;
    }

    if( windingNumberList[i].second >= windingPairConfidence )
      cerr << "    found more ...  " << endl;
  }

    
  // Find the last winding number set that is above the limit.
  while( i < windingNumberList.size() &&
         windingNumberList[i].second >= windingPairConfidence )
  {
    cerr << (IntersectCheck( poloidal_puncture_pts,
                             windingNumberList[i].first.first) ?
             "  possible toroidal/poloidal Winding  " :
             "  overlaps toroidal/poloidal Winding  ")
         << windingNumberList[i].first.first << "/"
         << windingNumberList[i].first.second << "  ("
         << ((double) windingNumberList[i].first.first /
             (double) windingNumberList[i].first.second) << ")  "
         << "consistency " << 100.0 * windingNumberList[i].second
         << "%"
         << endl;

    ++i; ++iter;
  }
  
  // Remove the winding number sets that are after the limit.
  if( i < windingNumberList.size() )
    windingNumberList.erase( iter, windingNumberList.end() );


  vector< pair< unsigned int, double > > poloidalStats;
  vector< pair< unsigned int, double > > toroidalStats;

  double consistency;

  // Find the best toroidal periodicity. For a flux surface the period
  // will be the toroidal winding number. For an island chain the
  // period will be the toroidal winding number times the number of
  // nodes.

  if( verboseFlag )
    cerr << "Toroidal Winding via poloidal punctures " << endl;

  toroidalPeriod = periodicityChecks( poloidal_puncture_pts,
                                      toroidalStats,
                                      consistency,
                                      toroidalWinding == 1 &&
                                      poloidalWinding == 1 );
  
  // Make sure there are enough points to do the consistency analysis.
  if( (float) poloidal_puncture_pts.size() / (float) toroidalPeriod < 1.75 )
  {
    if( nPuncturesNeeded < 1.75 * toroidalPeriod + 1 )
      nPuncturesNeeded = 1.75 * toroidalPeriod + 1;

    if( verboseFlag )
      cerr << "Not enough poloidal punctures; "
           << "need " << 2*toroidalPeriod << " "
           << "have " << poloidal_puncture_pts.size() << " "
           << "asking for " << nPuncturesNeeded << " puncture points"
           << endl;
  }

  else if( consistency < periodicityConsistency )
  {
    if( nPuncturesNeeded < 1.25 * poloidal_puncture_pts.size() )
      nPuncturesNeeded = 1.25 * poloidal_puncture_pts.size();

    if( verboseFlag )
      cerr << "Inconsistent poloidal period "
           << toroidalPeriod << "  " << consistency
           << " asking for " << nPuncturesNeeded << " puncture points"
           << endl;
  }
  
  // Find the best poloidal periodicity. For a flux surface the period
  // will be the poloidal winding number. For an island chain the
  // period will be the poloidal winding number times the number of
  // nodes.

  if( verboseFlag )
    cerr << "Poloidal Winding via Ridgeline points" << endl;

  poloidalPeriod = periodicityChecks( ridgeline_points,
                                      poloidalStats,
                                      consistency,
                                      toroidalWinding == 1 &&
                                      poloidalWinding == 1 );
  
  // Make sure there are enough points to do the additional analysis.
  // i.e. if the number of points is approximately twice the period
  // then it is difficult to check for consistency.
  if( (float) ridgeline_points.size() / (float) poloidalPeriod < 2.0 )
  {
    if( nPuncturesNeeded < 2.0 * (poloidalPeriod+1) * safetyFactor + 1)
      nPuncturesNeeded = 2.0 * (poloidalPeriod+1) * safetyFactor + 1;

    if( verboseFlag )
      cerr << "Not enough ridgeline points; "
           << "need " << 2*poloidalPeriod << " "
           << "have " << ridgeline_points.size() << " "
           << "asking for " << nPuncturesNeeded << " puncture points"
           << endl;
  }
  else if( consistency < periodicityConsistency )
  {
    if( nPuncturesNeeded < 1.25 * poloidal_puncture_pts.size() )
      nPuncturesNeeded = 1.25 * poloidal_puncture_pts.size();

    if( verboseFlag )
      cerr << "Inconsistent ridgeline period "
           << poloidalPeriod << "  " << consistency
           << " asking for " << nPuncturesNeeded << " puncture points"
           << endl;
  }
  
  if( verboseFlag && nPuncturesNeeded)
    cerr << "Too few puncture points, at least "
         << nPuncturesNeeded
         << " are needed for the analysis."
         << endl;

  // First check the obvious 

  // Have a rational or flux surface
  if( toroidalPeriod == toroidalWinding &&
      poloidalPeriod == poloidalWinding )
  {

    if( verboseFlag )
      cerr << "Exact flux surface match between winding and periodic checks  "
           << toroidalPeriod << "  " << poloidalPeriod << "  "
           << endl;

    // Check to see if the fieldline is periodic. I.e. on a rational
    // surface.  If within "delta" of the distance the fieldline is
    // probably on a rational surface.
    if( rationalCheck( poloidal_puncture_pts, toroidalWinding,
                       nnodes, delta/2.0 ) ) 
    {
      type = FieldlineInfo::RATIONAL;
      complete = true;
      
      islands = 0;
      if( verboseFlag )
        cerr << "Appears to be a rational surface " << delta/2.0 << endl;
    }
    else
      type = FieldlineInfo::FLUX_SURFACE;
    
    confidence += 0.50;
  }

  // Have an island chain
  else if( toroidalPeriod / toroidalWinding ==
           poloidalPeriod / poloidalWinding &&

           toroidalPeriod % toroidalWinding == 0 &&
           poloidalPeriod % poloidalWinding == 0 )
  {
      if( verboseFlag )
        cerr << "Exact island match between winding and periodic checks  "
             << toroidalPeriod << "  " << poloidalPeriod << "  "
             << endl;

      type = FieldlineInfo::ISLAND_CHAIN;
      confidence += 0.50;

      islands = toroidalWinding;

      nnodes = toroidalPeriod / toroidalWinding;
  }

  else
  {
    // Check for a match with a higher order rational or flux surface.
    for( unsigned int i=0; i<windingNumberList.size(); ++i )
    {
      if( toroidalPeriod == windingNumberList[i].first.first &&
          poloidalPeriod == windingNumberList[i].first.second )
      {
        if( verboseFlag )
          cerr << "Exact flux surface match between winding and periodic checks "
               << toroidalPeriod << "  " << poloidalPeriod << "  "
               << endl;

        // Check to see if the fieldline is periodic. I.e. on a rational
        // surface.  If within "delta" of the distance the fieldline is
        // probably on a rational surface.
        if( rationalCheck( poloidal_puncture_pts, toroidalWinding,
                           nnodes, delta/2.0 ) ) 
        {
          type = FieldlineInfo::RATIONAL;
          complete = true;
      
          islands = 0;
          if( verboseFlag )
            cerr << "Appears to be a rational surface " << delta/2.0 << endl;
        }
        else
          type = FieldlineInfo::FLUX_SURFACE;
        
        confidence += 0.40;

        if( i == 0 )
          confidence += 0.10;

        break;
      }
    }
  }

  if( type == FieldlineInfo::UNKNOWN )
  {
    double distance = 1.0e9;
    int windingIndex = -1, t, p, tr, pr;

    int toroidalTest, toroidalIndex, toroidalRatio;
    int poloidalTest, poloidalIndex, poloidalRatio;

    // Check for a match with a higher order rational or flux surface.
    for( unsigned int i=0; i<windingNumberList.size(); ++i )
    {
      toroidalIndex = -1;

//      cerr << "i  " << i << "  " << windingNumberList[i].first.first << endl;

      for( unsigned int j=0; j<toroidalStats.size(); ++j )
      {
//      cerr << "j  " << j << "  " << toroidalStats[j].first << endl;
  
        if( toroidalStats[j].first % windingNumberList[i].first.first == 0 )
        {
          toroidalTest = toroidalStats[j].first;
          toroidalIndex = j;

          toroidalRatio =
            toroidalStats[j].first / windingNumberList[i].first.first;

          poloidalIndex = -1;
      
          // Look for a poloidal winding with the same ratio
          for( unsigned int k=0; k<poloidalStats.size(); ++k )
          {
//          cerr << "k r  " << k << "  " << poloidalStats[k].first << endl;

            poloidalRatio =
              poloidalStats[k].first / windingNumberList[i].first.second;
              
            if( poloidalStats[k].first %
                windingNumberList[i].first.second == 0 &&

                poloidalRatio == toroidalRatio )
            {
              poloidalTest = poloidalStats[k].first;
              poloidalIndex = k;
              
              break;
            }
          }
      
          if( toroidalIndex > -1 && poloidalIndex > - 1 )
          {
            if( distance > (toroidalStats[toroidalIndex].second *
                            toroidalStats[toroidalIndex].second +
                            poloidalStats[poloidalIndex].second *
                            poloidalStats[poloidalIndex].second ) )
            {
              distance = (toroidalStats[toroidalIndex].second *
                          toroidalStats[toroidalIndex].second +
                          poloidalStats[poloidalIndex].second *
                          poloidalStats[poloidalIndex].second );
          
              windingIndex = i;

              t = toroidalIndex;
              p = poloidalIndex;

              tr = toroidalRatio;
              pr = poloidalRatio;
            }
          }
        }
      }
    }

    if( windingIndex > -1)
    {
      toroidalIndex = t;
      poloidalIndex = p;

      toroidalRatio = tr;
      poloidalRatio = pr;

      toroidalPeriod = toroidalStats[toroidalIndex].first;
      poloidalPeriod = poloidalStats[poloidalIndex].first;

      // Check to see if the fieldline is periodic. I.e. on a rational
      // surface.  If within "delta" of the distance the fieldline is
      // probably on a rational surface.
      if( rationalCheck( poloidal_puncture_pts, toroidalWinding,
                         nnodes, delta/2.0 ) ) 
      {
        type = FieldlineInfo::RATIONAL;
        complete = true;

        islands = 0;
        if( verboseFlag )
          cerr << "Appears to be a rational surface " << delta/2.0 << endl;
      }
      else if( (float) toroidalPeriod / (float) poloidalPeriod ==
               (float) toroidalWinding/ (float) poloidalWinding &&
               toroidalRatio > 3 )
      {
        islands = toroidalWinding;
        nnodes = toroidalRatio;

        type = FieldlineInfo::ISLAND_CHAIN;
      }
      else 
      {
        islands = 0;
        nnodes = 0;

        type = FieldlineInfo::FLUX_SURFACE;
      }

      confidence += 0.30;

      if( windingIndex <= windingNumberMatchIndex )
        confidence += 0.10;

      if( t == 0 && p == 0 )
        confidence += 0.10;

      if( verboseFlag )
        cerr << "Secondary ratio match between winding and periodic checks "
             << toroidalStats[toroidalIndex].first << " (" << toroidalRatio << ")  "
             << poloidalStats[poloidalIndex].first << " (" << poloidalRatio << ")  "
             << toroidalIndex << "  " << poloidalIndex << "  "
             << endl;
    }

    else // if( windingIndex == -1 )
    {
      // Check for a match with a higher order rational or flux surface.
      for( unsigned int i=0; i<windingNumberList.size(); ++i )
      {
        toroidalIndex = -1;

//      cerr << "i  " << i << "  " << windingNumberList[i].first.first << endl;

        for( unsigned int j=0; j<toroidalStats.size(); ++j )
        {
//        cerr << "j  " << j << "  " << toroidalStats[j].first << endl;
          
          if( toroidalStats[j].first % windingNumberList[i].first.first == 0 )
          {
            toroidalTest = toroidalStats[j].first;
            toroidalIndex = j;

            toroidalRatio =
              toroidalStats[j].first / windingNumberList[i].first.first;

            poloidalIndex = -1;

            // Look for a poloidal winding
            for( unsigned int k=0; k<poloidalStats.size(); ++k )
            {
//            cerr << "k r  " << k << "  " << poloidalStats[k].first << endl;
              
              if( poloidalStats[k].first %
                  windingNumberList[i].first.second == 0 )
              {
                poloidalTest = poloidalStats[k].first;
                poloidalIndex = k;
                
                poloidalRatio =
                  poloidalStats[k].first / windingNumberList[i].first.second;
                
                break;
              }
            }

            if( toroidalIndex > -1 && poloidalIndex > - 1 )
            {
              if( distance > (toroidalStats[toroidalIndex].second *
                              toroidalStats[toroidalIndex].second +
                              poloidalStats[poloidalIndex].second *
                              poloidalStats[poloidalIndex].second ) )
              {
                  distance = (toroidalStats[toroidalIndex].second *
                              toroidalStats[toroidalIndex].second +
                              poloidalStats[poloidalIndex].second *
                              poloidalStats[poloidalIndex].second );
                  
                  windingIndex = i;
                  
                  t = toroidalIndex;
                  p = poloidalIndex;
                  
                  tr = toroidalRatio;
                  pr = poloidalRatio;
              }
            }
          }
        }
      }

      if( windingIndex > -1)
      {
        // Check to see if the fieldline is periodic. I.e. on a rational
        // surface.  If within "delta" of the distance the fieldline is
        // probably on a rational surface.
        if( rationalCheck( poloidal_puncture_pts, toroidalWinding,
                           nnodes, delta/2.0 ) ) 
        {
          type = FieldlineInfo::RATIONAL;
          complete = true;

          islands = 0;
          if( verboseFlag )
            cerr << "Appears to be a rational surface " << delta/2.0 << endl;
        }
        else
          type = FieldlineInfo::FLUX_SURFACE;

        confidence += 0.30;

        toroidalPeriod = toroidalStats[t].first / tr;
        poloidalPeriod = poloidalStats[p].first / pr;

        if( windingIndex <= windingNumberMatchIndex )
          confidence += 0.10;

        if( t == 0 && p == 0 )
          confidence += 0.10;

        if( verboseFlag )
          cerr << "Secondary match between winding and periodic checks "
               << toroidalStats[t].first << " (" << tr << ")  "
               << poloidalStats[p].first << " (" << pr << ")  "
               << t << "  " << p << "  "
               << endl;
      }
    }
  }

  windingGroupOffset = Blankinship( toroidalWinding, poloidalWinding );


  // At this point assume the surface is irrational.
  if( type == FieldlineInfo::FLUX_SURFACE )
  {
    // Get the direction based on the first two points in a group.
    Vector v0 = (Vector) poloidal_puncture_pts[toroidalWinding] -
      (Vector) poloidal_puncture_pts[0];

    // Get the direction based on the first points from adjacent groups.
    Vector v1 = (Vector) poloidal_puncture_pts[windingGroupOffset] -
      (Vector) poloidal_puncture_pts[0];
      
    // If the windingGroupOffset and point ordering are opposite in
    // directions then the previous group is the
    // windingGroupOffset. Otherwise is they have the same direction
    // then toroidalWinding-windingGroupOffset is the previous group.
    int offsetDir = (Dot( v0, v1 ) > 0.0 ? 1 : -1);
      
    vector< unsigned int > nodes(toroidalWinding);

    nnodes = 0;

    // Find the first point from another group that overlaps the first
    // group. This only works if there is an overlap between groups.
    for( unsigned int i=0; i<toroidalWinding; ++i ) 
    {
      nodes[i] = poloidal_puncture_pts.size() / toroidalWinding;

      // The next group
      unsigned int j =
        (i+offsetDir*windingGroupOffset + toroidalWinding) % toroidalWinding;
        
      Vector firstPoint = (Vector) poloidal_puncture_pts[j];
      Vector nextPoint  = (Vector) poloidal_puncture_pts[j+toroidalWinding];
        
      Vector v0 = nextPoint - firstPoint;

      for( unsigned int k=i; k<poloidal_puncture_pts.size(); k+=toroidalWinding ) 
      {
        Vector  testPoint = (Vector) poloidal_puncture_pts[k];
        
        Vector v1 = testPoint - firstPoint;
        
        if( (poloidalWinding > 2 || v1.length() / v0.length() < 5.0)
            && Dot( v0, v1 ) > 0.0 )
        {
          complete = true;

          nodes[i] = k / toroidalWinding;

          break;
        }
        else
          complete = false;
      }

      if( nodes[i] < 1 )
        nodes[i] = poloidal_puncture_pts.size() / toroidalWinding;

      nnodes += nodes[i];
    }

    // Get the average number of nodes.
    nnodes /= (float) toroidalWinding;

    unsigned int cc = 0;
    unsigned int index = 0;
    unsigned int new_nnodes = 0;
      
    for( unsigned int i=0; i<toroidalWinding; i++ )
    {
      if( nodes[i] < nnodes - 1 || nnodes + 1 < nodes[i] )
      {
        cc++;
        index = i;
      }
      else
        new_nnodes += nodes[i];
    }
    
    if( cc == 1 )
    {
      nnodes = new_nnodes / (float) (toroidalWinding-cc);
    }
      
    for( unsigned int i=0; i<toroidalWinding; i++ )
    {
      if( nodes[i] < nnodes - 1 || nnodes + 1 < nodes[i] )
      {
        if( verboseFlag )
          cerr << "Appears to be a flux surface but not self consistent, "
               << "average number of nodes  " << nnodes << "   (  ";
          
        for( unsigned int i=0; i<toroidalWinding; i++ )
          if( verboseFlag )
            cerr << nodes[i] << "  ";

        cerr << ")" << endl;

        break;
      }
    }  

    if( !complete )
    {
      unsigned int additionalPts = 0;

      // Get the direction based on the first two points in a group.
      Vector v0 = (Vector) poloidal_puncture_pts[toroidalWinding] -
        (Vector) poloidal_puncture_pts[0];

      // Get the direction based on the first points from adjacent groups.
      Vector v1 = (Vector) poloidal_puncture_pts[windingGroupOffset] -
        (Vector) poloidal_puncture_pts[0];

      // If the windingGroupOffset and point ordering are opposite in directions
      // then the previous group is the windingGroupOffset. Otherwise is they have
      // the same direction then toroidalWinding-windingGroupOffset is the previous
      // group.
      int offsetDir = (Dot( v0, v1 ) > 0.0 ? 1 : -1);

      for( unsigned int i=0; i<toroidalWinding; ++i ) 
      {
        // The next group
        unsigned int j =
          (i+offsetDir*windingGroupOffset + toroidalWinding) % toroidalWinding;

        Vector firstPoint = (Vector) poloidal_puncture_pts[j];
        Vector nextPoint  = (Vector) poloidal_puncture_pts[j+toroidalWinding];
          
        Vector  lastPoint =
          (Vector) poloidal_puncture_pts[i+(nnodes-1)*toroidalWinding];
        Vector  prevPoint =
          (Vector) poloidal_puncture_pts[i+(nnodes-2)*toroidalWinding];
          
        Vector v0 = nextPoint - firstPoint;
        Vector v1 = lastPoint - firstPoint;

        unsigned int needPts = ( (firstPoint-lastPoint).length() /
                                 (prevPoint-lastPoint).length() + 0.5 );
          
        if( additionalPts < needPts )
          additionalPts = needPts;
      }

      complete = additionalPts ? false : true;

      if( additionalPts )
      {
        if( nPuncturesNeeded == 0 )
          nPuncturesNeeded = (nnodes+additionalPts) * toroidalWinding + 1;
        
        if( verboseFlag )
          cerr << "Too few puncture points, at least "
               << (nnodes+additionalPts) * toroidalWinding
               << " are needed to complete the boundary."
               << endl;
      }
    }
  }


  FieldlineInfo fi;

  fi.type = type;

  fi.toroidalWinding = toroidalWinding;
  fi.poloidalWinding = poloidalWinding;
  fi.windingGroupOffset = windingGroupOffset;
  fi.islands = islands;
  fi.nnodes  = nnodes;

  fi.confidence        = confidence;
  fi.analysisStatus    = (complete ?
                          FieldlineInfo::COMPLETED :
                          FieldlineInfo::IN_PROCESS);

  fi.nPuncturesNeeded  = nPuncturesNeeded;
  fi.toroidalPeriod   = toroidalPeriod;
  fi.poloidalPeriod   = poloidalPeriod;
  fi.ridgelineVariance = ridgelineVariance;

  if( complete && islands && !islandCenters.empty() )
  {
    fi.analysisStatus |= FieldlineInfo::ADD_O_POINTS;

    fi.OPoints = islandCenters;
  }

  return fi;
}


unsigned int
FieldlineLib::
islandProperties( vector< Point > &points,
                  Vector &baseCentroid,
                  unsigned int &startIndex,
                  unsigned int &middleIndex,
                  unsigned int &stopIndex,
                  unsigned int &nodes )
{
  // Get the local centroid for the group.
  Vector localCentroid(0,0,0);

  for( unsigned int i=0; i<points.size(); i++ )
    localCentroid += (Vector) points[i];

  localCentroid /= (float) points.size();

  // Get the principal axes of the points.
  float Ixx = 0.0;
  float Ixz = 0.0;
  float Izz = 0.0;

  double maxDist = 0;

  // Get the moments of intertial for each point. It assumed that
  // everything is in the Y plane as such there the moments of
  // intertial along the Y axis are zero.
  for( unsigned int i=0; i<points.size(); i++ ) {

    Vector vec = (Vector) points[i] - localCentroid;
    
    if( maxDist < vec.length() )
      maxDist = vec.length();
    
    Ixx += vec.z*vec.z;
    Ixz -= vec.x*vec.z;
    Izz += vec.x*vec.x;
  }
  
  // Short cut to the principal axes because the Y moments of
  // intertial are zero.
  float alpha = atan( 2.0 * Ixz / (Ixx - Izz) ) / 2.0;

  //       cerr << "PRINCIPAL AXES " << alpha * 180.0 / M_PI << "    "
  //       << Ixx + Ixz * sin(alpha       )/cos(alpha       ) << "    "
  //       << Izz + Ixz * cos(alpha+M_PI/2)/sin(alpha+M_PI/2) << endl;

  // Use the principal axes to get an offset from the local
  // centroid which gives a point outside the island.
  baseCentroid = localCentroid;
  
  if( Ixx + Ixz * sin(alpha       )/cos(alpha       ) >
      Izz + Ixz * cos(alpha+M_PI/2)/sin(alpha+M_PI/2) )
    baseCentroid -= Vector(  cos(alpha), 0, sin(alpha) ) * maxDist;
  else
    baseCentroid -= Vector( -sin(alpha), 0, cos(alpha) ) * maxDist;

  // Determine if islands exists. If an island exists there will be
  // both clockwise and counterclockwise sections when compared to the
  // global centroid.
  unsigned int turns = 0;

  startIndex = middleIndex = stopIndex = 0;

  Vector v0 = (Vector) points[0] - baseCentroid;
  Vector v1 = (Vector) points[1] - baseCentroid;

  bool lastCCW = (ccw(v0, v1) == 1);
  v0 = v1;

  for( unsigned int j=2; j<points.size(); j++ ) {

    v1 = (Vector) points[j] - baseCentroid;

    bool CCW = (ccw(v0, v1) == 1);
    v0 = v1;

    if( CCW != lastCCW ) {
      turns++;

      if( turns == 1 )      startIndex  = j - 1;
      else if( turns == 2 ) middleIndex = j - 1;
      else if( turns == 3 ) stopIndex   = j - 1;

      if( turns == 3 )
        break;

      lastCCW = CCW;
    }
  }

  if( turns == 0 ) {
    stopIndex   = points.size() - 1;
    middleIndex = 0;
    startIndex  = 0;

    nodes = points.size();
  } else if( turns == 1 ) {
    stopIndex   = points.size() - 1;
    middleIndex = startIndex;
    startIndex  = 0;

    nodes = points.size();
  } else {

    if( turns == 3 ) {

      // Check for a negative epsilon.
      double length0 = 0;
      double length1 = 0;

      for( unsigned j=0, k=nodes, l=nodes+1; l<points.size(); j++, k++, l++ ) {
        length0 += ((Vector) points[j] - (Vector) points[k]).length();
        length1 += ((Vector) points[j] - (Vector) points[l]).length();
      }

      if( length0 < length1 )
        stopIndex--;

      if( 2*startIndex == middleIndex + 1 ) {
        // First point is actually the start point.
//      cerr << "First point is actually the start point.\n";

        stopIndex   = middleIndex;
        middleIndex = startIndex;
        startIndex  = 0;
      }
    } else if( turns == 2 ) {

      if( 2*startIndex == middleIndex + 1 ) {
        // First point is actually the start point.
//      cerr << "First point is actually the start point.\n";

        stopIndex   = middleIndex;
        middleIndex = startIndex;
        startIndex  = 0;
      
        turns = 3;

      } else if( points.size() < 2 * (middleIndex - startIndex) - 1 ) {
        // No possible over lap.
//      cerr <<  "islandProperties - No possible over lap.\n";

        stopIndex = startIndex + points.size() - 1;
      }
    }

    nodes = stopIndex - startIndex + 1;

    for( unsigned int j=middleIndex, k=middleIndex+1;
         j<points.size() && k<points.size();
         ++j, ++k ) {

      // See if the test point is between the first section.
      if( Dot( (Vector) points[0] - (Vector) points[j],
               (Vector) points[1] - (Vector) points[j] )
          < 0.0 ) {
        stopIndex = startIndex + j;
        nodes = j;
        turns = 3;
//      cerr << "islandProperties - A point overlaps the first section at " << j-1 << endl;
        break;
      }

      // See if the first point is between the test section.
      if( Dot( (Vector) points[j] - (Vector) points[0],
               (Vector) points[k] - (Vector) points[0] )
          < 0.0 ) {
        stopIndex = startIndex + j;
        nodes = j;
        turns = 3;
//      cerr <<  "islandProperties - First point overlaps another section after " << j-1 << endl;
        break;
      }
    }
      
    // No overlap found
    if( turns == 2 ) {
      stopIndex = startIndex + points.size() - 1;
      nodes = points.size();
//      cerr << "islandProperties - No overlap found\n";
    }
  }

  return turns;
}


unsigned int
FieldlineLib::
surfaceOverlapCheck( vector< vector< Point > > &bins,
                     unsigned int toroidalWinding,
                     unsigned int offset,
                     unsigned int &nnodes )
{
  nnodes = bins[0].size();

  // First make sure none of the groups overlap themselves.
  for( unsigned int i=0; i<toroidalWinding; i++ ) {

    if( nnodes > bins[i].size() )
      nnodes = bins[i].size();

    for( unsigned int j=2; j<nnodes; j++ ) {

      Vector v0 = (Vector) bins[i][0] - (Vector) bins[i][j];
      Vector v1 = (Vector) bins[i][1] - (Vector) bins[i][j];
      
      if( Dot( v0, v1 ) < 0.0 ) {
        nnodes = j;
        break;
      }

      Vector midPt = ((Vector) bins[i][j] + (Vector) bins[i][j-1]) / 2.0;

      v0 = (Vector) bins[i][0] - midPt;
      v1 = (Vector) bins[i][1] - midPt;
      
      if( Dot( v0, v1 ) < 0.0 ) {
        nnodes = j;
        break;
      }

    }
  }

  if( toroidalWinding == 1 || nnodes <= 2 )
    return nnodes;

  Vector v0 = (Vector) bins[0   ][1] - (Vector) bins[0][0];
  Vector v1 = (Vector) bins[offset][0] - (Vector) bins[0][0];

  // If the offset and point ordering are opposite in directions then the
  // previous group is the offset. Otherwise is they have the same
  // direction then toroidalWinding-offset is the previous group.
  int offsetDir;

  if( Dot( v0, v1 ) < 0.0 )
    offsetDir = 1;
  else
    offsetDir = -1;

  // Second make sure none of the groups overlap each other.
  for( unsigned int i=0; i<toroidalWinding; i++ ) {

    // The previous group
    unsigned int j = (i + offsetDir*offset + toroidalWinding) % toroidalWinding;

    // Check for a point in the previous group being between the first
    // two points in the current group.
    for( unsigned int k=0; k<nnodes; k++ ) {

      Vector v0 = (Vector) bins[i][0] - (Vector) bins[j][k];
      Vector v1 = (Vector) bins[i][1] - (Vector) bins[j][k];
      
      if( Dot( v0, v1 ) < 0.0 ) {
        nnodes = k;
//        cerr << "adjacent overlap1 " << i << "  " << j << "  " << k << endl;
      }
    }

    // Check for a point in the current group being between two points
    // in the previous group.
    for( unsigned int k=1; k<nnodes; k++ ) {

      Vector v0 = (Vector) bins[j][k  ] - (Vector) bins[i][0];
      Vector v1 = (Vector) bins[j][k-1] - (Vector) bins[i][0];
      
      if( Dot( v0, v1 ) < 0.0 ) {
        nnodes = k;
//        cerr << "adjacent overlap2 " << i << "  " << j << "  " << k << endl;
        break;
      }
    }
  }

  return nnodes;
}


unsigned int
FieldlineLib::
surfaceGroupCheck( vector< vector< Point > > &bins,
                   unsigned int i,
                   unsigned int j,
                   unsigned int nnodes ) {

  unsigned int nodes = nnodes;

  while( nodes < bins[i].size() ) {
    // Check to see if the first overlapping point is really a
    // fill-in point. This happens because the spacing between
    // toroidalWinding groups varries between groups.
    Vector v0 = (Vector) bins[j][0      ] - (Vector) bins[i][nodes];
    Vector v1 = (Vector) bins[i][nodes-1] - (Vector) bins[i][nodes];
    
    if( Dot( v0, v1 ) < 0.0 )
      nodes++;
    else
      break;
  }

  return nodes;
}


unsigned int
FieldlineLib::
removeOverlap( vector< vector < Point > > &bins,
               unsigned int &nnodes,
               unsigned int toroidalWinding,
               unsigned int poloidalWinding,
               unsigned int offset,
               unsigned int island )
{
  Vector globalCentroid = Vector(0,0,0);;

  for( unsigned int i=0; i<toroidalWinding; i++ )
    for( unsigned int j=0; j<nnodes; j++ )
      globalCentroid += (Vector) bins[i][j];
  
  globalCentroid /= (toroidalWinding*nnodes);
    
  if( island && nnodes == 1 ) {

  } else if( island ) {

    for( unsigned int i=0; i<toroidalWinding; i++ ) {
      unsigned int nodes = 0;
      bool completeIsland = false;

      // If just a single island search for an overlap immediately.
      if( toroidalWinding == 1 )
      {
        unsigned int i = 0;

        // See if the first point overlaps another section.
        for( unsigned int  j=nnodes/2; j<bins[i].size(); j++ ) {
          if( Dot( (Vector) bins[i][j  ] - (Vector) bins[i][0],
                   (Vector) bins[i][j-1] - (Vector) bins[i][0] )
              < 0.0 ) {

            nodes = j;
        
            completeIsland = true;
            break;
          }
        }

        // See if a point overlaps the first section.
        if( nodes == 0 ) {
          for( unsigned int j=nnodes/2; j<bins[i].size(); j++ ) {
            if( Dot( (Vector) bins[i][0] - (Vector) bins[i][j],
                     (Vector) bins[i][1] - (Vector) bins[i][j] )
                < 0.0 ) {

              nodes = j;

              completeIsland = true;
              break;
            }
          }
        }
      }

      // Toroidal winding is greater than 1 as such get detailed data
      // for the removal of points so that each island has the same of
      // resulting nodes.
      else
      {
        unsigned int startIndex;
        unsigned int middleIndex;
        unsigned int stopIndex;
        
        vector< Point > points;
        points.resize( bins[i].size() );

        for( unsigned int j=0; j<bins[i].size(); j++ )
          points[j] = bins[i][j];
        
        if( islandProperties( points, globalCentroid,
                              startIndex, middleIndex, stopIndex, nodes ) == 3 )

          completeIsland = true;
      }

      if( nodes == 0 )
      {
        if( verboseFlag )
          cerr << "removeOverlap - Island properties returned ZERO NODES for island " << i << endl;

        nodes = bins[i].size();
      }

      // No more than one point should be added.
      if( nodes > nnodes+1 )
      {
        if( verboseFlag )
          cerr << "removeOverlap - Island " << i
               << " nnodes mismatch " << nnodes << "  " << nodes << endl;
      }

      // Erase all of the overlapping points.
      bins[i].erase( bins[i].begin()+nnodes, bins[i].end() );
      
      // Close the island if it is complete
      if( completeIsland )
        bins[i].push_back( bins[i][0] );
    }

  } else {  // Surface

    // This gives the minimal number of nodes for each group.

    surfaceOverlapCheck( bins, toroidalWinding, offset, nnodes );
    
    if( nnodes == 0 ) {

      if( verboseFlag )
          cerr << "removeOverlap - Surface properties returned ZERO NODES for island " << endl;

      nnodes = bins[0].size();

      for( unsigned int i=1; i<toroidalWinding; i++ ) {
        if( nnodes > bins[i].size())
          nnodes = bins[i].size();
      }
    }

    for( unsigned int i=0; i<toroidalWinding; i++ ) {

      // Add back in any nodes that may not overlap.
      unsigned int nodes =
        surfaceGroupCheck( bins, i, (i+offset)%toroidalWinding, nnodes );

      // No more than one point should be added.
      if( nodes > nnodes+1 )
        if( verboseFlag )
          cerr << "removeOverlap - Surface " << i
             << " nnodes mismatch " << nnodes << "  " << nodes << endl;

      // Erase all of the overlapping points.
      bins[i].erase( bins[i].begin()+nodes, bins[i].end() );
    }
  }

  return nnodes;
}


unsigned int
FieldlineLib::
smoothCurve( vector< vector < Point > > &bins,
             unsigned int &nnodes,
             unsigned int toroidalWinding,
             unsigned int poloidalWinding,
             unsigned int offset,
             unsigned int island )
{
  Vector globalCentroid = Vector(0,0,0);;

  for( unsigned int i=0; i<toroidalWinding; i++ )
    for( unsigned int j=0; j<nnodes; j++ )
      globalCentroid += (Vector) bins[i][j];
  
  globalCentroid /= (toroidalWinding*nnodes);

  unsigned int add = 2;

  if( island ) {

    for( unsigned int i=0; i<toroidalWinding; i++ ) {
//      for( unsigned int s=0; s<add; s++ )
      {
        vector< pair< Point, unsigned int > > newPts;

        newPts.resize( add*nnodes );

        for( unsigned int j=0; j<add*nnodes; j++ )
          newPts[j] = pair< Point, unsigned int > (Point(0,0,0), 0 );
        
        for( unsigned int j=1; j<nnodes-1; j++ ) {

          unsigned int j_1 = (j-1+nnodes) % nnodes;
          unsigned int j1  = (j+1+nnodes) % nnodes;

          Vector v0 = (Vector) bins[i][j1] - (Vector) bins[i][j  ];
          Vector v1 = (Vector) bins[i][j ] - (Vector) bins[i][j_1];

          if( verboseFlag )
            cerr << i << " smooth " << j_1 << " "  << j << " "  << j1 << "  "
                 << ( v0.length() > v1.length() ?
                      v0.length() / v1.length() :
                      v1.length() / v0.length() ) << endl;

          if( Dot( v0, v1 ) > 0 &&
              ( v0.length() > v1.length() ?
                v0.length() / v1.length() :
                v1.length() / v0.length() ) < 10.0 ) {

            Vector center = (Vector) circle( bins[i][j_1],
                                             bins[i][j  ],
                                             bins[i][j1 ] );

            double rad = ((Vector) bins[i][j] - center).length();


            for( unsigned int s=0; s<add; s++ ) {
              Vector midPt = (Vector) bins[i][j_1] +
                (double) (add-s) / (double) (add+1) *
                ((Vector) bins[i][j] - (Vector) bins[i][j_1] );
                

              Vector midVec = midPt - center;

              midVec.normalize();

              newPts[add*j+s].first += center + midVec * rad;
              newPts[add*j+s].second += 1;

              midPt = (Vector) bins[i][j] +
                (double) (add-s) / (double) (add+1) *
                ((Vector) bins[i][j1] - (Vector) bins[i][j] );

              midVec = midPt - center;

              midVec.normalize();

              newPts[add*j1+s].first += center + midVec * rad;
              newPts[add*j1+s].second += 1;
            }
          }
        }

        for( unsigned int j=nnodes-1; j>0; j-- ) {

          for( unsigned int s=0; s<add; s++ ) {

            unsigned int k = add * j + s;

            if( newPts[k].second > 0 ) {
              
              newPts[k].first /= newPts[k].second;
              
//            cerr << i << " insert " << j << "  " << newPts[k] << endl;
              
              bins[i].insert( bins[i].begin()+j, newPts[k].first );
            }
          }
        }

        for( unsigned int s=0; s<add; s++ ) {

          unsigned int k = add - 1 - s;

          if( newPts[k].second > 0 ) {
              
            newPts[k].first /= newPts[k].second;
              
//            cerr << i << " insert " << 0.0<< "  " << newPts[k] << endl;
              
            bins[i].push_back( newPts[k].first );
          }
        }
      }
    }

  } else {

    for( unsigned int i=0; i<toroidalWinding; i++ ) {

      if( bins[i].size() < 2 )
        continue;

      // Index of the next toroidalWinding group
      unsigned int j = (i+offset)%toroidalWinding;

      // Insert the first point from the next toroidalWinding so the curve
      // is contiguous.
      bins[i].push_back( bins[j][0] );

      //for( unsigned int s=0; s<add; s++ )
      {
        unsigned int nodes = bins[i].size();

        vector<pair< Point, unsigned int > > newPts(add*nodes);

        for( unsigned int j=0; j<add*nodes; j++ )
          newPts[j] = pair< Point, unsigned int > (Point(0,0,0), 0 );
        
        for( unsigned int j=1; j<nodes-1; j++ ) {

          unsigned int j_1 = j - 1;
          unsigned int j1  = j + 1;

          Vector v0 = (Vector) bins[i][j1] - (Vector) bins[i][j  ];
          Vector v1 = (Vector) bins[i][j ] - (Vector) bins[i][j_1];

          //      cerr << i << " smooth " << j_1 << " "  << j << " "  << j1 << "  "
          //           << ( v0.length() > v1.length() ?
          //                v0.length() / v1.length() :
          //                v1.length() / v0.length() ) << endl;

          if( Dot( v0, v1 ) > 0 &&
              ( v0.length() > v1.length() ?
                v0.length() / v1.length() :
                v1.length() / v0.length() ) < 10.0 ) {

            Vector center = (Vector) circle( bins[i][j_1],
                                             bins[i][j  ],
                                             bins[i][j1 ] );

            double rad = ((Vector) bins[i][j] - center).length();


            for( unsigned int s=0; s<add; s++ ) {
              Vector midPt = (Vector) bins[i][j_1] +
                (double) (add-s) / (double) (add+1) *
                ((Vector) bins[i][j] - (Vector) bins[i][j_1] );
                

              Vector midVec = midPt - center;

              midVec.normalize();

              newPts[add*j+s].first += center + midVec * rad;
              newPts[add*j+s].second += 1;

              midPt = (Vector) bins[i][j] +
                (double) (add-s) / (double) (add+1) *
                ((Vector) bins[i][j1] - (Vector) bins[i][j] );

              midVec = midPt - center;

              midVec.normalize();

              newPts[add*j1+s].first += center + midVec * rad;
              newPts[add*j1+s].second += 1;
            }
          }
        }

        for( int j=nodes-1; j>=0; j-- ) {

          for( unsigned int s=0; s<add; s++ ) {

            unsigned int k = add * j + s;

            if( newPts[k].second > 0 ) {
              
              newPts[k].first /= newPts[k].second;
              
              if( verboseFlag )
                cerr << i << " insert " << j << "  " << newPts[k].first << endl;
              
              bins[i].insert( bins[i].begin()+j, newPts[k].first );
            }
          }
        }
      }

      // Remove the last point so it is possilble to see the groups.
      bins[i].erase( bins[i].end() );
    }
  }

  return toroidalWinding*(add+1)*nnodes;
}


unsigned int
FieldlineLib::
mergeOverlap( vector< vector < Point > > &bins,
              unsigned int &nnodes,
              unsigned int toroidalWinding,
              unsigned int poloidalWinding,
              unsigned int offset,
              unsigned int island )
{
  Vector globalCentroid = Vector(0,0,0);;

  for( unsigned int i=0; i<toroidalWinding; i++ )
    for( unsigned int j=0; j<nnodes; j++ )
      globalCentroid += (Vector) bins[i][j];
  
  globalCentroid /= (toroidalWinding*nnodes);
    
  if( island ) {

    vector < vector < Point > > tmp_bins;

    tmp_bins.resize( toroidalWinding );

    for( unsigned int i=0; i<toroidalWinding; i++ ) {
      
      unsigned int startIndex;
      unsigned int middleIndex;
      unsigned int stopIndex;
      unsigned int nodes;
        
      vector< Point > points;
      points.resize( bins[i].size() );

      for( unsigned int j=0; j<bins[i].size(); j++ )
        points[j] = bins[i][j];

      unsigned int turns =
        islandProperties( points, globalCentroid,
                          startIndex, middleIndex, stopIndex, nodes );

      // Merge only if there are overlapping points.
      if( turns == 3) {

        if( nnodes == bins[i].size() )
          continue;

        // Store the overlapping points.
        for( unsigned int j=nnodes; j<bins[i].size(); j++ )
          tmp_bins[i].push_back( bins[i][j] );

        if( verboseFlag )
          cerr << i << " stored extra points " << tmp_bins[i].size() << endl;

        // Erase all of the overlapping points.
        bins[i].erase( bins[i].begin()+nnodes, bins[i].end() );
          
        // Insert the first point so the curve is contiguous.
        bins[i].insert( bins[i].begin()+nnodes, bins[i][0] );

        unsigned int index_prediction = 1;
        unsigned int prediction_true  = 0;
        unsigned int prediction_false = 0;

        unsigned int modulo = bins[i].size() - 1;
 
        // Insert the remaining points.
        for( unsigned int j=0; j<tmp_bins[i].size(); j++ ) {

          Vector v0 = (Vector) bins[i][0] -
            (Vector) tmp_bins[i][j];

          double angle = 0;
          double length = 99999;
          unsigned int angleIndex = 0;
          unsigned int lengthIndex = 0;

          for( unsigned int k=1; k<bins[i].size(); k++ ) {

            Vector v1 = (Vector) bins[i][k] -
              (Vector) tmp_bins[i][j];

            double ang = acos( Dot(v0, v1) / (v0.length() * v1.length()) );

            if( angle < ang ) {
              angle = ang;
              angleIndex = k;
            }

            if( length < v1.length() ) {
              length = v1.length();
              lengthIndex = k;
            }

            // Go on.
            v0 = v1;
          }

          // Insert it between the other two.
          if( angle > M_PI / 3.0 )
            bins[i].insert( bins[i].begin()+angleIndex, tmp_bins[i][j] );

          if( verboseFlag )
            cerr << i << "  " << modulo << "  " << j + nnodes
               << "  Prediction " << index_prediction
               << " actual " << angleIndex << "  "
               << (angleIndex == index_prediction) << endl;

          // Check to see if the prediction and the actual index are
          // the same.
          if( angleIndex == index_prediction )
            prediction_true++;
          else // if( angleIndex != index_prediction )
            prediction_false++;

          // Predict where the next insertion will take place.
          if( (j+1) % modulo == 0 )
            index_prediction = 1 + (j+1) / modulo;
          else
            index_prediction = angleIndex + (j+1) / modulo + 2;
        }

        if( verboseFlag )
          cerr << "ToroidalWinding " << i << " inserted "
               << prediction_true+prediction_false << " nodes "
               << " True " << prediction_true
               << " False " << prediction_false << endl;

        // If more of the predictions are incorrect than correct
        // insert based on the predictions.
        if( 0 && prediction_true < prediction_false ) {

          if( verboseFlag )
            cerr << "ToroidalWinding " << i << " bad predicted insertion ";

          unsigned int cc = 0;

          for( unsigned int j=0; j<tmp_bins[i].size(); j++ ) {

            vector< Point >::iterator inList =
              find( bins[i].begin(), bins[i].end(), tmp_bins[i][j] );
              
            if( inList != bins[i].end() ) {
              bins[i].erase( inList );

              cc++;
            }
          }

          if( verboseFlag )
            cerr << "removed " << cc << " points" << endl;

          unsigned int index = 1;
            
          for( unsigned int j=0; j<tmp_bins[i].size(); j++ ) {
            
            // Insert it between the other two.
            bins[i].insert( bins[i].begin()+index, tmp_bins[i][j] );

            if( verboseFlag )
              cerr << i << "  " << modulo << "  " << j + nnodes
                   << " actual " << index << endl;

            // Predict where the next insertion will take place.
            if( (j+1) % modulo == 0 )
              index = 1 + (j+1) / modulo;
            else
              index += (j+1) / modulo + 2;
          }
        }

        unsigned int start0  = 0;
        unsigned int end0    = 0;
        unsigned int start1  = 0;
        unsigned int end1    = 0;

        if( prediction_true > prediction_false ) {
          // See if any of the segments cross.
          for( unsigned int j=0; 0 && j<bins[i].size()-1; j++ ) {
              
            Point l0_p0 = bins[i][j];
            Point l0_p1 = bins[i][j+1];
              
            for( unsigned int k=j+2; k<bins[i].size()-1; k++ ) {
                
              Point l1_p0 = bins[i][k];
              Point l1_p1 = bins[i][k+1];
                
              if( intersect( l0_p0, l0_p1, l1_p0, l1_p1 ) == 1 ) {
                if( start0 == 0 ) {
                  start0  = j + 1;
                  end1    = k + 1;
                } else {
                  end0   = j + 1;
                  start1 = k + 1;

                  if( verboseFlag )
                    cerr << " merge self intersection " 
                         << start0 << "  " << end0 << "  "
                         << start1 << "  " << end1 << endl;

                  if( 0 ) {
                    vector < Point > tmp_bins[2];

                    for( unsigned int l=start0; l<end0; l++ )
                      tmp_bins[0].push_back( bins[i][l] );

                    for( unsigned int l=start1; l<end1; l++ )
                      tmp_bins[1].push_back( bins[i][l] );

                    bins[i].erase( bins[i].begin()+start1,
                                   bins[i].begin()+end1 );

                    bins[i].erase( bins[i].begin()+start0,
                                   bins[i].begin()+end0 );

                    for( unsigned int l=0; l<tmp_bins[1].size(); l++ )
                      bins[i].insert( bins[i].begin()+start0,
                                      tmp_bins[1][l] );

                    for( unsigned int l=0; l<tmp_bins[0].size(); l++ )
                      bins[i].insert( bins[i].begin() + start1 -
                                      tmp_bins[0].size() +
                                      tmp_bins[1].size(),
                                      tmp_bins[0][l] );
                  }

                  start0 = 0;
                }
              }
            }
          }
        }
      }
    }
  } else {

    vector<vector < Point > > tmp_bins(toroidalWinding);

    // This gives the minimal number of nodes for each group.
    surfaceOverlapCheck( bins, toroidalWinding, offset, nnodes );

    if( nnodes == 0 ) {

      for( unsigned int i=1; i<toroidalWinding; i++ ) {
        if( nnodes > bins[i].size())
          nnodes = bins[i].size();
      }
    }

    for( unsigned int i=0; i<toroidalWinding; i++ ) {

      // Add back in any nodes that may not overlap.
      unsigned int nodes =
        surfaceGroupCheck( bins, i, (i+offset)%toroidalWinding, nnodes );

      // No more than one point should added.
      if( nodes > nnodes+1 )
        if( verboseFlag )
          cerr << "Surface fill " << i
               << " nnodes mismatch " << nodes << endl;

      // Store the overlapping points.
      for( unsigned int j=nodes; j<bins[i].size(); j++ )
        tmp_bins[i].push_back( bins[i][j] );

      if( verboseFlag )
        cerr << i << " stored extra points " << tmp_bins[i].size() << endl;

      // Erase all of the overlapping points.
      bins[i].erase( bins[i].begin()+nodes, bins[i].end() );

      // Insert the first point from the next toroidalWinding so the curve
      // is contiguous.
      bins[i].push_back( bins[(i+offset)%toroidalWinding][0] );
    }


    for( unsigned int i=0; i<toroidalWinding; i++ )
    {
      unsigned int toroidalWinding_prediction = (i+offset)%toroidalWinding;
      unsigned int index_prediction = 1;
      unsigned int prediction_true  = 0;
      unsigned int prediction_false = 0;

      for( unsigned int i0=0; i0<tmp_bins[i].size(); i0++ ) {

        double angle = 0;
        unsigned int index_wd = 0;
        unsigned int index_pt = 0;

        for( unsigned int j=0; j<toroidalWinding; j++ ) {

          Vector v0 = (Vector) bins[j][0] -
            (Vector) tmp_bins[i][i0];

          for( unsigned int j0=1; j0<bins[j].size(); j0++ ) {
            Vector v1 = (Vector) bins[j][j0] -
              (Vector) tmp_bins[i][i0];
        
            double ang = acos( Dot(v0, v1) / (v0.length() * v1.length()) );

            if( angle < ang ) {

              angle = ang;
              index_wd = j;
              index_pt = j0;
            }

            // Go on.
            v0 = v1;
          }
        }

        // Insert it between the other two.
        bins[index_wd].insert( bins[index_wd].begin()+index_pt,
                               tmp_bins[i][i0] );

        if( verboseFlag )
          cerr << "ToroidalWinding prediction " << toroidalWinding_prediction
               << " actual " << index_wd
               << "  Index prediction  " << index_prediction
               << " actual " << index_pt << "  "
               << (index_wd == toroidalWinding_prediction &&
                 index_pt == index_prediction) << endl;

        // Check to see if the prediction of where the point was inserted
        // is correct;
        if( index_wd == toroidalWinding_prediction && index_pt == index_prediction )
          prediction_true++;
        else 
          prediction_false++;

        // Prediction of where the next insertion will take place.
        index_prediction = index_pt + 2;
        toroidalWinding_prediction = index_wd;
      }

      if( verboseFlag )
        cerr << "ToroidalWinding " << i << " inserted "
             << prediction_true+prediction_false << " nodes "
             << " True " << prediction_true
             << " False " << prediction_false << endl;
    }

    // Remove the last point so it is possilble to see the groups.
    for( unsigned int i=0; i<toroidalWinding; i++ )
      bins[i].erase( bins[i].end() );
  }

  // Update the approximate node count.
  nnodes = 9999;

  for( unsigned int i=0; i<toroidalWinding; i++ )
    if( nnodes > bins[i].size() )
      nnodes = bins[i].size();

  return nnodes;
}

// ****************************************************************************
//  Method: FieldlineLib::findIslandCenter
//
//  Purpose: Finds the geometric center of an island.
//
//  Arguments:
//
//  Returns:      void
//
//  Programmer: Allen Sanderson
//  Creation:   Wed Feb 25 09:52:11 EST 2009
//
//  Modifications:
//
// ****************************************************************************

void
FieldlineLib::findIslandCenter( vector < Point > &points,
                                unsigned int nnodes,
                                unsigned int toroidalWinding,
                                unsigned int poloidalWinding,
                                vector< Point > &centers )
{
#ifdef STRAIGHTLINE_SKELETON

  // Loop through each toroidial group
  for( unsigned int i=0; i<toroidalWinding; ++i ) 
  {
    cerr << "Island " << i << "  ";

    // temp storage incase the order needs to be reversed.
    vector< Point > tmp_points;

    bool selfIntersect = false;

    // Loop through each point in toroidial group
    for( unsigned int j=i, jc=0;
         j<points.size() && jc<nnodes;
         j+=toroidalWinding, ++jc )
    {
      tmp_points.push_back( points[j] );
    }

    // Points for the convex hull check
    for( unsigned int j=0; j<tmp_points.size()-1; ++j )
    {
      // Check for self intersections
      for( unsigned int k=j+2; k<tmp_points.size()-1; ++k )
      {
        int result;

        if( result = intersect( tmp_points[j], tmp_points[j+1],
                                tmp_points[k], tmp_points[k+1] ) )
        {
          cerr << " self intersects  " << j << "  " << k << endl;
          
          // Self intersection found so skip this island.
          selfIntersect = true;
          break;
        }
      }

      if( selfIntersect )
        break;
    }
 
    // Self intersection found so skip this island as the skeleton
    // will not work.
    if( selfIntersect )
      continue;

    // Get the convex hull and the direction.
    int direction;
    hullCheck( tmp_points, direction );
      
    // Store the points a 2D vector.
    Skeleton::PointVector pointVec;

    for( unsigned int j=0; j<tmp_points.size()-1; j+=3 )
      pointVec.push_back( Skeleton::Point( tmp_points[j].x,
                                           tmp_points[j].z ) );
      
    // If the points are clockwise reverse them as the skeleton needs
    // the points to be in a counter clockwise direction.
    if( direction == 1 )
      reverse( pointVec.begin(), pointVec.end() );

    cerr << " Skeleton check ";
      
    Skeleton::Skeleton s (Skeleton::makeSkeleton (pointVec));
      
    // Delete all of the hull points.
    list<Skeleton::SkeletonLine>::iterator SL = s.begin();
    list<Skeleton::SkeletonLine>::iterator deleteSL;
      
    // Remove all of the points on the boundary while getting
    // the cord length of the remains interior segments.
      
    double cordLength = 0;
    
    map< int, int > indexCount;
    map<int, int>::iterator ic;

    cerr << " cordLength ";

    while ( SL != s.end() )
    {
      if( (*SL).lower.vertex->ID < pointVec.size() ||
          (*SL).higher.vertex->ID < pointVec.size() ) 
      {
        // Boundary point so delete it.
        deleteSL = SL;
        ++SL;
        s.erase( deleteSL );
      }
      else
      {
        // Running cord length.
        cordLength += sqrt( ((*SL).higher.vertex->point.x-
                             (*SL).lower.vertex->point.x) *
                            ((*SL).higher.vertex->point.x-
                             (*SL).lower.vertex->point.x) +
                            
                            ((*SL).higher.vertex->point.y -
                             (*SL).lower.vertex->point.y) * 
                            ((*SL).higher.vertex->point.y -
                             (*SL).lower.vertex->point.y) );
        
        // Count the number of times each end point is used
        
        // Lower end point
        int index = (*SL).lower.vertex->ID;
        
        ic = indexCount.find( index );
        
        if( ic == indexCount.end() )
          indexCount.insert( pair<int, int>( index,1) );
        else (*ic).second++;
        
        // Higher end point
        index = (*SL).higher.vertex->ID;
        
        ic = indexCount.find( index );
        
        if( ic == indexCount.end() )
          indexCount.insert( pair<int, int>( index,1) );
        else (*ic).second++;
        
        ++SL;
      }
    }                   
        

    int startIndex, endIndex;
    int cc = 0;
    ic = indexCount.begin();
    
    while( ic != indexCount.end() )
    {
      // Find the end points that are used but once. 
      // There should only be two.
      if( (*ic).second == 1 )
      {
        if( cc == 1 )
          startIndex = (*ic).first;
        else if( cc == 0 )
          endIndex = (*ic).first;
        
        ++cc;
      }
      
      if( (*ic).second > 2 )
      {
        cerr << "double segment ??? " << (*ic).first << endl;
        
        SL = s.begin();
        
        while( SL != s.end() &&
               (*SL).lower.vertex->ID != (*ic).first &&
               (*SL).higher.vertex->ID != (*ic).first )
          SL++;
        
        // Have an index so process
        if( SL != s.end() )
        {
          // Remove double segments;
          if( (*SL).lower.vertex->ID == (*ic).first &&
              (*indexCount.find( (*SL).higher.vertex->ID )).second > 2 )
          {
            cerr << "removing double segment ??? " << (*ic).first << "  "
                 << (*indexCount.find( (*SL).higher.vertex->ID )).first
                 << endl;
            
            (*ic).second--;
            (*indexCount.find( (*SL).higher.vertex->ID )).second--;
            
            s.erase( SL );
          }
          
          // Remove double segments;
          if( (*SL).higher.vertex->ID == (*ic).first &&
              
              (*indexCount.find( (*SL).lower.vertex->ID )).second > 2 )
          {
            cerr << "removing double segment ??? " << (*ic).first << "  "
                 << (*indexCount.find( (*SL).lower.vertex->ID )).first
                 << endl;
            
            
            (*ic).second--;
            (*indexCount.find( (*SL).lower.vertex->ID )).second--;
            
                    s.erase( SL );
          }
        }                    
      }
      
      ++ic;
    }
    
    if( cc < 2 )
    {
      cerr << "Not enough start points " << cc << endl;
      continue;
    }
    
    else if( cc > 2 )
    {
      cerr << "Too many start points " << cc << endl;
      continue;
    }

    cerr << "Island " << i << " New skeleton "
         << "start index " << startIndex
         << "  end index " << endIndex
         << endl;
        
    double currentCord = 0;

    int nextIndex = startIndex;

    unsigned int nlines = 0;

    // Loop through all of the lines which are described
    // as set of paired points.
    while ( s.begin() != s.end() )
    {
      // Index of the first point.
      SL = s.begin();

      while( SL != s.end() &&
             (*SL).lower.vertex->ID != nextIndex &&
             (*SL).higher.vertex->ID != nextIndex )
        SL++;
                    
      // Have an index so process
      if( SL != s.end() )
      {
        //          cerr << "Island " << j
        //               << " Line segment " << nlines++
        //               << " index " << nextIndex;

        int lastIndex = nextIndex;

        // Index of the leading point.
        if( (*SL).lower.vertex->ID == nextIndex )
          nextIndex = (*SL).higher.vertex->ID;
        else // if( (*SL).higher.vertex->ID == startIndex )
          nextIndex = (*SL).lower.vertex->ID;
            
        //          cerr << " index " << nextIndex
        //               << endl;

        double localCord = sqrt( ((*SL).higher.vertex->point.x-
                                  (*SL).lower.vertex->point.x) *
                                 ((*SL).higher.vertex->point.x-
                                  (*SL).lower.vertex->point.x) +
                                     
                                 ((*SL).higher.vertex->point.y -
                                  (*SL).lower.vertex->point.y) * 
                                 ((*SL).higher.vertex->point.y -
                                  (*SL).lower.vertex->point.y) );

        // Check to see if the segment spans the mid cord.
        if( currentCord < cordLength/2 &&
            cordLength/2.0 <= currentCord + localCord )
        {
          double t = (cordLength/2.0 - currentCord) / localCord;

          Point center;

          // Already updated the nextIndex so use the lastIndex
          // for figuring out the closest to the cordlength center.
          if( (*SL).lower.vertex->ID == lastIndex )
          {
            center.x = (*SL).lower.vertex->point.x + t *
              ((*SL).higher.vertex->point.x-(*SL).lower.vertex->point.x);
            center.y = 0;
            center.z = (*SL).lower.vertex->point.y + t *
              ((*SL).higher.vertex->point.y-(*SL).lower.vertex->point.y);
          }

          else //if( (*SL).higher.vertex->ID == lastIndex )
          {
            center.x = (*SL).higher.vertex->point.x + t *
              ((*SL).lower.vertex->point.x-(*SL).higher.vertex->point.x);
            center.y = 0;
            center.z = (*SL).higher.vertex->point.y + t *
              ((*SL).lower.vertex->point.y-(*SL).higher.vertex->point.y);
          }
          
          cerr << "O Point " << center << endl;
          centers.push_back( center );
        }
        
        // Update the running cord length so that the mid cord is
        // not calculated again.
        currentCord += localCord;
        
        // Remove this point from the list so that it is
        // not touched again.
        deleteSL = SL;
        ++SL;
        s.erase( deleteSL );
      }
      else
      {
        if( nextIndex != endIndex )
        {
          cerr << "Did not find end index  "
               << nextIndex << "  " <<  endIndex
               << endl;
          
          break;
        }
      }
    }
  }
#endif
}
}


#ifdef COMMENTOUT

    // Check to see if the fieldline creates a set of islands.
    else if( islandChecks( poloidal_puncture_pts, toroidalWinding,
                           islands, nnodes, complete ) ) 
    {
      type = FieldlineInfo::ISLAND_CHAIN;

      // Decide if there are enough poloidal puncture points to
      // complete the boundary.
      if( complete )
      {
        // Number of toroidal pucture points needed for the ridgeline
        // analysis. Add a couple of extra as the nnodes is just an
        // initial guess.
        unsigned int nRidgelinePtsNeeded = 2.5 * poloidalWinding * nnodes;

        if( ridgeline_points.size() >= nRidgelinePtsNeeded )
        {
          // Find the best poloidal periodicity.
          if( verboseFlag )
            cerr << "Polodial " << endl;
          ridgelineGCD = periodicityChecks( nnodes,
                                            poloidal_puncture_pts,
                                            periodicityStats,
                                            false );
          
          // Find the best ridgeline periodicity.
          if( verboseFlag )
            cerr << "Ridgeline " << endl;
          ridgelineGCD = periodicityChecks( nnodes,
                                            ridgeline_points,
                                            periodicityStats,
                                            true );
          
          poloidalPeriod = periodicityStats[0].first;

          ridgelineVariance =
            (ridgeline_dc_var-periodicityStats[0].second)/ridgeline_dc_var;

          cerr << "ridgeline variance comparison  " << ridgelineVariance
               << endl;

          // If the best island poloidal winding value is the poloidal
          // winding value times the number of nodes then there is self
          // consistency. Note allow for +/-cc of the number of nodes
          // as it can be difficult to determine the last node in an
          // island chain.

          // The variance is based on the approximate number of points
          // along the boundary.
          int cc = nnodes / 33 + 1;
      
          for( int i=-cc; i<=cc; ++i )
          {
            // If the best is the same value of the base then a perfect match.
            if( poloidalPeriod == (poloidalWinding * (nnodes+i) ) )
            {
              confidence += 0.30;
              if( i == 0 ) confidence += 0.10;

              nnodes += i;
              
              if( verboseFlag )
              {
                cerr << "Island poloidal via nodes ";
                if(i)  cerr << i << "  ";
                cerr << poloidalPeriod << endl;
              }
              
              break;
            }

            // If the best is an integer value of the base then the result is
            // the same because more groups can create smaller regions.
            else if( poloidalPeriod % (poloidalWinding * (nnodes+i)) == 0 )
            {
              confidence += 0.20;
              if( i == 0 ) confidence += 0.10;
              
              nnodes += i;
              
              if( verboseFlag )
              {
                cerr << "Integer Island poloidal via nodes ";
                if(i)  cerr << i << "  ";
                cerr << poloidalPeriod << endl;
              }
              
              break;
            }
            
            // If the best is an integer value of the base then the result is
            // the same because more groups can create smaller regions.
            else if( poloidalPeriod % (poloidalWinding * (2*nnodes+i)) == 0 )
            {
              confidence += 0.20;
              if( i == 0 ) confidence += 0.10;
              
              nnodes += i;
              
              if( verboseFlag )
              {
                cerr << "Integer Island poloidal via half nodes ";
                if(i<0)  cerr << nnodes << " + " << nnodes-i << " % ";
                if(i>0)  cerr << nnodes-i << " + " << nnodes << " % ";
                cerr << poloidalPeriod << endl;
              }
              
              break;
            }
          }

          // The above assumes the ridgeline fundemental period is
          // correct, but the period may be off due to noise.
          if( confidence < 0.50 )
          {
            for( int i=-cc; i<=cc; ++i )
            {
              // If the best is an integer value of the base then the result is
              // the same because more groups can create smaller regions.
              if( (poloidalPeriod + i) %
                  (poloidalWinding * (unsigned int) nnodes) == 0 )
              {
                confidence += 0.20;
                if( i == 0 ) confidence += 0.10;
                
                if( verboseFlag )
                {
                  cerr << "Integer Island poloidal via ridgeline ";
                  if(i)  cerr << i << "  ";
                  cerr << poloidalPeriod << endl;
                }
                
                break;
              }
            }
          }

          // At this point the ridgeline period is valid, if the
          // fieldline is stable then there should not be any segments
          // crossing each other.
          if( confidence > 0.60 )
          {
            if( !IntersectCheck( poloidal_puncture_pts,
                                 toroidalWinding*nnodes ) ||
                ridgelineVariance < 0.9 )
            {
              cerr << " *********Entering the chaotic regime*********" << endl;
            }

            for( unsigned int cc=nnodes/2;
                 cc<=2*nnodes && cc<poloidal_puncture_pts.size()/2;
                 ++cc )
            {
              if( IntersectCheck( poloidal_puncture_pts,
                                  toroidalWinding*cc ) )
                cerr << "toroidalWinding " << toroidalWinding << "  "
                     << "cc " << cc << " valid"
                     << endl;
            }       
            
            if( findIslandCenters )
              findIslandCenter( poloidal_puncture_pts, (unsigned int) nnodes,
                                toroidalWinding, poloidalWinding, islandCenters );
          }
          else // if( confidence < 0.60 )
          {
            // If the best is an integer value of the base then the result
            // is the same because more groups can create smaller regions.
            unsigned int cc = 0;
            
            while( cc < periodicityStats.size() &&
                     (periodicityStats[cc].first % poloidalWinding == poloidalWinding-1 ||
                      periodicityStats[cc].first % poloidalWinding == 0 ||
                      periodicityStats[cc].first % poloidalWinding == 1) )
              ++cc;

            if( cc )
            {
              type = FieldlineInfo::UNKNOWN;
              islands = 0;
              //        nPuncturesNeeded = 0;
              
              unsigned int i = 0;
              vector< pair < pair<unsigned int, unsigned int >, double > >::iterator
                iter = windingNumberList.begin();

              // Erase everything up to but not including the current match.
              if( i<toroidalMatchIndex )
              {
                while( i<toroidalMatchIndex ) { ++i; ++iter; }
                
                windingNumberList.erase(windingNumberList.begin(), iter);
              }

              // Erase everything after the current match.
              ++i;
              ++iter;
              
              if( i < windingNumberList.size() )
                windingNumberList.erase( iter, windingNumberList.end() );
              
              if( verboseFlag )
                cerr << endl << "!!!!!!!!!!!!!!!!!!!!!!  "
                     << "Probably not an island assuming a surface - poloidal surface matched " << cc << "  "
                     << "!!!!!!!!!!!!!!!!!!!!!!" << endl;
              
              toroidalMatchIndex = 0;
              poloidalMatchIndex = -1;
            }
            
            if( islands && confidence < 0.70 && verboseFlag )
              cerr << endl << "!!!!!!!!!!!!!!!!!!!!!!  "
                   << "Best island poloidal  " << poloidalPeriod << "  "
                   << "!!!!!!!!!!!!!!!!!!!!!!" << endl;
          }
        }

        // Not enough points for the ridgeline so get more points.
        else // if( ridgeline_points.size() < nRidgelinePtsNeeded )
        {
          // Find the best poloidal periodicity.
          if( verboseFlag )
            cerr << "Polodial " << endl;
          ridgelineGCD = periodicityChecks( toroidalWinding,
                                            nnodes,
                                            poloidal_puncture_pts,
                                            periodicityStats,
                                            false );
          
          // Find the best ridgeline periodicity.
          if( verboseFlag )
            cerr << "Ridgeline " << endl;
          ridgelineGCD = periodicityChecks( poloidalWinding,
                                            nnodes,
                                            ridgeline_points,
                                            periodicityStats,
                                            true );
          
          // There is one more poloidal point than ridgeline points.
          nPuncturesNeeded =
            ((nRidgelinePtsNeeded+1) / poloidalWinding + 5) * toroidalWinding;

          if( verboseFlag )
            cerr << "Too few puncture points, at least "
                 << nPuncturesNeeded
                 << " are needed to complete the ridgeline analysis"
                 << endl;
        }
      }

      // Not enough points for the boundary so get more points.
      else // if( !complete )
      {
        nnodes = poloidal_puncture_pts.size() / toroidalWinding;

        // Find the best poloidal periodicity.
        if( verboseFlag )
          cerr << "Polodial " << endl;
        ridgelineGCD = periodicityChecks( toroidalWinding,
                                          nnodes,
                                          poloidal_puncture_pts,
                                          periodicityStats,
                                          false );
          
        // Find the best ridgeline periodicity.
        if( verboseFlag )
          cerr << "Ridgeline " << endl;
        ridgelineGCD = periodicityChecks( poloidalWinding,
                                          nnodes,
                                          ridgeline_points,
                                          periodicityStats,
                                          true );
          

        unsigned int additionalPts = 0;

        for( unsigned int i=0; i<toroidalWinding; ++i ) 
        {
          Vector firstPoint = (Vector) poloidal_puncture_pts[i];
          Vector nextPoint  = (Vector) poloidal_puncture_pts[i+toroidalWinding];
          
          Vector lastPoint =
            (Vector) poloidal_puncture_pts[i+(nnodes-1)*toroidalWinding];
          Vector nexttolastPoint =
            (Vector) poloidal_puncture_pts[i+(nnodes-2)*toroidalWinding];
          
          Vector v0 = nextPoint - firstPoint;
          Vector v1 = nexttolastPoint - lastPoint;

          double gap = (firstPoint-lastPoint).length();
          double dist = (v0.length() + v1.length()) / 2.0;

          if( additionalPts < gap / dist)
            additionalPts = gap / dist;
        }

        if( additionalPts )
        {
          nPuncturesNeeded = (nnodes + additionalPts + 5) * toroidalWinding;

          if( verboseFlag )
            cerr << "Too few puncture points, at least "
                 << nPuncturesNeeded
                 << " are needed to complete the boundary."
                 << endl;
        }
      }

    }

    // At this point assume the surface is irrational.
    if( type == FieldlineInfo::UNKNOWN )
    {
      type = FieldlineInfo::FLUX_SURFACE;
      
      // Find the first point from another group that overlaps the first
      // group. This only works if there is an overlap between groups.
      nnodes = poloidal_puncture_pts.size() / toroidalWinding;
      
      // Get the direction based on the first two points in a group.
      Vector v0 = (Vector) poloidal_puncture_pts[toroidalWinding] -
        (Vector) poloidal_puncture_pts[0];

      // Get the direction based on the first points from adjacent groups.
      Vector v1 = (Vector) poloidal_puncture_pts[windingGroupOffset] -
        (Vector) poloidal_puncture_pts[0];
      
      // If the windingGroupOffset and point ordering are opposite in directions
      // then the previous group is the windingGroupOffset. Otherwise is they have
      // the same direction then toroidalWinding-windingGroupOffset is the previous
      // group.
      int offsetDir = (Dot( v0, v1 ) > 0.0 ? 1 : -1);
      
      vector< unsigned int > nodes(toroidalWinding);

      nnodes = 0;

      for( unsigned int i=0; i<toroidalWinding; ++i ) 
      {
        nodes[i] = poloidal_puncture_pts.size() / toroidalWinding;

        // The next group
        unsigned int j = (i+offsetDir*windingGroupOffset + toroidalWinding) % toroidalWinding;
        
        Vector firstPoint = (Vector) poloidal_puncture_pts[j];
        Vector nextPoint  = (Vector) poloidal_puncture_pts[j+toroidalWinding];
        
        Vector v0 = nextPoint - firstPoint;

        for( unsigned int k=i; k<poloidal_puncture_pts.size(); k+=toroidalWinding ) 
        {
          Vector  testPoint = (Vector) poloidal_puncture_pts[k];
        
          Vector v1 = testPoint - firstPoint;
        
          if( (poloidalWinding > 2 || v1.length() / v0.length() < 5.0)
              && Dot( v0, v1 ) > 0.0 )
          {
             complete = true;
            
             nodes[i] = k / toroidalWinding;

            break;
          }
          else
             complete = false;
        }

        nnodes += nodes[i];
      }

      // Get the average number of nodes.
      nnodes /= (float) toroidalWinding;

      unsigned int cc = 0;
      unsigned int index = 0;
      unsigned int new_nnodes = 0;
      
      for( unsigned int i=0; i<toroidalWinding; i++ )
      {
        if( nodes[i] < nnodes - 1 || nnodes + 1 < nodes[i] )
        {
          cc++;
          index = i;
        }
        else
          new_nnodes += nodes[i];
      }
      
      if( cc == 1 )
      {
        nnodes = new_nnodes / (float) (toroidalWinding-cc);
      }
      
      for( unsigned int i=0; i<toroidalWinding; i++ )
      {
        if( nodes[i] < nnodes - 1 || nnodes + 1 < nodes[i] )
        {
          if( verboseFlag )
            cerr << "Appears to be a flux surface but not self consistent, "
                 << "average number of nodes  " << nnodes << "   (  ";
          
          for( unsigned int i=0; i<toroidalWinding; i++ )
            if( verboseFlag )
              cerr << nodes[i] << "  ";

          cerr << ")" << endl;

          break;
        }
      }  

      if( !complete )
      {
        unsigned int additionalPts = 0;

        // Get the direction based on the first two points in a group.
        Vector v0 = (Vector) poloidal_puncture_pts[toroidalWinding] -
          (Vector) poloidal_puncture_pts[0];

        // Get the direction based on the first points from adjacent groups.
        Vector v1 = (Vector) poloidal_puncture_pts[windingGroupOffset] -
          (Vector) poloidal_puncture_pts[0];

        // If the windingGroupOffset and point ordering are opposite in directions
        // then the previous group is the windingGroupOffset. Otherwise is they have
        // the same direction then toroidalWinding-windingGroupOffset is the previous
        // group.
        int offsetDir = (Dot( v0, v1 ) > 0.0 ? 1 : -1);

        for( unsigned int i=0; i<toroidalWinding; ++i ) 
        {
          // The next group
          unsigned int j = (i+offsetDir*windingGroupOffset + toroidalWinding) % toroidalWinding;

          Vector firstPoint = (Vector) poloidal_puncture_pts[j];
          Vector nextPoint  = (Vector) poloidal_puncture_pts[j+toroidalWinding];
          
          Vector  lastPoint =
            (Vector) poloidal_puncture_pts[i+(nnodes-1)*toroidalWinding];
          Vector  prevPoint =
            (Vector) poloidal_puncture_pts[i+(nnodes-2)*toroidalWinding];
          
          Vector v0 = nextPoint - firstPoint;
          Vector v1 = lastPoint - firstPoint;

          unsigned int needPts = ( (firstPoint-lastPoint).length() /
                                   (prevPoint-lastPoint).length() + 0.5 );
          
          if( additionalPts < needPts )
            additionalPts = needPts;
        }

        complete = additionalPts ? false : true;

        if( additionalPts )
        {
          nPuncturesNeeded = (nnodes+additionalPts) * toroidalWinding + 1;

          if( verboseFlag )
            cerr << "Too few puncture points, at least "
                 << (nnodes+additionalPts) * toroidalWinding
                 << " are needed to complete the boundary."
                 << endl;
        }
      }

      // Find the best polodial periodicity.
      if( verboseFlag )
        cerr << "Polodial " << endl;
      ridgelineGCD = periodicityChecks( toroidalWinding,
                                        nnodes,
                                        poloidal_puncture_pts,
                                        periodicityStats,
                                        false );
      
      // Find the best ridgeline periodicity.
      if( verboseFlag )
        cerr << "Ridgeline " << endl;
      ridgelineGCD = periodicityChecks( poloidalWinding,
                                        nnodes,
                                        ridgeline_points,
                                        periodicityStats,
                                        true );
      
      poloidalPeriod = periodicityStats[0].first;
      ridgelineVariance =
        (ridgeline_dc_var-periodicityStats[0].second)/ridgeline_dc_var;

      cerr << "ridgeline variance comparison  " << ridgelineVariance << endl;

      if( ridgelineVariance < 0.9 && nPuncturesNeeded == 0 )
      {
        nPuncturesNeeded = poloidal_puncture_pts.size() * 2;

        if( verboseFlag )
          cerr << "Too few toroidal points, at least "
               << nPuncturesNeeded
               << " are needed for ridgeline analysis."
               << endl;
        }
    }

    // Determine the confidence of the fieldline analysis
    if( type == FieldlineInfo::RATIONAL ||
        type == FieldlineInfo::FLUX_SURFACE )
    {
      for( unsigned int i=0; i<windingNumberList.size(); ++i )
      {
        if( poloidalPeriod == windingNumberList[i].first.second )
        {
          confidence += 0.40;

          poloidalMatchIndex = i;

          if( verboseFlag )
            cerr << "Exact Poloidal Winding "
                 << windingNumberList[i].first.first << "/"
                 << windingNumberList[i].first.second
                 << endl;

          break;
        }
      }

      // Look for the best ridgeline to match one of the winding
      // number sets.
      if( poloidalMatchIndex == -1 )

      for( unsigned int i=0; i<windingNumberList.size(); ++i )
      {
        // If the best is an integer value of the base then the result
        // is the same because more groups can create smaller regions.
        if( poloidalPeriod % windingNumberList[i].first.second == 0 )
        {
          confidence += 0.30;
        
          poloidalMatchIndex = i;

          if( verboseFlag )
            cerr << "Integer Poloidal Winding "
                 << windingNumberList[i].first.first << "/"
                 << windingNumberList[i].first.second
                 << endl;

          break;
        }

        // If the best is an integer value of the base then the result
        // is the same because more groups can create smaller regions.
        // Note allow for +/-1 of the number of windings as it can be
        // difficult to determine the last winding point.
        else if( poloidalPeriod % windingNumberList[i].first.second ==
                 windingNumberList[i].first.second-1 ||
                 poloidalPeriod % windingNumberList[i].first.second == 1 )
        {
          confidence += 0.20;
        
          poloidalMatchIndex = i;

          if( verboseFlag )
            cerr << "Integer +/-1 Poloidal Winding " << poloidalPeriod
                 << endl;

          break;
        }
      }

      // Look for the poloidalWinding to match one of the ridgeline
      // periods.
      if( poloidalMatchIndex == -1 )

      for( unsigned int i=0; i<periodicityStats.size(); ++i )
      {
        if( periodicityStats[i].second > 10.0 * periodicityStats[0].second )
          break;

        // If the best is an integer value of the base then the result
        // is the same because more groups can create smaller regions.
        if( periodicityStats[i].first % poloidalWinding == 0 )
        {
          confidence += 0.30;
        
          poloidalMatchIndex = 0;
          poloidalPeriod = periodicityStats[i].first;

          if( verboseFlag )
            cerr << "Integer Ridgeline Period " << poloidalPeriod
                 << endl;

          break;
        }

        // If the best is an integer value of the base then the result
        // is the same because more groups can create smaller regions.
        // Note allow for +/-1 of the number of windings as it can be
        // difficult to determine the last winding point.
        else if( periodicityStats[i].first % poloidalWinding ==
                 periodicityStats[i].first-1 % poloidalWinding ||
                 periodicityStats[i].first % poloidalWinding == 1 )
        {
          confidence += 0.20;
        
          poloidalMatchIndex = 0;
          poloidalPeriod = periodicityStats[i].first;

          if( verboseFlag )
            cerr << "Integer +1/-1 Ridgeline Period " << poloidalPeriod
                 << endl;

          break;
        }
      }

      if( verboseFlag )
        cerr << "toroidalMatchIndex  " << toroidalMatchIndex << "  "
             << "poloidalMatchIndex  " << poloidalMatchIndex << endl;

      if( poloidalMatchIndex == - 1 )
      {
        if( verboseFlag )
          cerr << "No match between the ridgeline and the poloidal winding number" << endl;
        
      }

      else if( poloidalMatchIndex == toroidalMatchIndex )
        confidence += 0.10;

      // In this case the toroidal winding could be higher order than
      // the user desired or it could have failed the intersection test.
      else if( poloidalMatchIndex < toroidalMatchIndex )
      {
        if( windingNumberList[poloidalMatchIndex].first.first >
            maxToroidalWinding &&
            IntersectCheck( poloidal_puncture_pts,
                            windingNumberList[poloidalMatchIndex].first.first ) )
        {
          if( verboseFlag )
            cerr << endl << "!!!!!!!!!!!!!!!!!!!!!!  "
                 << "Higher order may be better  "
                 << windingNumberList[poloidalMatchIndex].first.first << "  "
                 << windingNumberList[poloidalMatchIndex].first.second << "  "
                 << "!!!!!!!!!!!!!!!!!!!!!!" << endl;
        }
      }

      // In this case it is possible that the toroidal matching for
      // two set is basically the same. Yet the poloidal matching is
      // better so take it instead.
      else if( poloidalMatchIndex > toroidalMatchIndex )
      {
        float diff = (windingNumberList[toroidalMatchIndex].second -
                      windingNumberList[poloidalMatchIndex].second ) / 
          windingNumberList[toroidalMatchIndex].second;

        if( windingNumberList[poloidalMatchIndex].first.first <
            maxToroidalWinding &&
            IntersectCheck( poloidal_puncture_pts,
                            windingNumberList[poloidalMatchIndex].first.first ) &&
            diff < 0.02 )
        {
          if( verboseFlag )
            cerr << endl << "!!!!!!!!!!!!!!!!!!!!!!  "
                 << "Poloidal match is better overall  "
                 << "!!!!!!!!!!!!!!!!!!!!!!" << endl;

          toroidalMatchIndex = poloidalMatchIndex;
          toroidalWinding = windingNumberList[poloidalMatchIndex].first.first;
          poloidalWinding = windingNumberList[poloidalMatchIndex].first.second;
        }          
      }
    }
  }
#endif
