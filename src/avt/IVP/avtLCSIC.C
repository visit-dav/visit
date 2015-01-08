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
//                             avtLCSIC.C                             //
// ************************************************************************* //

#include <avtLCSIC.h>

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
//  Method: avtLCSIC constructor
//
//  Programmer: Allen Sanderson
//  Creation:   August 14, 2013
//
//  Modifications:
//
// ****************************************************************************
avtLCSIC::avtLCSIC(
    int maxSteps_,
    bool doDistance_,
    double maxDistance_,
    bool doTime_,
    double maxTime_,
    const avtIVPSolver* model, 
    Direction dir,
    const double& t_start_,
    const avtVector &p_start_,
    const avtVector &v_start_,
    int ID) : avtIntegralCurve(model, dir, t_start_, p_start_, v_start_, ID)
{
    numSteps = 0;
    maxSteps = maxSteps_;

    arcLength = 0.;
    distance = 0.;
    doDistance = doDistance_;
    maxDistance = maxDistance_;

    time = 0.;
    doTime = doTime_;
    maxTime = maxTime_;
    
    terminatedBecauseOfMaxSteps = false;

    p_start = p_start_;
    p_end = p_start_; /* the end and start begin at the same location */

    summation0 = 0.;
    summation1 = 0.;
}


// ****************************************************************************
//  Method: avtLCSIC constructor
//
//  Programmer: Allen Sanderson
//  Creation:   August 14, 2013
//
//  Modifications:
//
// ****************************************************************************
avtLCSIC::avtLCSIC() : avtIntegralCurve()
{
    numSteps = 0;

    maxSteps = 0;
    doDistance = false;
    maxDistance = 0.;
    doTime = false;
    maxTime = 0.;

    time      = 0.;
    arcLength = 0.;
    distance  = 0.;

    summation0 = 0.;
    summation1 = 0.;
    
    terminatedBecauseOfMaxSteps = false;
}


// ****************************************************************************
//  Method: avtLCSIC destructor
//
//  Programmer: Allen Sanderson
//  Creation:   August 14, 2013
//
// ****************************************************************************

avtLCSIC::~avtLCSIC()
{
}


// ****************************************************************************
//  Method: avtLCSIC::LessThan
//
//  Programmer: Allen Sanderson
//  Creation:   August 14, 2013
//
//  Modifications:
//
// ****************************************************************************
bool avtLCSIC::LessThan(const avtIntegralCurve *ic) const
{
  if (ic != NULL && (ic->id == id))
  {
    return numSteps < ((avtLCSIC *)ic)->numSteps;
  }

  return avtIntegralCurve::LessThan(ic);
}


// ****************************************************************************
//  Method: avtLCSIC::MergeIntegralCurveSequence
//
//  Programmer: Allen Sanderson
//  Creation:   August 14, 2013
//
//  Modifications:
//
// ****************************************************************************
avtIntegralCurve*
avtLCSIC::MergeIntegralCurveSequence(std::vector<avtIntegralCurve *> &v)
{
    for (size_t i = 0 ; i < v.size() ; i++)
    {
        avtLCSIC *ic = (avtLCSIC *) v[i];

        if (ic->numSteps > numSteps)
        {
          numSteps = ic->numSteps;
          p_end = ic->p_end;
        }
    }
    return this;
}


// ****************************************************************************
//  Method: avtLCSIC::AnalyzeStep
//
//  Purpose:
//      Analyzes the current step.
//
//  Programmer: Allen Sanderson
//  Creation:   August 14, 2013
//
//  Modifications:
//
// ****************************************************************************
void avtLCSIC::AnalyzeStep( avtIVPStep &step,
                            avtIVPField *field)
{
    if (CheckForTermination(step, field))
        status.SetTerminationMet();

    // These must be called after CheckForTermination, because 
    // CheckForTermination will modify the step if it goes beyond the
    // termination criteria.  (Example: streamlines will split a step if it
    // is terminating by distance.)
    p_end = step.GetP1();

    time = step.GetT1();
    arcLength += step.GetLength();
    distance  += step.GetLength();

    summation0 += (p_start - p_end).length();
    summation1  = (p_start - p_end).length();
}

// ****************************************************************************
//  Method: avtLCSIC::CheckForTermination
//
//  Purpose:
//      Checks to see if we should terminate the LCS.
//
//  Programmer: Allen Sanderson
//  Creation:   August 14, 2013
//
//  Modifications:
//
// ****************************************************************************

bool
avtLCSIC::CheckForTermination(avtIVPStep& step, avtIVPField *field)
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
        shouldTerminate = true;
    }

    // Update other termination criteria.
    numSteps += 1;

    return shouldTerminate;
}


// ****************************************************************************
//  Method: avtLCSIC::Serialize
//
//  Purpose:
//      Serializes a LCS so it can be sent to another processor.
//
//  Programmer: Allen Sanderson
//  Creation:   August 14, 2013
//
//  Modifications:
//
// ****************************************************************************
void avtLCSIC::Serialize(MemStream::Mode mode, MemStream &buff,
                          avtIVPSolver *solver, SerializeFlags serializeFlags)
{
    avtIntegralCurve::Serialize(mode,buff,solver,serializeFlags);

    buff.io(mode, numSteps);
    buff.io(mode, maxSteps);
    buff.io(mode, doDistance);
    buff.io(mode, maxDistance);
    buff.io(mode, doTime);
    buff.io(mode, maxTime);
    buff.io(mode, distance);
    buff.io(mode, terminatedBecauseOfMaxSteps);
    buff.io(mode, p_start);
    buff.io(mode, p_end);
}
