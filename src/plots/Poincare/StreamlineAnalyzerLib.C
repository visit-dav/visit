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
#include <float.h>
#include <iostream>

using namespace std;

namespace FusionPSE {

#define SIGN(x) ((x) < 0.0 ? -1 : 1)

//using namespace SCIRun;

Point FieldlineLib::interpert( Point lastPt, Point currPt, double t ) {

  return Point( Vector( lastPt ) + Vector( currPt - lastPt ) * t );
}


int FieldlineLib::ccw( Vector v0, Vector v1 ) {
    
  if( v0.x * v1.z - v0.z * v1.x > FLT_MIN ) return  1;    //  CCW
  if( v0.z * v1.x - v0.x * v1.z > FLT_MIN ) return -1;    //  CW
  if( v0.x * v1.x < 0.0 || v0.z * v1.z < 0.0 ) return -1; // CW
    
  if( v0.x*v0.x+v0.z*v0.z >=
      v1.x*v1.x+v1.z*v1.z ) return 0;               //  ON LINE
    
  return 1;                                                 //  CCW
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


void FieldlineLib::convexHull( vector< Point > &hullPts,
                               vector< unsigned int > &ordering,
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
    if( hullPts[ min ].z > hullPts[i].z )
      min = i;
  }

  // Store the minimum point so that we know when we are done.
  hullPts[npts] = hullPts[min];
  ordering[npts] = ordering[min];

  do {

    // Swap the point that is the farthest to the right of all others.
    if( min != m ) {
      Point tmpPt = hullPts[m];
      hullPts[m] = hullPts[min];
      hullPts[min] = tmpPt;

      unsigned tmpIndex = ordering[m];
      ordering[m] = ordering[min];
      ordering[min] = tmpIndex;
    }

    m++;
    min = m;

    // Find the next point that is the farthest to the right of all others.
    for( unsigned int i=min+1; i<npts+1; i++ ) {

      Vector v0 = (Vector) hullPts[m-1] - (Vector) hullPts[min];
      Vector v1 = (Vector) hullPts[i  ] - (Vector) hullPts[min];

      int CCW = ccw( v0, v1 );

      // The next point that is the farthest to the right of all
      // others will be in a clockwise direction (i.e. the convex
      // hull is in the counter clockwise direction.
      if( CCW == dir )
        min = i;

      // In this case the points are co-linear so take the point
      // that is closest to the starting point.
      else if( ordering[i] != ordering[m-1] && CCW == 0 ) {
        v1 = (Vector) hullPts[m-1] - (Vector) hullPts[i];

        if( v0.length2() > v1.length2() )
          min = i;
      }
    }

    // Stop when the first point is found again.
  } while( min != npts );
}


bool FieldlineLib::hullCheck( vector< Point > &points,
                              unsigned int npts ) {

  // If one, two, or three points the ordering makes no difference and
  // it is convex.
  if( npts <= 3 ) {
    return true;
  }

  vector< Point > hullPts;
  vector< unsigned int > ordering;

  // Store the points and their order in a temporary vector.
  for( unsigned int i=0; i<npts; i++ ) {
    hullPts.push_back( points[i] );
    ordering.push_back( i );
  }

  // Add one more point as a terminal.
  hullPts.push_back( points[0] );
  ordering.push_back( 0 );

  unsigned int cc = 0;
  unsigned int start = 0;
  unsigned int last = 0;

  int dir = 1;

  // Get the convex hull(s).
  do {

    // Swap the direction back and forth so to get the correct ordering.
    dir *= -1;

    convexHull( hullPts, ordering, start, npts, dir );

//     cerr << "npts " << npts << "  "
//       << "start " << last << "  "
//       << "points " << start - last << "  "
//       << endl;

    last = start;

    ++cc;

  } while( start != npts );

  return (cc == 1);
}


unsigned int FieldlineLib::factorial( unsigned int n0, unsigned int n1 ) {

  unsigned int min = n0 < n1 ? n0 : n1;

  for( unsigned int i=min; i>1; i-- )
    if( n0 % i == 0 && n1 % i == 0 )
      return i;

  return 0;
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
             unsigned int offset ) {

  unsigned int skip;

  if( toroidalWinding > 1 && poloidalWinding != 0 ) {
    //  To find the skip find the mutual primes via the
    //  (Blankinship Algorithm).
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


void FieldlineLib::
poloidalWindingCheck( vector< Point > &points,
                      vector< unsigned int > &poloidalWindingset,
                      unsigned int maxToroidalWinding,
                      unsigned int &bestToroidalWinding,
                      unsigned int &bestPoloidalWinding,
                      double &bestMatchPercent,
                      unsigned int &nextBestToroidalWinding,
                      unsigned int &nextBestPoloidalWinding,
                      double &nextBestMatchPercent,
                      unsigned int level ) {

  bestToroidalWinding = 0;
  bestPoloidalWinding = 0;
  nextBestToroidalWinding = 0;
  nextBestPoloidalWinding = 0;
  
  bestMatchPercent = 0;
  nextBestMatchPercent = 0;

  unsigned int npts = points.size();

  // The premise is that for a given toroidal winding the poloidal
  // winding should be consistant between each Nth punction point,
  // where N is the toroidal winding. For instance, if the toroidal
  // winding is 5 and the poloidal winding is 2. Then the pattern
  // should be:

  // 0 1 1 1 2 - 2 3 3 3 4 - 4 5 5 5 6

  // In this case the different between every 5th value (the toroidal
  // winding) should be 2 (the poloidal winding).

  for( unsigned int toroidalWinding=1;
       (toroidalWinding<maxToroidalWinding && toroidalWinding<npts/2);
       ++toroidalWinding )
  {

    // If the first two connections of one toroidal winding group
    // crosses the first two connections of another group skip it.
    if( level == 1 && !IntersectCheck( points, toroidalWinding ) )
      continue;

    // Get the average value of the poloidal winding.
    double poloidalWindingAve = 0;

    for( unsigned int i=0; i<npts-toroidalWinding; i++)
      poloidalWindingAve += (poloidalWindingset[i+toroidalWinding] -
                          poloidalWindingset[i]);

    poloidalWindingAve =
      (float) poloidalWindingAve / (float) (npts-toroidalWinding);
      
    if( poloidalWindingAve < 0.5 )
      continue;

    // Round the average value to the nearest integer value.
    unsigned int poloidalWinding = (poloidalWindingAve + 0.5);

    if( level == 2 )
    {
        debug5<< "tested toroidalWinding/poloidalWinding  "
              << toroidalWinding << "  "
              << poloidalWinding << "  hits  ";
    }

    // Count the number of times the poloidal winding matches the
    // windings between puncture points (i.e. the poloidal winding
    // set).
    unsigned int nMatches = 0;

    for( unsigned int i=0; i<npts-toroidalWinding; ++i) {
      if( poloidalWinding ==
          poloidalWindingset[i+toroidalWinding] - poloidalWindingset[i] ) {
        ++nMatches;

        if( level == 2 ) {
            debug5 << 1 << " ";
        }
      } else if ( level == 2 ) {
          debug5 << 0 << " ";
      }
    }

    if( level == 2 )
      debug5 << endl;

    double matchPercent = (double) nMatches / (double) (npts-toroidalWinding);

    if( level >= 1 )
    {
        debug5 << "tested toroidalWinding/poloidalWinding  "
               << toroidalWinding << "  "
               << poloidalWinding << "  ("
               << (double)toroidalWinding/(double)poloidalWinding << ")  "
               << "consistency "
               << 100.0 * matchPercent
               << " percent" << endl;
    }

    if( bestMatchPercent <= matchPercent ) {
      nextBestMatchPercent = bestMatchPercent;
      nextBestToroidalWinding = bestToroidalWinding;
      nextBestPoloidalWinding = bestPoloidalWinding;

      bestMatchPercent = matchPercent;

      // Keep the low order toroidalWinding / poloidalWinding
      if( bestToroidalWinding == 0 ||
          !(toroidalWinding % bestToroidalWinding == 0 &&
            poloidalWinding % bestPoloidalWinding == 0 &&
            toroidalWinding / bestToroidalWinding == poloidalWinding / bestPoloidalWinding) ) {
        bestToroidalWinding = toroidalWinding;
        bestPoloidalWinding = poloidalWinding;
      }

    } else if( nextBestMatchPercent <= matchPercent ) {

      // Do not save higher order toroidalWinding / poloidalWinding as
      // next best.
      if( !(toroidalWinding % bestToroidalWinding == 0 &&
            poloidalWinding % bestPoloidalWinding == 0 &&
            toroidalWinding / bestToroidalWinding == poloidalWinding / bestPoloidalWinding) ) {

        nextBestMatchPercent = matchPercent;
              
        // Keep the low order toroidalWinding / poloidalWinding
        if( nextBestToroidalWinding == 0 ||
            !(toroidalWinding % nextBestToroidalWinding == 0 &&
              poloidalWinding % nextBestPoloidalWinding == 0 &&
              toroidalWinding / nextBestToroidalWinding == poloidalWinding / nextBestPoloidalWinding) ) {
          nextBestToroidalWinding = toroidalWinding;
          nextBestPoloidalWinding = poloidalWinding;
        }
      }
    }
  }

  // If the toroidalWinding and poloidalWinding have a common
  // denominator find the greatest denominator and remove it.
  if( bestToroidalWinding != bestPoloidalWinding ) {

    for( unsigned int i=bestPoloidalWinding; i>1; --i) {
      if( bestToroidalWinding % i == 0 && bestPoloidalWinding % i == 0 ) {
        bestToroidalWinding /= i;
        bestPoloidalWinding /= i;
              
        i = bestPoloidalWinding;
      }
    }
  }
}


double FieldlineLib::
poloidalWindingStats( vector< Point >& poloidalWindingPoints,
                      unsigned int poloidalWinding )
{
  if( poloidalWinding > poloidalWindingPoints.size() / 2 )
  {
    return 99999.9;
  }

  // Find the average standard deviation for each of the poloidal
  // groups. If the poloidal winding is correct the standard deviation
  // should be a small compared it a wrong value. However, this is
  // based on having a reasonable first guess, i.e. really bad guesses
  // could give a small standard deviation yet be incorrect.

  // Note for a rational surface the standard deviation should be
  // zero.

  double average_sd = 0;
  
  for( unsigned int i=0; i<poloidalWinding; ++i ) {

    // Find the centroid of the points based on the poloidal winding.
    Vector centroid(0,0,0);
    unsigned int npts = 0;
    
    for( unsigned int j=i;
         j<poloidalWindingPoints.size();
         j+=poloidalWinding ) {
      centroid += (Vector) poloidalWindingPoints[j];
      ++npts;
    }
    
    centroid /= (double) npts;

    // Get the L2 norm distance for each group.
    double sumofsquares = 0;
    
    for( unsigned int j=i;
         j<poloidalWindingPoints.size();
         j+=poloidalWinding ) {
      sumofsquares += Vector(poloidalWindingPoints[j]-centroid).length2();
    }

    // Calculate the standard deviation
    average_sd += sqrt( sumofsquares / (double) npts );
    
//     cerr << poloidalWinding << "  "
//       << i << "  "
//       << cc << "  "
//       << sqrt( sumofsquares / (double) npys ) << endl;
  }

  // Return the average standard deviation from all groups.
  return average_sd /= (double) poloidalWinding;
}


bool FieldlineLib::
rationalCheck( vector< Point >& points,
               unsigned int toroidalWinding,
               unsigned int &islands,
               float &avenode,
               float delta)
{
  // Look at the distance between the centroid of each toroidal and
  // the points that are in it. If it is smaller then the distance
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

//      cerr << delta << "  " << vec.length() << endl;

      if( vec.length() > delta ) {
        return false;
      }
    }
  }

  islands = 0;
  avenode = 1;

  return true;
}


bool FieldlineLib::
islandChecks( vector< Point >& points,
              unsigned int toroidalWinding,
              unsigned int &islands,
              float &nnodes ) {

  islands = 0;
  nnodes = 0;

  // Get the centroid for all of the points.
  Vector globalCentroid(0,0,0);

  for( unsigned int i=0; i<points.size(); i++ )
    globalCentroid += (Vector) points[i];

  globalCentroid /= (float) points.size();


  bool convex;

  if( toroidalWinding == 1 ) {

    // Check to see if it overlaps itself - it should or more points
    // need to be obtained.
    unsigned int npts = points.size() / toroidalWinding;

    for( unsigned int j=toroidalWinding, k=j+toroidalWinding;
         k<points.size();
         j+=toroidalWinding, k+=toroidalWinding ) {
      
      // See if the test point is between the first secton.
      Vector v0 = (Vector) points[0] - (Vector) points[k];
      Vector v1 = (Vector) points[1] - (Vector) points[k];
        
      if( Dot( v0, v1 ) < 0.0 ) {
        npts = k / toroidalWinding;
        break;
      }

      // See if the first point is between the test section.
      v0 = (Vector) points[0] - (Vector) points[j];
      v1 = (Vector) points[0] - (Vector) points[k];
      
      if( Dot( v0, v1 ) < 0.0) {
        npts = k / toroidalWinding;
        break;
      }
    }

    convex = hullCheck( points, (unsigned int) npts );

  } else {

    convex = hullCheck( points, toroidalWinding );
  }

  // Check for islands. Islands will exists if there is a change in
  // direction of the connected points relative to a base point. If
  // the hull is convex the base point may the centroid of all of the
  // points or based upon a point that is perpendicular to the
  // principal axis of the group of points.
  vector<unsigned int>nodes(toroidalWinding);

  for( unsigned int i=0; i<toroidalWinding; i++ ) {

    Vector baseCentroid;

    // If the hull is convex the global centroid can be used because
    // all of the islands will surround it in a radial manner.
    if( convex ) {
      baseCentroid = globalCentroid;

      // A convex hull for a toroidalWinding of 1 iis defined as not being an
      // island.
      if( toroidalWinding == 1 ) {
//      cerr << "toroidalWinding of 1 and convex hull" << endl;
        return false;
      }

      // Otherwise use an offset from the local centroid of each group.
    } else {

      // Get the local centroid for the group.
      Vector localCentroid(0,0,0);

      unsigned int npts = 0;

      for( unsigned int j=i; j<points.size(); j+=toroidalWinding, npts++ )
        localCentroid += (Vector) points[j];

      localCentroid /= (float) npts;

      // Get the principal axes of the points.
      float Ixx = 0.0;
      float Ixz = 0.0;
      float Izz = 0.0;

      float maxDist = 0;

      // Get the moments of intertial for each point. It assumed that
      // everything is in the Y plane as such there the moments of
      // intertial along the Y axis are zero.
      for( unsigned int j=i; j<points.size(); j+=toroidalWinding ) {

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
//       << Pxx << "  " << Pzz << "  " << Pxx/Pzz << "  " << Pzz/Pxx << endl
        ;

      // For a toroidalWinding of 1 if the moment of interia of one axis is
      // similar to the other axis then use the centroid directly.
      if( toroidalWinding == 1 && Pxx / Pzz < 5.0 && Pzz / Pxx < 5.0 ) {
          baseCentroid = localCentroid;
      } else {
        // Use the principal axes to get an offset from the local
        // centroid which gives a point outside the island.
        if( Pxx > Pzz )
          baseCentroid = localCentroid -
            Vector(  cos(alpha), 0, sin(alpha) ) * maxDist;
        else
          baseCentroid = localCentroid -
            Vector( -sin(alpha), 0, cos(alpha) ) * maxDist;
      }
    }

    unsigned int turns = 0;
    unsigned int firstTurn = 0;
    unsigned int   midTurn = 0;
    unsigned int  lastTurn = 0;

    // Get the direction based on the first two points.
    Vector v0 = (Vector) points[i        ] - baseCentroid;
    Vector v1 = (Vector) points[i+toroidalWinding] - baseCentroid;

    bool lastCCW = (ccw( v0, v1 ) == 1);
    v0 = v1;

    // Get the direction based on the remaining points.
    for( unsigned int j=i+2*toroidalWinding; j<points.size(); j+=toroidalWinding ) {
      v1 = (Vector) points[j] - baseCentroid;

      bool localCCW = (ccw( v0, v1 ) == 1);

      // A switch in direction indicates that an island is present.
      if( localCCW != lastCCW ) {

        lastCCW = localCCW;

        ++turns;

        if( turns == 1 )
          ++islands;

        if( turns == 1 )     firstTurn = j - toroidalWinding;
        else if( turns == 2 )  midTurn = j - toroidalWinding;
        else if( turns == 3 ) lastTurn = j - toroidalWinding;

        if( turns == 3 )
          break;

      } else {
        v0 = v1;
      }
    }

    unsigned int overlap = 0;

    if( turns < 2 ) {
      nodes[i] = points.size() / toroidalWinding;
    } else {

      if( turns == 2 ) {
        nodes[i] = points.size() / toroidalWinding;
      } else if( turns == 3 ) {
        // Okay initial approximatation. It really depending on where
        // the turns are relative to each other/
        nodes[i] = (lastTurn - firstTurn) / toroidalWinding + 1;
      }

      // Check to see if the island overlaps itself. Only use this if
      // there are two or more turns. Otherwise the it can appear that
      // it overlaps itself earlier.
      for( unsigned int j=midTurn, k=j+toroidalWinding;
           k<points.size();
           j+=toroidalWinding, k+=toroidalWinding ) {

        // See if the test point is between the first secton.
        v0 = (Vector) points[i        ] - (Vector) points[k];
        v1 = (Vector) points[i+toroidalWinding] - (Vector) points[k];
        
        if( Dot( v0, v1 ) < 0.0 ) {
          overlap = k;
          nodes[i] = k / toroidalWinding;
          turns = 3;
          break;
        }

        // See if the first point is between the test section.
        v0 = (Vector) points[i] - (Vector) points[j];
        v1 = (Vector) points[i] - (Vector) points[k];
        
        if( Dot( v0, v1 ) < 0.0 ) {
          overlap = k;
          nodes[i] = k / toroidalWinding;
          turns = 3;
          break;
        }
      }
    }
    
    nnodes += nodes[i];
    
    if( turns )
    {

        debug5 << "Island " << i << " has "
               << turns << " turns with "
               << nodes[i]
               << " nodes, overlap node " << overlap
               << " firstTurn " << firstTurn
               <<   " midTurn " << midTurn
               <<  " lastTurn " << lastTurn << endl;
    }
  }

  // Get the average number of nodes.
  nnodes /= (float) toroidalWinding;

  if( islands ) {
    for( unsigned int i=0; i<toroidalWinding; i++ ) {
      if( nodes[i] < nnodes - 1 || nnodes + 1 < nodes[i] ) {
          debug5 << "Appears to be islands but not self consistant." << endl;
          
        for( unsigned int i=0; i<toroidalWinding; i++ )
            debug5 << nnodes << "  " << nodes[i] << endl;
        
        break;
      }
    }  
  }

  return (bool) islands;
}


bool
FieldlineLib::
basicChecks( vector< Point >& points,
             Vector &globalCentroid,
             unsigned int &toroidalWinding,
             unsigned int &poloidalWinding,
             unsigned int &skip,
             unsigned int &type,
             unsigned int &island,
             float &avenode,
             bool &groupCCW,
             unsigned int &toroidalWindingNextBest ) {

  float confidence = 0;

  Vector v0, v1;

  // Get the centroid for all of the points.
  globalCentroid = Vector(0,0,0);

  for( unsigned int i=0; i<points.size(); i++ )
    globalCentroid += (Vector) points[i];

  globalCentroid /= (double) points.size();


  toroidalWindingNextBest = 0;
  avenode = 0;
  island = 0;
  
  bool convex;

  // A toroidalWinding of one is valid only if it is an island. An island with
  // one toroidalWinding must have a non-convex hull otherwise it is a surface.
  if( toroidalWinding == 1 ) {

    unsigned int nodes = points.size();;

    // Check to see if it overlaps itself - it should or more points
    // need to be obtained.
    for( unsigned int i=1, j=2; j<points.size(); i++, j++ ) {
        
      v0 = (Vector) points[0] - (Vector) points[i];
      v1 = (Vector) points[0] - (Vector) points[j];
        
      if( Dot( v0, v1 ) < 0.0) {
        nodes = j;
        break;
      }
    }

    // The skip should be 1 and it should not be convex.
//    convex = hullCheck( points, nodes, skip );

    debug5 << "nodes " << nodes << "  "
           << "convex " << convex << "  "
           << "skip " << skip << endl;

    // The ordering can be in either direction so skip may be flipped.
    if( skip != 1 && skip + 1 == nodes )
      skip = 1;

    if( skip == 1 & !convex ) {
      poloidalWinding  = 1;
      type = 0;
      island = 1;
      avenode = nodes;
      return true;

    } else {
      type = 0;
      debug5 << "TOROIDALWINDING OF 1 BUT NOT AN ISLAND "
             << (convex ? " CONVEX" : " CONCAVE") << " HULL "
             << endl;
      return false;
    }

  // If a toroidalWinding of two then then the ordering is immaterial.
  } else if( toroidalWinding == 2 ) {
    skip   = 1;
    convex = true;

  } else {

    // Find the skip based on the hull(s).
//    convex = hullCheck( points, toroidalWinding, skip );

    // If the hull is convex the skip can be found using the
    // centroid because it will be located inside the hull.
    if( convex ) {
      unsigned int tmpSkip = skip;

      v0 = (Vector) points[   0] - globalCentroid;
      v1 = (Vector) points[skip] - globalCentroid;

      double angleMin = acos( Dot( v0, v1 ) / (v0.length() * v1.length()) );

      for( unsigned int i=1; i<toroidalWinding; i++ ) {
        v1 = (Vector) points[i] - globalCentroid;

        double angle = acos( Dot( v0, v1 ) / (v0.length() * v1.length()) );

        // The skip will be found based on the minimum angle between two
        // groups.
        if( angleMin > angle ) {
          angleMin = angle;
          tmpSkip = i;

          //      cerr << " angle " << angle
          //           << " angleMin " << angleMin
          //           << " tmpSkip " << tmpSkip
          //           << " skip " << skip
          //           << endl;
        }
      }

      if( tmpSkip != skip && tmpSkip != toroidalWinding - skip ) {
          debug5 << endl << toroidalWinding << "CONVEX HULL BUT IRREGULAR SKIP "
                 << tmpSkip << "  " <<  skip << endl;
      }
    }

    // Find the first point from another group that overlaps the first
    // group. This only works if there is an overlap between groups.
    unsigned int tmpSkip = skip;

    for( unsigned int i=0, j=toroidalWinding;
         i<points.size() && j<points.size();
         i+=toroidalWinding, j+=toroidalWinding ) {

      for( unsigned int k=1; k<toroidalWinding; k++ ) {

        v0 = (Vector) points[i] - (Vector) points[k];
        v1 = (Vector) points[j] - (Vector) points[k];

        v0.normalize();
        v1.normalize();

        //      cerr << "Indexes " << i << "  " << j << "  " << k << " dot  "
        //           << Dot( v0, v1 ) << endl;

        if( Dot( v0, v1 ) < 0.0 ) {
          tmpSkip = k;    
          i = k = points.size();
          break;
        }
      }
    }

    if( tmpSkip != skip && tmpSkip != toroidalWinding - skip ) {
        debug5 << endl << toroidalWinding << " OVERLAP BUT IRREGULAR SKIP "
               << tmpSkip << "  " <<  skip << endl;
    }
  }


  // Island sanity check make sure no island overlaps another island.
  if( islandChecks( points, toroidalWinding, island, avenode ) ) {

    type = 0;

//     // If islands are consistant then get the average.
    avenode /= (float) toroidalWinding;


    // The islands should all go in the same direction but not always.
//     if( lastCCW != baseCCW ) {

//       cerr << "Warning island " << i << " is in the opposite direction."
//         << endl;
//     }

//     if( island == toroidalWinding &&
//      completeIslands == toroidalWinding &&
//      nodesMin+1 >= (unsigned int) avenode && 
//      nodesMax-1 <= (unsigned int) avenode &&
//      firstTurnMax - firstTurnMin == toroidalWinding-1 &&
//      midTurnMax   - midTurnMin   == toroidalWinding-1 &&
//      lastTurnMax  - lastTurnMin  == toroidalWinding-1 ) {
//       confidence = 1.0;      
//     }

//     // The max nodes to check is set in case there are lots of points.
//     unsigned int maxNodeCheck = 2 * (unsigned int) avenode * toroidalWinding;

//     if( maxNodeCheck > points.size() )
//       maxNodeCheck = points.size();

//     if( !convex && island != toroidalWinding &&
//      (firstTurnMax - firstTurnMin)/toroidalWinding > 3 ) {

//       cerr << "PROBABLY NOT ISLANDS" << endl;
//     }


//     for( unsigned int i=0; i<toroidalWinding && island; i++ ) {

//       for( unsigned int g=0; g<2 && island; g++ ) {

//      unsigned int offset;

//      if( g == 0 )  // previous
//        offset = (i - skip + toroidalWinding) % toroidalWinding;
//      else         // next
//        offset = (i + skip)           % toroidalWinding;

//      // Check for a point in the previous/next group being between
//      // the first two points in the current group.
//      if( island ) {
//        for( unsigned int j=i+toroidalWinding, k=offset;
//             j<maxNodeCheck && k<maxNodeCheck;
//             k+=toroidalWinding ) {
            
//          v0 = (Vector) points[i] - (Vector) points[k];
//          v1 = (Vector) points[j] - (Vector) points[k];
            
//          if( Dot( v0, v1 ) < 0.0) {
//            island = 0;
//            type = 2;
              
//            v0.normalize();
//            v1.normalize();
              
//            cerr << "FAILED ISLAND SANITY CHECK #1  "
//                 << i << "  " << j << "  " << k << "  "
//                 << Dot( v0, v1 ) << endl;
              
//            return false;
//          }
//        }
//      }

//      // Check for the first point in the current group being between
//      // two points in the previous/next group.
//      if( island ) {
//        for( unsigned int j=offset, k=j+toroidalWinding;
//             j<maxNodeCheck && k<maxNodeCheck;
//             j+=toroidalWinding, k+=toroidalWinding ) {

//          v0 = (Vector) points[i] - (Vector) points[j];
//          v1 = (Vector) points[i] - (Vector) points[k];

//          if( Dot( v0, v1 ) < 0.0) {
//            island = 0;
//            type = 2;

//            v0.normalize();
//            v1.normalize();

//            cerr << "FAILED ISLAND SANITY CHECK #2  "
//                 << i << "  " << j << "  " << k << "  "
//                 << Dot( v0, v1 ) << endl;

//            return false;
//          }
//        }
//      }
//       }
//     }

  } else {

    type = 1;

    // Surface Checks

    // Get the direction of the points for the first group.
    groupCCW = (ccw( (Vector) points[0      ] - globalCentroid, 
                     (Vector) points[toroidalWinding] - globalCentroid ) == 1);

    if( convex ) {
      // Make sure that the direction is the same for all of the other
      // groups - only valid for convex hulls.
      for( unsigned int i=1, j=i+toroidalWinding; i<toroidalWinding; i++, j++ ) {

        // Get the direction based on the first two points.
        bool baseCCW = (ccw( (Vector) points[i] - globalCentroid, 
                             (Vector) points[j] - globalCentroid ) == 1);

        if( baseCCW != groupCCW ) {
            debug5 << "CONVEX HULL BUT GROUPS GO IN DIFFERENT DIRECTIONS" << endl;

          return false;
        }
      }
    }

    // Make sure the skip direction is the same as the group
    // direction.
    bool baseCCW = (ccw( (Vector) points[0   ] - globalCentroid, 
                         (Vector) points[skip] - globalCentroid ) == 1);
    
    if( groupCCW != baseCCW ) {
        debug5 << "FLIPPING THE SKIP DIRECTION" << endl;
      skip = toroidalWinding - skip;
    }

    unsigned int previous = toroidalWinding - skip;

    // See if a point overlaps the first section.
    for( unsigned int j=previous; j<points.size(); j+=toroidalWinding ) {

      v0 = (Vector) points[0      ] - (Vector) points[j];
      v1 = (Vector) points[toroidalWinding] - (Vector) points[j];

      if( Dot( v0, v1 ) < 0.0) {
        if( j > toroidalWinding ) {
          toroidalWindingNextBest = j - toroidalWinding;
          break;

        } else {
            debug5 << "CAN NOT DETERMINE NEXT BEST TOROIDALWINDING"
                   << " point " << j
                   << " toroidalWinding " << toroidalWinding
                   << endl;

          return false;
        }
      }
    }

    // See if the first point overlaps another section.
    if( toroidalWindingNextBest == 0 ) {

      for( unsigned int j=previous, k=j+toroidalWinding;
           j<points.size() && k<points.size();
           j+=toroidalWinding, k+=toroidalWinding ) {

        v0 = (Vector) points[0] - (Vector) points[j];
        v1 = (Vector) points[0] - (Vector) points[k];

        if( Dot( v0, v1 ) < 0.0) {
          if( j > toroidalWinding ) {
            toroidalWindingNextBest = j;
            break;

          } else {
              debug5 << "CAN NOT DETERMINE NEXT BEST TOROIDALWINDING"
                     << " point " << j
                     << " toroidalWinding " << toroidalWinding
                     << endl;

            return false;
          }
        }
      }
    }


    // If next best toroidalWinding is set then there is an overlap. As such,
    // check to see if the number of points that would be in each
    // group is the same.
    if( toroidalWindingNextBest ) {

      avenode = 0;

      unsigned int nnodes = toroidalWindingNextBest / toroidalWinding;

      unsigned int jMin = points.size();
      unsigned int jMax = 0;

      unsigned int kMin = points.size();
      unsigned int kMax = 0;

      unsigned int lMin = points.size();
      unsigned int lMax = 0;

      unsigned int nodesMin = points.size();
      unsigned int nodesMax = 0;

      // Search each group and check for overlap with the next group.
      for( unsigned int i=0; i<toroidalWinding; i++ ) {

        unsigned int nodes = points.size() / toroidalWinding;

        // Get the first point in the next group.
        unsigned int j = (i + skip) % toroidalWinding;

        for( unsigned int k=i+nnodes*toroidalWinding, l=k-toroidalWinding;
             k<points.size() && l<points.size();
             k+=toroidalWinding, l+=toroidalWinding ) {

          // Check to see if the first overlapping point is really a
          // fill-in point. This happens because the spacing between
          // toroidalWinding groups varies between groups.
          v0 = (Vector) points[k] - (Vector) points[j];
          v1 = (Vector) points[l] - (Vector) points[j];

          if( Dot( v0, v1 ) < 0.0 ) {
            nodes = l / toroidalWinding + 1;

//          cerr << "Found overlap at "
//               << i << "  " << j << "  " << k << "  " << l << "  "
//               << "nodes  " << nodes << "  "
//               << endl;

            if( nodesMin > nodes ) nodesMin = nodes;
            if( nodesMax < nodes ) nodesMax = nodes;

            if( jMin > nodes ) jMin = j;
            if( jMax < nodes ) jMax = j;

            if( kMin > nodes ) kMin = k;
            if( kMax < nodes ) kMax = k;

            if( lMin > nodes ) lMin = l;
            if( lMax < nodes ) lMax = l;

            break;
          }
        }

        avenode += nodes;
      }

      avenode /= toroidalWinding;

      if( nodesMin+1 >= (unsigned int) avenode && 
          nodesMax-1 <= (unsigned int) avenode &&
          jMax - jMin == toroidalWinding-1 &&
          kMax - kMin == toroidalWinding-1 &&
          lMax - lMin == toroidalWinding-1 ) {
        confidence = 1.0;
      }

    } else {

      // In this case there is no overlap so the number of nodes can
      // not be determined correctly so just use the average.
      avenode = (float) points.size() / (float) toroidalWinding;

      confidence = 0.5;
    }
  }


  // In order to find the poloidalWinding find the mutual primes
  // (Blankinship Algorithm). In this case we only care about
  // the first one becuase the second is just the number of
  // toroidalWindings done to get there.

  for( poloidalWinding=1; poloidalWinding<toroidalWinding; poloidalWinding++ )
    if( poloidalWinding * skip % toroidalWinding == 1 )
      break;

  if( poloidalWinding == toroidalWinding ) {
      debug5 << endl
             << "ERROR in finding the - TOROIDALWINDING - POLOIDALWINDING - SKIP"
             << " toroidalWinding " << toroidalWinding
             << " poloidalWinding " << poloidalWinding
             << " skip " << skip
             << endl;

    return false;

  } else if( convex && toroidalWinding > 2 ) {

    // Sanity check for the poloidalWinding/skip calculation
    double angleSum = 0;

    for( unsigned int i=0; i<toroidalWinding; i++ ) {

      unsigned int start = i;
      unsigned int stop  = (start +    1) % toroidalWinding;
      unsigned int next  = (start + skip) % toroidalWinding;

      Vector v0 = (Vector) points[start] - globalCentroid;

      // Get the angle traveled from the first group to the second
      // for one toroidalWinding. This is done by summing the angle starting
      // with the first group and going to the geometrically next
      // group stopping when the logically next group is found.

      do {
        Vector v1 = (Vector) points[next] - globalCentroid;

        angleSum += acos( Dot( v0, v1 ) / (v0.length() * v1.length()) );

        //        cerr << " toroidalWinding " << i
        //             << " start " << start
        //             << " next " << next
        //             << " stop " << stop
        //             << " poloidalWinding angle "
        //             << acos( Dot( v0, v1 ) / (v0.length() * v1.length()) ) << endl;

        start = next;
        next  = (start + skip) % toroidalWinding;

        v0 = v1;
      } while( start != stop );
    }

    // The total number of poloidalWinding should be the same. Account for small
    // rounding errors by adding 25% of the distanced traveled in one
    // toroidalWinding.
    unsigned int poloidalWindingCheck =
      (unsigned int) (angleSum / (2.0 * M_PI) + M_PI/2.0/toroidalWinding);

    if( poloidalWindingCheck != poloidalWinding ) {
        debug5 << endl
               << "WARNING - POLOIDALWINDING MISMATCH "
               << " angle sum " << (angleSum / (2.0 * M_PI))
               << " toroidalWinding " << toroidalWinding
               << " poloidalWindingCheck " << poloidalWindingCheck
               << " poloidalWinding " << poloidalWinding
               << " skip " << skip
               << endl;
    }
  }

  if( avenode > (float) points.size() / (float) toroidalWinding + 1 ) {
      debug5 << endl
             << "ERROR in finding the average nodes"
             << " toroidalWinding " << toroidalWinding
             << " poloidalWinding " << poloidalWinding
             << " skip " << skip
             << " toroidalWindingNextBest " << toroidalWindingNextBest
             << " avenode " << avenode
             << " number of points " << points.size()
             << endl;
  }

  return true;
}


FieldlineInfo
FieldlineLib::fieldlineProperties( vector< Point > &ptList,
                                   unsigned int override,
                                   unsigned int maxToroidalWinding,
                                   float matchPercentLimit )
{
  vector< unsigned int > safetyFactorSet;
  safetyFactorSet.clear();

  // First find out how many toroidal windings it takes to get back to the
  // starting point at the Phi = 0 plane. 

  vector< Point > toroidal_points;
  vector< Point > poloidal_points;
  toroidal_points.clear();
  poloidal_points.clear();

  debug5 << "Analyzing  " << endl;

  unsigned int npts = 0;
  float delta = 0.0;

  Point lastPt, currPt = ptList[0];

  // Set up the Y plane equation as the base analysis takes place in
  // the X-Z plane.
  Vector planeNY( 0, 1, 0 );
  Vector planePt(0,0,0);

  double planeY[4];
      
  planeY[0] = planeNY.x;
  planeY[1] = planeNY.y;
  planeY[2] = planeNY.z;
  planeY[3] = Dot(planePt, planeNY);

  double lastDistY, currDistY = Dot(planeNY, currPt) - planeY[3];

  // Set up the Z plane equation.
  Vector planeNZ( 0, 0, 1 );

  double planeZ[4];
      
  planeZ[0] = planeNZ.x;
  planeZ[1] = planeNZ.y;
  planeZ[2] = planeNZ.z;
  planeZ[3] = Dot(planePt, planeNZ);

  double lastDistZ, currDistZ = Dot(planeNZ, currPt) - planeZ[3];

  for( unsigned int i=1; i<ptList.size(); ++i)
  {
    lastPt = currPt;
    currPt = ptList[i];

    // Save the distance between points to use for finding periodic
    // fieldlines (i.e. rational surfaces).
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
        
        toroidal_points.push_back( point );

        safetyFactorSet.push_back( poloidal_points.size() );

//      if( points.size() && points.size() < maxToroidalWinding )
//        cerr << "toroidalWinding/poloidal  "
//             << points.size() - 1 << "  "
//             << poloidal << "  "
//             << (double) (points.size()-1)/(double)poloidal << endl;
      }
    }

    // Find the positive zero crossings which indicate a poloidal winding.
    if( toroidal_points.size() )
    {
      // Poloidal plane distances.
      lastDistZ = currDistZ;
      currDistZ = Dot( planeNZ, currPt ) - planeZ[3];

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
        
          poloidal_points.push_back( point );
        }
      }
    }
  }

  // At this point all of the puncture points (toroidal points) as
  // well as the poloidal points have been found.
    debug5 << "analyzing with "
           << toroidal_points.size() << " toroidal points and "
           << poloidal_points.size() << " poloidal points " << endl;

  // Get the average distance between points. If within 1/10 of the
  // distance the fieldline is probably on a rational surface.
  delta /= (float) npts;
  delta /= 10.0;
  
  FieldlineType type = UNKNOWN;

  unsigned int toroidalWinding, poloidalWinding;
  unsigned int skip, islands;
  float nnodes;

  if( override ) 
  {
    toroidalWinding = override;
    poloidalWinding = safetyFactorSet.size() /
      safetyFactorSet[safetyFactorSet.size()-1];

    skip = Blankinship( toroidalWinding, poloidalWinding );
  } 
  else 
  {
    unsigned int bestToroidalWinding = 0;
    unsigned int bestPoloidalWinding = 0;
    unsigned int nextBestToroidalWinding = 0;
    unsigned int nextBestPoloidalWinding = 0;
    
    double bestMatchPercent = 0;
    double nextBestMatchPercent = 0;

    // Check the consistancy of the safety factor set. 
    poloidalWindingCheck( toroidal_points, safetyFactorSet, maxToroidalWinding,
                          bestToroidalWinding, bestPoloidalWinding, bestMatchPercent,
                          nextBestToroidalWinding, nextBestPoloidalWinding, nextBestMatchPercent, 1 );

    // Test against a user setable match limit. Default is 0.90 (90%)
    if( bestMatchPercent < matchPercentLimit ) 
    {
        debug5 << "Poor consistancy - probably chaotic" << endl;

        debug5 << "Poloidal set ";

      for( unsigned int i=0; i<safetyFactorSet.size(); ++i)
          debug5 << "(" << i << "," << safetyFactorSet[i] << ") ";

      debug5 << endl;

      // Check the consistancy of the safety factor set but report the
      // findings.
      poloidalWindingCheck( toroidal_points, safetyFactorSet, maxToroidalWinding,
                            bestToroidalWinding, bestPoloidalWinding, bestMatchPercent,
                            nextBestToroidalWinding, nextBestPoloidalWinding, nextBestMatchPercent, 2 );

      toroidalWinding = 0;
      poloidalWinding = 0;
      skip = 0;
      islands = 0;

      type = CHAOTIC;
    } 

    // The number of matches is better than the user defined limit so
    // gather some other stats.
    else 
    {
      toroidalWinding = bestToroidalWinding;
      poloidalWinding = bestPoloidalWinding;
      skip = Blankinship( toroidalWinding, poloidalWinding );

        debug5 << "final toroidalWinding/poloidal  "
               << toroidalWinding << "  "
               << poloidalWinding << "  ("
               << (double)toroidalWinding/(double)poloidalWinding << ")  "
               << "consistency "
               << 100.0 * bestMatchPercent
               << " percent" << endl;

      // Find the base poloidal periodicity standard deviation.
      unsigned int bestPoloidalWinding = poloidalWinding;
      double best_sd =
        poloidalWindingStats( poloidal_points, poloidalWinding );

      debug5 << "Base poloidal  " << poloidalWinding
             << "  " << best_sd << endl;

      // Find the best poloidal periodicy standard deviation. 
      // Note dividing by three insures that there are at least three
      // points in each group.
      for( unsigned int i=1; i<=poloidal_points.size()/3; ++i ) 
      {
        double average_sd = poloidalWindingStats( poloidal_points, i );

        if( best_sd > average_sd ) 
        {
          best_sd = average_sd;
          bestPoloidalWinding = i;
        }
      }

      // If the best is an in integer value of the base then the
      // result is the same because more groups can create smaller
      // regions.
      if( bestPoloidalWinding % poloidalWinding == 0 )
          debug5<< "Integer ";

      debug5 << "Best Poloidal Winding " << bestPoloidalWinding
                << "  " << best_sd << endl;

//     ++safetyFactorConsistant;

      if( toroidalWinding && poloidalWinding &&
          bestPoloidalWinding % poloidalWinding == 0 )
      {
//      ++poloidalPeriodicyMatch;
      }
      else if( nextBestToroidalWinding && nextBestPoloidalWinding &&
               bestPoloidalWinding % nextBestPoloidalWinding == 0) 
      {
//      ++poloidalPeriodicyMismatch;

        if( nextBestMatchPercent / bestMatchPercent < 0.95 )
            debug5 << endl << "**********************  "
                   << nextBestToroidalWinding << "  " << nextBestPoloidalWinding
                   << "  **********************" << endl;
      } 
      else 
      {
          debug5 << endl << "!!!!!!!!!!!!!!!!!!!!!!  "
                 << (bestPoloidalWinding > poloidalWinding ? "Higher " : "Lower ")
                 << "order may be better"
                 << "  !!!!!!!!!!!!!!!!!!!!!!" << endl;
      }
    }
  }

  // Check to see if the fieldline is periodic. I.e. on a
  // rational surface.
  if( rationalCheck( toroidal_points, toroidalWinding,
                     islands, nnodes, delta ) ) 
  {
    type = RATIONAL;
    
    debug5 << "Appears to be a rational surface "
         << delta << endl;
  }
  
  // Check to see if the fieldline creates a set of islands.
  else if( islandChecks( toroidal_points, toroidalWinding,
                         islands, nnodes ) ) 
  {
    type = ISLAND_CHAIN;
    
    // The number of nodes is set so that when connected the islands
    // are complete. The poloidal winding times the number of nodes is
    // also the valve at which points start repeating so it is similar
    // the poloidal winding value for an irrational surface.
    nnodes = ceil(nnodes);

    // Find the base island poloidal periodicity standard deviation.
    unsigned int island_poloidal = poloidalWinding * (unsigned int) nnodes;
    double island_sd =
      poloidalWindingStats( poloidal_points, island_poloidal );
    
    debug5 << "Base island poloidal  " << island_poloidal
           << "  " << island_sd << endl;

    // Find the best island poloidal periodicy standard deviation.
    // Note dividing by two insures that there are at least two
    // points in each group.
    for( unsigned int i=0; i<poloidal_points.size()/2; ++i ) 
    {
      double average_sd = poloidalWindingStats( poloidal_points, i );
      
      if( island_sd > average_sd ) 
      {
        island_sd = average_sd;
        island_poloidal = i;
      }
    }

    // If the best is an integer value of the base then the result is
    // the same because more groups can create smaller regions.
    if( island_poloidal % (poloidalWinding * (unsigned int) nnodes) == 0 )
        debug5<< "Integer ";
    
    debug5 << "Island poloidal  " << island_poloidal
           << "  " << island_sd << endl;

    // If the best island poloidal winding value is the poloidal
    // winding value times the number of nodes then there is self
    // consistancy. Note allow for +1/-1 on the number of nodes as it
    // can be difficult to determine the last node in an island chain.
    for( int i=-1; i<=1; ++i )
    {
      if( island_poloidal &&
          island_poloidal % (poloidalWinding * (unsigned int)(nnodes+i)) == 0 ) 
      {
          debug5 << "Zero crossing period matches the number of nodes ("
                 << nnodes << " " << i
                 <<  ") in the island" << endl;

        nnodes += i;

        break;
      }
    }
  }

  // At this point assume the surface is irrational.
  else 
  {
    type = IRRATIONAL;
    
    // Find the first point from another group that overlaps the first
    // group. This only works if there is an overlap between groups.
    nnodes = (double) toroidal_points.size() / (double) toroidalWinding;
    
    Vector v0, v1;
    
    for( unsigned int i=0, j=toroidalWinding;
         j<toroidal_points.size();
         i+=toroidalWinding, j+=toroidalWinding ) 
    {
      for( unsigned int k=1; k<toroidalWinding; k++ ) 
      {
        v0 = (Vector) toroidal_points[i] - (Vector) toroidal_points[k];
        v1 = (Vector) toroidal_points[j] - (Vector) toroidal_points[k];
          
        if( Dot( v0, v1 ) < 0.0 ) 
        {
          nnodes = (double) i / (double) toroidalWinding + 1.0;
          j = toroidal_points.size();
          
          if( k != toroidalWinding - skip && k != skip ) 
          {
              debug5 << endl
                     << "ERROR in finding the overlap"
                     << " toroidalWinding " << toroidalWinding
                     << " poloidal " << poloidalWinding
                     << " skip " << skip
                     << " overlap group " << k
                     << endl;
          }
          
          break;
        }
      }
    }
  }

  debug5 << "End of streamline " << 0 // fieldlineInfo.size()
         << "  type " << type 
         << "  toroidal windings " << toroidalWinding 
         << "  poloidal windings " << poloidalWinding
         << "  ("<< (double)toroidalWinding/(double)poloidalWinding << ")"
         << "  skip "   << skip
         << "  islands " << islands
         << "  nnodes " << nnodes
         << endl << endl;

  FieldlineInfo fi;

  fi.type = type;

  fi.toroidalWinding = toroidalWinding;
  fi.poloidalWinding = poloidalWinding;
  fi.skip = skip;
  fi.islands = islands;
  fi.nnodes = nnodes;

