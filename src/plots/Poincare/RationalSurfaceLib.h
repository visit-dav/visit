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
//                            RationaSurfaceLib.h                            //
// ************************************************************************* //

#ifndef RATIONAL_SURFACE_LIB_H
#define RATIONAL_SURFACE_LIB_H

#include <avtVector.h>
#include <avtPoincareIC.h>

#include <vector>


const bool RATIONAL_DEBUG = 1;

const double golden_R = 0.61803399, golden_C = 1 - golden_R;

/**     
 *
 *      pythDist
 *      Get the pythagorean distance between two points on the xz-plane
 *
 **/
double pythDist( avtVector p1, avtVector p2 );

/**     
 *
 *      getAngle
 *      Get the angle between three points, let c be the center.
 *
 */
double getAngle( avtVector a, avtVector b, avtVector c );

/**
 *      getSeeds
 *      Return a vector of seed points for the given rational
 *
 **/
std::vector<avtVector> getSeeds( avtPoincareIC *poincare_ic,
                                 double maxDistance = .025);


/**
 *
 * Look at the distance between the centroid of each toroidal group
 * and the points that are in it.
 *
 **/
float rationalDistance( std::vector< avtVector >& points,
                        unsigned int toroidalWinding,
                        unsigned int &index );

/**
 *      findMinimizationIndex
 *      Takes in a curve, returns index of puncture point contained between
 *      pt1 and pt2
 *
 **/
int findMinimizationIndex( std::vector<avtVector> puncturePts,
                           avtVector pt1,
                           avtVector pt2);
#endif
