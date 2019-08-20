// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                              avtIVPLeapfrog.C                       //
// ************************************************************************* //

#include <avtIVPLeapfrog.h>
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
//  Method: avtIVPLeapfrog constructor
//
//  Purpose:
//      Creates the frog
//
//  Creationist: Allen Sanderson
//  Creation:   20 June 2011
//
// ****************************************************************************

avtIVPLeapfrog::avtIVPLeapfrog()
{
    // set (somewhat) reasonable defaults
    tol = 1e-8;
    h = 1e-5;
    t = 0.0;

    firstStep = true;
    order = 2; // Highest order ODE that the integrator can support.
}

// ****************************************************************************
//  Method: avtIVPLeapfrog destructor
//
//  Purpose:
//      Destroys the frog.
//
//  Creationist: Allen Sanderson
//  Creation:   20 June 2011
//
// ****************************************************************************


avtIVPLeapfrog::~avtIVPLeapfrog()
{
}


// ****************************************************************************
//  Method: avtIVPLeapfrog::Reset
//
//  Purpose:
//      Resets data members.  Called by the constructors.
//
//  Creationist: Allen Sanderson
//  Creation:   20 June 2011
//
// ****************************************************************************

void 
avtIVPLeapfrog::Reset(const double& t_start,
                      const avtVector &y_start,
                      const avtVector &v_start)
{
    t = t_start;
    yCur = y_start;
    vCur = v_start;
    h = h_max;

    firstStep = true;
}


// ****************************************************************************
//  Method: avtIVPLeapfrog::OnExitDomain
//
//  Purpose:
//      Post processing tasks after domain exit.
//
//  Creationist: Allen Sanderson
//  Creation:   20 June 2011
//
// ****************************************************************************

void
avtIVPLeapfrog::OnExitDomain()
{
}

// ****************************************************************************
//  Method: avtIVPLeapfrog::Step
//
//  Purpose:
//      Take a step and return the result.
//
//  Creationist: Allen Sanderson
//  Creation:   20 June 2011
//
// ****************************************************************************

avtIVPSolver::Result 
avtIVPLeapfrog::Step(avtIVPField* field, double t_max, avtIVPStep* ivpstep)
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
            debug5 << "\tavtIVPLeapfrog::Step(): exiting at t = " 
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

        if( firstStep )
            vNew = vCur + aCur * h;      // New velocity
        else
            vNew = vCur + aCur * h/2.0;  // Initial velocity at half step
        
        yNew = yCur + vNew * h;          // New position
    }
    else  //if( field->GetOrder() == 1 )
    {
        if ((fieldResult = (*field)(t_local, yCur, vCur)) != avtIVPField::OK)
            return ConvertResult(fieldResult);

        vNew = vCur;
        yNew = yCur + vCur * h;     // New position
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
//  Method: avtIVPLeapfrog::AcceptStateVisitor
//
//  Purpose:
//      Loads the state into the state helper.
//
//  Creationist: Allen Sanderson
//  Creation:   20 June 2011
//
// ****************************************************************************
void
avtIVPLeapfrog::AcceptStateVisitor(avtIVPStateHelper& aiss)
{
    avtIVPSolver::AcceptStateVisitor(aiss);

    aiss.Accept(firstStep);
}
