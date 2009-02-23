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
//  Modifications:
//    Dave Pugmire, Fri Aug  8 16:05:34 EDT 2008
//    Improved version of A-B solver that builds function history from
//    initial Euler steps.
//
//    Dave Pugmire, Tue Aug 19, 17:38:03 EDT 2008
//    Changed how distanced based termination is computed.
//
//    Dave Pugmire, Wed Aug 20, 12:54:44 EDT 2008
//    Add a tolerance and counter for handling stiffness detection.
//
// ****************************************************************************

avtIVPAdamsBashforth::avtIVPAdamsBashforth()
{
    // set (somewhat) reasonable defaults
    tol = 1e-8;
    h = 1e-5;
    t = 0.0;
    d = 0.0;
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
//    initial Euler steps.
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
//    initial Euler steps.
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
//    initial Euler steps.
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
    history.resize(yCur.dim(), 0);
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
//    initial Euler steps.
//
// ****************************************************************************

void
avtIVPAdamsBashforth::OnExitDomain()
{
    history.resize(yCur.dim(), 0);
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
//  Modifications:
//    Kathleen Bonnell, Thu Aug  7, 08:29:42 PDT 2008
//    Changed for loops to use size_t to eliminate signed/unsigned int 
//    comparison warnings.
//
//    Dave Pugmire, Fri Aug  8 16:05:34 EDT 2008
//    Improved version of A-B solver that builds function history from
//    initial Euler steps.
//
// ****************************************************************************

bool
avtIVPAdamsBashforth::HasConverged(avtVec &y0, avtVec &y1, double epsilon)
{
    double maxY0 = y0.values()[0], maxY1 = y1.values()[0];
    
    for ( size_t i = 1; i < y0.dim(); i++ )
    {
        if ( y0.values()[i] > maxY0 )
            maxY0 = y0.values()[i];
        if ( y1.values()[i] > maxY1 )
            maxY1 = y1.values()[i];
    }
    
    if ( maxY0 > 1.0 && maxY1 > 1.0 )
        epsilon *= fabs(maxY1);
    
    for ( size_t i = 0; i < y0.dim(); i++ )
        if ( fabs( y0.values()[i] - y1.values()[i] ) > epsilon )
            return false;
    
    return true;
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
//  Modifications:
//    Kathleen Bonnell, Thu Aug  7, 08:29:42 PDT 2008
//    Changed for loops to use size_t to eliminate signed/unsigned int 
//    comparison warnings.
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
       for ( size_t j = 0; j < x.dim(); j++ )
           delta.values()[j] += moulton[i] * history[n].values()[j];
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
//  Modifications:
//    Kathleen Bonnell, Thu Aug  7, 08:29:42 PDT 2008
//    Changed for loops to use size_t to eliminate signed/unsigned int 
//    comparison warnings.
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
        for ( size_t j = 0; j < y.dim(); j++ )
            delta.values()[j] += bashforth[i] * history[n].values()[j];
    }
    
    avtVec yStep(y.dim());
    for ( size_t i = 0; i < y.dim(); i++)
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
//  Modifications:
//    Kathleen Bonnell, Thu Aug  7, 08:29:42 PDT 2008
//    Removed unused variable 'converged'.
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

    // Calculate the predictor using the Adams-Bashforth formula 
    history[STEPS-1] = (*field)(t, x0);

    *y_bashforth  = AdamsBashforth5Steps( ys[0], h );
    for (i = 0; i < STEPS - 1; i++) 
        history[i] = history[i+1];
    
    // Calculate the corrector using the Adams-Moulton formula 
    ys[1] = *y_bashforth;
    avtVec x0_h = x0;
    x0_h += h;

    return AdamsMoulton4Steps( field, x0_h, t, h, tolerance,
                               iterations );
}

// ****************************************************************************
//  Method: avtIVPAdamsBashforth::UpdateHistory
//
//  Purpose:
//      Take a step and return the result.
//
//  Programmer: Dave Pugmire
//  Creation:   August 8, 2008
//
// ****************************************************************************

void
avtIVPAdamsBashforth::UpdateHistory( const avtVec &yNew )
{
    if ( history.size() < 5 )
        history.resize( history.size()+1 );
 
    for ( int i = 0; i < history.size()-1; i++ )
        history[i] = history[i+1];
    
    history[history.size()-1] = yNew;
}


// ****************************************************************************
//  Method: avtIVPAdamsBashforth::EulerStep
//
//  Purpose:
//      Take a step and return the result.
//
//  Programmer: Dave Pugmire
//  Creation:   August 5, 2008
//
// ****************************************************************************

avtIVPSolver::Result 
avtIVPAdamsBashforth::EulerStep(const avtIVPField* field,
                                avtVec &yNew )
{
    avtVec v = (*field)(t,yCur);
    yNew = yCur + (v*h);

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
// ****************************************************************************

avtIVPSolver::Result 
avtIVPAdamsBashforth::ABStep(const avtIVPField* field,
                                avtVec &newY )
{
    int maxIter = 100;
    avtVec yb(yCur.dim());
    ys[0] = yCur;

    if ( Adams5Steps( field, yCur, t, h, &yb, tol, maxIter ) > maxIter )
        return avtIVPSolver::UNSPECIFIED_ERROR;

    newY = ys[1];
    ys[0] = ys[1];
    
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
// ****************************************************************************

avtIVPSolver::Result 
avtIVPAdamsBashforth::Step(const avtIVPField* field,
                           const TerminateType &type,
                           const double &end,                                              
                           avtIVPStep* ivpstep)
{
    return OK;
#if 0
    
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
    if ( history.size() < 5 )
    {
        res = EulerStep( field, yNew );
        UpdateHistory(yNew);

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
        if (!timeMode)
        {
            double len = ivpstep->length();
            
            //debug1<<"ABStep: "<<t<<" d: "<<d<<" => "<<(d+len)<<" h= "<<h<<" len= "<<len<<" sEps= "<<stiffness_eps<<endl;
            if (len < stiffness_eps)
            {
                degenerate_iterations++;
                if (degenerate_iterations > 15)
                {
                    //debug1<<"********** STIFFNESS ***************************\n";
                    return avtIVPSolver::STIFFNESS_DETECTED;
                }
            }
            else
                degenerate_iterations = 0;

            if ( d+len > d_max )
            {
                //debug1<<"********** TOO LONG ***************************\n";
                throw avtIVPField::Undefined();
            }
            d = d+len;
        }

        ivpstep->velStart = (*field)(t,yCur);
        ivpstep->velEnd = (*field)((t+h),yNew);

        t = t+h;
        yCur = yNew;
    }

    // Reset the step size on sucessful step..
    h = h_max;
    return res;
#endif
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
//    Dave Pugmire, Mon Feb 23, 09:11:34 EST 2009
//    Serialize the numStep member data.
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
        .Accept(history)
        .Accept(ys[0])
        .Accept(ys[1]);
}
