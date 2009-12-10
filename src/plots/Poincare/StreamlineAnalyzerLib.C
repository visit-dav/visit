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

#define SIGN(x) ((x) < 0.0 ? (int) -1 : (int) 1)

//using namespace SCIRun;

Point FieldlineLib::interpert( Point lastPt, Point currPt, double t ) {

  return Point( Vector( lastPt ) + Vector( currPt - lastPt ) * t );
}


int FieldlineLib::ccw( Vector v0, Vector v1 ) {
    
  if( v0.x * v1.z - v0.z * v1.x > FLT_MIN ) return  1;    // CCW
  if( v0.z * v1.x - v0.x * v1.z > FLT_MIN ) return -1;    // CW
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
             unsigned int offset /* = 1 */ )
{

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


int
compareWindingSet( const pair < pair<unsigned int, unsigned int >, double > s0,
                   const pair < pair<unsigned int, unsigned int >, double > s1
                   )
{
  return (s0.second > s1.second );
}


void FieldlineLib::
poloidalWindingCheck( vector< unsigned int > &poloidalWindingSet,
                      vector< pair < pair<unsigned int,
                                          unsigned int >,
                                     double > > &windingSetList )
{
  windingSetList.clear();

  unsigned int nsets = poloidalWindingSet.size();

  float safetyfactor = (float) nsets / (float) poloidalWindingSet[nsets-1];

  if( verboseFlag )
    cerr << "Base safetyfactor " << safetyfactor << endl;

  // The premise is that for a given toroidal winding the poloidal
  // winding should be consistant between each Nth punction point,
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
      poloidalWindingAve += (poloidalWindingSet[i+toroidalWinding] -
                             poloidalWindingSet[i]);

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
          poloidalWindingSet[i+toroidalWinding] - poloidalWindingSet[i] )
        ++nMatches;
    }

    double matchPercent = (double) nMatches / (double) (nsets-toroidalWinding);

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

          // If the match percent happens to be higher for the higher
          // order keep it instead. Typically the lower order math is
          // better.
          if( windingSetList[i].second < matchPercent )
            windingSetList[i].second = matchPercent;
          
          break;
        }
      }
    }

    // Did not fina a lower order match so record the set.
    if( ! lowOrder )
    {
      pair < pair<unsigned int, unsigned int >, double >
        windingSetMatch( pair<unsigned int, unsigned int >(t, p),
                         matchPercent );

      windingSetList.push_back( windingSetMatch );
    }
  }

  // Now sort the results.
  sort( windingSetList.begin(), windingSetList.end(),
        compareWindingSet );
}


double FieldlineLib::
calculatePeriodVariance( vector< Point >& points,
                         unsigned int period,
                         bool zCheckOnly )
{
  if( period > points.size() / 2 )
    return 99999.9;

  // Find the variance for each of the periods. If the period is
  // correct the variance should be a small compared it a wrong
  // value.

  // Note: for a rational surface the variance should be zero.

  unsigned int nSamples = 0;

  double variance = 0;
    
  for( unsigned int i=0; i<period; ++i )
  {
    // Find the centroid of the points based on the poloidal winding.
    Vector centroid(0,0,0);
    unsigned int npts = 0;
    
    for( unsigned int j=i; j<points.size(); j+=period )
    {
      centroid += points[j];
      ++npts;
    }
    
    centroid /= (double) npts;

    // Get the variance for each group.
    for( unsigned int j=i; j<points.size(); j+=period )
    {
      Vector diff = points[j] - centroid;

      ++nSamples;

      if( zCheckOnly )
        variance += (diff.z * diff.z);
      else
        variance += diff.length2();
    }
  }

  return variance;
}

int
compareSecond( const pair< unsigned int, double > s0,
               const pair< unsigned int, double > s1 )
{
  return ( s0.second < s1.second );
}

