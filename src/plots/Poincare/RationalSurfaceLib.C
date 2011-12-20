/*****************************************************************************
*
* Copyright (c) 2000 - 2011, Lawrence Livermore National Security, LLC
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
float pythDist(avtVector p1, avtVector p2)
{
  return (p1-p2).length();
}

/**     
 *
 *      getAngle
 *      Get the angle between three points, let c be the center.
 *
 */
float getAngle(avtVector a, avtVector b, avtVector c)
{
  return acos( (pythDist(c,a)*pythDist(c,a) + 
                pythDist(c,b)*pythDist(c,b) -
                pythDist(b,a)*pythDist(b,a)) / 
               (2.0f * pythDist(c,a) * pythDist(c,b)) );
}
        
/**     
 *
 *      getPunctureAngles
 *      Get the angle each puncture point forms with its immediate neighbors
 *
 */
std::vector<float> getPunctureAngles(std::vector<avtVector> puncturePts,
                                     int skip)
{
  std::vector<float> punctureAngles;
  avtVector p0,p1,p2;

  for( int i=0; i<puncturePts.size(); ++i )
  {
    p0 = puncturePts[i]; //vertex we want the angle of
    p1 = puncturePts[(i+skip) % puncturePts.size()];

    if (i - skip < 0)
      p2 = puncturePts[puncturePts.size() + i - skip];
    else
      p2 = puncturePts[(i-skip) % puncturePts.size()];
    
    float angle = getAngle( p2, p1, p0 );

    punctureAngles.push_back(angle);
  }

  return punctureAngles;
}


/**
 *
 * return a vector of seed points for the given rational
 *
 **/
std::vector<avtVector> getSeeds(avtPoincareIC *poincare_ic)
{
  std::vector<avtVector> puncturePts;
  avtVector xzplane(0,1,0);
  FieldlineLib fieldlib;
  fieldlib.getPunctures(poincare_ic->points,xzplane,puncturePts);
  int skip = fieldlib.Blankinship(poincare_ic->properties.toroidalWinding, poincare_ic->properties.poloidalWinding, 1);
  
  // Calculate angle size for each puncture points
  std::vector<float> puncturePtAngles;
  avtVector p0,p1,p2;

  double maxAngle = 0;
  int max = 0;

  for( int i=0; i<puncturePts.size(); ++i )
  {
    p0 = puncturePts[i]; //vertex we want the angle of
    p1 = puncturePts[(i+skip) % puncturePts.size()];

    if (i - skip < 0)
      p2 = puncturePts[puncturePts.size() + i - skip];
    else
      p2 = puncturePts[(i-skip) % puncturePts.size()];
    
    float angle = getAngle( p2, p1, p0 );

    if (maxAngle < angle)
    {
      maxAngle = angle;

      max = i;
    }
  }

  // Get Circle Equation
  avtVector pt1 = puncturePts[(puncturePts.size() + max + skip) % puncturePts.size()];
  avtVector pt2 = puncturePts[max]; // this is the puncture point at the center of the max angle
  avtVector pt3 = puncturePts[(puncturePts.size() + max - skip) % puncturePts.size()];
  // slopes
  float ma = (pt2[2] - pt1[2]) / (pt2[0] - pt1[0]);
  float mb = (pt3[2] - pt2[2]) / (pt3[0] - pt2[0]);

  // center
  float x = (ma * mb * (pt1[2] - pt3[2]) + mb * (pt1[0] + pt2[0]) - ma * (pt2[0] + pt3[0])) / (2 * (mb - ma));
  float z = -(1/ma) * (x - (pt1[0] + pt2[0]) / 2) + (pt1[2] + pt2[2]) / 2;
  avtVector center;
  center[0] = x;
  center[2] = z;

  // radius
  float dx = pt1[0] - x;
  float dz = pt1[2] - z;
  float r = sqrt(dx*dx + dz*dz);

  // Figure out the angle to increment by
  std::vector<avtVector> cPoints;
  float dist = pythDist(pt2,pt3);
  float n = dist / .005; // Number of total points needed to cover the range between rational points
                        //  The optimal spacing factor is hardcoded here...
  float alpha = getAngle(pt2,pt3,center) / n;
  int i = 0;
  avtVector point;

  //Add seeds stretching between two of the rationals points
  avtVector horizontal;
  horizontal[0] = x+r;
  horizontal[2] = z;
  float theta = getAngle(horizontal,pt2,center);

  while (i++ < n)
  {
    point[0] = cos(theta) * r + x;
    point[2] = sin(theta) * r + z;
    cPoints.push_back(point);
    theta += alpha;
  }

  return cPoints;
}


// Takes in a curve, returns index of puncture point contained between pt1 and pt2
int findMinimizationIndex(std::vector<avtVector> puncturePts,
                          avtVector pt1,
                          avtVector pt2)
{
  for (int i = 0; i < puncturePts.size(); i++)
  {
    avtVector puncture = puncturePts[i];
    float c = pythDist(pt1,pt2);
    float a = pythDist(pt1,puncture);
    float b = pythDist(pt2,puncture);

    if ((pt1[0] < puncture[0] && pt2[0] > puncture[0] || 
         pt1[0] > puncture[0] && pt2[0] < puncture[0]) ||
        (pt1[2] < puncture[2] && pt2[2] > puncture[2] || 
         pt1[2] > puncture[2] && pt2[2] < puncture[2]) && 
        a < c &&
        b < c)
    {
      return i;
    }
  }

  if (RATIONAL_DEBUG)
  {
    std::cerr << __LINE__ << "  "
              << "Failed to locate index for points: "
              << pt1 <<" , " << pt2 << std::endl;
    std::cerr << __LINE__ << "  "
              << "Size of puncturePts: "<<puncturePts.size() << std::endl;
    std::cerr << __LINE__ << "  " << std::endl;
  }

  return -1;
}
