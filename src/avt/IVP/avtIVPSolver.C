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
//                              avtIVPSolver.C                               //
// ************************************************************************* //

#include <avtIVPSolver.h>
#include <avtIVPStateHelper.h>


// ****************************************************************************
//  Method: avtIVPSolver::GetState
//
//  Purpose:
//      Gets the state of the IVP solver.
//
//  Programmer: Christoph Garth
//  Creation:   February 25, 2008
//
// ****************************************************************************

avtIVPSolver::avtIVPSolver() : convertToCartesian(false), convertToCylindrical(false),
                               order(1), yCur(avtVector()), h(1e-5), h_max(1e-5),
                               tol(1e-8), t(0.0), period(0), baseTime(0), maxTime(1),
                               direction(DIRECTION_BACKWARD)
{
}


// ****************************************************************************
//  Method: avtIVPSolver::GetCurrentY
//
//  Purpose:
//      Gets the current Y.
//
//  Programmer: Dave Pugmire
//  Creation:   August 5, 2008
//
// ****************************************************************************

avtVector 
avtIVPSolver::GetCurrentY() const
{
    return yCur;
}

// ****************************************************************************
//  Method: avtIVPSolver::SetCurrentY
//
//  Purpose:
//      Sets the current Y.
//
//  Programmer: Dave Pugmire
//  Creation:   August 5, 2008
//
// ****************************************************************************

void
avtIVPSolver::SetCurrentY(const avtVector &newY)
{
    yCur = newY;
}


// ****************************************************************************
//  Method: avtIVPSolver::GetCurrentT
//
//  Purpose:
//      Gets the current T.
//
//  Programmer: Dave Pugmire
//  Creation:   August 5, 2008
//
// ****************************************************************************

double 
avtIVPSolver::GetCurrentT() const 
{
    double t_local = t;

    if( period > 0 )
    {
      if( direction == DIRECTION_BACKWARD)
        while (t_local <= baseTime) t_local += period;
      else // if( direction == DIRECTION_FORWARD)
        while (t_local >= maxTime) t_local -= period;
    }

    return t_local;
}


// ****************************************************************************
//  Method: avtIVPSolver::SetCurrentT
//
//  Purpose:
//      Sets the current T.
//
//  Programmer: Dave Pugmire
//  Creation:   August 5, 2008
//
// ****************************************************************************

void
avtIVPSolver::SetCurrentT(double newT)
{
    t = newT;
}


// ****************************************************************************
//  Method: avtIVPSolver::SetNextStepSize
//
//  Purpose:
//      Sets the step size for the next step.
//
//  Programmer: Allen Sanderson
//  Creation:   October 24, 2009
//
// ****************************************************************************

void 
avtIVPSolver::SetNextStepSize(const double& newH)
{
    h = newH;
}


// ****************************************************************************
//  Method: avtIVPSolver::GetNextStepSize
//
//  Purpose:
//      Gets the step size for the next step.
//
//  Programmer: Allen Sanderson
//  Creation:   October 24, 2009
//
// ****************************************************************************

double 
avtIVPSolver::GetNextStepSize() const
{
    return h;
}


// ****************************************************************************
//  Method: avtIVPSolver::SetMaximumStepSize
//
//  Purpose:
//      Sets the maximum step size for the next step.
//
//  Programmer: Allen Sanderson
//  Creation:   October 24, 2009
//
// ****************************************************************************

void
avtIVPSolver::SetMaximumStepSize(const double& maxH)
{
    h_max = maxH;
}


// ****************************************************************************
//  Method: avtIVPSolver::GetMaximumStepSize
//
//  Purpose:
//      Gets the maximum step size for the next step.
//
//  Programmer: Allen Sanderson
//  Creation:   October 24, 2009
//
// ****************************************************************************

double
avtIVPSolver::GetMaximumStepSize() const
{
    return h_max;
}


// ****************************************************************************
//  Method: avtIVPSolver::SetTolerances
//
//  Purpose:
//      Sets the tolerance.
//
//  Programmer: Dave Pugmire
//  Creation:   August 5, 2008
//
// ****************************************************************************

void
avtIVPSolver::SetTolerances(const double& relt, const double& abst)
{
    tol = abst;
}


// ****************************************************************************
//  Method: avtIVPSolver::GetOrder
//
//  Purpose:
//      Gets the order
//
//  Programmer: Allen Sanderson
//  Creation:   April 16, 2014
//
// ****************************************************************************

unsigned int
avtIVPSolver::GetOrder() const
{
  return order;
}


// ****************************************************************************
//  Method: avtIVPSolver::SetOrder
//
//  Purpose:
//      Sets the order.
//
//  Programmer: Allen Sanderson
//  Creation:   April 16, 2014
//
// ****************************************************************************

void
avtIVPSolver::SetOrder(unsigned int o)
{
    order = o;
}


// ****************************************************************************
//  Method: avtIVPSolver::GetToCartesian
//
//  Purpose:
//      Gets the ToCartesian
//
//  Programmer: Allen Sanderson
//  Creation:   April 16, 2014
//
// ****************************************************************************