unsigned int FieldlineLib::
poloidalWindingStats( unsigned int base_period,
                      vector< Point >& ridgelinePoints,
                      vector< Point >& poloidalPoints,
                      vector< pair< unsigned int,
                                    double > >& ridgelineSetList )
{
  ridgelineSetList.clear();

 // Find the base poloidal periodicity variance.
  unsigned int best_period = base_period;
  double best_var = calculatePeriodVariance( poloidalPoints, base_period );

  if( verboseFlag )
    cerr << "Base poloidal  " << best_period << "  " << best_var << "  ";

  // Find the period with the best variance.  Note dividing by two
  // insures that there are at least two points in each group.
 
  for( unsigned int i=1; i<=poloidalPoints.size()/2; ++i ) 
  {
    double test_var = calculatePeriodVariance( poloidalPoints, i );
    
    //         cerr << "Test poloidal  " << i  << "  " << test_var << endl;
    
    if( best_var > test_var ) 
    {
      best_var = test_var;
      best_period = i;
    }
  }
  
  cerr << "Best poloidal  " << best_period << "  " << best_var << endl;



 // Find the base poloidal periodicity variance.
  best_period = base_period;
  best_var = calculatePeriodVariance( ridgelinePoints, base_period, true );

  if( verboseFlag )
    cerr << "Base ridgeline  " << base_period << "  " << best_var << "  ";

  // Find the period with the best variance.  Note dividing by two
  // insures that there are at least two points in each group.
  
  for( unsigned int i=1; i<=ridgelinePoints.size()/2; ++i ) 
  {
    double test_var = calculatePeriodVariance( ridgelinePoints, i, true );

    ridgelineSetList.push_back( pair< unsigned int, double >
                                (i, test_var ) );

    //    cerr << "Test ridgeline  " << i << "  " << test_var << endl;

    if( best_var > test_var ) 
    {
      best_var = test_var;
      best_period = i;
    }
  }

  cerr << "Best ridgeline  " << best_period << "  " << best_var << endl;

  // Now sort the results.
  sort( ridgelineSetList.begin(), ridgelineSetList.end(), compareSecond );


  // Debug info
  if( verboseFlag )
  {  
    for( unsigned int i=0; i<10 && i<ridgelineSetList.size(); ++i )
    {
      if( ridgelineSetList[i].second > 10.0 * ridgelineSetList[0].second )
        break;

      cerr << "tested period  " << ridgelineSetList[i].first << "  "
           << "variance  " << ridgelineSetList[i].second << "  "
           << endl;
    }
  }

  // Z max test.
  unsigned int period = 0;
  unsigned int cc = 0;
  unsigned int index = 0;

  cerr << "max test ";

  double dc = 0;
  for( unsigned int i=0; i<ridgelinePoints.size(); ++i ) 
    dc += ridgelinePoints[i].z;
  
  dc /= (float) ridgelinePoints.size();

  for( unsigned int j=1; j<ridgelinePoints.size()-1; ++j )
  {
    if( ridgelinePoints[j].z > dc &&
        ridgelinePoints[j].z >  ridgelinePoints[j-1].z &&
        ridgelinePoints[j].z >= ridgelinePoints[j+1].z )
    {
      if( index == 0 )
        index = j;
      else
      {
        cerr << (j - index) << "  ";
        
        period += (j - index);
        index = j;
        ++cc;
      }
    }
  }

  period = (( (float) period / (float) cc ) + 0.5);
  cerr << "average period " << period << endl;
  
  return best_period;
}


bool FieldlineLib::
rationalCheck( vector< Point >& points,
               unsigned int toroidalWinding,
               unsigned int &islands,
               float &avenode,
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

  islands = 0;
  avenode = 1;

  return true;
}


bool FieldlineLib::
islandChecks( vector< Point >& points,
              unsigned int toroidalWinding,
              unsigned int &islands,
              float &nnodes,
              bool &complete )
{
  islands = 0;
  nnodes = 0;
  complete = 0;

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

    convex = hullCheck( points, (unsigned int) npts );

  } else {

    convex = hullCheck( points, toroidalWinding );
  }

//  cerr << "Convex  " << convex << endl;

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
 
      // A convex hull for a toroidalWinding of 1 is defined as not being an
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

