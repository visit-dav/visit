// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                             avtIntegralCurveIC.C                             //
// ************************************************************************* //

#include <avtIntegralCurveIC.h>

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
//  Method: avtIntegralCurveIC constructor
//
//  Programmer: Hank Childs
//  Creation:   October 4, 2010
//
//  Modifications:
//
//    Hank Childs, Sun Dec  5 10:18:13 PST 2010
//    Initialize new data members for issuing warnings.
//
//    Hank Childs, Sun Mar 11 11:16:56 PDT 2012
//    Change handling for backwards pathlines and maxTime ... it is now
//    handled at a higher level.
//
// ****************************************************************************

avtIntegralCurveIC::avtIntegralCurveIC(
    int maxSteps_,
    bool doDistance_,
    double maxDistance_,
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
    numSteps = 0;

    maxSteps = maxSteps_;
    doDistance = doDistance_;
    maxDistance = maxDistance_;

    doTime = doTime_;
    maxTime = maxTime_;

    terminatedBecauseOfMaxSteps  = false;
}


// ****************************************************************************
//  Method: avtIntegralCurveIC constructor
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

avtIntegralCurveIC::avtIntegralCurveIC() : avtStateRecorderIntegralCurve()
{
    numSteps = 0;

    maxSteps = 0;
    doDistance = false;
    maxDistance = 0.;
    doTime = false;
    maxTime = 0.;

    terminatedBecauseOfMaxSteps = false;
}


// ****************************************************************************
//  Method: avtIntegralCurveIC destructor
//
//  Programmer: Hank Childs
//  Creation:   October 4, 2010
//
// ****************************************************************************

avtIntegralCurveIC::~avtIntegralCurveIC()
{
}


// ****************************************************************************
//  Method: avtIntegralCurveIC::CheckForTermination
//
//  Purpose:
//      Checks to see if we should terminate the integral curve.
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
avtIntegralCurveIC::CheckForTermination(avtIVPStep& step, avtIVPField *field)
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

    ++numSteps;

    if( !shouldTerminate && numSteps >= maxSteps )
    {
        terminatedBecauseOfMaxSteps = true;
        shouldTerminate = true;
    }

    return shouldTerminate;
}


// ****************************************************************************
//  Method: avtIntegralCurveIC::Serialize
//
//  Purpose:
//      Serializes a integral curve so it can be sent to another processor.
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
//   David Camp, Wed Mar  7 10:43:07 PST 2012
//   Added a Serialize flag to the arguments. This is to support the restore
//   ICs code.
//
// ****************************************************************************

void
avtIntegralCurveIC::Serialize(MemStream::Mode mode, MemStream &buff, 
                           avtIVPSolver *solver, SerializeFlags serializeFlags)
{
    // Have the base class serialize its part
    avtStateRecorderIntegralCurve::Serialize(mode, buff, solver, serializeFlags);

    buff.io(mode, numSteps);
    buff.io(mode, maxSteps);
    buff.io(mode, doDistance);
    buff.io(mode, maxDistance);
    buff.io(mode, doTime);
    buff.io(mode, maxTime);
    buff.io(mode, terminatedBecauseOfMaxSteps);
}


// ****************************************************************************
//  Method: avtIntegralCurveIC::MergeIntegralCurve
//
//  Purpose:
//      Merge a values from one curve into another
//
//  Programmer: Allen Sanderson
//  Creation:   August 4, 2015
//
// ****************************************************************************

void
avtIntegralCurveIC::MergeIntegralCurve(avtIntegralCurve *ic)
{
  avtIntegralCurveIC *sic = (avtIntegralCurveIC* ) ic;

  numSteps = sic->numSteps;
  terminatedBecauseOfMaxSteps = sic->terminatedBecauseOfMaxSteps;
}
