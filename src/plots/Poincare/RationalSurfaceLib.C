/*****************************************************************************
*
* Copyright (c) 2000 - 2012, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-442911
* All rights reserved.
*
* This file is  part of VisIt. For  details, see https://visit.llnl.gov/.  The
* full copyright notice is contained in the file COPYRIGHT located at the root
* of the VisIt distribution or at http://www.llnl.gov/visit/copyright.html.
*
* Redistribution  and  use  in  source  and  binary  forms,  with  or  without
* modification, are permitted provided that the following conditions are met:
*
*  - Redistributions of  source code must  retain the above  copyright notice,
*    this list of conditions and the disclaimer below.
*  - Redistributions in binary form must reproduce the above copyright notice,
*    this  list of  conditions  and  the  disclaimer (as noted below)  in  the
*    documentation and/or other materials provided with the distribution.
*  - Neither the name of  the LLNS/LLNL nor the names of  its contributors may
*    be used to endorse or promote products derived from this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED. IN  NO EVENT  SHALL LAWRENCE  LIVERMORE NATIONAL  SECURITY,
* LLC, THE  U.S.  DEPARTMENT OF  ENERGY  OR  CONTRIBUTORS BE  LIABLE  FOR  ANY
* DIRECT,  INDIRECT,   INCIDENTAL,   SPECIAL,   EXEMPLARY,  OR   CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

// ************************************************************************* //
//                            RationaSurfaceLib.C                            //
// ************************************************************************* //

#include "RationalSurfaceLib.h"
#include "FieldlineAnalyzerLib.h"

#include <math.h>
#include <map>

#define _USE_MATH_DEFINES

/**     
 *
 *      pythDist
 *      Get the pythagorean distance between two points
 *
 **/
double pythDist( avtVector p1, avtVector p2 )
{
  return (p1 - p2).length();
}


/**     
 *
 *      getAngle
 *      Get the angle between three points, let b be the center.
 *
 */
double getAngle( avtVector a, avtVector b, avtVector c )
{
  return acos( (pythDist(b,a)*pythDist(b,a) + 
                pythDist(b,c)*pythDist(b,c) -
                pythDist(c,a)*pythDist(c,a)) / 
               (2.0f * pythDist(b,a) * pythDist(b,c)) );
}
        

/**
 *
 * return a vector of seed points for the given rational
 *
 **/
