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
//                              avtIVPLeapfrog.C                       //
// ************************************************************************* //

#include <avtIVPLeapfrog.h>
#include <avtIVPStateHelper.h>
#include <DebugStream.h>

#include <limits>
#include <cmath>
#include <float.h>

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

avtIVPLeapfrog::avtIVPLeapfrog() : vCur(0,0,0)
{
    // set (somewhat) reasonable defaults
    tol = 1e-8;
    h = 1e-5;
    t = 0.0;
    numStep = 0;

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
//  Method: avtIVPLeapfrog::GetCurrentV
//
//  Purpose:
//      Gets the current V.
//
//  Programmer: Dave Pugmire
//  Creation:   August 5, 2008
//
//  Modifications:

// ****************************************************************************

avtVector 
avtIVPLeapfrog::GetCurrentV() const
{
    return vCur;
}

// ****************************************************************************
//  Method: avtIVPLeapfrog::SetCurrentV
//
//  Purpose:
//      Sets the current V.
//
//  Programmer: Dave Pugmire
//  Creation:   August 5, 2008
//
//  Modifications:
//
// ****************************************************************************

void
avtIVPLeapfrog::SetCurrentV(const avtVector &newV)
{
    vCur = newV;
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
    numStep = 0;

    yCur = y_start;
    vCur = v_start;
    h = h_max;
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

    // stepsize underflow?
    if( 0.1*std::abs(h) <= std::abs(t_local)*epsilon )
        return avtIVPSolver::STEPSIZE_UNDERFLOW;
    
    avtIVPField::Result fieldResult;
    avtVector yNew, vNew;

    if( field->GetOrder() == 2 )
    {
        avtVector aCur;
        if ((fieldResult = (*field)(t_local, yCur, vCur, aCur)) != avtIVPField::OK)
            return ConvertResult(fieldResult);

        if( numStep )
            vNew = vCur + aCur * h;      // New velocity
        else
            vNew = vCur + aCur * h/2.0;  // Initial velocity at half step
        
        yNew = yCur + vNew * h;          // New position
    }
    else  //if( field->GetOrder() == 1 )
    {
        if ((fieldResult = (*field)(t_local, yCur, vCur)) != avtIVPField::OK)
            return ConvertResult(fieldResult);

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

    // Update for the next step.
    numStep++;
    
    yCur = yNew;
    vCur = vNew;
    t = t+h;
    
    if( period && last )
      t += FLT_EPSILON;

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
    aiss.Accept(numStep)
        .Accept(tol)
        .Accept(h)
        .Accept(h_max)
        .Accept(t)
        .Accept(yCur)
        .Accept(vCur);
}
