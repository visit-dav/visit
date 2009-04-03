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
//                              avtStreamline.C                              //
// ************************************************************************* //

#include <avtStreamline.h>

#include <list>
#include <iostream>
#include <limits>
#include <avtVecArray.h>
#include <DebugStream.h>

// ****************************************************************************
//  Method: avtStreamline constructor
//
//  Programmer: Christoph Garth
//  Creation:   February 25, 2008
//
//  Modifications:
//  
//    Dave Pugmire, Mon Feb 23, 09:11:34 EST 2009
//    Reworked the termination code. Added a type enum and value. Made num steps
//    a termination criterion. Code cleanup: We no longer need fwd/bwd solvers.
//  
// ****************************************************************************

avtStreamline::avtStreamline(const avtIVPSolver* model, const double& t_start,
                             const avtVec& p_start, int ID)
{
    _t0 = t_start;
    _p0 = p_start;
    id = ID;
    
    _ivpSolver = model->Clone();
    _ivpSolver->Reset(_t0, _p0);
    wantVorticity = false;
}


// ****************************************************************************
//  Method: avtStreamline constructor
//
//  Programmer: Christoph Garth
//  Creation:   February 25, 2008
//
// ****************************************************************************

avtStreamline::avtStreamline()
{
    _ivpSolver = NULL;
    wantVorticity = false;
}


// ****************************************************************************
//  Method: avtStreamline destructor
//
//  Programmer: Christoph Garth
//  Creation:   February 25, 2008
//
//  Modifications:
//    Dave Pugmire, Mon Feb 23, 09:11:34 EST 2009
//    Reworked the termination code. Added a type enum and value. Made num steps
//    a termination criterion. Code cleanup: We no longer need fwd/bwd solvers.
//    
// ****************************************************************************

avtStreamline::~avtStreamline()
{
    if ( _ivpSolver )
        delete _ivpSolver;
    
    _ivpSolver = NULL;
    for(iterator si = begin(); si != end(); si++)
         delete *si;
}    


// ****************************************************************************
//  Method: avtStreamline::Advance
//
//  Purpose:
//      Advances the streamline.
//
//  Programmer: Christoph Garth
//  Creation:   February 25, 2008
//
//  Modifications:
//
//    Dave Pugmire, Wed Aug 13 10:58:32 EDT 2008
//    Modify how data without ghost zones are handled.
//
//    Dave Pugmire, Tue Aug 19, 17:38:03 EDT 2008
//    Changed how distanced based termination is computed.
//
//    Dave Pugmire, Wed Dec  3 08:33:42 EST 2008
//    Added maxSteps argument to optionally control how many integration steps
//    are taken.
//
//    Dave Pugmire, Mon Feb 23, 09:11:34 EST 2009
//    Reworked the termination code. Added a type enum and value. Made num steps
//    a termination criterion. Code cleanup: We no longer need fwd/bwd solvers.
//
// ****************************************************************************

avtIVPSolver::Result 
avtStreamline::Advance(const avtIVPField* field,
                       avtIVPSolver::TerminateType termType,
                       double end,
                       bool vorticity,
                       bool haveGhostZones,
                       double *extents)
{
    wantVorticity = vorticity;
    avtIVPSolver::Result res = DoAdvance(_ivpSolver, field, termType, end,
                                         haveGhostZones, extents);
    return res;
}


// ****************************************************************************
//  Method: avtStreamline::DoAdvance
//
//  Purpose:
//      Does the real work of advancing the streamline.
//
//  Programmer: Christoph Garth
//  Creation:   February 25, 2008
//
//  Modifications:
//    Kathleen Bonnell, Thu Aug  7, 08:29:42 PDT 2008
//    Removed unused variable in catch.
//
//    Dave Pugmire, Wed Aug 13 10:58:32 EDT 2008
//    Modify how data without ghost zones are handled.
//
//    Dave Pugmire, Tue Aug 19, 17:38:03 EDT 2008
//    Changed how distanced based termination is computed.
//
//    Dave Pugmire, Wed Aug 20, 07:43:58 EDT 2008
//    Bug fix. Check to see if _steps is empty before using front/back.
//
//    Dave Pugmire, Thu Aug 21, 15:46:31 EDT 2008
//    Fixed a memory leak. If the solver step fails, delete the avtIVPStep
//    before continuing.
//
//    Dave Pugmire, Wed Dec  3 08:33:42 EST 2008
//    Added maxSteps argument to optionally control how many integration steps
//    are taken.
//
//    Dave Pugmire, Tue Feb  3 10:54:34 EST 2009
//    More debug statements.
//
//    Dave Pugmire, Mon Feb 23, 09:11:34 EST 2009
//    Reworked the termination code. Added a type enum and value. Made num steps
//    a termination criterion. Code cleanup: We no longer need fwd/bwd solvers.
//    
//    Hank Childs, Thu Apr  2 16:40:08 PDT 2009
//    Fix problem with stalling out during initialization in case where
//    seed point is close to boundary of domain.  Done in consultation with
//    Christoph.
//
// ****************************************************************************

