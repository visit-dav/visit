// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                              avtIVPEuler.C                       //
// ************************************************************************* //

#include <avtIVPEuler.h>
#include <avtIVPStateHelper.h>
#include <DebugStream.h>

#include <limits>
#include <cmath>

#include <avtIVPFlashField.h>

static const double epsilon = std::numeric_limits<double>::epsilon();

// helper function
// returns a with the same sign as b
static inline double sign( const double& a, const double& b )
{
    return (b > 0.0) ? std::abs(a) : -std::abs(a);
}

// ****************************************************************************
//  Method: avtIVPEuler constructor
//
//  Programmer: Dave Pugmire
//  Creation:   August 5, 2008
//
//  Modifications:
//    Dave Pugmire, Fri Aug  8 16:05:34 EDT 2008
//    Improved version of A-B solver that builds function history from
//    initial RK4 steps.
//
//    Dave Pugmire, Tue Aug 19, 17:38:03 EDT 2008
//    Changed how distanced based termination is computed.
//
//    Dave Pugmire, Wed Aug 20, 12:54:44 EDT 2008
//    Add a tolerance and counter for handling stiffness detection.
//
//    Dave Pugmire, Tue Feb 24 14:35:38 EST 2009
//    Remove moulton corrector code, use RK4 at startup, terminate on numStepss.
//
// ****************************************************************************

avtIVPEuler::avtIVPEuler()
{
    // set (somewhat) reasonable defaults
    tol = 1e-8;
    h = 1e-5;
    t = 0.0;

    order = 2; // Highest order ODE that the integrator can support.
}

// ****************************************************************************
//  Method: avtIVPEuler destructor
//
//  Programmer: Dave Pugmire
//  Creation:   August 5, 2008
//
// ****************************************************************************


avtIVPEuler::~avtIVPEuler()
{
}

// ****************************************************************************
//  Method: avtIVPEuler::Reset
//
//  Purpose:
//      Resets data members.  Called by the constructors.
//
//  Programmer: Dave Pugmire
//  Creation:   August 5, 2008
//
//  Modifications:
//    Dave Pugmire, Fri Aug  8 16:05:34 EDT 2008
//    Improved version of A-B solver that builds function history from
//    initial RK4 steps.
//
//    Dave Pugmire, Tue Aug 19, 17:38:03 EDT 2008
//    Changed how distanced based termination is computed.
//
//    Dave Pugmire, Wed Aug 20, 12:54:44 EDT 2008
//    Add a tolerance and counter for handling stiffness detection.
//
//    Dave Pugmire, Mon Feb 23, 09:11:34 EST 2009
//    Reworked the termination code. Added a type enum and value. Made num steps
//    a termination criterion. Code cleanup: We no longer need fwd/bwd solvers.
//    Removed the plane intersection code.
//
//    Dave Pugmire, Tue Mar 10 12:41:11 EDT 2009
//    Bug fix in parallel communication of solver state.
//
//    Dave Pugmire, Tue May  5 10:43:05 EDT 2009
//    Memory issue with history init. Make sure vecs are of proper size.
//
//    Dave Pugmire, Tue Dec  1 11:50:18 EST 2009
//    Switch from avtVec to avtVector.
//
// ****************************************************************************

void 
avtIVPEuler::Reset(const double& t_start,
                   const avtVector &y_start,
                   const avtVector &v_start)
{
    t = t_start;
    yCur = y_start;
    vCur = v_start;
    h = h_max;
}


// ****************************************************************************
//  Method: avtIVPEuler::OnExitDomain
//
//  Purpose:
//      Post processing tasks after domain exit.
//
//  Programmer: Dave Pugmire
//  Creation:   August 5, 2008
//
//  Modifications:
//    Dave Pugmire, Fri Aug  8 16:05:34 EDT 2008
//    Improved version of A-B solver that builds function history from
//    initial RK4 steps.
//
//    Dave Pugmire, Tue Mar 10 12:41:11 EDT 2009
//    Bug fix in parallel communication of solver state.
//
// ****************************************************************************

void
avtIVPEuler::OnExitDomain()
{
}

// ****************************************************************************
//  Method: avtIVPEuler::Step
//
//  Purpose:
//      Take a step and return the result.
//
//  Programmer: Dave Pugmire
//  Creation:   August 5, 2008
//
// ****************************************************************************

avtIVPSolver::Result 
avtIVPEuler::Step(avtIVPField* field, double t_max, avtIVPStep* ivpstep)
{
    double t_local = GetLocalTime();

    const double direction = sign( 1.0, t_max - t_local );
    
    h = sign( h, direction );
    
    bool last = false;

    // do not run past integration end
    if( (t_local + 1.01*h - t_max) * direction > 0.0 ) 
    {
        last = true;
        h = t_max - t_local;
    }

    // stepsize underflow??
    if( 0.1*std::abs(h) <= std::abs(t_local)*epsilon )
    {
        if (DebugStream::Level5())
        {
            debug5 << "\tavtIVPEuler::Step(): exiting at t = " 
                   << t << ", step size too small (h = " << h << ")\n";
        }
        return avtIVPSolver::STEPSIZE_UNDERFLOW;
    }

    avtIVPField::Result fieldResult;
    avtVector yNew, vNew;

    if( field->GetOrder() == 2 )
    {
        avtVector aCur;
        
        if ((fieldResult = (*field)(t_local, yCur, vCur, aCur)) != avtIVPField::OK)
            return ConvertResult(fieldResult);
        
        vNew = vCur + aCur * h;  // New velocity
        yNew = yCur + vNew * h;  // New position
    }
    else  //if( field->GetOrder() == 1 )
    {
        if ((fieldResult = (*field)(t_local, yCur, vCur)) != avtIVPField::OK)
            return ConvertResult(fieldResult);

        vNew = vCur;
        yNew = yCur + h * vCur;     // New position
    }
    
    // Convert and save the position.
    ivpstep->resize(2);
    
    if( convertToCartesian )
    {
        (*ivpstep)[0] = field->ConvertToCartesian( yCur );
        (*ivpstep)[1] = field->ConvertToCartesian( yNew );
    }
    else
    {
        (*ivpstep)[0] = yCur;
        (*ivpstep)[1] = yNew;
    }
    
    ivpstep->t0 = t;
    ivpstep->t1 = t + h;

    yCur = yNew;
    vCur = vNew;
    t = t + h;

    if( period && last )
      t += epsilon;

    // Reset the step size on sucessful step.
    h = h_max;
    
    return (last ? avtIVPSolver::TERMINATE : avtIVPSolver::OK);
}

// ****************************************************************************
//  Method: avtIVPEuler::AcceptStateVisitor
//
//  Purpose:
//      Loads the state into the state helper.
//
//  Programmer: Dave Pugmire
//  Creation:   August 5, 2008
//
//  Modifications:
//
//    Dave Pugmire, Wed Aug 20, 12:54:44 EDT 2008
//    Add a tolerance and counter for handling stiffness detection.
//
//    Dave Pugmire, Tue Mar 10 12:41:11 EDT 2009
//    Bug fix in parallel communication of solver state.
//
// ****************************************************************************
void
avtIVPEuler::AcceptStateVisitor(avtIVPStateHelper& aiss)
{
    avtIVPSolver::AcceptStateVisitor(aiss);
}