//      cerr << "localCentroid  " << localCentroid << endl;

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
//       << Pxx << "  " << Pzz << "  " << Pxx/Pzz << "  " << Pzz/Pxx << endl;

      // For a toroidalWinding of 1 if the moment of interia of one axis is
      // similar to the other axis then use the centroid directly.
      if( toroidalWinding == 1 && Pxx / Pzz < 5.0 && Pzz / Pxx < 5.0 ) {
          baseCentroid = localCentroid;

//          cerr << "Using local centroid " << endl;

      } else {

        // Find the approximate center if point were projected onto a
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

//        cerr << "center  " << center << endl;

       // Use the principal axes to get an offset from the local
        // centroid which gives a point outside the island.

        // The direction along the axis is determined by where the
        // center is located.

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

    bool lastCCW = (ccw( v0, v1 ) == 1);
    v0 = v1;

    // Get the direction based on the remaining points.
    for( unsigned int j=i+2*toroidalWinding; j<points.size(); j+=toroidalWinding ) {
      v1 = (Vector) points[j] - baseCentroid;

      bool localCCW;

      // Points are parallel - this result should not occur as the
      // base centroid point should on the concave side of the island.
      if (ccw( v0, v1 ) == 0 )
        localCCW = lastCCW;
      else
        localCCW = (ccw( v0, v1 ) == 1);

      // A change in direction indicates that an island is present.
      if( localCCW != lastCCW ) {

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

      } else {
        v0 = v1;
      }
    }


    unsigned int overlap = 0;

    // With no or one turn the island has no overlaps.
    if( turns < 2 ) {
      nodes[i] = points.size() / toroidalWinding;

    } else {

      // Approximate number of points in the islands. 
      if( turns == 2 ) {
        nodes[i] = points.size() / toroidalWinding;

      } else if( turns == 3 ) {
        // Okay initial approximatation. It really depending on where
        // the turns are relative to each other/
        nodes[i] = (lastTurn - firstTurn) / toroidalWinding + 1;
      }

      // Look for the overlap between the ends of the island which are
      // flatter. Further, the offset must be an integer value of
      // the toroidal winding. 
      unsigned int offset = (((firstTurn + midTurn) / 2)
                             / toroidalWinding) * toroidalWinding;

//       if( turns && verboseFlag )
//         cerr << "Island " << i << " has "
//              << turns << " turns with "
//              << nodes[i]
//              << " nodes, offset " << offset
//              << " firstTurn " << firstTurn
//              <<   " midTurn " << midTurn
//              <<  " lastTurn " << lastTurn
//              <<  " complete " << complete
//              << endl;

      // Check to see if the island overlaps itself. Only use this if
      // there are two or more turns. Otherwise the it can appear that
      // it overlaps itself earlier.
      for( unsigned int j=midTurn, k=j+toroidalWinding;
           k<points.size()-offset;
           j+=toroidalWinding, k+=toroidalWinding ) {

        // See if the test point is between the first secton.
        v0 = ((Vector) points[i                +offset] -
              (Vector) points[k                +offset]);
        v1 = ((Vector) points[i+toroidalWinding+offset] -
              (Vector) points[k                +offset]);
        
        if( Dot( v0, v1 ) < 0.0 ) {
          overlap = k;
          nodes[i] = k / toroidalWinding;
          turns = 3;
          complete = true;
          break;
        }

        // See if the first point is between the test section.
        v0 = (Vector) points[i+offset] - (Vector) points[j+offset];
        v1 = (Vector) points[i+offset] - (Vector) points[k+offset];
        
        if( Dot( v0, v1 ) < 0.0 ) {
          overlap = k;
          nodes[i] = k / toroidalWinding;
          turns = 3;
          complete = true;
          break;
        }
      }
    }
    
    nnodes += nodes[i];
    
    if( turns && verboseFlag )
      cerr << "Island " << i << " has "
           << turns << " turns with "
           << nodes[i]
           << " nodes, overlap node " << overlap
           << " firstTurn " << firstTurn
           <<   " midTurn " << midTurn
           <<  " lastTurn " << lastTurn
           <<  " complete " << complete;

    // Started on a surface but jumpped to an island - i.e. a
    // separtrice
    if( !complete && lastTurn &&
        (firstTurn / toroidalWinding - nodes[i]/2) > 2 )
    {
      --islands;

      if( turns && verboseFlag )
        cerr << "  !!!!!!!!!!!!!!! Separatrice !!!!!!!!!!!!!!!";
    }

    if( turns && verboseFlag )
      cerr << endl;
  }

  // Get the average number of nodes.
  nnodes /= (float) toroidalWinding;

  if( islands ) {
    for( unsigned int i=0; i<toroidalWinding; i++ ) {
      if( nodes[i] < nnodes - 1 || nnodes + 1 < nodes[i] ) {
          if( verboseFlag )
            cerr << "Appears to be islands but not self consistant, "
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
                                   float matchPercentLimit )
{
  vector< unsigned int > safetyFactorSet;
  safetyFactorSet.clear();

  // First find out how many toroidal windings it takes to get back to the
  // starting point at the Phi = 0 plane. 

  vector< Point > toroidal_points;
  vector< Point > poloidal_points;
  vector< Point > ridgeline_points;

  toroidal_points.clear();
  poloidal_points.clear();
  ridgeline_points.clear();

  if( verboseFlag )
    cerr << "Analyzing  " << ptList[0] << "  "
         << "with  " << ptList.size() << " fieldline points"
         << endl;

  unsigned int npts = 0;
  float delta = 0.0;
  float zDelta = 0.0;

  Point lastPt, currPt;

  // Set up the Y plane equation as the base analysis takes place in
  // the X-Z plane.
  Vector planeNY( 0, 1, 0 );
  Vector planePt( 0, 0, 0 );

  double planeY[4];
      
  double lastDistY, currDistY;

  // Get the centroid of the puncture points.
  Vector centroid[4];

  for( unsigned int cc=0; cc<4; ++cc)
  {
    if( cc == 0 )
      planeNY = Vector( 1, 0, 0 );
    else if( cc == 1 )
      planeNY = Vector( -1, 0, 0 );
    else if( cc == 2 )
      planeNY = Vector( 0, -1, 0 );
    else if( cc == 3 )
      planeNY = Vector( 0, 1, 0 );

    planeY[0] = planeNY.x;
    planeY[1] = planeNY.y;
    planeY[2] = planeNY.z;
    planeY[3] = Dot( planePt, planeNY );

    currPt = ptList[0];
    currDistY = Dot(planeNY, currPt) - planeY[3];

    centroid[cc] = Vector(0,0,0);
    npts = 0;
    
    for( unsigned int i=1; i<ptList.size(); ++i)
    {
      lastPt = currPt;
      currPt = ptList[i];
      
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
        
          centroid[cc] += (Vector) point;
          ++npts;
        }
      }
    }

    centroid[cc] /= (float) npts;
  }


  Vector normalZ0 = Cross(centroid[3] - centroid[0], centroid[2] - centroid[0]);
  Vector normalZ1 = Cross(centroid[2] - centroid[1], centroid[3] - centroid[1]);

  normalZ0.normalize();
  normalZ1.normalize();

  Vector normalZ = (normalZ0 + normalZ1) / 2.0;

  // Set up the Z plane equation. Because the centroid of the puncture
  // plot is not on the z axis use it as the normal for the z plane.
  Vector planeNZ = normalZ;
  double planeZ[4];
      
  planeZ[0] = planeNZ.x;
  planeZ[1] = planeNZ.y;
  planeZ[2] = planeNZ.z;
  planeZ[3] = Dot(planePt, planeNZ);

  double lastDistZ, currDistZ = Dot(planeNZ, currPt) - planeZ[3];

  planeNY = Vector( 0, 1, 0 );

  // Now collect the points.
  currPt = ptList[0];
  currDistY = Dot(planeNY, currPt) - planeY[3];

  npts = 0;

  bool haveFirstIntersection = false;
  double maxZ = 0;

  for( unsigned int i=1; i<ptList.size(); ++i)
  {
    lastPt = currPt;
    currPt = ptList[i];

    // Save the distance between points to use for finding periodic
    // fieldlines (i.e. rational surfaces).
    Vector vec = (Vector) lastPt - (Vector) currPt;

    zDelta += fabs( vec.z );
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

    // Poloidal plane distances.
    lastDistZ = currDistZ;
    currDistZ = Dot( planeNZ, currPt ) - planeZ[3];
    
    // Find the positive zero crossings which indicate a poloidal
    // winding. Do this check after the first toroidal puncture.
    if( toroidal_points.size() )
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
        
          poloidal_points.push_back( point );

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

  // Find the mean value and variance of the ridgeline curve.
  double ridgeline_dc = 0;
  for( unsigned int i=0; i<ridgeline_points.size(); ++i ) 
    ridgeline_dc += ridgeline_points[i].z;
  
  ridgeline_dc /= (float) ridgeline_points.size();
  
  double ridgeline_dc_var = 0;
  for( unsigned int i=0; i<ridgeline_points.size(); ++i ) 
    ridgeline_dc_var += ((ridgeline_points[i].z - ridgeline_dc) *
                         (ridgeline_points[i].z - ridgeline_dc));
  
  // At this point all of the puncture points (toroidal points) as
  // well as the poloidal points have been found.
    if( verboseFlag )
      cerr << "analyzing with "
           << toroidal_points.size() << " toroidal points, "
           << poloidal_points.size() << " poloidal points, and "
           << ridgeline_points.size() << " ridgeline points " << endl;

  if( ptList.size() == 0 ||
      toroidal_points.size() == 0 ||
      poloidal_points.size() == 0 ||
      ridgeline_points.size() == 0 )
  {
    FieldlineInfo fi;

    fi.type = UNKNOWN;

    fi.toroidalWinding = 0;
    fi.poloidalWinding = 0;
    fi.skip    = 0;
    fi.islands = 0;
    fi.nnodes  = 0;

    fi.confidence        = 0;
    fi.complete          = 0;
    fi.nPuncturesNeeded  = 0;
    fi.ridgelinePeriod   = 0;
    fi.ridgelineVariance = 0;

    return fi;
  }

  // Get the average distance between points.
  zDelta /= (float) npts;
  delta  /= (float) npts;

  if( verboseFlag )
    cerr << "Delta values (z, overall)  " << zDelta << "  " << delta << endl;

  FieldlineType type = UNKNOWN;

  unsigned int toroidalWinding = 0, poloidalWinding = 0;
  unsigned int skip, islands;
  float nnodes, confidence = 0, ridgelineVariance = 0;

  bool complete = 0;
  unsigned int nPuncturesNeeded = 0;
  unsigned int ridgelinePeriod = 0;

  unsigned int toroidalMatchIndex = -1;
  unsigned int poloidalMatchIndex = -1;

  vector< pair < pair<unsigned int, unsigned int >, double > >
    windingSetList;

  // The user has set the toroidal winding get the poloidal winding
  // based on the data found.
  if( override ) 
  {
    toroidalWinding = override;

    // Get the average value of the poloidal winding.
    double poloidalWindingAve = 0;

    unsigned int npts = safetyFactorSet.size();

    for( unsigned int i=0; i<npts-toroidalWinding; ++i)
      poloidalWindingAve += (safetyFactorSet[i+toroidalWinding] -
                             safetyFactorSet[i]);

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
          safetyFactorSet[i+toroidalWinding] - safetyFactorSet[i] )
        ++nMatches;
    }

    double matchPercent = (double) nMatches / (double) (npts-toroidalWinding);

    skip = Blankinship( toroidalWinding, poloidalWinding );

    if( verboseFlag )
      cerr << "final toroidalWinding/poloidal  "
           << toroidalWinding << "  "
           << poloidalWinding << "  ("
           << (double)toroidalWinding/(double)poloidalWinding << ")  "
           << "consistency "
           << 100.0 * matchPercent
           << " percent" << endl;
  } 
  else 
  {
    // Check the consistancy of the safety factor set. 
    poloidalWindingCheck( safetyFactorSet, windingSetList );

    // Get the first set that passes the intersection test and passes
    // the user setable match limit. Default is 0.90 (90%)
    for( unsigned int i=0; i<windingSetList.size(); ++i )
    {
      if( windingSetList[i].first.first <= maxToroidalWinding &&
          windingSetList[i].second >= matchPercentLimit &&
          IntersectCheck( toroidal_points,
                          windingSetList[i].first.first ) )
      {
        toroidalMatchIndex = i;

        toroidalWinding = windingSetList[i].first.first;
        poloidalWinding = windingSetList[i].first.second;

        skip = Blankinship( toroidalWinding, poloidalWinding );

        // Base value from the periodicity check. This check is quite
        // accurate for stable systems.
        confidence = 0.40;

        // If the poloidal winding check was really good bump up the
        // confidence.
        if( windingSetList[i].second > 0.99 )
          confidence += 0.10;

        if( verboseFlag )
          cerr << "using  toroidal / poloidal Winding  "
               << windingSetList[i].first.first << " / "
               << windingSetList[i].first.second << "  ("
               << ((double) windingSetList[i].first.first /
                   (double) windingSetList[i].first.second) << ")  "
               << "consistency "
               << 100.0 * windingSetList[i].second
               << " percent ************" << endl;
        
        break;
      }
      else
      {
        // Debug info
        if( verboseFlag )
          cerr << "tested toroidal / poloidal Winding  "
               << windingSetList[i].first.first << "   "
               << windingSetList[i].first.second << "  ("
               << ((double) windingSetList[i].first.first /
                   (double) windingSetList[i].first.second) << ")  "
               << "consistency " << 100.0 * windingSetList[i].second
               << " percent  "
               << endl;

        if( windingSetList[i].second < matchPercentLimit )
          break;
      }
    }

    // Match consistancy is less than the user set value. Run more
    // expensive tests to identify the fieldline.
    if( toroidalMatchIndex == -1 )
    {
      if( verboseFlag )
        cerr << "Poor consistancy - probably chaotic" << endl;

      toroidalWinding = 0;
      poloidalWinding = 0;

      nnodes = 0;

      skip = 0;
      islands = 0;

      complete = 0;
      nPuncturesNeeded = 0;

      confidence = 0;
      ridgelinePeriod = 0;

      type = CHAOTIC;
    }
  }

  if( type != CHAOTIC )
  {

    // Remove the winding sets that are after the best.
    unsigned int i = 0;

    vector< pair < pair<unsigned int, unsigned int >, double > >::iterator
      iter = windingSetList.begin();
    
    while( i<toroidalMatchIndex+1 ) { ++i; ++iter; }
    
    if( i < windingSetList.size() )
    {
      // Keep the next one plus the next ones that are 2 percent below.
      if( verboseFlag )
        cerr << "tested toroidal / poloidal Winding  "
             << windingSetList[i].first.first << "   "
             << windingSetList[i].first.second << "  ("
             << ((double) windingSetList[i].first.first /
                 (double) windingSetList[i].first.second) << ")  "
             << "consistency " << 100.0 * windingSetList[i].second
             << " percent  "
             << endl;

      double cutoff = windingSetList[i].second;

      if( windingSetList[i].second >= 0.9 )
        cutoff -= 0.02;
      else if( windingSetList[i].second >= 0.8 )
        cutoff -= 0.01;

      for( ++i, ++iter; i<windingSetList.size(); ++i, ++iter )
      {
        if( windingSetList[i].second < cutoff ||
            windingSetList[i].second < 0.60 )
          break;
      
        // Debug info
        if( verboseFlag )
          cerr << "tested toroidalWinding/poloidalWinding  "
               << windingSetList[i].first.first << "   "
               << windingSetList[i].first.second << "  ("
               << ((double) windingSetList[i].first.first /
                   (double) windingSetList[i].first.second) << ")  "
               << "consistency " << 100.0 * windingSetList[i].second
               << " percent  "
               << endl;
      }
    
      if( i < windingSetList.size() )
        windingSetList.erase( iter, windingSetList.end() );
    }

    // Check to see if the fieldline is periodic. I.e. on a rational
    // surface.  If within 1/10 of the distance the fieldline is
    // probably on a rational surface.
    if( rationalCheck( toroidal_points, toroidalWinding,
                       islands, nnodes, delta/2.0 ) ) 
    {
      type = RATIONAL;
      complete = true;
      
      if( verboseFlag )
        cerr << "Appears to be a rational surface "
             << delta/10.0 << endl;

      vector< pair< unsigned int, double > > ridgelineList;

      // Find the best poloidal periodicity based on the ridge points.
      ridgelinePeriod = poloidalWindingStats( poloidalWinding,
                                              ridgeline_points,
                                              poloidal_points,
                                              ridgelineList );

      ridgelineVariance =
        (ridgeline_dc_var-ridgelineList[0].second)/ridgeline_dc_var;

      if( verboseFlag )
        cerr << "ridgeline variance comparison  "
             << "baseline " << ridgeline_dc_var << "  "
             << "ridgeline " << ridgelineList[0].second << "  "
             << "percent " << ridgelineVariance << endl;

      if( ridgeline_dc_var < 1.0e-04 )
      {
        if( verboseFlag )
          cerr << endl << "!!!!!!!!!!!!!!!!!!!!!!  "
               << "Ridgeline variance appears to be too large for a rational surface. "
                   << "!!!!!!!!!!!!!!!!!!!!!!" << endl;
      }
    }
  
    // Check to see if the fieldline creates a set of islands.
    else if( islandChecks( toroidal_points, toroidalWinding,
                           islands, nnodes, complete ) ) 
    {
      type = ISLAND_CHAIN;
      
      // The number of nodes is set so that when connected the islands
      // are complete. The poloidal winding times the number of nodes is
      // also the valve at which points start repeating so it is similar
      // the poloidal winding value for an irrational surface.
      nnodes = floor(nnodes+0.5);

      // Find the base island poloidal periodicity standard deviation.
      unsigned int basePoloidalWinding =
        poloidalWinding * (unsigned int) nnodes;

 
      // Decide if there are enough points.
      if( !complete ||
          ridgeline_points.size() < 2 * basePoloidalWinding )
      {
        nPuncturesNeeded =
          (2 * basePoloidalWinding + 1) * toroidalWinding / poloidalWinding + 1;

        if( verboseFlag )
          cerr << "Too few poloidal points, at least "
               << nPuncturesNeeded - 1
               << "  puncture points are needed"
               << endl;
      }

      vector< pair< unsigned int, double > > ridgelineList;

      // Find the best poloidal periodicity.
      ridgelinePeriod =
        poloidalWindingStats( poloidalWinding * (unsigned int) nnodes,
                              ridgeline_points,
                              poloidal_points,
                              ridgelineList );


      ridgelineVariance =
        (ridgeline_dc_var-ridgelineList[0].second)/ridgeline_dc_var;

      cerr << "ridgeline variance comparison  " << ridgelineVariance << endl;

      // If the best island poloidal winding value is the poloidal
      // winding value times the number of nodes then there is self
      // consistancy. Note allow for +/-1 of the number of nodes as it
      // can be difficult to determine the last node in an island chain.
      for( int i=-1; i<=1; ++i )
      {
        // If the best is the same value of the base then a perfect match.
        if( ridgelinePeriod ==
            (poloidalWinding * (unsigned int) (nnodes+i)) )
        {
          confidence += 0.40;
          
          nnodes += i;

          if( verboseFlag )
          {
            cerr << "Exact Island poloidal  ";
            if(i)  cerr << i << "  ";
            cerr << ridgelinePeriod << endl;
          }

          break;
        }

        // If the best is an integer value of the base then the result is
        // the same because more groups can create smaller regions.
        else if( ridgelinePeriod %
            (poloidalWinding * (unsigned int) (nnodes+i)) == 0 )
        {
          confidence += 0.30;
          
          nnodes += i;
          
          if( verboseFlag )
          {
            cerr << "Integer Island poloidal via nodes ";
            if(i)  cerr << i << "  ";
            cerr << ridgelinePeriod << endl;
          }

          break;
        }

        // If the best is an integer value of the base then the result is
        // the same because more groups can create smaller regions.
        else if( (ridgelinePeriod + i) %
                 (poloidalWinding * (unsigned int) nnodes) == 0 )
        {
          confidence += 0.20;
          
          if( verboseFlag )
          {
            cerr << "Integer Island poloidal via ridgeline ";
            if(i)  cerr << i << "  ";
            cerr << ridgelinePeriod << endl;
          }

          break;
        }
      }

      if( confidence < 0.60 )
      {
        // If the best is an integer value of the base then the result
        // is the same because more groups can create smaller regions.
        unsigned int cc = 0;

        while( cc < ridgelineList.size() &&
               (ridgelineList[cc].first % poloidalWinding == poloidalWinding-1 ||
                ridgelineList[cc].first % poloidalWinding == 0 ||
                ridgelineList[cc].first % poloidalWinding == 1) )
          ++cc;

        if( cc )
        {
          type = UNKNOWN;
          islands = 0;
//            nPuncturesNeeded = 0;
          
          unsigned int i = 0;
          vector< pair < pair<unsigned int, unsigned int >, double > >::iterator
            iter = windingSetList.begin();

          // Erase everything up to but not including the current match.
          if( i<toroidalMatchIndex )
          {
            while( i<toroidalMatchIndex ) { ++i; ++iter; }
            
            windingSetList.erase(windingSetList.begin(), iter);
          }

          // Erase everything after the current match.
          ++i;
          ++iter;

          if( i < windingSetList.size() )
            windingSetList.erase( iter, windingSetList.end() );

          if( verboseFlag )
              cerr << endl << "!!!!!!!!!!!!!!!!!!!!!!  "
                   << "Probably not an island assuming a surface - poloidal surface matched " << cc << "  "
                   << "!!!!!!!!!!!!!!!!!!!!!!" << endl;

          toroidalMatchIndex = 0;
          poloidalMatchIndex = -1;
        }

        if( islands && confidence < 0.70 && verboseFlag )
          cerr << endl << "!!!!!!!!!!!!!!!!!!!!!!  "
               << "Best island poloidal  " << ridgelinePeriod << "  "
               << "!!!!!!!!!!!!!!!!!!!!!!" << endl;
      }
    }

    // At this point assume the surface is irrational.
    if( type == UNKNOWN )
    {
      type = IRRATIONAL;
      
      // Find the first point from another group that overlaps the first
      // group. This only works if there is an overlap between groups.
      nnodes = toroidal_points.size() / toroidalWinding;
      
      // Get the direction based on the first two points in a group.
      Vector v0 = (Vector) toroidal_points[toroidalWinding] -
        (Vector) toroidal_points[0];

      // Get the direction based on the first points from adjacent groups.
      Vector v1 = (Vector) toroidal_points[skip] -
        (Vector) toroidal_points[0];
      
      // If the skip and point ordering are opposite in directions
      // then the previous group is the skip. Otherwise is they have
      // the same direction then toroidalWinding-skip is the previous
      // group.
      int skipDir = (Dot( v0, v1 ) > 0.0 ? 1 : -1);
      
      for( unsigned int i=0; i<toroidalWinding; ++i ) 
      {
        // The next group
        unsigned int j = (i+skipDir*skip + toroidalWinding) % toroidalWinding;
        
        Vector firstPoint = (Vector) toroidal_points[j];
        Vector nextPoint  = (Vector) toroidal_points[j+toroidalWinding];
        
        Vector v0 = nextPoint - firstPoint;

        for( unsigned int k=i; k<toroidal_points.size(); k+=toroidalWinding ) 
        {
          Vector  testPoint = (Vector) toroidal_points[k];
        
          Vector v1 = testPoint - firstPoint;
        
          if( (poloidalWinding > 2 || v1.length() / v0.length() < 5.0)
              && Dot( v0, v1 ) > 0.0 )
          {
             complete = true;
            
             nnodes = k / toroidalWinding;

            break;
          }
          else
             complete = false;
        }
      }

      if( !complete )
      {
        unsigned int additionalPts = 0;

        // Get the direction based on the first two points in a group.
        Vector v0 = (Vector) toroidal_points[toroidalWinding] -
          (Vector) toroidal_points[0];

        // Get the direction based on the first points from adjacent groups.
        Vector v1 = (Vector) toroidal_points[skip] -
          (Vector) toroidal_points[0];

        // If the skip and point ordering are opposite in directions
        // then the previous group is the skip. Otherwise is they have
        // the same direction then toroidalWinding-skip is the previous
        // group.
        int skipDir = (Dot( v0, v1 ) > 0.0 ? 1 : -1);

        for( unsigned int i=0; i<toroidalWinding; ++i ) 
        {
          // The next group
          unsigned int j = (i+skipDir*skip + toroidalWinding) % toroidalWinding;

          Vector firstPoint = (Vector) toroidal_points[j];
          Vector nextPoint  = (Vector) toroidal_points[j+toroidalWinding];
          
          Vector  lastPoint =
            (Vector) toroidal_points[i+(nnodes-1)*toroidalWinding];
          Vector  prevPoint =
            (Vector) toroidal_points[i+(nnodes-2)*toroidalWinding];
          
          Vector v0 = nextPoint - firstPoint;
          Vector v1 = lastPoint - firstPoint;

          if( Dot( v0, v1 ) < 0.0 )
          {
            unsigned int needPts = ( (firstPoint-lastPoint).length() /
                                     (prevPoint-lastPoint).length() + 0.5 );
          
            if( additionalPts < needPts )
              additionalPts = needPts;
          }
        }

        complete = additionalPts ? false : true;

        if( additionalPts )
        {
          nPuncturesNeeded = (nnodes+additionalPts) * toroidalWinding + 1;

          if( verboseFlag )
            cerr << "Too few toroidal points, at least "
                 << (nnodes+additionalPts) * toroidalWinding
                 << "  puncture points are needed."
                 << endl;
        }
      }

      vector< pair< unsigned int, double > > ridgelineList;

      // Find the best poloidal periodicity based on the ridge points.
      ridgelinePeriod = poloidalWindingStats( poloidalWinding,
                                              ridgeline_points,
                                              poloidal_points,
                                              ridgelineList );
      ridgelineVariance =
        (ridgeline_dc_var-ridgelineList[0].second)/ridgeline_dc_var;

      cerr << "ridgeline variance comparison  " << ridgelineVariance << endl;

      if( ridgelineVariance < 0.9 && nPuncturesNeeded == 0 )
      {
        nPuncturesNeeded = toroidal_points.size() * 2;

        if( verboseFlag )
          cerr << "Too few toroidal points, at least "
               << nPuncturesNeeded
               << "  puncture points are needed for ridgeline analysis."
               << endl;
        }
    }

    // Determine the confidence of the fieldline analysis
    if( type == RATIONAL || type == IRRATIONAL )
    {
      for( unsigned int i=0; i<windingSetList.size(); ++i )
      {
        if( ridgelinePeriod == windingSetList[i].first.second )
        {
          confidence += 0.40;

          poloidalMatchIndex = i;

          if( verboseFlag )
            cerr << "Exact Poloidal Winding " << ridgelinePeriod
                 << endl;

          break;
        }
      }

      if( poloidalMatchIndex == -1 )

      for( unsigned int i=0; i<windingSetList.size(); ++i )
      {
        // If the best is an integer value of the base then the result
        // is the same because more groups can create smaller regions.
        if( ridgelinePeriod % windingSetList[i].first.second == 0 )
        {
          confidence += 0.30;
        
          poloidalMatchIndex = i;

          if( verboseFlag )
            cerr << "Integer Poloidal Winding " << ridgelinePeriod
                 << endl;

          break;
        }

        // If the best is an integer value of the base then the result
        // is the same because more groups can create smaller regions.
        // Note allow for +/-1 of the number of windings as it can be
        // difficult to determine the last winding point.
        else if( ridgelinePeriod % windingSetList[i].first.second ==
                 windingSetList[i].first.second-1 ||
                 ridgelinePeriod % windingSetList[i].first.second == 1 )
        {
          confidence += 0.20;
        
          poloidalMatchIndex = i;

          if( verboseFlag )
            cerr << "Integer +/-1 Poloidal Winding " << ridgelinePeriod
                 << endl;

          break;
        }
      }

      cerr << "toroidalMatchIndex  " << toroidalMatchIndex << "  "
           << "poloidalMatchIndex  " << poloidalMatchIndex << endl;

      if( poloidalMatchIndex == toroidalMatchIndex )
        confidence += 0.10;

      // In this case the toroidal winding could be higher order than
      // the user desired or it could have failed the intersection test.
      else if( poloidalMatchIndex < toroidalMatchIndex )
      {
        if( windingSetList[poloidalMatchIndex].first.first >
            maxToroidalWinding &&
            IntersectCheck( toroidal_points,
                            windingSetList[poloidalMatchIndex].first.first ) )
        {
          if( verboseFlag )
            cerr << endl << "!!!!!!!!!!!!!!!!!!!!!!  "
                 << "Higher order may be better  "
                 << windingSetList[poloidalMatchIndex].first.first << "  "
                 << windingSetList[poloidalMatchIndex].first.second << "  "
                 << "!!!!!!!!!!!!!!!!!!!!!!" << endl;
        }
      }

      // In this case it is possible that the toroidal matching for
      // two set is basically the same. Yet the poloidal matching is
      // better so take it instead.
      else if( poloidalMatchIndex > toroidalMatchIndex )
      {
        float diff = (windingSetList[toroidalMatchIndex].second -
                      windingSetList[poloidalMatchIndex].second ) / 
          windingSetList[toroidalMatchIndex].second;

        if( windingSetList[poloidalMatchIndex].first.first <
            maxToroidalWinding &&
            IntersectCheck( toroidal_points,
                            windingSetList[poloidalMatchIndex].first.first ) &&
            diff < 0.02 )
        {
          if( verboseFlag )
            cerr << endl << "!!!!!!!!!!!!!!!!!!!!!!  "
                 << "Poloidal match is better overall  "
                 << "!!!!!!!!!!!!!!!!!!!!!!" << endl;

          toroidalMatchIndex = poloidalMatchIndex;
          toroidalWinding = windingSetList[poloidalMatchIndex].first.first;
          poloidalWinding = windingSetList[poloidalMatchIndex].first.second;
        }          
      }
    }
  }

  FieldlineInfo fi;

  fi.type = type;

  fi.toroidalWinding = toroidalWinding;
  fi.poloidalWinding = poloidalWinding;
  fi.skip    = skip;
  fi.islands = islands;
  fi.nnodes  = nnodes;

  fi.confidence        = confidence;
  fi.complete          = complete;
  fi.nPuncturesNeeded  = nPuncturesNeeded;
  fi.ridgelinePeriod   = ridgelinePeriod;
  fi.ridgelineVariance = ridgelineVariance;

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
      bins[i].erase( bins[i].begin()+nodes, bins[i].end() );
      
      // Close the island if it is complete
      if( completeIsland )
        bins[i].push_back( bins[i][0] );
    }

  } else {  // Surface

    // This gives the minimal number of nodes for each group.

    surfaceOverlapCheck( bins, toroidalWinding, skip, nnodes );
    
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
        surfaceGroupCheck( bins, i, (i+skip)%toroidalWinding, nnodes );

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
      unsigned int j = (i+skip)%toroidalWinding;

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

}
