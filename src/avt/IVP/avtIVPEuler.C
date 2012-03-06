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
//    Remove moulton corrector code, use RK4 at startup, terminate on numSteps.
//
// ****************************************************************************

avtIVPEuler::avtIVPEuler() : vCur(0,0,0)
{
    // set (somewhat) reasonable defaults
    tol = 1e-8;
    h = 1e-5;
    t = 0.0;
    d = 0.0;
    numStep = 0;

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
//  Method: avtIVPEuler::GetCurrentY
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
//    Dave Pugmire, Tue Dec  1 11:50:18 EST 2009
//    Switch from avtVec to avtVector.
//
// ****************************************************************************

avtVector 
avtIVPEuler::GetCurrentY() const
{
    return yCur;
}

// ****************************************************************************
//  Method: avtIVPEuler::SetCurrentY
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
//    Dave Pugmire, Tue Dec  1 11:50:18 EST 2009
//    Switch from avtVec to avtVector.
//
// ****************************************************************************

void
avtIVPEuler::SetCurrentY(const avtVector &newY)
{
    yCur = newY;
}


// ****************************************************************************
//  Method: avtIVPEuler::GetCurrentV
//
//  Purpose:
//      Gets the current V.
//
//  Programmer: Dave Pugmire
//  Creation:   August 5, 2008
//
// ****************************************************************************

avtVector 
avtIVPEuler::GetCurrentV() const
{
    return vCur;
}

// ****************************************************************************
//  Method: avtIVPEuler::SetCurrentV
//
//  Purpose:
//      Sets the current V.
//
//  Programmer: Dave Pugmire
//  Creation:   August 5, 2008
//
// ****************************************************************************

void
avtIVPEuler::SetCurrentV(const avtVector &newV)
{
    vCur = newV;
}


// ****************************************************************************
//  Method: avtIVPEuler::GetCurrentT
//
//  Purpose:
//      Gets the current T.
//
//  Programmer: Dave Pugmire
//  Creation:   August 5, 2008
//
// ****************************************************************************

double 
avtIVPEuler::GetCurrentT() const 
{
    return t;
}


// ****************************************************************************
//  Method: avtIVPEuler::SetCurrentT
//
//  Purpose:
//      Sets the current T.
//
//  Programmer: Dave Pugmire
//  Creation:   August 5, 2008
//
// ****************************************************************************

void
avtIVPEuler::SetCurrentT(double newT)
{
    t = newT;
}


// ****************************************************************************
//  Method: avtIVPEuler::SetNextStepSize
//
//  Purpose:
//      Sets the step size for the next step.
//
//  Programmer: Dave Pugmire
//  Creation:   August 5, 2008
//
// ****************************************************************************

void 
avtIVPEuler::SetNextStepSize(const double& _h)
{
    h = _h;
}


// ****************************************************************************
//  Method: avtIVPEuler::GetNextStepSize
//
//  Purpose:
//      Gets the step size for the next step.
//
//  Programmer: Dave Pugmire
//  Creation:   August 5, 2008
//
// ****************************************************************************

double 
avtIVPEuler::GetNextStepSize() const
{
    return h;
}


// ****************************************************************************
//  Method: avtIVPEuler::SetMaximumStepSize
//
//  Purpose:
//      Sets the maximum step size for the next step.
//
//  Programmer: Dave Pugmire
//  Creation:   August 5, 2008
//
// ****************************************************************************

void
avtIVPEuler::SetMaximumStepSize(const double& hMax)
{
    h_max = hMax;
}


// ****************************************************************************
//  Method: avtIVPEuler::SetTolerances
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
avtIVPEuler::SetTolerances(const double& relt, const double& abst)
{
    tol = abst;
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
    d = 0.0;
    numStep = 0;

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
    const double direction = sign( 1.0, t_max - t );
    
    h = sign( h, direction );
    
    bool last = false;

    // do not run past integration end
    if( (t + 1.01*h - t_max) * direction > 0.0 ) 
    {
        last = true;
        h = t_max - t;
    }

    // stepsize underflow?
    if( 0.1*std::abs(h) <= std::abs(t)*epsilon )
        return avtIVPSolverResult::STEPSIZE_UNDERFLOW;

    avtIVPSolver::Result res = avtIVPSolverResult::OK;
    avtVector yNew, vNew;

    if( field->GetOrder() == 2 )
    {
      avtVector aCur;

      if( (res = (*field)(t, yCur, vCur, aCur)) != avtIVPSolverResult::OK )
        return( res );

      vNew = vCur + aCur * h;  // New velocity

      yNew = yCur + vNew * h;  // New position
    }
    else  //if( field->GetOrder() == 1 )
    {
      if( (res = (*field)(t, yCur, vCur)) != avtIVPSolverResult::OK )
        return( res );

      yNew = yCur + vCur * h;     // New position
    }

    if( res == avtIVPSolverResult::OK )
    {
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
        numStep++;

        yCur = yNew;
        vCur = vNew;
        t = t+h;

        if( last )
            res = avtIVPSolverResult::TERMINATE;
    }

    // Reset the step size on sucessful step.
    h = h_max;
    return res;
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
    aiss.Accept(numStep)
        .Accept(tol)
        .Accept(h)
        .Accept(h_max)
        .Accept(t)
        .Accept(d)
        .Accept(yCur)
        .Accept(vCur);
}
