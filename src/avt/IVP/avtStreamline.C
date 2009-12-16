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
//                              avtStreamline.C                              //
// ************************************************************************* //

#include <avtStreamline.h>

#include <list>
#include <iostream>
#include <limits>
#include <ImproperUseException.h>
#include <DebugStream.h>
#include <vtkPlane.h>

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
//   Dave Pugmire, Mon Jun 8 2009, 11:44:01 EDT 2009
//   Remove wantVorticity, intialize scalarValueType.
//
//   Dave Pugmire, Tue Aug 11 10:25:45 EDT 2009
//   Add new termination criterion: Number of intersections with an object.
//
//   Dave Pugmire, Tue Aug 18 08:47:40 EDT 2009
//   Don't record intersection points, just count them.
//
//    Dave Pugmire, Tue Dec  1 11:50:18 EST 2009
//    Switch from avtVec to avtVector.
//  
// ****************************************************************************

avtStreamline::avtStreamline(const avtIVPSolver* model, const double& t_start,
                             const avtVector &p_start, int ID) :
    scalarValueType(NONE)
{
    _t0 = t_start;
    _p0 = p_start;
    id = ID;
    intersectionsSet = false;
    numIntersections = 0;
    
    _ivpSolver = model->Clone();
    _ivpSolver->Reset(_t0, _p0);
}


// ****************************************************************************
//  Method: avtStreamline constructor
//
//  Programmer: Christoph Garth
//  Creation:   February 25, 2008
//
//  Modifications:
//
//   Dave Pugmire, Mon Jun 8 2009, 11:44:01 EDT 2009
//   Remove wantVorticity, intialize scalarValueType.
//
//   Dave Pugmire, Tue Aug 11 10:25:45 EDT 2009
//   Add new termination criterion: Number of intersections with an object.
//
//   Dave Pugmire, Tue Aug 18 08:47:40 EDT 2009
//   Don't record intersection points, just count them.
//
// ****************************************************************************