bool
avtIVPSolver::GetToCartesian() const
{
  return convertToCartesian;;
}


// ****************************************************************************
//  Method: avtIVPSolver::SetToCartesian
//
//  Purpose:
//      Sets the ToCartesian.
//
//  Programmer: Allen Sanderson
//  Creation:   April 16, 2014
//
// ****************************************************************************

void
avtIVPSolver::SetToCartesian(bool val)
{
    convertToCartesian = val;
}


// ****************************************************************************
//  Method: avtIVPSolver::GetToCylindrical
//
//  Purpose:
//      Gets the ToCylindrical
//
//  Programmer: Allen Sanderson
//  Creation:   April 16, 2014
//
// ****************************************************************************

bool
avtIVPSolver::GetToCylindrical() const
{
  return convertToCylindrical;;
}


// ****************************************************************************
//  Method: avtIVPSolver::SetToCylindrical
//
//  Purpose:
//      Sets the ToCylindrical.
//
//  Programmer: Allen Sanderson
//  Creation:   April 16, 2014
//
// ****************************************************************************

void
avtIVPSolver::SetToCylindrical(bool val)
{
    convertToCylindrical = val;
}


// ****************************************************************************
//  Method: avtIVPSolver::GetPeriod
//
//  Purpose:
//      Gets the period.
//
//  Programmer: Allen Sanderson
//  Creation:   April 16, 2014
//
// ****************************************************************************

double 
avtIVPSolver::GetPeriod() const
{
    return period;
}

// ****************************************************************************
//  Method: avtIVPSolver::SetPeriod
//
//  Purpose:
//      Sets the period.
//
//  Programmer: Allen Sanderson
//  Creation:   April 16, 2014
//
// ****************************************************************************

void
avtIVPSolver::SetPeriod(const double &p)
{
    period = p;
    maxTime = baseTime + period;
}


// ****************************************************************************
//  Method: avtIVPSolver::GetBaseTime
//
//  Purpose:
//      Gets the baseTime.
//
//  Programmer: Allen Sanderson
//  Creation:   April 16, 2014
//
// ****************************************************************************

double 
avtIVPSolver::GetBaseTime() const
{
    return baseTime;
}

// ****************************************************************************
//  Method: avtIVPSolver::SetBaseTime
//
//  Purpose:
//      Sets the baseTime.
//
//  Programmer: Allen Sanderson
//  Creation:   April 16, 2014
//
// ****************************************************************************

void
avtIVPSolver::SetBaseTime(const double &time)
{
    baseTime = time;
    maxTime = baseTime + period;
}


// ****************************************************************************
//  Method: avtIVPSolver::GetDirection
//
//  Purpose:
//      Gets the direction.
//
//  Programmer: Allen Sanderson
//  Creation:   April 16, 2014
//
// ****************************************************************************

avtIVPSolver::Direction
avtIVPSolver::GetDirection() const
{
    return direction;
}

// ****************************************************************************
//  Method: avtIVPSolver::SetDirection
//
//  Purpose:
//      Sets the direction.
//
//  Programmer: Allen Sanderson
//  Creation:   April 16, 2014
//
// ****************************************************************************

void
avtIVPSolver::SetDirection(const Direction &d)
{
    direction = d;
}


// ****************************************************************************
//  Method: avtIVPSolver::GetState
//
//  Purpose:
//      Gets the state of the IVP solver.
//
//  Programmer: Christoph Garth
//  Creation:   February 25, 2008
//
// ****************************************************************************

void
avtIVPSolver::GetState( avtIVPState& state )
{
    avtIVPStateHelper aiss(avtIVPStateHelper::GET, 0);
    this->AcceptStateVisitor(aiss);

    state.allocate(aiss.size());
    
    aiss.Reset(avtIVPStateHelper::GET, state._data);
    this->AcceptStateVisitor(aiss);
}


// ****************************************************************************
//  Method: avtIVPSolver::PutState
//
//  Purpose:
//      Sets the state of the IVP solver.
//
//  Programmer: Christoph Garth
//  Creation:   February 25, 2008
//
// ****************************************************************************

void 
avtIVPSolver::PutState(const avtIVPState& state)
{
    avtIVPStateHelper aiss(avtIVPStateHelper::PUT, state._data);
    this->AcceptStateVisitor(aiss);
}


avtIVPSolver::Result
avtIVPSolver::ConvertResult(const avtIVPField::Result &res) const
{
    if (res == avtIVPField::OK)
        return OK;
    else if (res == avtIVPField::OUTSIDE_SPATIAL)
        return OUTSIDE_SPATIAL;
    else if (res == avtIVPField::OUTSIDE_BOTH)
        return OUTSIDE_SPATIAL;
    else if (res == avtIVPField::OUTSIDE_TEMPORAL)
        return OUTSIDE_TEMPORAL;
    else
        return UNSPECIFIED_ERROR;
}