avtIVPSolver::Result
avtStreamline::DoAdvance(avtIVPSolver* ivp,
                         const avtIVPField* field,
                         avtIVPSolver::TerminateType termType,
                         double end,
                         bool haveGhostZones,
                         double *extents)
{
    avtIVPSolver::Result result;
    
    // catch cases where the start position is outside the 
    // domain of field
    if (!field->IsInside(ivp->GetCurrentT(), ivp->GetCurrentY()))
        return avtIVPSolver::OUTSIDE_DOMAIN;
    
    while (1)
    {
        // record state for later restore, if needed
        avtIVPState state;
        ivp->GetState( state );

        // create new step to be filled in by ivp
        avtIVPStep* step = new avtIVPStep;

        try
        {
            debug5<<"Step( mode= "<<termType<<" end= "<<end<<endl;
            result = ivp->Step(field, termType, end, step);
            debug5<<"   T= "<<ivp->GetCurrentT()<<" "<<ivp->GetCurrentY()<<endl;
        }
        catch( avtIVPField::Undefined& )
        {
            debug5<<ivp->GetCurrentY()<<" not in domain\n";
            // integrator left the domain, retry with smaller step
            // if step size is below given minimum, give up

            // restore old state to before failed step
            double hBeforePush = ivp->GetNextStepSize();
            ivp->PutState( state );
            if (ivp->GetNextStepSize() == 0.)
            {
                // This can happen if we try to look a few points out
                // for the very first step and one of those points
                // is outside the domain.  Just set the step size
                // back to what it was before so we can try again
                // with a smaller step.
                ivp->SetNextStepSize(hBeforePush);
            }

            double h = ivp->GetNextStepSize();

            h = h/2;
            if( fabs(h) < 1e-9 )
            {
                delete step;
                HandleGhostZones((end > 0.0), haveGhostZones, extents);
                debug5<<"avtStreamline::DoAdvance() DONE  result= OUTSIDE_DOMAIN\n";
                return avtIVPSolver::OUTSIDE_DOMAIN;            
            }
            
            ivp->SetNextStepSize( h );

            // retry step
            delete step;
            continue;
        }
        
        catch( std::exception& )
        {
        }
        // record step if it was successful
        if (result == avtIVPSolver::OK ||
            result == avtIVPSolver::TERMINATE)
        {
            if ( wantVorticity )
                step->ComputeVorticity( field );

            if (end < 0) //backwards
                _steps.push_front( step );
            else 
                _steps.push_back( step );

            if (result == avtIVPSolver::TERMINATE)
                break;
        }
        else
        {
            delete step;
            break;
        }
    }
    return result;
}


// ****************************************************************************
//  Method: avtStreamline::HandleGhostZones
//
//  Purpose:
//      Handles the logic for when we jump out into a ghost zone.
//
//  Programmer: Christoph Garth
//  Creation:   February 25, 2008
//
//  Modifications:
//
//    Dave Pugmire, Wed Aug 13 10:58:32 EDT 2008
//    Modify how data without ghost zones are handled. Pass in a dataset extents
//    array. Use that to do adaptive jumping out on the velocity vector.
//
//    Dave Pugmire, Tue Feb  3 10:54:34 EST 2009
//    More debug statements.
//
//    Dave Pugmire, Mon Feb 23, 09:11:34 EST 2009
//    Code cleanup. 
//
//    Dave Pugmire,  Tue Mar 31 17:08:29 EDT 2009
//    Set the step's T value when leaping out.  
//
// ****************************************************************************

void
avtStreamline::HandleGhostZones(bool forward,
                                bool haveGhostZones,
                                double *extents)
{
    if ( haveGhostZones || extents==NULL || size() == 0 )
        return;

    // Determine the minimum non-zero data extent.
    double range[3], minRange = -1.0;
    range[0] = (extents[1]-extents[0]);
    range[1] = (extents[3]-extents[2]);
    range[2] = (extents[5]-extents[4]);
    for ( int i = 0; i < 3; i++ )
    {
        if (range[i] > 0.0 )
        {
            if ( minRange < 0 || range[i] < minRange )
                minRange = range[i];
        }
    }
    
    if ( minRange < 0.0 )
        return;

    avtVec dir, pt;
    if (forward)
    {
        iterator si = _steps.end();
        si--;
        dir = (*si)->velEnd;
        pt = (*si)->front();
    }
    else
    {
        iterator si = _steps.begin();
        dir = (*si)->velEnd;
        dir *= -1.0;
        pt = (*si)->front();
    }
    
    double len = dir.length();
    if ( len == 0.0 )
        return;

    //Jump out .1% of the min distance.
    dir /= len;
    double leapingDistance = minRange * 0.001;

    debug5<< "Leaping: "<<leapingDistance<< " dir = "<<dir<<endl;
    debug5<< "Leap: "<<pt;
    dir *= leapingDistance;
    avtVec newPt = pt + dir;
    _ivpSolver->SetCurrentY(newPt);
    _ivpSolver->SetCurrentT(_ivpSolver->GetCurrentT() + leapingDistance);
    
    if (forward)
        (*(--_steps.end()))->tEnd += leapingDistance;
    else
        (*_steps.begin())->tEnd -= leapingDistance;
    
    debug5<<" ==> "<<newPt<<" T: "<<_ivpSolver->GetCurrentT()<<endl;
}


