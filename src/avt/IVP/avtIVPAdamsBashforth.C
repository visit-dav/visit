/*****************************************************************************
*
* Copyright (c) 2000 - 2008, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400142
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
static const double moulton[] = { 251.0, 646.0, -264.0, 106.0, -19.0 };
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
// ****************************************************************************

avtIVPAdamsBashforth::avtIVPAdamsBashforth()
{
    // set (somewhat) reasonable defaults
    tol = 1e-8;
    h = 1e-5;
    t = 0.0;
    initialized = 0;
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
// ****************************************************************************

avtVec 
avtIVPAdamsBashforth::GetCurrentY() const
{
    return ys[0];
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
// ****************************************************************************

void
avtIVPAdamsBashforth::SetCurrentY(const avtVec &newY)
{
    ys[0] = newY;
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
// ****************************************************************************

void
avtIVPAdamsBashforth::SetTolerances(const double& relt, const double& abst)
{
    tol = relt;
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
// ****************************************************************************

void 
avtIVPAdamsBashforth::Reset(const double& t_start, const avtVecRef& y_start)
{
    t = t_start;
    x = y_start;
    h = h_max;
    initialized = 0;
    
    // Set the self start values.
    ys[0] = y_start;
    
    fns[0] = y_start;
    fns[1] = y_start;
    fns[2] = y_start;
    fns[3] = y_start;
}

// ****************************************************************************
//  Method: avtIVPAdamsBashforth::Initialize
//
//  Purpose:
//      Initialize the self start data.
//
//  Programmer: Dave Pugmire
//  Creation:   August 5, 2008
//
// ****************************************************************************

void
avtIVPAdamsBashforth::Initialize(const avtIVPField *field)
{
    /*
    avtVec newY, x1 = x;

    t += h;
    x1 += h;
    newY = (*field)( t, x1 );
    fns[3] = newY;

    t += h;
    x1 += h;
    newY = (*field)( t, x1 );
    fns[2] = newY;
    
    t += h;
    x1 += h;
    newY = (*field)( t, x1 );
    fns[1] = newY;
    t += h;
    
    newY = (*field)( t, newY );
    fns[0] = newY;

    t += h;
    x1 += h;
    newY = (*field)( t, x1 );
    ys[0] = newY;
    */

    //set *something*
    ys[0] = x;
    fns[0] = x;
    fns[1] = x;
    fns[2] = x;
    fns[3] = x;
    
    h = h_max;
    initialized = 1;
}


// ****************************************************************************
//  Method: avtIVPAdamsBashforth::HasConverged
//
//  Purpose:
//      Check for convergence.
//
//  Programmer: Dave Pugmire
//  Creation:   August 5, 2008
//
// ****************************************************************************

bool
avtIVPAdamsBashforth::HasConverged(avtVec &y0, avtVec &y1, double epsilon)
{
    double maxY0 = y0.values()[0], maxY1 = y1.values()[0];
    
    for ( int i = 1; i < y0.dim(); i++ )
    {
        if ( y0.values()[i] > maxY0 )
            maxY0 = y0.values()[i];
        if ( y1.values()[i] > maxY1 )
            maxY1 = y1.values()[i];
    }
    
    if ( maxY0 > 1.0 && maxY1 > 1.0 )
        epsilon *= fabs(maxY1);
    
    for ( int i = 0; i < y0.dim(); i++ )
        if ( fabs( y0.values()[i] - y1.values()[i] ) > epsilon )
            return false;
    
    return true;

    /*
    if ( fabs(y0) > 1.0 && fabs(y1) > 1.0 )
        epsilon *= fabs(y1);
    if ( fabs(y0 - y1) < epsilon )
        return 1;
    return 0;
    */
}

// ****************************************************************************
//  Method: avtIVPAdamsBashforth::AdamsMoulton4Steps
//
//  Purpose:
//      Adams-Moulton method.
//
//  Programmer: Dave Pugmire
//  Creation:   August 5, 2008
//
// ****************************************************************************

