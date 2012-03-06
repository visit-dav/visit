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
//                             avtStreamlineIC.C                             //
// ************************************************************************* //

#include <avtStreamlineIC.h>

#include <list>
#include <cmath>
#include <cassert>
#include <iostream>
#include <limits>
#include <ImproperUseException.h>
#include <DebugStream.h>
#include <avtVector.h>
#include <algorithm>


// ****************************************************************************
//  Method: avtStreamlineIC constructor
//
//  Programmer: Hank Childs
//  Creation:   October 4, 2010
//
//  Modifications:
//
//    Hank Childs, Sun Dec  5 10:18:13 PST 2010
//    Initialize new data members for issuing warnings.
//
// ****************************************************************************

avtStreamlineIC::avtStreamlineIC(
    int maxSteps_,
    bool doDistance_,
    double maxDistance_,
    bool doTime_,
    double maxTime_,
    unsigned char mask,
    const avtIVPSolver* model, 
    Direction dir,
    const double& t_start,
    const avtVector &p_start,
    const avtVector &v_start,
    int ID) :
    avtStateRecorderIntegralCurve(mask, model, dir, t_start, p_start, v_start, ID)
{
    numSteps = 0;

    maxSteps = maxSteps_;
    doDistance = doDistance_;
    maxDistance = maxDistance_;

    doTime = doTime_;
    maxTime = maxTime_;
    if (dir == DIRECTION_BACKWARD)
        maxTime = -maxTime;

    terminatedBecauseOfMaxSteps  = false;
    speedAtTermination = 0.;
}


// ****************************************************************************
//  Method: avtStreamlineIC constructor
//
//  Programmer: Hank Childs
//  Creation:   October 4, 2010
//
//  Modifications:
//
//    Hank Childs, Sun Dec  5 10:18:13 PST 2010
//    Initialize new data members for issuing warnings.
//
// ****************************************************************************

avtStreamlineIC::avtStreamlineIC() : avtStateRecorderIntegralCurve()
{
    numSteps = 0;

    maxSteps = 0;
    doDistance = false;
    maxDistance = 0.;
    doTime = false;
    maxTime = 0.;

    terminatedBecauseOfMaxSteps = false;
    speedAtTermination = 0.;
}


// ****************************************************************************
//  Method: avtStreamlineIC destructor
//
//  Programmer: Hank Childs
//  Creation:   October 4, 2010
//
// ****************************************************************************

avtStreamlineIC::~avtStreamlineIC()
{
}


// ****************************************************************************
//  Method: avtStreamlineIC::CheckForTermination
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
//    Add avtIVPField argument.  Only terminate at critical points when we
//    are advecting over an instantaneous velocity field.  Otherwise, with
//    pathlines, we might have zero vel for a while that picks up later in
//    time.
//
// ****************************************************************************

bool
avtStreamlineIC::CheckForTermination(avtIVPStep& step, avtIVPField *field)
{
    bool shouldTerminate = false;

    // Check for termination.
    if( doTime )
    {
        if( (direction == DIRECTION_FORWARD  && step.GetT1() >= maxTime) ||
            (direction == DIRECTION_BACKWARD && step.GetT1() <= maxTime) )
            shouldTerminate = true;
    }
    if( doDistance )
    {
        double Lstep = step.GetLength();
        double Ltogo = std::abs(maxDistance) - distance;

        if( Lstep > Ltogo )
        {
            step.ClampToLength( Ltogo );
            shouldTerminate = true;
        }
        else if (field->VelocityIsInstantaneous() &&
                 (Lstep / std::abs(step.t1 - step.t0) < 1e-8))
        {
            {
                // Above condition ensures that the curve makes progress 
                // w.r.t. distance to avoid infinite integration into a 
                // critical point.
                //
                // FIXME: I don't like the above hardcoded threshold -
                // this should probably be something like 
                // Lstep / Lmax < 1e-6 ?
                //
                // FIXME: Also, this should only be tested in the stationary 
                // case, since in a time-varying scenario, the critical point
                // might move.
                shouldTerminate = true;
            }
        }
    }

    if( !shouldTerminate && numSteps >= maxSteps )
    {
        terminatedBecauseOfMaxSteps = true;
        speedAtTermination = step.GetV(step.GetT1()).length();
        shouldTerminate = true;
    }

    // Update other termination criteria.
    numSteps += 1;

    // Done by state recorder, which may need to record state
    //distance += step.GetLength();

    return shouldTerminate;
}


// ****************************************************************************
//  Method: avtStreamlineIC::Serialize
//
//  Purpose:
//      Serializes a streamline so it can be sent to another processor.
//
//  Programmer: Hank Childs
//  Creation:   October 4, 2010
//
//  Modifications:
//
//    Hank Childs, Sun Dec  5 11:43:46 PST 2010
//    Communicate new data members.
//
//    David Camp, Mon Jan 17 12:48:18 PST 2011
//    Added missing data members to list.
//
// ****************************************************************************

void
avtStreamlineIC::Serialize(MemStream::Mode mode, MemStream &buff, 
                           avtIVPSolver *solver)
{
    // Have the base class serialize its part
    avtStateRecorderIntegralCurve::Serialize(mode, buff, solver);

    buff.io(mode, numSteps);
    buff.io(mode, maxSteps);
    buff.io(mode, doDistance);
    buff.io(mode, maxDistance);
    buff.io(mode, doTime);
    buff.io(mode, maxTime);
    buff.io(mode, terminatedBecauseOfMaxSteps);
    buff.io(mode, speedAtTermination);
}
