/*****************************************************************************
*
* Copyright (c) 2000 - 2015, Lawrence Livermore National Security, LLC
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

#include <float.h>

// ****************************************************************************
//  Method: avtPoincareIC constructor
//
//  Programmer: Hank Childs
//  Creation:   October 4, 2010
//
// ****************************************************************************

avtPoincareIC::avtPoincareIC(
    int maxSteps_,
    bool doTime_,
    double maxTime_,
    unsigned int mask,
    const avtIVPSolver* model, 
    Direction dir,
    const double& t_start,
    const avtVector &p_start,
    const avtVector &v_start,
    int ID) :
    avtStateRecorderIntegralCurve(mask, model, dir, t_start, p_start, v_start, ID)
{
    numIntersections = 0;
    maxIntersections = 0;

    terminatedBecauseOfMaxIntersections = false;

    puncturePeriod          = 0;
    puncturePeriodTolerance = 0;

    numSteps = 0;
    maxSteps = maxSteps_;

    doTime = doTime_;
    maxTime = maxTime_;

    terminatedBecauseOfMaxSteps  = false;
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
    numIntersections = 0;
    maxIntersections = 0;

    puncturePeriod          = 0;
    puncturePeriodTolerance = 0;

    terminatedBecauseOfMaxIntersections = false;

    puncturePeriod          = 0;
    puncturePeriodTolerance = 0;

    numSteps = 0;
    maxSteps = 0;

    doTime = false;
    maxTime = 0;

    terminatedBecauseOfMaxSteps  = false;
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
    bool shouldTerminate = false;

    // Check for termination.
    if (IntersectPlane( step.GetP0(), step.GetP1(),
                        step.GetT0(), step.GetT1() ))
    {
        numIntersections++;

        if (numIntersections >= maxIntersections)
        {
            terminatedBecauseOfMaxIntersections = true;
            shouldTerminate = true;
        }
    }

    // When doing a double puncture plot many points are exained and
    // one may not get enough points for the analysis thus allow a cut
    // off so not to integrate forever.
    if( puncturePeriod )
    {
      if( doTime )
      {
        if( (direction == DIRECTION_FORWARD  && step.GetT1() >= maxTime) ||
            (direction == DIRECTION_BACKWARD && step.GetT1() <= maxTime) )
            shouldTerminate = true;
      }
      
      // If max steps is zero ignore the test.
      if( !shouldTerminate && maxSteps && numSteps >= maxSteps )
      {
        terminatedBecauseOfMaxSteps = true;
        shouldTerminate = true;
      }
    }

    // Update other termination criteria.
    numSteps += 1;

    return shouldTerminate;
}


// ****************************************************************************
//  Method: avtPoincareIC::SetIntersectionCriteria
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
//  Method: avtPoincareIC::SetPuncturePeriodCriteria
//
//  Purpose:
//      Defines an object for integral curve puncture period intersection.
//
//      This criteria is used for the double Poincare section
//
//  Programmer: Allen Sanderson
//  Creation:   April 10, 2014
//
//  Modifications:
//
// ****************************************************************************

void
avtPoincareIC::SetPuncturePeriodCriteria( double period, double tolerance )
{
    puncturePeriod          = period;
    puncturePeriodTolerance = tolerance;
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
//   Dave Pugmire, Tue Dec  1 11:50:18 EST 2009
//   Switch from avtVec to avtVector.
//
// ****************************************************************************

bool
avtPoincareIC::IntersectPlane(const avtVector &p0, const avtVector &p1,
                              const double    &t0, const double    &t1)
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
    if( //distP0 == 0.0 || distP1 == 0.0 ||
        SIGN(distP0) != SIGN(distP1) )
    {
      avtVector dir(p1-p0);
      
      double dot = intersectPlaneEq[0] * dir.x +
                   intersectPlaneEq[1] * dir.y +
                   intersectPlaneEq[2] * dir.z +
                   intersectPlaneEq[3];

      // If the segment is in the same direction as the poloidal plane
      // then find where it intersects the plane.
      if( dot > 0.0 )
      {
        // Double Poincare puncture test.
        if( puncturePeriod > 0 )
        {
          avtVector planePt(0,0,0);

          avtVector w = (avtVector) p0 - planePt;
          
          double t = -(intersectPlaneEq[0] * w.x +
                       intersectPlaneEq[1] * w.y +
                       intersectPlaneEq[2] * w.z +
                       intersectPlaneEq[3]) / dot;
          
//        avtVector point = avtVector(p0 + dir * t);

          double time = t0 + (t1-t0) * t;

          // Get the number of periods traversed.
          double nPeriods = time / puncturePeriod;

          // Get the factional part - should be close
          // to zero for an even period.
          double intPart, fracPart = modf(nPeriods, &intPart);
          
          // std::cerr << t0 << "  " << t1 << "  "
          //        << time << "  " << nPeriods << "  "
          //        << intPart << "  " << fracPart << "  "
          //        << (fracPart < puncturePeriodTolerance ||
          //            1.0-puncturePeriodTolerance < fracPart ? "save" : "")
          //        << std::endl;
          
          if( fracPart < puncturePeriodTolerance ||
              1.0-puncturePeriodTolerance < fracPart )
            return true;
        }
        else
          return true;
      }
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
//   David Camp, Wed Mar  7 10:43:07 PST 2012
//   Added a Serialize flag to the arguments. This is to support the restore
//   ICs code.
//
// ****************************************************************************

void
avtPoincareIC::Serialize(MemStream::Mode mode, MemStream &buff, 
                         avtIVPSolver *solver, SerializeFlags serializeFlags)
{
    // Have the base class serialize its part
    avtStateRecorderIntegralCurve::Serialize(mode, buff, solver, serializeFlags);

    buff.io(mode, numIntersections);
}
