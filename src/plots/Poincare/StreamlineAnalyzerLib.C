/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2004 Scientific Computing and Imaging Institute,
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
 *  Copyright (C) 2006 SCI Group
 */

/* NOTES:
windingNumber = number of revs. to get back to begining.

*/

//#include <Packages/FusionPSE/Dataflow/Modules/Fields/StreamlineAnalyzerLib.h>

#include "StreamlineAnalyzerLib.h"
#include <DebugStream.h>
#include <vector>
#include "Point.h"
#include <Vector.h>
#include <Utils.h>
#include <float.h>
#include <iostream>

using namespace std;
using namespace SLIVR;

// Basic interface between the outside world and the base libs.
void getFieldlineProperties( const vector<avtVector> &pts,
                             unsigned int maxWindings,
                             unsigned int *windingRet,
                             unsigned int *twistRet,
                             unsigned int *islandRet )
{
    // Move all of the points over to C++ structures which makes the
    // library interface easier to maintain.
    vector< Point > points;

    for( unsigned int i=0; i<pts.size(); i++ ) 
        points.push_back( Point( pts[i].x, pts[i].y, pts[i].z ) );


    unsigned int winding, twist, skip, type, island, windingNextBest;
    float avenode;
    
    FusionPSE::FieldlineLib fl;

    fl.fieldlineProperties( points, maxWindings,
                            winding, twist, skip, type,
                            island, avenode, windingNextBest );
    
    *windingRet = winding;
    *twistRet   = twist;
  *islandRet  = island;
}


namespace FusionPSE {