// ****************************************************************************
//  Method: avtStreamline::TMin
//
//  Purpose:
//      Returns the minimum t value.
//
//  Programmer: Christoph Garth
//  Creation:   February 25, 2008
//
// ****************************************************************************

double 
avtStreamline::TMin() const
{
    return  _steps.empty() ? _t0 : (*(_steps.begin()))->tStart;
}

// ****************************************************************************
//  Method: avtStreamline::TMax
//
//  Purpose:
//      Returns the manimum t value.
//
//  Programmer: Christoph Garth
//  Creation:   February 25, 2008
//
// ****************************************************************************

double
avtStreamline::TMax() const
{
    return  _steps.empty() ? _t0 : (*(--_steps.end()))->tEnd;
}


// ****************************************************************************
//  Method: avtStreamline::begin
//
//  Purpose:
//      Returns the first iterator.
//
//  Programmer: Christoph Garth
//  Creation:   February 25, 2008
//
// ****************************************************************************

avtStreamline::iterator
avtStreamline::begin() const
{
    return _steps.begin();
}


// ****************************************************************************
//  Method: avtStreamline::end
//
//  Purpose:
//      Returns the last iterator.
//
//  Programmer: Christoph Garth
//  Creation:   February 25, 2008
//
// ****************************************************************************

avtStreamline::iterator
avtStreamline::end() const
{
    return _steps.end();
}


// ****************************************************************************
//  Method: avtStreamline::size
//
//  Purpose:
//      Returns the number of iterations to do.
//
//  Programmer: Christoph Garth
//  Creation:   February 25, 2008
//
// ****************************************************************************

size_t
avtStreamline::size() const
{
    return _steps.size();
}


// ****************************************************************************
//  Method: avtStreamline::PtEnd
//
//  Purpose:
//      Sets the ending locations for the forward and backward integrations.
//
//  Programmer: Christoph Garth
//  Creation:   February 25, 2008
//
//  Modifications:
//
//    Dave Pugmire, Mon Feb 23, 09:11:34 EST 2009
//    Reworked the termination code. Added a type enum and value. Made num steps
//    a termination criterion. Code cleanup: We no longer need fwd/bwd solvers.
//
// ****************************************************************************

void
avtStreamline::PtEnd(avtVec &end)
{
    end = _ivpSolver->GetCurrentY();
}


// ****************************************************************************
//  Method: avtStreamline::Serialize
//
//  Purpose:
//      Serializes a streamline so it can be sent to another processor.
//
//  Programmer: Christoph Garth
//  Creation:   February 25, 2008
//
//  Modifications:
//    Kathleen Bonnell, Thu Aug  7, 08:29:42 PDT 2008
//    Changed for loop to use size_t to eliminate signed/unsigned int 
//    comparison warnings.
//
//    Hank Childs, Tue Aug 19 17:05:38 PDT 2008
//    Initialize the sz variable to make purify happy.
//
//    Dave Pugmire, Tue Feb  3 10:54:34 EST 2009
//    More debug statements.
//
//    Dave Pugmire, Mon Feb 23, 09:11:34 EST 2009
//    Code cleanup: We no longer need fwd/bwd solvers.
//
// ****************************************************************************

void
avtStreamline::Serialize(MemStream::Mode mode, MemStream &buff, 
                         avtIVPSolver *solver)
{
    buff.io( mode, _p0 );
    buff.io( mode, _t0 );

    // R/W the steps.
    if ( mode == MemStream::WRITE )
    {
        size_t sz = _steps.size();
        buff.io( mode, sz );
        for ( iterator si = _steps.begin(); si != _steps.end(); si++ )
            (*si)->Serialize( mode, buff );
    }
    else
    {
        debug5 << "avtStreamline READ: listSz = " << _steps.size() <<endl;
        _steps.clear();
        size_t sz = 0;
        buff.io( mode, sz );
        for ( size_t i = 0; i < sz; i++ )
        {
            avtIVPStep *s = new avtIVPStep;
            s->Serialize( mode, buff );
            _steps.push_back( s );
        }
    }

    if ( mode == MemStream::WRITE )
    {
        avtIVPState solverState;

        _ivpSolver->GetState(solverState);
        solverState.Serialize(mode, buff);
    }
    else
    {
        // TODO:
        //_ivpSolver->Serialize( mode, buff );

        avtIVPState solverState;
        solverState.Serialize(mode, buff);

        if (_ivpSolver)
            delete _ivpSolver;
        
        _ivpSolver = solver->Clone();
        _ivpSolver->PutState(solverState);
    }
    debug5 << "DONE: avtStreamline::Serialize. sz= "<<buff.buffLen() << endl;
}