avtStreamline::avtStreamline() :
    scalarValueType(NONE)
{
    intersectionsSet = false;
    _ivpSolver = NULL;
    numIntersections = 0;
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
//   Dave Pugmire, Mon Jun 8 2009, 11:44:01 EDT 2009
//   Removed the wantVorticity, extents and ghostzone flags. Extents and ghost
//   zones are handled by the vtkDataSet itself. The wantVorticity was replaced
//   with a scalarValueType which can be 'or'-d together to specify what to
//   compute.
//
// ****************************************************************************

avtIVPSolver::Result 
avtStreamline::Advance(const avtIVPField* field,
                       avtIVPSolver::TerminateType termType,
                       double end)
{
    avtIVPSolver::Result res = DoAdvance(_ivpSolver, field, termType, end);
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
//    Hank Childs, Fri Apr 10 23:31:22 CDT 2009
//    Put if statements in front of debug's.  The generation of strings to
//    output to debug was doubling the total integration time.
//
//    Mark C. Miller, Wed Apr 22 13:48:13 PDT 2009
//    Changed interface to DebugStream to obtain current debug level.
//
//    Dave Pugmire, Mon Jun 8 2009, 11:44:01 EDT 2009
//    Removed the wantVorticity, extents and ghostzone flags. Extents and ghost
//    zones are handled by the vtkDataSet itself. The wantVorticity was replaced
//    with a scalarValueType which can be 'or'-d together to specify what to
//    compute.
//
//   Dave Pugmire, Tue Aug 11 10:25:45 EDT 2009
//   Add new termination criterion: Number of intersections with an object.
//
// ****************************************************************************

avtIVPSolver::Result
avtStreamline::DoAdvance(avtIVPSolver* ivp,
                         const avtIVPField* field,
                         avtIVPSolver::TerminateType termType,
                         double end)
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
            if (DebugStream::Level5())
                debug5<<"Step( mode= "<<termType<<" end= "<<end<<endl;
            result = ivp->Step(field, termType, end, step);
            if (DebugStream::Level5())
                debug5<<"   T= "<<ivp->GetCurrentT()<<" "<<ivp->GetCurrentY()<<endl;
            if (intersectionsSet)
              HandleIntersections((end>0), step, termType, end, &result);
        }
        catch( avtIVPField::Undefined& )
        {
            if (DebugStream::Level5())
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
                if (!field->HasGhostZones())
                {
                    double bbox[6];
                    field->GetExtents(bbox);
                    HandleGhostZones((end > 0.0), bbox);
                }

                if (DebugStream::Level5())
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
            //Set scalar value, if any...
            if (scalarValueType & VORTICITY)
                step->ComputeVorticity(field);
            if (scalarValueType & SPEED)
                step->ComputeSpeed(field);
            if (scalarValueType & SCALAR_VARIABLE)
                step->ComputeScalarVariable(field);
            
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
//    Hank Childs, Fri Apr 10 23:31:22 CDT 2009
//    Put if statements in front of debug's.  The generation of strings to
//    output to debug was doubling the total integration time.
//
//    Mark C. Miller, Wed Apr 22 13:48:13 PDT 2009
//    Changed interface to DebugStream to obtain current debug level.
//
//    Dave Pugmire, Tue Nov  3 09:15:41 EST 2009
//    Replace size() with much more efficient empty().
//
//    Dave Pugmire, Tue Dec  1 11:50:18 EST 2009
//    Switch from avtVec to avtVector.
//
// ****************************************************************************

void
avtStreamline::HandleGhostZones(bool forward, double *extents)
{
    if (_steps.empty() || extents == NULL)
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
    
    //Get the direction of the last step.
    avtVector dir, pt;
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

    if (DebugStream::Level5())
        debug5<< "Leaping: "<<leapingDistance<< " dir = "<<dir<<endl;
    if (DebugStream::Level5())
        debug5<< "Leap: "<<pt;
    dir *= leapingDistance;
    avtVector newPt = pt + dir;
    _ivpSolver->SetCurrentY(newPt);
    _ivpSolver->SetCurrentT(_ivpSolver->GetCurrentT() + leapingDistance);
    
    if (forward)
        (*(--_steps.end()))->tEnd += leapingDistance;
    else
        (*_steps.begin())->tEnd -= leapingDistance;
    
    if (DebugStream::Level5())
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
//    Dave Pugmire, Tue Dec  1 11:50:18 EST 2009
//    Switch from avtVec to avtVector.
//
// ****************************************************************************

void
avtStreamline::PtEnd(avtVector &end)
{
    end = _ivpSolver->GetCurrentY();
}

// ****************************************************************************
//  Method: avtStreamline::SetIntersectionObject
//
//  Purpose:
//      Defines an object for streamline intersection.
//
//  Programmer: Dave Pugmire
//  Creation:   August 10, 2009
//
//  Modifications:
//
//   Dave Pugmire, Tue Aug 18 08:47:40 EDT 2009
//   Store plane equation.
//
// ****************************************************************************

void
avtStreamline::SetIntersectionObject(vtkObject *obj)
{
    // Only plane supported for now.
    if (!obj->IsA("vtkPlane"))
        EXCEPTION1(ImproperUseException, "Only plane supported.");

    intersectionsSet = true;
    avtVector intersectPlanePt = avtVector(((vtkPlane *)obj)->GetOrigin());
    avtVector intersectPlaneNorm = avtVector(((vtkPlane *)obj)->GetNormal());

    intersectPlaneNorm.normalize();
    intersectPlaneEq[0] = intersectPlaneNorm.x;
    intersectPlaneEq[1] = intersectPlaneNorm.y;
    intersectPlaneEq[2] = intersectPlaneNorm.z;
    intersectPlaneEq[3] = intersectPlanePt.length();
}

// ****************************************************************************
//  Method: avtStreamline::HandleIntersections
//
//  Purpose:
//      Defines an object for streamline intersection.
//
//  Programmer: Dave Pugmire
//  Creation:   August 10, 2009
//
//  Modifications:
//
//   Dave Pugmire, Tue Aug 18 08:47:40 EDT 2009
//   Don't record intersection points, just count them.
//
// ****************************************************************************

void
avtStreamline::HandleIntersections(bool forward,
                                   avtIVPStep *step,
                                   avtIVPSolver::TerminateType termType,
                                   double end,
                                   avtIVPSolver::Result *result)
{
    if (step == NULL || _steps.size() == 0)
        return;
    
    avtIVPStep *step0 = (forward ? _steps.back() : _steps.front());

    if (IntersectPlane(step0->front(), step->front()))
    {
        numIntersections++;
        if (termType == avtIVPSolver::INTERSECTIONS &&
            numIntersections >= (int)end)
        {
            *result = avtIVPSolver::TERMINATE;
        }
    }
}

// ****************************************************************************
//  Method: avtStreamline::IntersectPlane
//
//  Purpose:
//      Intersect streamline with a plane.
//
//  Programmer: Dave Pugmire
//  Creation:   August 10, 2009
//
//  Modifications:
//
//   Dave Pugmire, Tue Aug 18 08:47:40 EDT 2009
//   Don't record intersection points, just count them.
//
//    Dave Pugmire, Tue Dec  1 11:50:18 EST 2009
//    Switch from avtVec to avtVector.
//
// ****************************************************************************

bool
avtStreamline::IntersectPlane(const avtVector &p0, const avtVector &p1)
{
    double distP0 = intersectPlaneEq[0] * p0.x +
                    intersectPlaneEq[1] * p0.y +
                    intersectPlaneEq[2] * p0.z +
                    intersectPlaneEq[3];

    double distP1 = intersectPlaneEq[0] * p1.x +
                    intersectPlaneEq[1] * p1.y +
                    intersectPlaneEq[2] * p1.z +
                    intersectPlaneEq[3];

#define SIGN(x) ((x) < 0.0 ? -1 : 1)

    // If either point on the plane, or points on opposite
    // sides of the plane, the line intersects.
    if (distP0 == 0.0 || distP1 == 0.0 ||
        SIGN(distP0) != SIGN(distP1))
    {
        return true;
    }

    return false;
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
//    Hank Childs, Fri Apr 10 23:31:22 CDT 2009
//    Put if statements in front of debug's.  The generation of strings to
//    output to debug was doubling the total integration time.
//
//    Mark C. Miller, Wed Apr 22 13:48:13 PDT 2009
//    Changed interface to DebugStream to obtain current debug level.
//
//    Dave Pugmire, Tue Aug 18 08:47:40 EDT 2009
//    Don't record intersection points, just count them.
//
//   Dave Pugmire, Thu Sep 24 13:52:59 EDT 2009
//   Option to serialize steps.
//
// ****************************************************************************

void
avtStreamline::Serialize(MemStream::Mode mode, MemStream &buff, 
                         avtIVPSolver *solver,
                         bool serializeSteps)
{
    if (DebugStream::Level5())
        debug5<<"  avtStreamline::Serialize "<<(mode==MemStream::READ?"READ":"WRITE")<<" serSteps= "<<serializeSteps<<endl;
    buff.io(mode, _p0);
    buff.io(mode, _t0);
    buff.io(mode, scalarValueType);
    buff.io(mode, numIntersections);

    // R/W the steps.
    if (mode == MemStream::WRITE)
    {
        size_t sz = _steps.size();
        if (serializeSteps)
        {
            buff.io(mode, sz);
            for (iterator si = _steps.begin(); si != _steps.end(); si++)
                (*si)->Serialize(mode, buff);
        }
        else
        {
            sz = 0;
            buff.io(mode, sz);
        }
    }
    else
    {
        _steps.clear();
        size_t sz = 0;
        buff.io( mode, sz );
        debug5<<"Read step cnt= "<<sz<<endl;
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
    if (DebugStream::Level5())
        debug5 << "DONE: avtStreamline::Serialize. sz= "<<buff.buffLen() << endl;
    
    //FIX THIS!!!
    if (intersectionsSet != true || intersectionsSet != false)
        intersectionsSet = false;
}