    //using namespace SCIRun;


unsigned int
FieldlineLib::
fieldlineProperties( vector< Point > &points,
                     unsigned int maxWindings,
                     unsigned int &winding,
                     unsigned int &twist,
                     unsigned int &skip,
                     unsigned int &type,
                     unsigned int &island,
                     float &avenode,
                     unsigned int &windingNextBest )
{
  unsigned int order = 2;
  Vector centroid;
  bool groupCCW;
  float bestNodes = 1;
  vector< unsigned int > windingList;

  // Find the best winding for each test.


  // 1. Weed out any winding numbers where first two line segments cross each other.
  for( winding=1; winding<=maxWindings; winding++ )
  {
          
    if( points.size() <= 2 * winding )
    {
      debug5 << "Streamline has too few points ("
           << points.size() << ") to determine the winding accurately"
           << " past " << winding << " windings " << endl;
      break;
    }
          
    // If the first two connections of any group crosses another
    // skip it.
    if( IntersectCheck( points, winding ) )
    {

      // Passed all checks so add it to the possibility list.
      windingList.push_back( winding );
      debug5 << winding << " Passed IntersectCheck\n";
    }
  }


  // 2. For those that survived step 1:
  for( unsigned int i=0; i<windingList.size(); i++ )
  {
    winding = windingList[i];

    // Do the basic checks
    if( !basicChecks( points, centroid,
                      winding, twist, skip, type,
                      island, avenode, groupCCW, windingNextBest ) ) {
               
      windingList.erase( windingList.begin()+i );

      i--;
                
      debug5 << "windings " << winding
           << " FAILED Basic checks REMOVING";

      debug5 << "  Windings ";
      for( unsigned int j=0; j<windingList.size(); j++ )
        debug5 << windingList[j] << "  ";
      debug5 << " Remain" << endl;


      //    if( type == 2 ) {
      //      windingList.erase( windingList.begin(), windingList.end() );
      //    }

    } else {

      debug5 << "windings " << winding 
           << "  twists " << twist
           << "  skip "   << skip
           << "  groupCCW " << groupCCW
           << "  islands " << island
           << "  avenodes " << avenode
           << "  next best winding " << windingNextBest;

      if( order == 2 && windingList.size() > 1 && island > 0 ) {

        // If the winding is one check to see if there might a
        // higher winding that is also islands.
        bool multipleIslands = false;

        if( winding == 1 ) {
          unsigned int windingNextBestTmp = 0;
          unsigned int islandTmp = 0;

          unsigned int windingTmp;
          unsigned int twistTmp, skipTmp, typeTmp;
          float avenodeTmp;
          bool groupCCWTmp;
        

          for( unsigned int j=i+1; j<windingList.size(); j++ ) {
            windingTmp = windingList[j];
            if( basicChecks( points, centroid,
                             windingTmp, twistTmp, skipTmp, typeTmp,
                             islandTmp, avenodeTmp, groupCCWTmp,
                             windingNextBestTmp ) && islandTmp > 0 ) {

              multipleIslands = true;
                    
              break;
            }
          }
        }

        if( winding == 1 && multipleIslands ) {
          debug5 << " REMOVED - Multiple Islands" << endl;
          windingList.erase( windingList.begin()+i );
          i--;
          continue;

        } else {
                
          if( i > 0 ) 
            debug5 << " REPLACED - Islands" << endl;
          else
            debug5 << " START - Islands" << endl;

          while ( i > 0 ) {
            windingList.erase( windingList.begin() );
            i--;
          }

          while ( windingList.size() > 1 ) {
            windingList.erase( windingList.begin()+1 );
          }

          continue;
        }

      } else if( order != 2 && (windingNextBest == 0 || island > 0) ) {
        debug5 << endl;
        continue;

        // If very low order and has less than three points in
        // each group skip it.
      } else if( order != 2 && 2*winding - skip == windingNextBest &&
                 winding < 5 && 
                 windingNextBest/winding < 2 ) {
              
        vector< unsigned int >::iterator inList =
          find( windingList.begin(),
                windingList.end(), windingNextBest );
            
        // Found the next best in the list so delete the current one.
        if( inList != windingList.end() ) {
          debug5 << " REMOVED - Too few points" << endl;
                
          windingList.erase( windingList.begin()+i );

          i--;
                
          continue;
        }
      } 

      if( order == 0 ) {

        debug5 << " KEEP - low" << endl;

        // Take the lower ordered surface.
        unsigned int windingNextBestTmp = windingNextBest;
        unsigned int islandTmp = island;

        while( windingNextBestTmp && islandTmp == 0 ) {
          vector< unsigned int >::iterator inList =
            find( windingList.begin(),
                  windingList.end(), windingNextBestTmp );
            
          if( inList != windingList.end() ) {

            windingList.erase( inList );

            unsigned int windingTmp = windingNextBestTmp;
            unsigned int twistTmp, skipTmp, typeTmp;
            float avenodeTmp;
            bool groupCCWTmp;
                  
            if( basicChecks( points, centroid,
                             windingTmp, twistTmp, skipTmp, typeTmp,
                             islandTmp, avenodeTmp, groupCCWTmp,
                             windingNextBestTmp ) ) {
                    
              debug5 << "windings " << windingTmp
                   << "  twists " << twistTmp
                   << "  skip "   << skipTmp
                   << "  groupCCW " << groupCCWTmp
                   << "  islands " << islandTmp
                   << "  avenodes " << avenodeTmp
                   << "  next best winding " << windingNextBestTmp
                   << " REMOVED - low" << endl;           
            }

          } else {
            windingNextBestTmp = 0;
          }
        }

      } else if( order == 1 ) {

        unsigned int windingTmp = windingNextBest;
        unsigned int twistTmp, skipTmp, typeTmp, islandTmp;
        unsigned int windingNextBestTmp;
        float avenodeTmp;
        bool groupCCWTmp;
                  
        if( basicChecks( points, centroid,
                         windingTmp, twistTmp, skipTmp, typeTmp,
                         islandTmp, avenodeTmp, groupCCWTmp,
                         windingNextBestTmp ) ) {
                    
          // Basic philosophy - take the higher ordered surface
          // winding which will give a smoother curve.        
          vector< unsigned int >::iterator inList =
            find( windingList.begin(),
                  windingList.end(), windingNextBest );
                
          if( inList != windingList.end() ) {
            debug5 << " REMOVED - high" << endl;
                  
            windingList.erase( windingList.begin()+i );
                  
            i--;
                  
          } else {
            debug5 << " KEEP - high" << endl;
          }
        }
      } else if( order == 2 ) {

        // Keep the winding where the number of nodes for each
        // group is closest to being the same.
        //            float diff =
        //              (avenode - floor(avenode)) < (ceil(avenode)-avenode) ?
        //              (avenode - floor(avenode)) : (ceil(avenode)-avenode);

        // For the first keep is as the base winding.
        if( i == 0 ) {
          bestNodes = avenode;

          debug5 << " START " << endl;
                
        } else if( windingList[0] <= 3 && bestNodes < 8 ) {
          bestNodes = avenode;
          windingList.erase( windingList.begin() );
                
          i--;

          debug5 << " REPLACED " << endl;

          // The current winding is the best so erase the first.
        } else if( bestNodes < avenode ) {
          bestNodes = avenode;
          windingList.erase( windingList.begin() );
                
          i--;

          debug5 << " REPLACED " << endl;

          // The first winding is the best so erase the current.
        } else {
          windingList.erase( windingList.begin()+i );
                
          i--;
                
          debug5 << " REMOVED " << endl;
        }
      }
    }
  }

  if( windingList.size() ) {
    winding = windingList[0];

    basicChecks( points, centroid,
                 winding, twist, skip, type,
                 island, avenode, groupCCW, windingNextBest );

    // If the twists is a factorial of the winding then rebin the points.
    if( 0 &&
        winding && twist != 1 &&
        factorial( winding, twist ) ) {
        
      unsigned int fact;
      while( (fact = factorial( winding, twist )) ) {
        winding /= fact;
        twist /= fact;
      }
    }

  } else {
    winding = 0;
    twist = 0;
    skip = 0;
    type = 0;
    island = 0;
    avenode = 0;
    windingNextBest = 0;
  }

  return 0;
}



// Linearly interpolate between points.
Point FieldlineLib::interpert( Point lastPt, Point currPt, double t )
{
    return Point( Vector( lastPt ) + Vector( currPt - lastPt ) * t );
}


int FieldlineLib::ccw( Vector v0, Vector v1 ) {
    
  if( v0.x() * v1.z() - v0.z() * v1.x() > FLT_MIN ) return  1;    //  CCW
  if( v0.z() * v1.x() - v0.x() * v1.z() > FLT_MIN ) return -1;    //  CW
  if( v0.x() * v1.x() < 0.0 || v0.z() * v1.z() < 0.0 ) return -1; // CW
    
  if( v0.x()*v0.x()+v0.z()*v0.z() >=
      v1.x()*v1.x()+v1.z()*v1.z() ) return 0;               //  ON LINE
    
  return 1;                                                 //  CCW
}


int FieldlineLib::intersect( Point l0_p0, Point l0_p1,
                             Point l1_p0, Point l1_p1 )
{
    //  See if the lines intersect.    
    if( ( ccw( Vector(l0_p1-l0_p0), Vector(l1_p0-l0_p0)) *
          ccw( Vector(l0_p1-l0_p0), Vector(l1_p1-l0_p0) ) <= 0 ) &&
        ( ccw( Vector(l1_p1-l1_p0), Vector(l0_p0-l1_p0)) *
          ccw( Vector(l1_p1-l1_p0), Vector(l0_p1-l1_p0) ) <= 0 ) )
    {
        //  See if there is not a shared point.
        if( l0_p0 != l1_p0 && l0_p0 != l1_p1 &&
            l0_p1 != l1_p0 && l0_p1 != l1_p1 )
            return 1;  // Simple intersection.
        
        //  See if there is a shared point.
        else if( l0_p0 == l1_p0 || l0_p0 == l1_p1 ||
                 l0_p1 == l1_p0 || l0_p1 == l1_p1 )
            return 2;  // Intersect at a vertex
        
        //  There must be a point that is on the other line.
        else
            return 3;
    }
    
    //  Lines do not intersect.
    return 0;
}


unsigned int
FieldlineLib::convexHull( vector< Point > &hullPts,
                          vector< unsigned int > &ordering,
                          unsigned int &m,
                          unsigned int winding,
                          int dir )
{
    if( winding - m < 3 )
    {
        m = winding;
        return winding;
    }

    unsigned int min = m;

    // Find the point with the minimum z value.
    for( unsigned int i=m; i<winding; i++ )
    {
        if( hullPts[ min ].z() > hullPts[i].z() )
            min = i;
    }

    // Store the minimum point so that we know when we are done.
    hullPts[winding] = hullPts[min];
    ordering[winding] = ordering[min];

    do
    {
        // Swap the point that is the farthest to the right of all others.
        if( min != m )
        {
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
        for( unsigned int i=min+1; i<winding+1; i++ )
        {
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
    } while( min != winding );

    return m;
}


unsigned int FieldlineLib::hull( vector< Point > &points,
                                 unsigned int winding,
                                 bool &convex )
{
  // If one or two points the ordering makes no difference.
  if( winding < 3 )
  {
      convex = true;
      return 1;
  }

  unsigned int skip = 0;

  vector< Point > hullPts;
  vector< unsigned int > ordering;
  // Store the points and their order in a temporary vector.
  for( unsigned int i=0; i<winding; i++ )
  {
    hullPts.push_back( points[i] );
    ordering.push_back( i );
  }

  // Add one more points as a terminal.
  hullPts.push_back( points[0] );
  ordering.push_back( 0 );

  unsigned int m = 0;
  unsigned int cc = 0;

  int dir = 1;

  vector< pair< unsigned int, unsigned int > > skips;

  // Get the convex hull(s).
  do {

    // Swap the direction back and forth so to get the correct ordering.
    dir *= -1;

    convexHull( hullPts, ordering, m, winding, dir );

    unsigned int npts = m - cc;

    if( npts > 1 ) {

      // Get the ordering skip(s) for this hull.
      for( unsigned int i=1; i<npts+1; i++ ) {

        skip = (ordering[cc+(i%npts)] - ordering[cc+i-1] + winding) % winding;

        unsigned int j;
        
        for( j=0; j<skips.size(); j++ ) {

          if( skips[j].first == skip )
            break;
        }

        if( j == skips.size() )
          skips.push_back( pair< unsigned int, unsigned int >( skip, 1 ) );
        else
          skips[j].second++;
      }
    }

    cc = m;

  } while( m != winding );


  // With a single skip all of the points are on a single convex
  // hull.
  if( skips.size() == 1 && skips[0].second == winding ) {

    convex = true;
    skip = skips[0].first;

  } else {

    // With multiple skips there are concavities in the hull.
    convex = false;

    unsigned int bestSkipIndex = 0;
        
    for( unsigned int j=1; j<skips.size(); j++ ) {
      if( skips[bestSkipIndex].second < skips[j].second )
        bestSkipIndex = j;
    }

    skip = skips[bestSkipIndex].first;

    // The skip should account for all of the points along the hull
    // (even with concavities), e.g. using the skip a single hull
    // should be able to be built.
    unsigned int index = skip;
    unsigned int npts  = 1;
    
    while( index != 0 && npts != winding ) {
      index = (index + skip) % winding;
      npts++;
    }

    if( index != 0 || npts != winding ) {

      unsigned int factor = winding / npts;
      unsigned int newSkip = skip / factor;

      if( factor*npts == winding && newSkip * factor == skip ) {
      }

      debug5 << "SKIP ERROR - THE SKIP DOES NOT ACCOUNT FOR ALL OF THE POINTS"
           << endl;
    }
  }

  return skip;
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
  double d21z = pt2.z() - pt1.z();
  double d21x = pt2.x() - pt1.x();
  double d32z = pt3.z() - pt2.z();
  double d32x = pt3.x() - pt2.x();
        
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
  Point center;
      
  double d21z = pt2.z() - pt1.z();
  double d21x = pt2.x() - pt1.x();
  double d32z = pt3.z() - pt2.z();
  double d32x = pt3.x() - pt2.x();
        
  if (fabs(d21x) < FLT_MIN && fabs(d32z ) < FLT_MIN ) {

    center.x( 0.5*(pt2.x() + pt3.x()) );
    center.y( pt1.y() );
    center.z( 0.5*(pt1.z() + pt2.z()) );

    return center;
  }
        
  // IsPerpendicular() assure that xDelta(s) are not zero
  double aSlope = d21z / d21x;
  double bSlope = d32z / d32x;

  // checking whether the given points are colinear.    
  if (fabs(aSlope-bSlope) > FLT_MIN) {
    
    // calc center
    center.x( (aSlope*bSlope*(pt1.z() - pt3.z()) +
               bSlope*(pt1.x() + pt2.x()) -
               aSlope*(pt2.x() + pt3.x()) ) / (2.0* (bSlope-aSlope) ) );

    center.y( pt1.y() );

    center.z( -(center.x() - (pt1.x()+pt2.x())/2.0) / aSlope +
              (pt1.z()+pt2.z())/2.0 );
  }

  return center;
}


// nbins = winding number.
// If this is a valid winding number then:
//    No line-line intersections between pairs of points.
//     (intersections allowed if there are shared vertices.)
bool
FieldlineLib::
IntersectCheck( vector< Point >& points, unsigned int nbins )
{
    // Note: i < nbins.
    // We are only checking the first line segment of each possible winding group.
    for( unsigned int i=0, j=nbins; i<nbins && j<points.size(); i++, j++ )
    {
        Point l0_p0 = points[i];
        Point l0_p1 = points[j];

        for( unsigned int k=i+1, l=j+1; k<nbins && l<points.size(); k++, l++ )
        {
            Point l1_p0 = points[k];
            Point l1_p1 = points[l];

            //      debug5 << nbins
            //     << "   " << i << "  " << j << "  " << k << "  " << l << endl;

            // intersect() == 1: If lines intersect, no shared vertex.
            // OK cases: No intersection, or any shared vertex.
            if( j != k && intersect( l0_p0, l0_p1, l1_p0, l1_p1 ) == 1)
                return false;
        }
  }

  return true;
}


bool
FieldlineLib::
basicChecks( vector< Point >& points,
             Vector &globalCentroid,
             unsigned int &winding,
             unsigned int &twist,
             unsigned int &skip,
             unsigned int &type,
             unsigned int &island,
             float &avenode,
             bool &groupCCW,
             unsigned int &windingNextBest )
{
  float confidence = 0;
  Vector v0, v1;
  // Get the centroid for all of the points.
  globalCentroid = Vector(0,0,0);

  for( unsigned int i=0; i<points.size(); i++ )
    globalCentroid += (Vector) points[i];

  globalCentroid /= (double) points.size();


  windingNextBest = 0;
  avenode = 0;
  island = 0;
  
  bool convex;

  // A winding of one is valid only if it is an island. An island with
  // one winding must have a non-convex hull otherwise it is a surface.
  if( winding == 1 ) {

    unsigned int nodes = 0;

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
    skip = hull( points, nodes, convex );

    debug5 << "nodes " << nodes << "  "
         << "convex " << convex << "  "
         << "skip " << skip << endl;

    // The ordering can be in either direction so skip may be flipped.
    if( skip != 1 && skip + 1 == nodes )
      skip = 1;

    if( skip == 1 & !convex ) {
      twist  = 1;
      type = 0;
      island = 1;
      avenode = nodes;
      return true;

    } else {
      type = 0;
      debug5 << "WINDING OF 1 BUT NOT AN ISLAND "
           << (convex ? " CONVEX" : " CONCAVE") << " HULL "
           << endl;
      return false;
    }
  }


  // If a winding of two then then the ordering is immaterial.
  else if( winding == 2 )
  {
    skip   = 1;
    convex = true;

  }
  else
  {
    // Find the skip based on the hull(s).
    skip = hull( points, winding, convex );

    // If the hull is convex the skip can be found using the
    // centroid because it will be located inside the hull.
    if( convex ) {
      unsigned int tmpSkip = skip;

      v0 = (Vector) points[   0] - globalCentroid;
      v1 = (Vector) points[skip] - globalCentroid;

      double angleMin = acos( Dot( v0, v1 ) / (v0.length() * v1.length()) );

      for( unsigned int i=1; i<winding; i++ ) {
        v1 = (Vector) points[i] - globalCentroid;

        double angle = acos( Dot( v0, v1 ) / (v0.length() * v1.length()) );

        // The skip will be found based on the minimum angle between two
        // groups.
        if( angleMin > angle ) {
          angleMin = angle;
          tmpSkip = i;

          //      debug5 << " angle " << angle
          //           << " angleMin " << angleMin
          //           << " tmpSkip " << tmpSkip
          //           << " skip " << skip
          //           << endl;
        }
      }

      if( tmpSkip != skip && tmpSkip != winding - skip ) {
        debug5 << endl << winding << "CONVEX HULL BUT IRREGULAR SKIP "
             << tmpSkip << "  " <<  skip << endl;
      }
    }

    // Find the first point from another group that overlaps the first
    // group. This only works if there is an overlap between groups.
    unsigned int tmpSkip = skip;

    for( unsigned int i=0, j=winding;
         i<points.size() && j<points.size();
         i+=winding, j+=winding ) {

      for( unsigned int k=1; k<winding; k++ ) {

        v0 = (Vector) points[i] - (Vector) points[k];
        v1 = (Vector) points[j] - (Vector) points[k];

        v0.safe_normalize();
        v1.safe_normalize();

        //      debug5 << "Indexes " << i << "  " << j << "  " << k << " dot  "
        //           << Dot( v0, v1 ) << endl;

        if( Dot( v0, v1 ) < 0.0 ) {
          tmpSkip = k;    
          i = k = points.size();
          break;
        }
      }
    }

    if( tmpSkip != skip && tmpSkip != winding - skip ) {
      debug5 << endl << winding << " OVERLAP BUT IRREGULAR SKIP "
           << tmpSkip << "  " <<  skip << endl;
    }
  }


  // Get the direction of the points for the first group. Assume that
  // the direction is the same for all of the other groups.
  bool baseCCW = (ccw( (Vector) points[0      ] - globalCentroid, 
                       (Vector) points[winding] - globalCentroid ) == 1);
  

  // Do a check for islands. Islands will exists if there is a change
  // in direction of the connected points relative to a base point. IF
  // the hull is convex the base point may the centroid is the of all
  // of the points or based upon a point that is perpendicular to the
  // principal axis of the group of points.
  unsigned int completeIslands = 0;

  unsigned int firstTurnMin = 0;
  unsigned int   midTurnMin = 0;
  unsigned int  lastTurnMin = 0;

  unsigned int firstTurnMax = points.size();
  unsigned int   midTurnMax = points.size();
  unsigned int  lastTurnMax = points.size();

  unsigned int  nodesMin = 0;
  unsigned int  nodesMax = points.size() / winding;


  for( unsigned int i=0; i<winding; i++ ) {

    Vector baseCentroid;

    // If the hull is convex the global centroid can be used because
    // all of the islands will surround it in a radial manner.
    if( convex ) {
      baseCentroid = globalCentroid;

      // Otherwise use an offset from the local centroid of each group.
    } else {

      // Get the local centroid for the group.
      Vector localCentroid(0,0,0);

      unsigned int npts = 0;

      for( unsigned int j=i; j<points.size(); j+=winding, npts++ )
        localCentroid += (Vector) points[j];

      localCentroid /= (float) npts;

      // Get the principal axes of the points.
      float Ixx = 0.0;
      float Ixz = 0.0;
      float Izz = 0.0;

      double maxDist = 0;

      // Get the moments of intertial for each point. It assumed that
      // everything is in the Y plane as such there the moments of
      // intertial along the Y axis are zero.
      for( unsigned int j=i; j<points.size(); j+=winding ) {

          Vector vec = (Vector) points[j] - localCentroid;

        if( maxDist < vec.length() )
          maxDist = vec.length();

        Ixx += vec.z()*vec.z();
        Ixz -= vec.x()*vec.z();
        Izz += vec.x()*vec.x();
      }

      // Short cut to the principal axes because the Y moments of
      // intertial are zero.
      float alpha = atan( 2.0 * Ixz / (Ixx - Izz) ) / 2.0;

      //       debug5 << "PRINCIPAL AXES " << alpha * 180.0 / M_PI << "    "
      //           << Ixx + Ixz * sin(alpha       )/cos(alpha       ) << "    "
      //           << Izz + Ixz * cos(alpha+M_PI/2)/sin(alpha+M_PI/2) << endl;

      // Use the principal axes to get an offset from the local
      // centroid which gives a point outside the island.
      baseCentroid = localCentroid;

      if( Ixx + Ixz * sin(alpha       )/cos(alpha       ) >
          Izz + Ixz * cos(alpha+M_PI/2)/sin(alpha+M_PI/2) )
        baseCentroid += Vector(  cos(alpha), 0, sin(alpha) ) * maxDist;
      else
        baseCentroid += Vector( -sin(alpha), 0, cos(alpha) ) * maxDist;
    }


    unsigned int turns = 0;
    unsigned int firstTurn = 0;
    unsigned int   midTurn = 0;
    unsigned int  lastTurn = 0;

    // Get the direction based on the first two points.
    v0 = (Vector) points[i        ] - baseCentroid;
    v1 = (Vector) points[i+winding] - baseCentroid;

    bool lastCCW = (ccw( v0, v1 ) == 1);
    v0 = v1;

    // The islands should all go in the same direction but not always.
    if( i && island && lastCCW != baseCCW ) {

      debug5 << "Warning island " << i << " is in the opposite direction."
           << endl;
    }

    baseCCW = lastCCW;

    // Get the direction based on the remaining points.
    for( unsigned int j=i+2*winding; j<points.size(); j+=winding ) {
      v1 = (Vector) points[j] - baseCentroid;

      bool localCCW = (ccw( v0, v1 ) == 1);

      // A switch in direction indicates that an island is present.
      if( localCCW != lastCCW ) {

        lastCCW = localCCW;

        if( turns++ == 0 )
          island++;

        if( turns == 1 )     firstTurn = j - winding;
        else if( turns == 2 )  midTurn = j - winding;
        else if( turns == 3 ) lastTurn = j - winding;

        if( turns == 3 )
          break;

      } else {
        v0 = v1;
      }
    }


    // Determine the approximate number of points in the group.
    unsigned int nodes;
    unsigned int overlap = 0;

    if( turns < 2 ) {
      nodes = points.size() / winding;
    } else {

      // Set the maximum number of points to check for the
      // overlap. This is because although at times the inflection
      // points can be found but not overlaps.
      unsigned int maxNodeCheck;

      if( turns == 2 ) {
        maxNodeCheck = midTurn + (midTurn - firstTurn) + 3 * winding;
        nodes = points.size() / winding;
      } else if( turns == 3 ) {
        maxNodeCheck = lastTurn + 3 * winding;
        nodes = (lastTurn - firstTurn) / winding + 1;
      }

      if( maxNodeCheck > points.size() )
        maxNodeCheck = points.size();

      // Check to see if the island overlaps itself.
      for( unsigned int j=midTurn, k=j+winding;
           j<maxNodeCheck && k<maxNodeCheck;
           j+=winding, k+=winding ) {
        
        v0 = (Vector) points[i] - (Vector) points[j];
        v1 = (Vector) points[i] - (Vector) points[k];
        
        if( Dot( v0, v1 ) < 0.0 ) {
          overlap = j;
          nodes = j / winding;
          turns = 3;
          break;
        }
      }
    }

    if( turns == 3 )
      completeIslands++;

    avenode += nodes;

    if( nodesMin > nodes ) nodesMin = nodes;
    if( nodesMax < nodes ) nodesMax = nodes;

    if( firstTurnMin > nodes ) firstTurnMin = firstTurn;
    if( firstTurnMax < nodes ) firstTurnMax = firstTurn;

    if( midTurnMin > nodes ) midTurnMin = midTurn;
    if( midTurnMax < nodes ) midTurnMax = midTurn;

    if( lastTurnMin > nodes ) lastTurnMin = lastTurn;
    if( lastTurnMax < nodes ) lastTurnMax = lastTurn;


    if( turns )
      debug5 << "Island " << i << " has "
           << turns << " turns with "
           << nodes << " nodes   " 
           << overlap << " overlap   " 
           << " firstTurn " << firstTurn
           <<   " midTurn " << midTurn
           <<  " lastTurn " << lastTurn << endl;
  }

  // Island sanity check make sure no island overlaps another island.
  if( island ) {

    type = 0;

    // If islands are consistant then get the average.
    avenode /= (float) winding;

    if( island == winding &&
        completeIslands == winding &&
        nodesMin+1 >= (unsigned int) avenode && 
        nodesMax-1 <= (unsigned int) avenode &&
        firstTurnMax - firstTurnMin == winding-1 &&
        midTurnMax -   midTurnMin == winding-1 &&
        lastTurnMax -  lastTurnMin == winding-1 ) {
      confidence = 1.0;      
    }

    // The max nodes to check is set in case there are lots of points.
    unsigned int maxNodeCheck = 2 * (unsigned int) avenode * winding;

    if( maxNodeCheck > points.size() )
      maxNodeCheck = points.size();

    if( !convex && island < winding &&
        (firstTurnMax - firstTurnMin)/winding > 3 ) {

      debug5 << "PROBABLY NOT ISLANDS" << endl;
    }


    for( unsigned int i=0; i<winding && island; i++ ) {

      for( unsigned int g=0; g<2 && island; g++ ) {

        unsigned int offset;

        if( g == 0 )  // previous
          offset = (i - skip + winding) % winding;
        else         // next
          offset = (i + skip)           % winding;

        // Check for a point in the previous/next group being between
        // the first two points in the current group.
        if( island ) {
          for( unsigned int j=i+winding, k=offset;
               j<maxNodeCheck && k<maxNodeCheck;
               k+=winding ) {
            
            v0 = (Vector) points[i] - (Vector) points[k];
            v1 = (Vector) points[j] - (Vector) points[k];
            
            if( Dot( v0, v1 ) < 0.0) {
              island = 0;
              type = 2;
              
              v0.safe_normalize();
              v1.safe_normalize();
              
              debug5 << "FAILED ISLAND SANITY CHECK #1  "
                   << i << "  " << j << "  " << k << "  "
                   << Dot( v0, v1 ) << endl;
              
              return false;
            }
          }
        }

        // Check for the first point in the current group being between
        // two points in the previous/next group.
        if( island ) {
          for( unsigned int j=offset, k=j+winding;
               j<maxNodeCheck && k<maxNodeCheck;
               j+=winding, k+=winding ) {

            v0 = (Vector) points[i] - (Vector) points[j];
            v1 = (Vector) points[i] - (Vector) points[k];

            if( Dot( v0, v1 ) < 0.0) {
              island = 0;
              type = 2;

              v0.safe_normalize();
              v1.safe_normalize();

              debug5 << "FAILED ISLAND SANITY CHECK #2  "
                   << i << "  " << j << "  " << k << "  "
                   << Dot( v0, v1 ) << endl;

              return false;
            }
          }
        }
      }
    }

  } else {

    type = 1;

    // Surface Checks

    // Get the direction of the points for the first group.
    groupCCW = (ccw( (Vector) points[0      ] - globalCentroid, 
                     (Vector) points[winding] - globalCentroid ) == 1);

    if( convex ) {
      // Make sure that the direction is the same for all of the other
      // groups - only valid for convex hulls.
      for( unsigned int i=1, j=i+winding; i<winding; i++, j++ ) {

        // Get the direction based on the first two points.
        baseCCW = (ccw( (Vector) points[i] - globalCentroid, 
                        (Vector) points[j] - globalCentroid ) == 1);

        if( baseCCW != groupCCW ) {
          debug5 << "CONVEX HULL BUT GROUPS GO IN DIFFERENT DIRECTIONS" << endl;

          return false;
        }
      }
    }

    // Make sure the skip direction is the same as the group
    // direction.
    baseCCW = (ccw( (Vector) points[0   ] - globalCentroid, 
                    (Vector) points[skip] - globalCentroid ) == 1);
    
    if( groupCCW != baseCCW ) {
      skip = winding - skip;
    }

    unsigned int previous = winding - skip;

    // See if a point overlaps the first section.
    for( unsigned int j=previous; j<points.size(); j+=winding ) {

      v0 = (Vector) points[0      ] - (Vector) points[j];
      v1 = (Vector) points[winding] - (Vector) points[j];

      if( Dot( v0, v1 ) < 0.0) {
        if( j > winding ) {
          windingNextBest = j - winding;
          break;

        } else {
          debug5 << "CAN NOT DETERMINE NEXT BEST WINDING"
               << " point " << j
               << " winding " << winding
               << endl;

          return false;
        }
      }
    }

    // See if the first point overlaps another section.
    if( windingNextBest == 0 ) {

      for( unsigned int j=previous, k=j+winding;
           j<points.size() && k<points.size();
           j+=winding, k+=winding ) {

        v0 = (Vector) points[0] - (Vector) points[j];
        v1 = (Vector) points[0] - (Vector) points[k];

        if( Dot( v0, v1 ) < 0.0) {
          if( j > winding ) {
            windingNextBest = j;
            break;

          } else {
            debug5 << "CAN NOT DETERMINE NEXT BEST WINDING"
                 << " point " << j
                 << " winding " << winding
                 << endl;

            return false;
          }
        }
      }
    }


    // If next best winding is set then there is an overlap. As such,
    // check to see if the number of points that would be in each
    // group is the same.
    if( windingNextBest ) {

      avenode = 0;

      unsigned int nnodes = windingNextBest / winding;

      unsigned int jMin = points.size();
      unsigned int jMax = 0;

      unsigned int kMin = points.size();
      unsigned int kMax = 0;

      unsigned int lMin = points.size();
      unsigned int lMax = 0;

      unsigned int nodesMin = points.size();
      unsigned int nodesMax = 0;

      // Search each group and check for overlap with the next group.
      for( unsigned int i=0; i<winding; i++ ) {

        unsigned int nodes = points.size() / winding;

        // Get the first point in the next group.
        unsigned int j = (i + skip) % winding;

        for( unsigned int k=i+nnodes*winding, l=k-winding;
             k<points.size() && l<points.size();
             k+=winding, l+=winding ) {

          // Check to see if the first overlapping point is really a
          // fill-in point. This happens because the spacing between
          // winding groups varies between groups.
          v0 = (Vector) points[k] - (Vector) points[j];
          v1 = (Vector) points[l] - (Vector) points[j];

          if( Dot( v0, v1 ) < 0.0 ) {
            nodes = l / winding + 1;

//          debug5 << "Found overlap at "
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

      avenode /= winding;

      if( nodesMin+1 >= (unsigned int) avenode && 
          nodesMax-1 <= (unsigned int) avenode &&
          jMax - jMin == winding-1 &&
          kMax - kMin == winding-1 &&
          lMax - lMin == winding-1 ) {
        confidence = 1.0;
      }

    } else {

      // In this case there is no overlap so the number of nodes can
      // not be determined correctly so just use the average.
      avenode = (float) points.size() / (float) winding;

      confidence = 0.5;
    }
  }


  // In order to find the twist find the mutual primes
  // (Blankinship Algorithm). In this case we only care about
  // the first one becuase the second is just the number of
  // windings done to get there.

  for( twist=1; twist<winding; twist++ )
    if( twist * skip % winding == 1 )
      break;

  if( twist == winding ) {
    debug5 << endl
         << "ERROR in finding the - WINDING - TWIST - SKIP"
         << " winding " << winding
         << " twist " << twist
         << " skip " << skip
         << endl;

    return false;

  } else if( convex && winding > 2 ) {

    // Sanity check for the twist/skip calculation
    double angleSum = 0;

    for( unsigned int i=0; i<winding; i++ ) {

      unsigned int start = i;
      unsigned int stop  = (start +    1) % winding;
      unsigned int next  = (start + skip) % winding;

      Vector v0 = (Vector) points[start] - globalCentroid;

      // Get the angle traveled from the first group to the second
      // for one winding. This is done by summing the angle starting
      // with the first group and going to the geometrically next
      // group stopping when the logically next group is found.

      do {
        Vector v1 = (Vector) points[next] - globalCentroid;

        angleSum += acos( Dot( v0, v1 ) / (v0.length() * v1.length()) );

        //        debug5 << " winding " << i
        //             << " start " << start
        //             << " next " << next
        //             << " stop " << stop
        //             << " twist angle "
        //             << acos( Dot( v0, v1 ) / (v0.length() * v1.length()) ) << endl;

        start = next;
        next  = (start + skip) % winding;

        v0 = v1;
      } while( start != stop );
    }

    // The total number of twist should be the same. Account for small
    // rounding errors by adding 25% of the distanced traveled in one
    // winding.
    unsigned int twistCheck =
      (unsigned int) (angleSum / (2.0 * M_PI) + M_PI/2.0/winding);

    if( twistCheck != twist ) {
      debug5 << endl
           << "WARNING - TWIST MISMATCH "
           << " angle sum " << (angleSum / (2.0 * M_PI))
           << " winding " << winding
           << " twistCheck " << twistCheck
           << " twist " << twist
           << " skip " << skip
           << endl;
    }
  }

  if( avenode > (float) points.size() / (float) winding + 1 ) {
    debug5 << endl
         << "ERROR in finding the average nodes"
         << " winding " << winding
         << " twist " << twist
         << " skip " << skip
         << " windingNextBest " << windingNextBest
         << " avenode " << avenode
         << " number of points " << points.size()
         << endl;
  }

  return true;
}


unsigned int
FieldlineLib::
islandProperties( vector< Point > &points,
                  Vector globalCentroid,
                  unsigned int &startIndex,
                  unsigned int &middleIndex,
                  unsigned int &stopIndex,
                  unsigned int &nodes )
{
  // Get the local centroid for the group.
  Vector baseCentroid, localCentroid(0,0,0);

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
    
    Ixx += vec.z()*vec.z();
    Ixz -= vec.x()*vec.z();
    Izz += vec.x()*vec.x();
  }
  
  // Short cut to the principal axes because the Y moments of
  // intertial are zero.
  float alpha = atan( 2.0 * Ixz / (Ixx - Izz) ) / 2.0;

  //       debug5 << "PRINCIPAL AXES " << alpha * 180.0 / M_PI << "    "
  //       << Ixx + Ixz * sin(alpha       )/cos(alpha       ) << "    "
  //       << Izz + Ixz * cos(alpha+M_PI/2)/sin(alpha+M_PI/2) << endl;

  // Use the principal axes to get an offset from the local
  // centroid which gives a point outside the island.
  baseCentroid = localCentroid;
  
  if( Ixx + Ixz * sin(alpha       )/cos(alpha       ) >
      Izz + Ixz * cos(alpha+M_PI/2)/sin(alpha+M_PI/2) )
    baseCentroid += Vector(  cos(alpha), 0, sin(alpha) ) * maxDist;
  else
    baseCentroid += Vector( -sin(alpha), 0, cos(alpha) ) * maxDist;


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
        debug5 << "First point is actually the start point.\n";

        stopIndex   = middleIndex;
        middleIndex = startIndex;
        startIndex  = 0;
      }
    } else if( turns == 2 ) {

      if( 2*startIndex == middleIndex + 1 ) {
        // First point is actually the start point.
        debug5 << "First point is actually the start point.\n";

        stopIndex   = middleIndex;
        middleIndex = startIndex;
        startIndex  = 0;
      
        turns = 3;

      } else if( points.size() < 2 * (middleIndex - startIndex) - 1 ) {
        // No possible over lap.
        debug5 <<  "islandProperties - No possible over lap.\n";

        stopIndex = startIndex + points.size() - 1;
      }
    }

    nodes = stopIndex - startIndex + 1;

    // See if the first point overlaps another section.
    for( unsigned int  j=middleIndex+1; j<points.size(); j++ ) {
      if( Dot( (Vector) points[j  ] - (Vector) points[0],
               (Vector) points[j-1] - (Vector) points[0] )
          < 0.0 ) {
        stopIndex = startIndex + (j-1) + 1; // Add one for the zeroth 
        nodes = j;
        turns = 3;
        debug5 <<  "islandProperties - First point overlaps another section after " << j-1 << endl;
        break;
      }
    }
      
    // See if a point overlaps the first section.
    for( unsigned int j=middleIndex; j<points.size(); j++ ) {
      if( Dot( (Vector) points[0] - (Vector) points[j],
               (Vector) points[1] - (Vector) points[j] )
          < 0.0 ) {
        stopIndex = startIndex + (j-1) + 1; // Add one for the zeroth 
        nodes = j;
        turns = 3;
        debug5 << "islandProperties - A point overlaps the first section at " << j-1 << endl;
        break;
      }
    }

    // No overlap found
    if( turns == 2 ) {
      stopIndex = startIndex + points.size() - 1;
      nodes = points.size();
      debug5 << "islandProperties - No overlap found\n";
    }
  }

  return turns;
}


unsigned int
FieldlineLib::
surfaceOverlapCheck( vector< vector< pair< Point, double > > > &bins,
                     unsigned int winding,
                     unsigned int skip,
                     unsigned int &nnodes )
{
  nnodes = bins[0].size();

  // First make sure none of the groups overlap themselves.
  for( unsigned int i=0; i<winding; i++ ) {

    if( nnodes > bins[i].size() )
      nnodes = bins[i].size();

    for( unsigned int j=2; j<nnodes; j++ ) {

      Vector v0 = (Vector) bins[i][0].first - (Vector) bins[i][j].first;
      Vector v1 = (Vector) bins[i][1].first - (Vector) bins[i][j].first;
      
      if( Dot( v0, v1 ) < 0.0 ) {
        nnodes = j;
        break;
      }
    }
  }

  if( winding == 1 )
    return nnodes;

  // Second make sure none of the groups overlap each other.
  for( unsigned int i=0; i<winding; i++ ) {

    // The previous group
    unsigned int j = (i - skip + winding) % winding;

    // Check for a point in the previous group being between the first
    // two points in the current group.
    for( unsigned int k=0; k<nnodes; k++ ) {

      Vector v0 = (Vector) bins[i][0].first - (Vector) bins[j][k].first;
      Vector v1 = (Vector) bins[i][1].first - (Vector) bins[j][k].first;
      
      if( Dot( v0, v1 ) < 0.0 ) {
        nnodes = k;
        break;
      }
    }

    // Check for a point in the current group being between two points
    // in the previous group.
    for( unsigned int k=1; k<nnodes; k++ ) {

      Vector v0 = (Vector) bins[j][k  ].first - (Vector) bins[i][0].first;
      Vector v1 = (Vector) bins[j][k-1].first - (Vector) bins[i][0].first;
      
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
surfaceGroupCheck( vector< vector< pair< Point, double > > > &bins,
                   unsigned int i,
                   unsigned int j,
                   unsigned int nnodes ) {

  unsigned int nodes = nnodes;

  while( nodes < bins[i].size() ) {
    // Check to see if the first overlapping point is really a
    // fill-in point. This happens because the spacing between
    // winding groups varries between groups.
    Vector v0 = (Vector) bins[j][0      ].first - (Vector) bins[i][nodes].first;
    Vector v1 = (Vector) bins[i][nodes-1].first - (Vector) bins[i][nodes].first;
    
    if( Dot( v0, v1 ) < 0.0 )
      nodes++;
    else
      break;
  }

  return nodes;
}


unsigned int
FieldlineLib::
removeOverlap( vector< vector < pair< Point, double > > > &bins,
               unsigned int &nnodes,
               unsigned int winding,
               unsigned int twist,
               unsigned int skip,
               unsigned int island )
{
  Vector globalCentroid = Vector(0,0,0);;

  for( unsigned int i=0; i<winding; i++ )
    for( unsigned int j=0; j<nnodes; j++ )
      globalCentroid += (Vector) bins[i][j].first;
  
  globalCentroid /= (winding*nnodes);
    
  if( island ) {

    for( unsigned int i=0; i<winding; i++ ) {
      unsigned int nodes = 0;
      bool completeIsland = false;

      if( winding == 1 ) {

        unsigned int i = 0;

        // See if the first point overlaps another section.
        for( unsigned int  j=nnodes/2; j<bins[i].size(); j++ ) {
          if( Dot( (Vector) bins[i][j  ].first - (Vector) bins[i][0].first,
                   (Vector) bins[i][j-1].first - (Vector) bins[i][0].first )
              < 0.0 ) {

            debug5 <<  "removeOverlap - First point overlaps another section after " << j-1 << endl;
            nodes = j;
        
            completeIsland = true;
            break;
          }
        }

        // See if a point overlaps the first section.
        if( nodes == 0 ) {
          for( unsigned int j=nnodes/2; j<bins[i].size(); j++ ) {
            if( Dot( (Vector) bins[i][0].first - (Vector) bins[i][j].first,
                     (Vector) bins[i][1].first - (Vector) bins[i][j].first )
                < 0.0 ) {
              debug5 << "removeOverlap - A point overlaps the first section at " << j-1 << endl;
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
          points[j] = bins[i][j].first;
        
        if( islandProperties( points, globalCentroid,
                              startIndex, middleIndex, stopIndex, nodes ) == 3 )

          completeIsland = true;
      }

      // No more than one point should added.
      if( nodes > nnodes+1 )
        debug5 << "Island " << i
             << " nnodes mismatch " << nnodes << "  " << nodes << endl;

      // Erase all of the overlapping points.
      bins[i].erase( bins[i].begin()+nodes, bins[i].end() );
      
      // Close the island if it is complete
      if( completeIsland )
        bins[i].push_back( bins[i][0] );
    }

  } else {  // Surface

    // This gives the minimal number of nodes for each group.
    surfaceOverlapCheck( bins, winding, skip, nnodes );
    
    if( nnodes == 0 ) {

      debug5 << "ZERO NODES " << endl;

      for( unsigned int i=0; i<winding; i++ )
        debug5 << i << "  " << bins[i].size() << endl;

      nnodes = 20;
    }

    for( unsigned int i=0; i<winding; i++ ) {

      // Add back in any nodes that may not overlap.
      unsigned int nodes =
        surfaceGroupCheck( bins, i, (i+skip)%winding, nnodes );

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
smoothCurve( vector< vector < pair< Point, double > > > &bins,
             unsigned int &nnodes,
             unsigned int winding,
             unsigned int twist,
             unsigned int skip,
             unsigned int island )
{
  Vector globalCentroid = Vector(0,0,0);;

  for( unsigned int i=0; i<winding; i++ )
    for( unsigned int j=0; j<nnodes; j++ )
      globalCentroid += (Vector) bins[i][j].first;
  
  globalCentroid /= (winding*nnodes);

  unsigned int add = 2;

  if( island ) {

    for( unsigned int i=0; i<winding; i++ ) {
      //      for( unsigned int s=0; s<add; s++ )
      {
        pair< Point, double > newPts[add*nnodes];

        for( unsigned int j=0; j<add*nnodes; j++ )
          newPts[j] = pair< Point, double > (Point(0,0,0), 0 );
        
        for( unsigned int j=1; j<nnodes-1; j++ ) {

          unsigned int j_1 = (j-1+nnodes) % nnodes;
          unsigned int j1  = (j+1+nnodes) % nnodes;

          Vector v0 = (Vector) bins[i][j1].first - (Vector) bins[i][j  ].first;
          Vector v1 = (Vector) bins[i][j ].first - (Vector) bins[i][j_1].first;

          debug5 << i << " smooth " << j_1 << " "  << j << " "  << j1 << "  "
               << ( v0.length() > v1.length() ?
                    v0.length() / v1.length() :
                    v1.length() / v0.length() ) << endl;

          if( Dot( v0, v1 ) > 0 &&
              ( v0.length() > v1.length() ?
                v0.length() / v1.length() :
                v1.length() / v0.length() ) < 10.0 ) {

            Vector center = (Vector) circle( bins[i][j_1].first,
                                             bins[i][j  ].first,
                                             bins[i][j1 ].first );

            double rad = ((Vector) bins[i][j].first - center).length();


            for( unsigned int s=0; s<add; s++ ) {
              Vector midPt = (Vector) bins[i][j_1].first +
                (double) (add-s) / (double) (add+1) *
                ((Vector) bins[i][j].first - (Vector) bins[i][j_1].first );
                

              Vector midVec = midPt - center;

              midVec.safe_normalize();

              newPts[add*j+s].first += center + midVec * rad;
              newPts[add*j+s].second += 1.0;

              midPt = (Vector) bins[i][j].first +
                (double) (add-s) / (double) (add+1) *
                ((Vector) bins[i][j1].first - (Vector) bins[i][j].first );

              midVec = midPt - center;

              midVec.safe_normalize();

              newPts[add*j1+s].first += center + midVec * rad;
              newPts[add*j1+s].second += 1.0;
            }
          }
        }

        for( unsigned int j=nnodes-1; j>0; j-- ) {

          for( unsigned int s=0; s<add; s++ ) {

            unsigned int k = add * j + s;

            if( newPts[k].second > 0 ) {
              
              newPts[k].first /= newPts[k].second;
              
              //              debug5 << i << " insert " << j << "  " << newPts[k].first << endl;
              
              bins[i].insert( bins[i].begin()+j, newPts[k] );
            }
          }
        }

        for( unsigned int s=0; s<add; s++ ) {

          unsigned int k = add - 1 - s;

          if( newPts[k].second > 0 ) {
              
            newPts[k].first /= newPts[k].second;
              
            //        debug5 << i << " insert " << 0.0<< "  " << newPts[k].first << endl;
              
            bins[i].push_back( newPts[k] );
          }
        }
      }
    }

  } else {

    for( unsigned int i=0; i<winding; i++ ) {

      if( bins[i].size() < 2 )
        continue;

      // Index of the next winding group
      unsigned int j = (i+skip)%winding;

      // Insert the first point from the next winding so the curve
      // is contiguous.
      bins[i].push_back( bins[j][0] );

      //for( unsigned int s=0; s<add; s++ )
      {
        unsigned int nodes = bins[i].size();

        pair< Point, double > newPts[add*nodes];

        for( unsigned int j=0; j<add*nodes; j++ )
          newPts[j] = pair< Point, double > (Point(0,0,0), 0 );
        
        for( unsigned int j=1; j<nodes-1; j++ ) {

          unsigned int j_1 = j - 1;
          unsigned int j1  = j + 1;

          Vector v0 = (Vector) bins[i][j1].first - (Vector) bins[i][j  ].first;
          Vector v1 = (Vector) bins[i][j ].first - (Vector) bins[i][j_1].first;

          //      debug5 << i << " smooth " << j_1 << " "  << j << " "  << j1 << "  "
          //           << ( v0.length() > v1.length() ?
          //                v0.length() / v1.length() :
          //                v1.length() / v0.length() ) << endl;

          if( Dot( v0, v1 ) > 0 &&
              ( v0.length() > v1.length() ?
                v0.length() / v1.length() :
                v1.length() / v0.length() ) < 10.0 ) {

            Vector center = (Vector) circle( bins[i][j_1].first,
                                             bins[i][j  ].first,
                                             bins[i][j1 ].first );

            double rad = ((Vector) bins[i][j].first - center).length();


            for( unsigned int s=0; s<add; s++ ) {
              Vector midPt = (Vector) bins[i][j_1].first +
                (double) (add-s) / (double) (add+1) *
                ((Vector) bins[i][j].first - (Vector) bins[i][j_1].first );
                

              Vector midVec = midPt - center;

              midVec.safe_normalize();

              newPts[add*j+s].first += center + midVec * rad;
              newPts[add*j+s].second += 1.0;

              midPt = (Vector) bins[i][j].first +
                (double) (add-s) / (double) (add+1) *
                ((Vector) bins[i][j1].first - (Vector) bins[i][j].first );

              midVec = midPt - center;

              midVec.safe_normalize();

              newPts[add*j1+s].first += center + midVec * rad;
              newPts[add*j1+s].second += 1.0;
            }
          }
        }

        for( int j=nodes-1; j>=0; j-- ) {

          for( unsigned int s=0; s<add; s++ ) {

            unsigned int k = add * j + s;

            if( newPts[k].second > 0 ) {
              
              newPts[k].first /= newPts[k].second;
              
              //              debug5 << i << " insert " << j << "  " << newPts[k].first << endl;
              
              bins[i].insert( bins[i].begin()+j, newPts[k] );
            }
          }
        }
      }

      // Remove the last point so it is possilble to see the groups.
      bins[i].erase( bins[i].end() );
    }
  }

  return winding*(add+1)*nnodes;
}


unsigned int
FieldlineLib::
mergeOverlap( vector< vector < pair< Point, double > > > &bins,
              unsigned int &nnodes,
              unsigned int winding,
              unsigned int twist,
              unsigned int skip,
              unsigned int island )
{
  Vector globalCentroid = Vector(0,0,0);;

  for( unsigned int i=0; i<winding; i++ )
    for( unsigned int j=0; j<nnodes; j++ )
      globalCentroid += (Vector) bins[i][j].first;
  
  globalCentroid /= (winding*nnodes);
    
  if( island ) {

    vector < pair< Point, double > > tmp_bins[winding];

    for( unsigned int i=0; i<winding; i++ ) {
      
      unsigned int startIndex;
      unsigned int middleIndex;
      unsigned int stopIndex;
      unsigned int nodes;
        
      vector< Point > points;
      points.resize( bins[i].size() );

      for( unsigned int j=0; j<bins[i].size(); j++ )
        points[j] = bins[i][j].first;

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

          Vector v0 = (Vector) bins[i][0].first -
            (Vector) tmp_bins[i][j].first;

          double angle = 0;
          double length = 99999;
          unsigned int angleIndex = 0;
          unsigned int lengthIndex = 0;

          for( unsigned int k=1; k<bins[i].size(); k++ ) {

            Vector v1 = (Vector) bins[i][k].first -
              (Vector) tmp_bins[i][j].first;

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

        debug5 << "Winding " << i << " inserted "
             << prediction_true+prediction_false << " nodes "
             << " True " << prediction_true
             << " False " << prediction_false << endl;

        // If more of the predictions are incorrect than correct
        // insert based on the predictions.
        if( 0 && prediction_true < prediction_false ) {

          debug5 << "Winding " << i << " bad predicted insertion ";

          unsigned int cc = 0;

          for( unsigned int j=0; j<tmp_bins[i].size(); j++ ) {

            vector< pair< Point, double > >::iterator inList =
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
              
            Point l0_p0 = bins[i][j].first;
            Point l0_p1 = bins[i][j+1].first;
              
            for( unsigned int k=j+2; k<bins[i].size()-1; k++ ) {
                
              Point l1_p0 = bins[i][k].first;
              Point l1_p1 = bins[i][k+1].first;
                
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
                    vector < pair< Point, double > > tmp_bins[2];

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

    vector < pair< Point, double > > tmp_bins[winding];

    // This gives the minimal number of nodes for each group.
    surfaceOverlapCheck( bins, winding, skip, nnodes );

    if( nnodes == 0 ) {

      debug5 << "ZERO NODES " << endl;

      for( unsigned int i=0; i<winding; i++ )
        debug5 << i << "  " << bins[i].size() << endl;

      return nnodes;
    }

    for( unsigned int i=0; i<winding; i++ ) {

      // Add back in any nodes that may not overlap.
      unsigned int nodes =
        surfaceGroupCheck( bins, i, (i+skip)%winding, nnodes );

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

      // Insert the first point from the next winding so the curve
      // is contiguous.
      bins[i].push_back( bins[(i+skip)%winding][0] );
    }


    for( unsigned int i=0; i<winding; i++ ) {
      
      unsigned int winding_prediction = (i+skip)%winding;
      unsigned int index_prediction = 1;
      unsigned int prediction_true  = 0;
      unsigned int prediction_false = 0;

      for( unsigned int i0=0; i0<tmp_bins[i].size(); i0++ ) {

        double angle = 0;
        unsigned int index_wd = 0;
        unsigned int index_pt = 0;

        for( unsigned int j=0; j<winding; j++ ) {

          Vector v0 = (Vector) bins[j][0].first -
            (Vector) tmp_bins[i][i0].first;

          for( unsigned int j0=1; j0<bins[j].size(); j0++ ) {
            Vector v1 = (Vector) bins[j][j0].first -
              (Vector) tmp_bins[i][i0].first;
        
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

        debug5 << "Winding prediction " << winding_prediction
             << " actual " << index_wd
             << "  Index prediction  " << index_prediction
             << " actual " << index_pt << "  "
             << (index_wd == winding_prediction &&
                 index_pt == index_prediction) << endl;

        // Check to see if the prediction of where the point was inserted
        // is correct;
        if( index_wd == winding_prediction && index_pt == index_prediction )
          prediction_true++;
        else 
          prediction_false++;

        // Prediction of where the next insertion will take place.
        index_prediction = index_pt + 2;
        winding_prediction = index_wd;
      }

      debug5 << "Winding " << i << " inserted "
           << prediction_true+prediction_false << " nodes "
           << " True " << prediction_true
           << " False " << prediction_false << endl;
    }

    // Remove the last point so it is possilble to see the groups.
    for( unsigned int i=0; i<winding; i++ )
      bins[i].erase( bins[i].end() );
  }

  // Update the approximate node count.
  nnodes = 9999;

  for( unsigned int i=0; i<winding; i++ )
    if( nnodes > bins[i].size() )
      nnodes = bins[i].size();

  return nnodes;
}

}
