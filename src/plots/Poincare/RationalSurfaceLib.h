/*************************************
****************************************
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
//                            RationalSurfaceLib.h                           //
// ************************************************************************* //

#ifndef RATIONAL_SURFACE_LIB_H
#define RATIONAL_SURFACE_LIB_H



#include "FieldlineAnalyzerLib.h"

#include <math.h>
#include <map>

#define _USE_MATH_DEFINES

#include <avtVector.h>
#include <avtPoincareIC.h>

#include <vector>


// Rational Search Settings
//--------------------------
// This is the max distance new seeds will be placed from each other
#define MAX_SEED_SPACING               0.0125

// Sometimes the rational search gets caught in a loop. This is a hard limit on that.
#define MAX_ITERATIONS                 50

// I'm not using this right now, the purpose was superceded by the introduction of MAX_SPACING
#define MINIMIZING_SPACING_FACTOR      0.01

// This clarifies that what I'm seeing are all rationals, to make life easier while programming
#define HIDE_NON_RATIONALS             1

// This is to enable logging
#define RATIONAL_DEBUG                 1

// This basically says how close two intersections have to be on the plane for the curve to be considered a rational
#define MAX_SPACING                    .0001


// This is a constant used in the minimization algorithm. The golden ratio is essentially used to guess
// a new point if a minimum has not been bracketed
const double golden_R = 0.61803399, golden_C = 1 - golden_R;

// HELPER METHODS //
double PythDist( avtVector p1, avtVector p2 );
double GetAngle( avtVector a, avtVector b, avtVector c );
std::vector<avtVector> GetSeeds( avtPoincareIC *poincare_ic,
                                 double maxDistance = .025); //MAX_SEED_SPACING
float RationalDistance( std::vector< avtVector >& points,
                        unsigned int toroidalWinding,
                        unsigned int &index );
int FindMinimizationIndex( std::vector<avtVector> puncturePts,
                           avtVector pt1,
                           avtVector pt2);
#endif
