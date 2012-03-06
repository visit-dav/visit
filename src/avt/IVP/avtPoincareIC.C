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
//                              avtPoincareIC.C                             //
// ************************************************************************* //

#include <avtPoincareIC.h>

#include <list>
#include <cmath>
#include <cassert>
#include <iostream>
#include <limits>
#include <ImproperUseException.h>
#include <DebugStream.h>
#include <avtVector.h>
#include <algorithm>

#include <vtkPlane.h>


// ****************************************************************************
//  Method: avtPoincareIC constructor
//
//  Programmer: Hank Childs
//  Creation:   October 4, 2010
//
// ****************************************************************************

avtPoincareIC::avtPoincareIC(
    unsigned char mask,
    const avtIVPSolver* model, 
    Direction dir,
    const double& t_start,
    const avtVector &p_start,
    const avtVector &v_start,
    int ID) :
    avtStateRecorderIntegralCurve(mask, model, dir, t_start, p_start, v_start, ID)
{
    intersectionsSet = false;
    numIntersections = 0;
    maxIntersections = 0;
}


// ****************************************************************************
//  Method: avtPoincareIC constructor
//
//  Programmer: Hank Childs
//  Creation:   October 4, 2010
//
// ****************************************************************************

avtPoincareIC::avtPoincareIC() : avtStateRecorderIntegralCurve()
{
    intersectionsSet = false;
    numIntersections = 0;
    maxIntersections = 0;
}


// ****************************************************************************
//  Method: avtPoincareIC destructor
//
//  Programmer: Hank Childs
//  Creation:   October 4, 2010
//
// ****************************************************************************

avtPoincareIC::~avtPoincareIC()
{
}


// ****************************************************************************
//  Method: avtPoincareIC::CheckForTermination
//
//  Purpose:
//      Checks to see if we should terminate the streamline.
//
//  Programmer: Hank Childs
//  Creation:   October 4, 2010
//
//  Modifications:
//
//    Hank Childs, Sun Dec  5 10:18:13 PST 2010
//    Add avtIVPField argument.
//
// ****************************************************************************

bool
avtPoincareIC::CheckForTermination(avtIVPStep& step, avtIVPField *)
{
    if( IntersectPlane( step.GetP0(), step.GetP1() ) )
    {
        numIntersections++;

        if( numIntersections >= maxIntersections )
           return true;
    }

    return false;
}


// ****************************************************************************
//  Method: avtPoincareIC::SetIntersectionObject
//
//  Purpose:
//      Defines an object for integral curve intersection.
//
//  Programmer: Dave Pugmire
//  Creation:   August 10, 2009
//
//  Modifications:
//
//   Dave Pugmire, Tue Aug 18 08:47:40 EDT 2009
//   Store plane equation.
//
// ****************************************************************************

void
avtPoincareIC::SetIntersectionCriteria(vtkObject *obj, int maxInts)
{
    // Only plane supported for now.
    if (!obj->IsA("vtkPlane"))
        EXCEPTION1(ImproperUseException, "Can not SetIntersectionCriteria in avtPoincare, the Poincare plot only supports plane intersections.");


    intersectionsSet = true;
    avtVector intersectPlanePt = avtVector(((vtkPlane *)obj)->GetOrigin());
    avtVector intersectPlaneNorm = avtVector(((vtkPlane *)obj)->GetNormal());

    intersectPlaneNorm.normalize();
    intersectPlaneEq[0] = intersectPlaneNorm.x;
    intersectPlaneEq[1] = intersectPlaneNorm.y;
    intersectPlaneEq[2] = intersectPlaneNorm.z;
    intersectPlaneEq[3] = intersectPlanePt.length();

    maxIntersections = maxInts;
}


// ****************************************************************************
//  Method: avtPoincareIC::IntersectPlane
//
//  Purpose:
//      Intersect integral curve with a plane.
//
//  Programmer: Dave Pugmire
//  Creation:   August 10, 2009
//
//  Modifications:
//
//   Dave Pugmire, Tue Aug 18 08:47:40 EDT 2009
//   Don't record intersection points, just count them.
//
//    Dave Pugmire, Tue Dec  1 11:50:18 EST 2009
//    Switch from avtVec to avtVector.
//
// ****************************************************************************

bool
avtPoincareIC::IntersectPlane(const avtVector &p0, const avtVector &p1)
{
    double distP0 = intersectPlaneEq[0] * p0.x +
                    intersectPlaneEq[1] * p0.y +
                    intersectPlaneEq[2] * p0.z +
                    intersectPlaneEq[3];

    double distP1 = intersectPlaneEq[0] * p1.x +
                    intersectPlaneEq[1] * p1.y +
                    intersectPlaneEq[2] * p1.z +
                    intersectPlaneEq[3];

#define SIGN(x) ((x) < 0.0 ? -1 : 1)

    // If either point on the plane, or points on opposite
    // sides of the plane, the line intersects.
    if (distP0 == 0.0 || distP1 == 0.0 ||
        SIGN(distP0) != SIGN(distP1))
    {
        return true;
    }

    return false;
}


// ****************************************************************************
//  Method: avtPoincareIC::Serialize
//
//  Purpose:
//      Serializes a streamline so it can be sent to another processor.
//
//  Programmer: Hank Childs
//  Creation:   October 4, 2010
//
// ****************************************************************************

void
avtPoincareIC::Serialize(MemStream::Mode mode, MemStream &buff, 
                           avtIVPSolver *solver)
{
    // Have the base class serialize its part
    avtStateRecorderIntegralCurve::Serialize(mode, buff, solver);

    buff.io(mode, numIntersections);
}
