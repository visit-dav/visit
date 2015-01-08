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
//                              avtIVPAdamsBashforth.C                       //
// ************************************************************************* //

#include <avtIVPAdamsBashforth.h>
#include <avtIVPStateHelper.h>
#include <DebugStream.h>

#include <limits>
#include <cmath>
#include <float.h>

static const double epsilon = std::numeric_limits<double>::epsilon();

// coefficients for the Adams-Bashforth scheme for up to five steps.
static const double bashforth[5][5] = //[ADAMS_BASHFORTH_NSTEPS][ADAMS_BASHFORTH_NSTEPS] =
  {{    1.,          0.,        0.,        0.,        0. },
   {    3./2.,      -1./2.,     0.,        0.,        0. },
   {   23./12.,     -4./3.,     5./12.,    0.,        0. },
   {   55./24.,    -59./24.,   37./24.,   -3./8.,     0. },
   { 1901./720., -1387./360., 109./30., -637./360., 251./720. }};

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
    numStep = 0;
    degenerate_iterations = 0;
    stiffness_eps = tol / 1000.0;
    abStep = 0;
    abNSteps = 1;
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
//    Dave Pugmire, Tue Dec  1 11:50:18 EST 2009
//    Switch from avtVec to avtVector.
//
// ****************************************************************************

void 
avtIVPAdamsBashforth::Reset(const double& t_start,
                            const avtVector &y_start,
                            const avtVector &v_start)
{
    t = t_start;
    numStep = 0;

    degenerate_iterations = 0;
    yCur = y_start;
    h = h_max;

    abStep = 0;
    abNSteps = 1;
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
//   Dave Pugmire, Tue Aug 11 10:25:45 EDT 2009
//   Add new termination criterion: Number of intersections with an object.
//
//   Dave Pugmire, Tue Feb 23 09:42:25 EST 2010
//   Set the velStart/velEnd direction based on integration direction.
//
//   Dave Pugmire, Wed May 26 13:48:24 EDT 2010
//   The velStart/velEnd direction was reversed.
//
// ****************************************************************************

avtIVPSolver::Result 
avtIVPAdamsBashforth::Step(avtIVPField* field, double t_max,
                           avtIVPStep* ivpstep)
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

    // stepsize underflow?
    if( 0.1*std::abs(h) <= std::abs(t_local)*epsilon )
        return avtIVPSolver::STEPSIZE_UNDERFLOW;

    avtIVPField::Result fieldResult;
    avtVector yNew, vCur, vNew(0,0,0);

    // Get the first vector value for the history. 
    if ((fieldResult = (*field)(t_local, yCur, vCur)) != avtIVPField::OK)
      return ConvertResult(fieldResult);

    // NOTE: DO NOT pass history[0] to the above as it gets a bogus
    // value when calculating pathlines.
    history[0] = vCur;

    // Calculate the new velocity using the Adams-Bashforth algorithm
    for (int i = 0; i < abNSteps; ++i)
        vNew += bashforth[abStep][i] * history[i];

    // Calculate the new position.
    yNew = yCur + h * vNew;

    // Increment the number of steps to be taken.
    if( abNSteps < ADAMS_BASHFORTH_NSTEPS )
    {
      ++abStep;    // Index of the coefficents for the step order.
      ++abNSteps;  // Number of steps to be taken
    }

    // Update the history to save the last N vector values. Note: the
    // history needs to be updated after the abStep is incremented.
    for (size_t i = abStep; i>0; --i)
      history[i] = history[i-1];

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

    // Update for the next step.
    numStep++;

    yCur = yNew;
    t = t+h;

    if( period && last )
      t += epsilon*10000.0;

    // Reset the step size on sucessful step.
    h = h_max;

    return (last ? avtIVPSolver::TERMINATE : avtIVPSolver::OK);
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
        .Accept(yCur)
        .Accept(history[0])
        .Accept(history[1])
        .Accept(history[2])
        .Accept(history[3])
        .Accept(history[4]);
}
