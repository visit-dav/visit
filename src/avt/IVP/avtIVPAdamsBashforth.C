/*****************************************************************************
*
* Copyright (c) 2000 - 2009, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400124
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
//                              avtIVPAdamsBashforth.C                       //
// ************************************************************************* //

#include "avtIVPAdamsBashforth.h"
#include <avtIVPDopri5.h>
#include <avtIVPStateHelper.h>
#include <DebugStream.h>

#include <limits>

static const double epsilon = std::numeric_limits<double>::epsilon();

// constants for the A-B scheme.
static const double bashforth[] = { 1901.0, -2774.0, 2616.0, -1274.0, 251.0 };
static const double divisor = 1.0 / 720.0;

#define STEPS sizeof(bashforth)/sizeof(bashforth[0])

// helper function
// returns a with the same sign as b
static inline double sign( const double& a, const double& b )
{
    return (b > 0.0) ? std::abs(a) : -std::abs(a);
}

// ****************************************************************************
//  Method: avtIVPAdamsBashforth constructor
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
//    Remove moulton corrector code, use RK4 at startup, terminate on numSteps.
//
// ****************************************************************************

avtIVPAdamsBashforth::avtIVPAdamsBashforth()
{
    // set (somewhat) reasonable defaults
    tol = 1e-8;
    h = 1e-5;
    t = 0.0;
    d = 0.0;
    numStep = 0;
    initialized = 0;
    degenerate_iterations = 0;
    stiffness_eps = tol / 1000.0;
}

// ****************************************************************************
//  Method: avtIVPAdamsBashforth destructor
//
//  Programmer: Dave Pugmire
//  Creation:   August 5, 2008
//
// ****************************************************************************


avtIVPAdamsBashforth::~avtIVPAdamsBashforth()
{
}


// ****************************************************************************
//  Method: avtIVPAdamsBashforth::GetCurrentT
//
//  Purpose:
//      Gets the current T.
//
//  Programmer: Dave Pugmire
//  Creation:   August 5, 2008
//
// ****************************************************************************

double 
avtIVPAdamsBashforth::GetCurrentT() const 
{
    return t;
}


// ****************************************************************************
//  Method: avtIVPAdamsBashforth::GetCurrentY
//
//  Purpose:
//      Gets the current Y.
//
//  Programmer: Dave Pugmire
//  Creation:   August 5, 2008
//
//  Modifications:
//    Dave Pugmire, Fri Aug  8 16:05:34 EDT 2008
//    Improved version of A-B solver that builds function history from
//    initial RK4 steps.
//
// ****************************************************************************

avtVec 
avtIVPAdamsBashforth::GetCurrentY() const
{
    return yCur;
}

// ****************************************************************************
//  Method: avtIVPAdamsBashforth::SetCurrentY
//
//  Purpose:
//      Sets the current Y.
//
//  Programmer: Dave Pugmire
//  Creation:   August 5, 2008
//
//  Modifications:
//    Dave Pugmire, Fri Aug  8 16:05:34 EDT 2008
//    Improved version of A-B solver that builds function history from
//    initial RK4 steps.
//
// ****************************************************************************

void
avtIVPAdamsBashforth::SetCurrentY(const avtVec &newY)
{
    yCur = newY;
}


// ****************************************************************************
//  Method: avtIVPAdamsBashforth::SetCurrentT
//
//  Purpose:
//      Sets the current T.
//
//  Programmer: Dave Pugmire
//  Creation:   August 5, 2008
//
// ****************************************************************************

void
avtIVPAdamsBashforth::SetCurrentT(double newT)
{
    t = newT;
}


// ****************************************************************************
//  Method: avtIVPAdamsBashforth::SetNextStepSize
//
//  Purpose:
//      Sets the step size for the next step.
//
//  Programmer: Dave Pugmire
//  Creation:   August 5, 2008
//
// ****************************************************************************

void 
avtIVPAdamsBashforth::SetNextStepSize(const double& _h)
{
    h = _h;
}


// ****************************************************************************
//  Method: avtIVPAdamsBashforth::GetNextStepSize
//
//  Purpose:
//      Gets the step size for the next step.
//
//  Programmer: Dave Pugmire
//  Creation:   August 5, 2008
//
// ****************************************************************************

double 
avtIVPAdamsBashforth::GetNextStepSize() const
{
    return h;
}


// ****************************************************************************
//  Method: avtIVPAdamsBashforth::SetMaximumStepSize
//
//  Purpose:
//      Sets the maximum step size for the next step.
//
//  Programmer: Dave Pugmire
//  Creation:   August 5, 2008
//
// ****************************************************************************

void
avtIVPAdamsBashforth::SetMaximumStepSize(const double& hMax)
{
    h_max = hMax;
}


// ****************************************************************************
//  Method: avtIVPAdamsBashforth::SetTolerances
//
//  Purpose:
//      Sets the tolerance.
//
//  Programmer: Dave Pugmire
//  Creation:   August 5, 2008
//
//  Modifications:
//    Dave Pugmire, Wed Aug 20, 12:54:44 EDT 2008
//    Add a tolerance and counter for handling stiffness detection.
//
// ****************************************************************************

void
avtIVPAdamsBashforth::SetTolerances(const double& relt, const double& abst)
{
    tol = abst;
    stiffness_eps = tol / 1000.0;
}

// ****************************************************************************
//  Method: avtIVPAdamsBashforth::Reset
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
// ****************************************************************************

void 
avtIVPAdamsBashforth::Reset(const double& t_start, const avtVecRef& y_start)
{
    t = t_start;
    d = 0.0;
    numStep = 0;

    degenerate_iterations = 0;
    yCur = y_start;
    h = h_max;
    initialized = 0;

    history[0] = avtVec(yCur.dim());
    history[1] = avtVec(yCur.dim());
    history[2] = avtVec(yCur.dim());
    history[3] = avtVec(yCur.dim());
    history[4] = avtVec(yCur.dim());
}


// ****************************************************************************
//  Method: avtIVPAdamsBashforth::OnExitDomain
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
avtIVPAdamsBashforth::OnExitDomain()
{
    initialized = 0;
}

// ****************************************************************************
//  Method: avtIVPAdamsBashforth::RK4Step
//
//  Purpose:
//      Take a step and return the result.
//
//  Programmer: Dave Pugmire
//  Creation:   Feb 24 2009
//
// ****************************************************************************

avtIVPSolver::Result 
avtIVPAdamsBashforth::RK4Step(const avtIVPField* field,
                              avtVec &yNew )
{
  avtVec f[4];

  f[0] = (*field)(t,yCur)              * h;
  f[1] = (*field)(t,yCur + f[0] * 0.5) * h;
  f[2] = (*field)(t,yCur + f[1] * 0.5) * h;
  f[3] = (*field)(t,yCur + f[2])       * h;

  yNew = yCur + (f[0] + 2.0 * f[1] + 2.0 * f[2] + f[3]) * (1.0 / 6.0);

  return avtIVPSolver::OK;
}


// ****************************************************************************
//  Method: avtIVPAdamsBashforth::ABStep
//
//  Purpose:
//      Take a step and return the result.
//
//  Programmer: Dave Pugmire
//  Creation:   August 5, 2008
//
//  Modifications:
//
//    Dave Pugmire, Tue Feb 24 14:35:38 EST 2009
//    Remove moulton corrector code, use RK4 at startup, terminate on numSteps.
//
//    Dave Pugmire, Tue Mar 10 12:41:11 EDT 2009
//    Bug fix in parallel communication of solver state.
//
// ****************************************************************************

avtIVPSolver::Result 
avtIVPAdamsBashforth::ABStep(const avtIVPField* field,
                             avtVec &yNew )
{
    // Calculate the predictor using the Adams-Bashforth formula
    yNew = yCur;

    for (size_t i = 0; i < STEPS; i++)
        yNew += h*divisor*bashforth[i] * history[i];

    return avtIVPSolver::OK;
}

// ****************************************************************************
//  Method: avtIVPAdamsBashforth::Step
//
//  Purpose:
//      Take a step and return the result.
//
//  Programmer: Dave Pugmire
//  Creation:   August 5, 2008
//
//  Modifications:
//
//    Dave Pugmire, Wed Aug 13 10:58:32 EDT 2008
//    Store the velocity with each step.
//
//    Dave Pugmire, Tue Aug 19, 17:38:03 EDT 2008
//    Chagned how distanced based termination is computed.
//
//    Dave Pugmire, Wed Aug 20, 12:54:44 EDT 2008
//    Add a tolerance and counter for handling stiffness detection.
//
//    Dave Pugmire, Mon Feb 23, 09:11:34 EST 2009
//    Reworked the termination code. Added a type enum and value. Made num steps
//    a termination criterion. Code cleanup: We no longer need fwd/bwd solvers.
//    Removed the plane intersection code.
//
//    Dave Pugmire, Tue Feb 24 14:35:38 EST 2009
//    Remove moulton corrector code, use RK4 at startup, terminate on numSteps.
//
//    Dave Pugmire, Tue Mar 10 12:41:11 EDT 2009
//    Bug fix in parallel communication of solver state.
//
// ****************************************************************************

avtIVPSolver::Result 
avtIVPAdamsBashforth::Step(const avtIVPField* field,
                           const TerminateType &termType,
                           const double &end,   
                           avtIVPStep* ivpstep)
{
    double t_max;

    if (termType == TIME)
        t_max = end;
    else if (termType == DISTANCE || termType == STEP)
    {
        t_max = std::numeric_limits<double>::max();
        if (end < 0)
            t_max = -t_max;
    }

    const double direction = sign( 1.0, t_max - t );
    
    h = sign( h, direction );
    
    // do not run past integration end
    if( (t + 1.01*h - t_max) * direction > 0.0 ) 
    {
        h = t_max - t;
    }

    // stepsize underflow?
    if( 0.1*std::abs(h) <= std::abs(t)*epsilon )
    {
        return avtIVPSolver::STEPSIZE_UNDERFLOW;
    }

    avtIVPSolver::Result res;
    avtVec yNew(yCur.dim());
    // Use a forth order Runga Kutta integration to seed the Adams-Bashforth.
    if ( initialized < STEPS )
    {
        // Save the first vector values in the history. 
        if( initialized == 0 )
        {
            history[0] = (*field)(t,yCur);
        }
        res = RK4Step( field, yNew );
        
        ++initialized;
    }
    else
    {
        res = ABStep( field, yNew );
    }

    if ( res == avtIVPSolver::OK )
    {
        ivpstep->resize( yCur.dim(), 2 );
        
        (*ivpstep)[0] = yCur;
        (*ivpstep)[1] = yNew;
        ivpstep->tStart = t;
        ivpstep->tEnd = t + h;
        numStep++;

        // Handle distanced based termination.
        if (termType == TIME)
        {
            if ((end > 0 && t >= end) ||
                (end < 0 && t <= end))
                return TERMINATE;
        }
        else if (termType == DISTANCE)
        {
            double len = ivpstep->length();
            
            //debug1<<"ABStep: "<<t<<" d: "<<d<<" => "<<(d+len)<<" h= "<<h<<" len= "<<len<<" sEps= "<<stiffness_eps<<endl;
            if (len < stiffness_eps)
            {
                degenerate_iterations++;
                if (degenerate_iterations > 15)
                {
                    //debug1<<"********** STIFFNESS ***************************\n";
                    return STIFFNESS_DETECTED;
                }
            }
            else
                degenerate_iterations = 0;

            if (d+len > fabs(end))
                throw avtIVPField::Undefined();
            else if (d+len >= fabs(end))
                return TERMINATE;

            d = d+len;
        }
        else if (termType == STEP &&
                 numStep >= (int)fabs(end))
            return TERMINATE;

        ivpstep->velStart = (*field)(t,yCur);
        ivpstep->velEnd = (*field)((t+h),yNew);

        // Update the history to save the last 5 vector values.
        history[4] = history[3];
        history[3] = history[2];
        history[2] = history[1];
        history[1] = history[0];
        history[0] = (*field)(t,yNew); 

        yCur = yNew;
        t = t+h;
    }

    // Reset the step size on sucessful step.
    h = h_max;
    return res;
}

// ****************************************************************************
//  Method: avtIVPAdamsBashforth::AcceptStateVisitor
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
avtIVPAdamsBashforth::AcceptStateVisitor(avtIVPStateHelper& aiss)
{
    aiss.Accept(numStep)
        .Accept(tol)
        .Accept(degenerate_iterations)
        .Accept(stiffness_eps)
        .Accept(h)
        .Accept(h_max)
        .Accept(t)
        .Accept(d)
        .Accept(yCur)
        .Accept(history[0])
        .Accept(history[1])
        .Accept(history[2])
        .Accept(history[3])
        .Accept(history[4])
        .Accept(initialized)
        .Accept(ys[0])
        .Accept(ys[1]);
}