std::vector<avtVector> getSeeds( avtPoincareIC *poincare_ic,
                                 double maxDistance )
{
  std::vector<avtVector> puncturePoints;

  FieldlineLib fieldlib;
  fieldlib.getPunctures(poincare_ic->points, avtVector(0, 1, 0), puncturePoints);

  unsigned int toroidalWinding = poincare_ic->properties.toroidalWinding;
  unsigned int windingGroupOffset = poincare_ic->properties.windingGroupOffset;
  
  // Calculate angle size for each puncture point and find the one
  // that forms the largest angle (i.e. flatest protion of the
  // surface).
  int nSeeds = 0;
  double maxAngle = 0;
  unsigned int index = 0;

  for( int i=0; i<toroidalWinding; ++i )
  {
    avtVector pt0 = puncturePoints[(i-windingGroupOffset+toroidalWinding) % toroidalWinding];
    avtVector pt1 = puncturePoints[i];
    avtVector pt2 = puncturePoints[(i+windingGroupOffset) % toroidalWinding];

    // Save the maximum angle angle and the index of the puncture point.
    double angle = getAngle( pt0, pt1, pt2 );

    if (maxAngle < angle)
    {
      maxAngle = angle;
      index = i;
    }

    // Get the number of points needed to cover the range between two of
    // the puncture points.
    double dist = pythDist(pt1, pt2);

    unsigned int seeds = dist / maxDistance;

    if( dist > (double) seeds * maxDistance )
      ++seeds;

    // Use the greatest number of seeds necessary.
    if (nSeeds < seeds)
    {
      nSeeds = seeds;
    }
  }

//   index = puncturePoints.size() - 1;
//   toroidalWinding = puncturePoints.size();

  if (RATIONAL_DEBUG)
    std::cerr << "************************************"
              << index << "  " << puncturePoints.size()-1 << std::endl;

  // Find the circle that intersects the three punctures points which
  // approximates the cross section of the surface.

  // Get circle equation 
  avtVector pt0 = puncturePoints[(index - windingGroupOffset+toroidalWinding) % toroidalWinding];
  avtVector pt1 = puncturePoints[index];
  avtVector pt2 = puncturePoints[(index + windingGroupOffset) % toroidalWinding];

  // Slopes
  double ma = (pt1[2] - pt2[2]) / (pt1[0] - pt2[0]);
  double mb = (pt0[2] - pt1[2]) / (pt0[0] - pt1[0]);

  // Center of the circle
  avtVector center;
  center[0] = (ma *
               mb * (pt2[2] - pt0[2]) +
               mb * (pt2[0] + pt1[0]) -
               ma * (pt1[0] + pt0[0])) / (2 * (mb - ma));
  center[1] = 0;
  center[2] = (-(1/ma) * (center[0] - (pt2[0] + pt1[0]) / 2) +
               (pt2[2] + pt1[2]) / 2);

  // Radius of the circle
  double radius = pythDist(center, pt1);

  // Angle between two of the puncture points.
  double delta = getAngle(pt1, center, pt0) / nSeeds;

  avtVector horizontal = center + avtVector(radius, 0, 0);

  // Base angle from the center puncture point.
  double theta = getAngle(horizontal,center,pt1);

  if (RATIONAL_DEBUG)
    std::cerr << "center " << center << "  radius " << radius << "  "
              << "nSeeds " << nSeeds
              << std::endl;

  // Add seeds stretching between two of the puncture points.
  std::vector<avtVector> seedPts;
  seedPts.resize( nSeeds );

  for( int i=0; i<nSeeds; ++i, theta += delta )
  {
    seedPts[i] = center + radius * avtVector(cos(theta), 0, sin(theta));

    if (RATIONAL_DEBUG)
      std::cerr << "seed " << i << "  " << seedPts[i] << std::endl;
  }

  return seedPts;
}


/**
 *
 * Look at the distance between the centroid of each toroidal group
 * and the points that are in it.
 *
 **/
float rationalDistance( std::vector< avtVector >& points,
                        unsigned int toroidalWinding,
                        unsigned int &index )
{
  float delta = 0;

  for( unsigned int i=0; i<toroidalWinding; i++ )
  {
    // Get the local centroid for the toroidal group.
    avtVector localCentroid(0,0,0);

    int npts = 0;

    for( unsigned int j=i; j<points.size(); j+=toroidalWinding, ++npts )
      localCentroid += (Vector) points[j];

    localCentroid /= (float) npts;

    for( unsigned int j=i; j<points.size(); j+=toroidalWinding )
    {
      avtVector vec = (avtVector) points[j] - localCentroid;

      if( delta < vec.length() )
      {
        delta = vec.length();
        index = j;
      }
    }
  }

  return delta;
}


/**
 *
 * Takes in a curve, returns the index of puncture point contained
 * between pt0 and pt1.
 *
 **/
int findMinimizationIndex( std::vector<avtVector> puncturePts,
                           avtVector pt0,
                           avtVector pt1 )
{
  for( unsigned int i=0; i<puncturePts.size(); ++i )
  {
    avtVector puncture = puncturePts[i];

    double a = pythDist(pt0, puncture);
    double b = pythDist(pt1, puncture);
    double c = pythDist(pt0, pt1);

    if( (pt0[0] < puncture[0] && pt1[0] > puncture[0] || 
         pt0[0] > puncture[0] && pt1[0] < puncture[0]) ||
        (pt0[2] < puncture[2] && pt1[2] > puncture[2] || 
         pt0[2] > puncture[2] && pt1[2] < puncture[2]) && 
        a < c &&
        b < c )
    {
      return i;
    }
  }

  if (RATIONAL_DEBUG)
  {
    std::cerr << __LINE__ << "  "
              << "Failed to locate index for points: "
              << pt0 <<" , " << pt1 << std::endl;
    std::cerr << __LINE__ << "  "
              << "Size of puncturePts: "<<puncturePts.size() << std::endl;
    std::cerr << __LINE__ << "  " << std::endl;
  }

  return -1;
}