//  fi.confidence = confidence;

  return fi;
}


FieldlineInfo
FieldlineLib::
fieldlineProperties( vector< Point > &points,
                     unsigned int maxToroidalWinding )
{
  unsigned int toroidalWinding, poloidalWinding;
  unsigned int skip, type;
  unsigned int islands;
  float nnodes;
  unsigned int toroidalWindingNextBest;

  unsigned int order = 2;

  Vector centroid;

  bool groupCCW;

  float bestNodes = 1;

  vector< unsigned int > toroidalWindingList;

  // Find the best toroidalWinding for each test.
  for( toroidalWinding=1;
       toroidalWinding<=maxToroidalWinding;
       ++toroidalWinding ) {
          
    if( points.size() <= 2 * toroidalWinding ) {
        debug5 << "Streamline has too few points ("
               << points.size() << ") to determine the toroidalWinding accurately"
               << " past " << toroidalWinding << " toroidalWindings " << endl;
      break;
    }
          
    // If the first two connections of any group crosses another
    // skip it.
    if( IntersectCheck( points, toroidalWinding ) ) {

      // Passed all checks so add it to the possibility list.
      toroidalWindingList.push_back( toroidalWinding );
      
      debug5 << toroidalWinding << " Passed IntersectCheck\n";
    }
  }

  for( unsigned int i=0; i<toroidalWindingList.size(); i++ ) {
    toroidalWinding = toroidalWindingList[i];

    // Do the basic checks
    if( !basicChecks( points, centroid,
                      toroidalWinding, poloidalWinding, skip, type,
                      islands, nnodes, groupCCW, toroidalWindingNextBest ) ) {
               
      toroidalWindingList.erase( toroidalWindingList.begin()+i );

      i--;
                
      debug5 << "toroidalWindings " << toroidalWinding
             << " FAILED Basic checks REMOVING";

      debug5 << "  ToroidalWindings ";
      for( unsigned int j=0; j<toroidalWindingList.size(); j++ )
          debug5 << toroidalWindingList[j] << "  ";
      debug5 << " Remain" << endl;


      //    if( type == 2 ) {
      //      toroidalWindingList.erase( toroidalWindingList.begin(), toroidalWindingList.end() );
      //    }

    } else {

        debug5 << "toroidalWindings " << toroidalWinding 
               << "  poloidalWindings " << poloidalWinding
               << "  skip "   << skip
               << "  groupCCW " << groupCCW
               << "  islands " << islands
               << "  nnodess " << nnodes
               << "  next best toroidalWinding " << toroidalWindingNextBest;

      if( order == 2 && toroidalWindingList.size() > 1 && islands > 0 ) {

        // If the toroidalWinding is one check to see if there might a
        // higher toroidalWinding that is also islands.
        bool multipleIslands = false;

        if( toroidalWinding == 1 ) {
          unsigned int toroidalWindingNextBestTmp = 0;
          unsigned int islandTmp = 0;

          unsigned int toroidalWindingTmp;
          unsigned int poloidalWindingTmp, skipTmp, typeTmp;
          float nnodesTmp;
          bool groupCCWTmp;
        

          for( unsigned int j=i+1; j<toroidalWindingList.size(); j++ ) {
            toroidalWindingTmp = toroidalWindingList[j];
            if( basicChecks( points, centroid,
                             toroidalWindingTmp, poloidalWindingTmp, skipTmp, typeTmp,
                             islandTmp, nnodesTmp, groupCCWTmp,
                             toroidalWindingNextBestTmp ) && islandTmp > 0 ) {

              multipleIslands = true;
                    
              break;
            }
          }
        }

        if( toroidalWinding == 1 && multipleIslands ) {
            debug5 << " REMOVED - Multiple Islands" << endl;
          toroidalWindingList.erase( toroidalWindingList.begin()+i );
          i--;
          continue;

        } else {
                
          if( i > 0 ) 
              debug5 << " REPLACED - Islands" << endl;
          else
              debug5 << " START - Islands" << endl;

          while ( i > 0 ) {
            toroidalWindingList.erase( toroidalWindingList.begin() );
            i--;
          }

          while ( toroidalWindingList.size() > 1 ) {
            toroidalWindingList.erase( toroidalWindingList.begin()+1 );
          }

          continue;
        }

      } else if( order != 2 && (toroidalWindingNextBest == 0 || islands > 0) ) {
          debug5 << endl;
        continue;

        // If very low order and has less than three points in
        // each group skip it.
      } else if( order != 2 && 2*toroidalWinding - skip == toroidalWindingNextBest &&
                 toroidalWinding < 5 && 
                 toroidalWindingNextBest/toroidalWinding < 2 ) {
              
        vector< unsigned int >::iterator inList =
          find( toroidalWindingList.begin(),
                toroidalWindingList.end(), toroidalWindingNextBest );
            
        // Found the next best in the list so delete the current one.
        if( inList != toroidalWindingList.end() ) {
            debug5 << " REMOVED - Too few points" << endl;
                
          toroidalWindingList.erase( toroidalWindingList.begin()+i );

          i--;
                
          continue;
        }
      } 

      if( order == 0 ) {

          debug5 << " KEEP - low" << endl;

        // Take the lower ordered surface.
        unsigned int toroidalWindingNextBestTmp = toroidalWindingNextBest;
        unsigned int islandTmp = islands;

        while( toroidalWindingNextBestTmp && islandTmp == 0 ) {
          vector< unsigned int >::iterator inList =
            find( toroidalWindingList.begin(),
                  toroidalWindingList.end(), toroidalWindingNextBestTmp );
            
          if( inList != toroidalWindingList.end() ) {

            toroidalWindingList.erase( inList );

            unsigned int toroidalWindingTmp = toroidalWindingNextBestTmp;
            unsigned int poloidalWindingTmp, skipTmp, typeTmp;
            float nnodesTmp;
            bool groupCCWTmp;
                  
            if( basicChecks( points, centroid,
                             toroidalWindingTmp, poloidalWindingTmp, skipTmp, typeTmp,
                             islandTmp, nnodesTmp, groupCCWTmp,
                             toroidalWindingNextBestTmp ) ) {
                    
                debug5 << "toroidalWindings " << toroidalWindingTmp
                       << "  poloidalWindings " << poloidalWindingTmp
                       << "  skip "   << skipTmp
                       << "  groupCCW " << groupCCWTmp
                       << "  islands " << islandTmp
                       << "  nnodess " << nnodesTmp
                       << "  next best toroidalWinding " << toroidalWindingNextBestTmp
                       << " REMOVED - low" << endl;           
            }

          } else {
            toroidalWindingNextBestTmp = 0;
          }
        }

      } else if( order == 1 ) {

        unsigned int toroidalWindingTmp = toroidalWindingNextBest;
        unsigned int poloidalWindingTmp, skipTmp, typeTmp, islandTmp;
        unsigned int toroidalWindingNextBestTmp;
        float nnodesTmp;
        bool groupCCWTmp;
                  
        if( basicChecks( points, centroid,
                         toroidalWindingTmp, poloidalWindingTmp, skipTmp, typeTmp,
                         islandTmp, nnodesTmp, groupCCWTmp,
                         toroidalWindingNextBestTmp ) ) {
                    
          // Basic philosophy - take the higher ordered surface
          // toroidalWinding which will give a smoother curve.        
          vector< unsigned int >::iterator inList =
            find( toroidalWindingList.begin(),
                  toroidalWindingList.end(), toroidalWindingNextBest );
                
          if( inList != toroidalWindingList.end() ) {
              debug5 << " REMOVED - high" << endl;
                  
            toroidalWindingList.erase( toroidalWindingList.begin()+i );
                  
            i--;
                  
          } else {
            debug5 << " KEEP - high" << endl;
          }
        }
      } else if( order == 2 ) {

        // Keep the toroidalWinding where the number of nodes for each
        // group is closest to being the same.
        //            float diff =
        //              (nnodes - floor(nnodes)) < (ceil(nnodes)-nnodes) ?
        //              (nnodes - floor(nnodes)) : (ceil(nnodes)-nnodes);

        // For the first keep is as the base toroidalWinding.
        if( i == 0 ) {
          bestNodes = nnodes;

          debug5 << " START " << endl;
                
        } else if( toroidalWindingList[0] <= 3 && bestNodes < 8 ) {
          bestNodes = nnodes;
          toroidalWindingList.erase( toroidalWindingList.begin() );
                
          i--;

          debug5 << " REPLACED " << endl;

          // The current toroidalWinding is the best so erase the first.
        } else if( bestNodes < nnodes ) {
          bestNodes = nnodes;
          toroidalWindingList.erase( toroidalWindingList.begin() );
                
          i--;

          debug5 << " REPLACED " << endl;

          // The first toroidalWinding is the best so erase the current.
        } else {
          toroidalWindingList.erase( toroidalWindingList.begin()+i );
                
          i--;
                
          debug5 << " REMOVED " << endl;
        }
      }
    }
  }

  if( toroidalWindingList.size() ) {
    toroidalWinding = toroidalWindingList[0];

    basicChecks( points, centroid,
                 toroidalWinding, poloidalWinding, skip, type,
                 islands, nnodes, groupCCW, toroidalWindingNextBest );

    // If the poloidalWindings is a factorial of the toroidalWinding then rebin the points.
    if( 0 &&
        toroidalWinding && poloidalWinding != 1 &&
        factorial( toroidalWinding, poloidalWinding ) ) {
        
      unsigned int fact;
      while( (fact = factorial( toroidalWinding, poloidalWinding )) ) {
        toroidalWinding /= fact;
        poloidalWinding /= fact;
      }
    }

  } else {
    toroidalWinding = 0;
    poloidalWinding = 0;
    skip = 0;
    type = 0;
    islands = 0;
    nnodes = 0;
    toroidalWindingNextBest = 0;
  }

  debug5 << "End of streamline " << 0 // fieldlineInfo.size()
            << "  type " << type 
            << "  toroidal windings " << toroidalWinding 
            << "  poloidal windings " << poloidalWinding
            << "  ("<< (double)toroidalWinding/(double)poloidalWinding << ")"
            << "  skip "   << skip
            << "  islands " << islands
            << "  nnodes " << nnodes
            << endl << endl;

  FieldlineInfo fi;

  fi.type = (FieldlineType) type;

  fi.toroidalWinding = toroidalWinding;
  fi.poloidalWinding = poloidalWinding;
  fi.skip = skip;
  fi.islands = islands;
  fi.nnodes = nnodes;

//  fi.confidence = confidence;

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
                     unsigned int skip,
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
    }
  }

  if( toroidalWinding == 1 )
    return nnodes;

  Vector v0 = (Vector) bins[0   ][1] - (Vector) bins[0][0];
  Vector v1 = (Vector) bins[skip][0] - (Vector) bins[0][0];

  // If the skip and point ordering are opposite in directions then the
  // previous group is the skip. Otherwise is they have the same
  // direction then toroidalWinding-skip is the previous group.
  int skipDir;

  if( Dot( v0, v1 ) < 0.0 )
    skipDir = 1;
  else
    skipDir = -1;

  // Second make sure none of the groups overlap each other.
  for( unsigned int i=0; i<toroidalWinding; i++ ) {

    // The previous group
    unsigned int j = (i + skipDir*skip + toroidalWinding) % toroidalWinding;

    // Check for a point in the previous group being between the first
    // two points in the current group.
    for( unsigned int k=0; k<nnodes; k++ ) {

      Vector v0 = (Vector) bins[i][0] - (Vector) bins[j][k];
      Vector v1 = (Vector) bins[i][1] - (Vector) bins[j][k];
      
      if( Dot( v0, v1 ) < 0.0 ) {
        nnodes = k;
      }
    }

    // Check for a point in the current group being between two points
    // in the previous group.
    for( unsigned int k=1; k<nnodes; k++ ) {

      Vector v0 = (Vector) bins[j][k  ] - (Vector) bins[i][0];
      Vector v1 = (Vector) bins[j][k-1] - (Vector) bins[i][0];
      
      if( Dot( v0, v1 ) < 0.0 ) {
        nnodes = k;
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
               unsigned int skip,
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

      if( toroidalWinding == 1 ) {

        unsigned int i = 0;

        // See if the first point overlaps another section.
        for( unsigned int  j=nnodes/2; j<bins[i].size(); j++ ) {
          if( Dot( (Vector) bins[i][j  ] - (Vector) bins[i][0],
                   (Vector) bins[i][j-1] - (Vector) bins[i][0] )
              < 0.0 ) {

//          cerr <<  "removeOverlap - First point overlaps another section after " << j-1 << endl;
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
//            cerr << "removeOverlap - A point overlaps the first section at " << j-1 << endl;
              nodes = j;

              completeIsland = true;
              break;
            }
          }
        }
      } else {
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

      // No more than one point should added.
      if( nodes > nnodes+1 )
      {
        debug5 << "Island " << i
             << " nnodes mismatch " << nnodes << "  " << nodes << endl;
      }

      // Erase all of the overlapping points.
      bins[i].erase( bins[i].begin()+nodes, bins[i].end() );
      
      // Close the island if it is complete
      if( completeIsland )
        bins[i].push_back( bins[i][0] );
    }

  } else {  // Surface

    // This gives the minimal number of nodes for each group.
    surfaceOverlapCheck( bins, toroidalWinding, skip, nnodes );
    
    if( nnodes == 0 ) {

      debug5 << "ZERO NODES " << endl;

      nnodes = bins[0].size();

      for( unsigned int i=1; i<toroidalWinding; i++ ) {
        if( nnodes > bins[i].size())
          nnodes = bins[i].size();
      }
    }

    for( unsigned int i=0; i<toroidalWinding; i++ ) {

      // Add back in any nodes that may not overlap.
      unsigned int nodes =
        surfaceGroupCheck( bins, i, (i+skip)%toroidalWinding, nnodes );

      // No more than one point should added.
      if( nodes > nnodes+1 )
        debug5 << "Surface " << i
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
             unsigned int skip,
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

          debug5 << i << " smooth " << j_1 << " "  << j << " "  << j1 << "  "
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
      unsigned int j = (i+skip)%toroidalWinding;

      // Insert the first point from the next toroidalWinding so the curve
      // is contiguous.
      bins[i].push_back( bins[j][0] );

      //for( unsigned int s=0; s<add; s++ )
      {
        unsigned int nodes = bins[i].size();

        vector<pair< Point, unsigned int >> newPts(add*nodes);

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
              
              debug5 << i << " insert " << j << "  " << newPts[k].first << endl;
              
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
              unsigned int skip,
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

        debug5 << i << " stored extra points " << tmp_bins[i].size() << endl;

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

          debug5 << i << "  " << modulo << "  " << j + nnodes
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

        debug5 << "ToroidalWinding " << i << " inserted "
             << prediction_true+prediction_false << " nodes "
             << " True " << prediction_true
             << " False " << prediction_false << endl;

        // If more of the predictions are incorrect than correct
        // insert based on the predictions.
        if( 0 && prediction_true < prediction_false ) {

          debug5 << "ToroidalWinding " << i << " bad predicted insertion ";

          unsigned int cc = 0;

          for( unsigned int j=0; j<tmp_bins[i].size(); j++ ) {

            vector< Point >::iterator inList =
              find( bins[i].begin(), bins[i].end(), tmp_bins[i][j] );
              
            if( inList != bins[i].end() ) {
              bins[i].erase( inList );

              cc++;
            }
          }

          debug5 << "removed " << cc << " points" << endl;

          unsigned int index = 1;
            
          for( unsigned int j=0; j<tmp_bins[i].size(); j++ ) {
            
            // Insert it between the other two.
            bins[i].insert( bins[i].begin()+index, tmp_bins[i][j] );

            debug5 << i << "  " << modulo << "  " << j + nnodes
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

                  debug5 << " merge self intersection " 
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

    vector<vector < Point >> tmp_bins(toroidalWinding);

    // This gives the minimal number of nodes for each group.
    surfaceOverlapCheck( bins, toroidalWinding, skip, nnodes );

    if( nnodes == 0 ) {

      for( unsigned int i=1; i<toroidalWinding; i++ ) {
        if( nnodes > bins[i].size())
          nnodes = bins[i].size();
      }
    }

    for( unsigned int i=0; i<toroidalWinding; i++ ) {

      // Add back in any nodes that may not overlap.
      unsigned int nodes =
        surfaceGroupCheck( bins, i, (i+skip)%toroidalWinding, nnodes );

      // No more than one point should added.
      if( nodes > nnodes+1 )
        debug5 << "Surface fill " << i
             << " nnodes mismatch " << nodes << endl;

      // Store the overlapping points.
      for( unsigned int j=nodes; j<bins[i].size(); j++ )
        tmp_bins[i].push_back( bins[i][j] );

      debug5 << i << " stored extra points " << tmp_bins[i].size() << endl;

      // Erase all of the overlapping points.
      bins[i].erase( bins[i].begin()+nodes, bins[i].end() );

      // Insert the first point from the next toroidalWinding so the curve
      // is contiguous.
      bins[i].push_back( bins[(i+skip)%toroidalWinding][0] );
    }


    for( unsigned int i=0; i<toroidalWinding; i++ ) {
      
      unsigned int toroidalWinding_prediction = (i+skip)%toroidalWinding;
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

        debug5 << "ToroidalWinding prediction " << toroidalWinding_prediction
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

      debug5 << "ToroidalWinding " << i << " inserted "
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

}