int
avtIVPAdamsBashforth::AdamsMoulton4Steps(const avtIVPField* field,
                                         avtVec x,
                                         double t,
                                         double h,
                                         double tolerance,
                                         int iterations )
{
    avtVec old_estimate(x.dim());
    avtVec delta(x.dim());
    int i;
    int n = STEPS - 2;
    int converged = -1;

    // Calculate the corrector using the Adams-Moulton formula 
    
   for (i = 1; i < STEPS; i++, n--)
   {
       for ( int j = 0; j < x.dim(); j++ )
           delta.values()[j] += moulton[i] * fns[n].values()[j];
   }
   
   for (i = 0; i < iterations; i++)
   {
       old_estimate = ys[1];
       ys[1] = ys[0] + h * divisor * ( moulton[0] * (*field)(t, x) + delta );
       if ( HasConverged(old_estimate, ys[1], tolerance) )
           break;
   }
   return i+1;
}


// ****************************************************************************
//  Method: avtIVPAdamsBashforth::AdamsBashforth5Steps
//
//  Purpose:
//      Adams-Bashforth
//
//  Programmer: Dave Pugmire
//  Creation:   August 5, 2008
//
// ****************************************************************************

avtVec
avtIVPAdamsBashforth::AdamsBashforth5Steps( avtVec &y,
                                            double h )
{
    avtVec delta(y.dim());
    int i;
    int n = STEPS - 1;

    // Calculate the predictor using the Adams-Bashforth formula 
    for (i = 0; i < STEPS; i++, n--)
    {
        for ( int j = 0; j < y.dim(); j++ )
            delta.values()[j] += bashforth[i] * fns[n].values()[j];
    }
    
    avtVec yStep(y.dim());
    for ( int i = 0; i < y.dim(); i++)
        yStep.values()[i] = y.values()[i] + h*divisor*delta.values()[i];
    return yStep;
}


// ****************************************************************************
//  Method: avtIVPAdamsBashforth::Adams5Steps
//
//  Purpose:
//      Adams-Bashforth
//
//  Programmer: Dave Pugmire
//  Creation:   August 5, 2008
//
// ****************************************************************************

int
avtIVPAdamsBashforth::Adams5Steps(const avtIVPField* field,
                                  avtVec x0,
                                  double t,
                                  double h,
                                  avtVec *y_bashforth,
                                  double tolerance,
                                  int iterations )
{
    int i;
    int converged;

    // Calculate the predictor using the Adams-Bashforth formula 
    fns[STEPS-1] = (*field)(t, x0);

    *y_bashforth  = AdamsBashforth5Steps( ys[0], h );
    for (i = 0; i < STEPS - 1; i++) 
        fns[i] = fns[i+1];
    
    // Calculate the corrector using the Adams-Moulton formula 
    ys[1] = *y_bashforth;
    avtVec x0_h = x0;
    x0_h += h;

    return AdamsMoulton4Steps( field, x0_h, t, h, tolerance,
                               iterations );
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
// ****************************************************************************

avtIVPSolver::Result 
avtIVPAdamsBashforth::Step(const avtIVPField* field,
                           const double& t_max,
                           avtIVPStep* ivpstep)
{
    const double direction = sign( 1.0, t_max - t );
    
    h = sign( h, direction );
    
    // do not run past integration end
    if( (t + 1.01*h - t_max) * direction > 0.0 ) 
    {
        h = t_max - t;
    }

    if ( initialized == 0 )
        Initialize( field );

    // stepsize underflow?
    if( 0.1*std::abs(h) <= std::abs(t)*epsilon )
    {
        return STEPSIZE_UNDERFLOW;
    }
    
    avtVec yb;
    int maxIter = 100;
    int r = Adams5Steps( field, x, t, h, &yb, tol, maxIter );
    
    if ( r >= maxIter )
        return UNSPECIFIED_ERROR;
    
    ivpstep->resize( ys[0].dim(), 2 );

    (*ivpstep)[0] = ys[0];
    (*ivpstep)[1] = ys[1]; 
    ivpstep->tStart = t;
    ivpstep->tEnd   = t + h;
    x = ys[1];

    //Inc for next step.
    t += h;
    ys[0] = ys[1];

    // Succesful step, we don't allow for adaptive step, so

    // set it to the initial value.
    h = h_max;

    return avtIVPSolver::OK;
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
// ****************************************************************************
void
avtIVPAdamsBashforth::AcceptStateVisitor(avtIVPStateHelper& aiss)
{
    aiss.Accept(tol)
        .Accept(h)
        .Accept(h_max)
        .Accept(t)
        .Accept(initialized)
        .Accept(ys[0])
        .Accept(ys[1])
        .Accept(fns[0])
        .Accept(fns[1])
        .Accept(fns[2])
        .Accept(fns[3]);
}
