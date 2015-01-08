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
//                                   avtIVPRK4.C                             //
// ************************************************************************* //

#include <avtIVPRK4.h>
#include <avtIVPStateHelper.h>
#include <DebugStream.h>

#include <limits>
#include <cmath>
#include <float.h>

static const double epsilon = std::numeric_limits<double>::epsilon();

// helper function
// returns a with the same sign as b
static inline double sign( const double& a, const double& b )
{
    return (b > 0.0) ? std::abs(a) : -std::abs(a);
}

// ****************************************************************************
//  Method: avtIVPRK4 constructor
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

avtIVPRK4::avtIVPRK4()
{
    // set (somewhat) reasonable defaults
    tol = 1e-8;
    h = 1e-5;
    t = 0.0;
    numStep = 0;

    order = 2; // Highest order ODE that the integrator can support.
}

// ****************************************************************************
//  Method: avtIVPRK4 destructor
//
//  Programmer: Dave Pugmire
//  Creation:   August 5, 2008
//
// ****************************************************************************


avtIVPRK4::~avtIVPRK4()
{
}


// ****************************************************************************
//  Method: avtIVPRK4::Reset
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
avtIVPRK4::Reset(const double& t_start,
                 const avtVector &y_start,
                 const avtVector &v_start)
{
    t = t_start;
    numStep = 0;

    yCur = y_start;
    h = h_max;
}


// ****************************************************************************
//  Method: avtIVPRK4::OnExitDomain
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
avtIVPRK4::OnExitDomain()
{
}

// ****************************************************************************
//  Method: avtIVPRK4::Step
//
//  Purpose:
//      Take a step and return the result.
//
//  Programmer: Dave Pugmire
//  Creation:   August 5, 2008
//
// ****************************************************************************

avtIVPSolver::Result 
avtIVPRK4::Step(avtIVPField* field, double t_max, avtIVPStep* ivpstep)
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

    // Compute the RK4 values.
    avtVector k1;
    if ((fieldResult = (*field)(t_local, yCur, k1)) != avtIVPField::OK )
        return ConvertResult(fieldResult);

    avtVector k2;
    if ((fieldResult = (*field)(t_local+0.5*h, yCur+0.5*h*k1, k2)) != avtIVPField::OK)
        return ConvertResult(fieldResult);
    
    avtVector k3;
    if ((fieldResult = (*field)(t_local+0.5*h, yCur+0.5*h*k2, k3)) != avtIVPField::OK)
        return ConvertResult(fieldResult);

    avtVector k4;
    if ((fieldResult = (*field)(t_local+h, yCur+h*k3, k4)) != avtIVPField::OK)
        return ConvertResult(fieldResult);

    // Calculate the new position.
    avtVector yNew = yCur + h*(k1 + 2.0*k2 + 2.0*k3 + k4)/6.0;

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
      t += FLT_EPSILON;

    // Reset the step size on sucessful step.
    h = h_max;
    
    return (last ? avtIVPSolver::TERMINATE : avtIVPSolver::OK);
}

// ****************************************************************************
//  Method: avtIVPRK4::AcceptStateVisitor
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
avtIVPRK4::AcceptStateVisitor(avtIVPStateHelper& aiss)
{
    aiss.Accept(numStep)
        .Accept(tol)
        .Accept(h)
        .Accept(h_max)
        .Accept(t)
        .Accept(yCur);
}
