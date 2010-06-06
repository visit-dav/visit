/*****************************************************************************
*
* Copyright (c) 2000 - 2010, Lawrence Livermore National Security, LLC
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
//                            avtIntegralCurve.C                             //
// ************************************************************************* //

#include <avtIntegralCurve.h>

#include <list>
#include <iostream>
#include <limits>
#include <ImproperUseException.h>
#include <DebugStream.h>
#include <avtVector.h>
#include <algorithm>

const double avtIntegralCurve::minH = 1e-9;


using namespace std;


// ****************************************************************************
//  Method: avtIntegralCurve constructor
//
//  Programmer: Christoph Garth
//  Creation:   February 25, 2008
//
//  Modifications:
//  
//   Dave Pugmire, Mon Feb 23, 09:11:34 EST 2009
//   Reworked the termination code. Added a type enum and value. Made num steps
//   a termination criterion. Code cleanup: We no longer need fwd/bwd solvers.
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
//   Dave Pugmire, Tue Dec  1 11:50:18 EST 2009
//   Switch from avtVec to avtVector.
//  
//   Hank Childs, Thu Jun  3 10:58:32 PDT 2010
//   Remove _t0 and _p0, which are no longer used.
//
//   Hank Childs, Fri Jun  4 19:58:30 CDT 2010
//   Incorporate avtStreamlineWrapper into avtStreamline.
//
//   Hank Childs, Sat Jun  5 16:29:56 CDT 2010
//   Separate out portions related to Streamline and Poincare into 
//   avtStateRecorderIntegralCurve.
//
// ****************************************************************************

avtIntegralCurve::avtIntegralCurve(const avtIVPSolver* model, 
                                   const double& t_start,
                                   const avtVector &p_start, int ID)
{
    _ivpSolver = model->Clone();
    _ivpSolver->Reset(t_start, p_start);

    status = STATUS_UNSET;
    domain = -1;
    terminated = false;
    id = ID;

    termination = 0.0;
    terminationType = avtIVPSolver::TIME;

    lastStepValid = false;

    sequenceCnt = 0;
    sortKey = 0;
    serializeFlags = 0;
}


// ****************************************************************************
//  Method: avtIntegralCurve constructor
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
//   Hank Childs, Fri Jun  4 19:58:30 CDT 2010
//   Incorporate avtStreamlineWrapper into avtStreamline.
//
//   Hank Childs, Sat Jun  5 16:29:56 CDT 2010
//   Separate out portions related to Streamline and Poincare into 
//   avtStateRecorderIntegralCurve.
//
// ****************************************************************************

avtIntegralCurve::avtIntegralCurve()
{
    _ivpSolver = NULL;

    status = STATUS_UNSET;
    domain = -1;
    terminated = false;
    id = -1;

    termination = 0.0;
    terminationType = avtIVPSolver::TIME;

    lastStepValid = false;

    sequenceCnt = 0;
    sortKey = 0;

    serializeFlags = 0;
}


// ****************************************************************************
//  Method: avtIntegralCurve destructor
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

avtIntegralCurve::~avtIntegralCurve()
{
    if ( _ivpSolver )
        delete _ivpSolver;
    
    _ivpSolver = NULL;
}

// ****************************************************************************
//  Method: avtIntegralCurve::Advance
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

avtIntegralCurve::Result 
avtIntegralCurve::Advance(const avtIVPField* field,
                          avtIVPSolver::TerminateType termType,
                          double end)
{
    return DoAdvance(_ivpSolver, field, termType, end);
}


// ****************************************************************************
//  Method: avtIntegralCurve::DoAdvance
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
//   Dave Pugmire, Tue Dec 29 14:37:53 EST 2009
//   Generalize the compute scalar variable.
//
//   Dave Pugmire, Wed Jan 20 09:28:59 EST 2010
//   Don't distinguish between forward/bwd integration for steps. Always add
//   add them to the back of the list.
//
//   Dave Pugmire, Wed May 26 13:48:24 EDT 2010
//   Use a new return code.
//
//   Hank Childs, Fri Jun  4 19:58:30 CDT 2010
//   Incorporate avtStreamlineWrapper into avtStreamline.
//
//   Hank Childs, Sat Jun  5 16:29:56 CDT 2010
//   Separate out portions related to Streamline and Poincare into 
//   avtStateRecorderIntegralCurve.
//
// ****************************************************************************

avtIntegralCurve::Result
avtIntegralCurve::DoAdvance(avtIVPSolver* ivp,
                            const avtIVPField* field,
                            avtIVPSolver::TerminateType termType,
                            double end)
{
    avtIVPSolver::Result result;

    // catch cases where the start position is outside the 
    // domain of field
    if (!field->IsInside(ivp->GetCurrentT(), ivp->GetCurrentY()))
        return avtIntegralCurve::RESULT_POINT_OUTSIDE_DOMAIN;

    avtIntegralCurve::Result rc;
    bool stepTaken = false;
    while (1)
    {
        // record state for later restore, if needed
        avtIVPState state;
        ivp->GetState(state);

        avtIVPStep step;

        try
        {
            result = ivp->Step(field, termType, end, &step);
            debug5<<"Step to "<<ivp->GetCurrentY()<<endl;
        }
        catch (avtIVPField::Undefined&)
        {
            if (DebugStream::Level5())
                debug5<< ivp->GetCurrentY() << " not in domain "
                      << ivp->GetNextStepSize() << endl;

            // integrator left the domain, retry with smaller step
            // if step size is below given minimum, give up
            // restore old state to before failed step
            double hBeforePush = ivp->GetNextStepSize();
            ivp->PutState(state);
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
            if (fabs(h) < minH)
            {
                if (!field->HasGhostZones())
                {
                    double bbox[6];
                    field->GetExtents(bbox);
                    HandleGhostZones((end>0.0), bbox);
                }

                if (DebugStream::Level5())
                    debug5<<"avtIntegralCurve::DoAdvance() DONE  result= "
                          <<"OUTSIDE_DOMAIN, step= " << h <<endl;

                rc = avtIntegralCurve::RESULT_EXIT_DOMAIN;
                break;
            }

            ivp->SetNextStepSize(h);
            continue;
        }
        
        catch (std::exception&)
        {
        }

        AnalyzeStep(&step, field, termType, end, &result);
        if (result == avtIVPSolver::TERMINATE || result == avtIVPSolver::OK)
        {
            lastStep = step;
            lastStepValid = true;
            stepTaken = true;
            if (result == avtIVPSolver::TERMINATE)
            {
                rc = avtIntegralCurve::RESULT_TERMINATE;
                break;
            }
        }
        else
        {
            rc = avtIntegralCurve::RESULT_ERROR;
            break;
        }
    }

    if (!stepTaken)
        rc = avtIntegralCurve::RESULT_ERROR;
    
    return rc;
}


// ****************************************************************************
//  Method: avtIntegralCurve::HandleGhostZones
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
//    Dave Pugmire, Tue Feb 23 09:42:25 EST 2010
//    Use a vector instead of a list for the integration steps.
//
//    Hank Childs, Sat Jun  5 16:29:56 CDT 2010
//    Set up jump direction and distance based on new data member for last 
//    step, since the list of steps is now in a derived type.
//
// ****************************************************************************

void
avtIntegralCurve::HandleGhostZones(bool forward, double *extents)
{
    if (!lastStepValid || extents == NULL)
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
    avtVector pt = lastStep.front();
    avtVector dir = lastStep.velEnd;
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
        lastStep.tEnd += leapingDistance;
    else
        lastStep.tEnd -= leapingDistance;
    
    if (DebugStream::Level5())
        debug5<<" ==> "<<newPt<<" T: "<<_ivpSolver->GetCurrentT()<<endl;
}


// ****************************************************************************
//  Method: avtIntegralCurve::CurrentTime
//
//  Purpose:
//      Returns the current t value.
//
//  Programmer: Christoph Garth
//  Creation:   February 25, 2008
//
//  Modifications:
//     
//     Hank Childs, Thu Jun  3 09:29:32 PDT 2010
//     If we haven't taken any steps, then get the time from the solver.
//     Renamed to CurrentTime.
//     
// ****************************************************************************

double
avtIntegralCurve::CurrentTime() const
{
    return _ivpSolver->GetCurrentT();
}


// ****************************************************************************
//  Method: avtIntegralCurve::CurrentLocation
//
//  Purpose:
//      Sets the current location of the integration.
//
//  Programmer: Hank Childs
//  Creation:   June 3, 2010
//
// ****************************************************************************

void
avtIntegralCurve::CurrentLocation(avtVector &end)
{
    end = _ivpSolver->GetCurrentY();
}


// ****************************************************************************
//  Method: avtIntegralCurve::Serialize
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
//    Dave Pugmire, Thu Sep 24 13:52:59 EDT 2009
//    Option to serialize steps.
//
//    Hank Childs, Thu Jun  3 10:58:32 PDT 2010
//    Remove _t0 and _p0, which are no longer used.
//
//    Hank Childs, Fri Jun  4 19:58:30 CDT 2010
//    Use avtStreamlines, not avtStreamlineWrappers.
//
//    Hank Childs, Sat Jun  5 16:29:56 CDT 2010
//    Separate out portions related to Streamline and Poincare into 
//    avtStateRecorderIntegralCurve.
//
// ****************************************************************************

void
avtIntegralCurve::Serialize(MemStream::Mode mode, MemStream &buff, 
                            avtIVPSolver *solver)
{
    if (DebugStream::Level5())
        debug5<<"  avtIntegralCurve::Serialize "
              <<(mode==MemStream::READ?"READ":"WRITE")<<endl;
    buff.io(mode, id);
    buff.io(mode, domain);
    buff.io(mode, status);
    buff.io(mode, terminated);
    buff.io(mode, termination);
    buff.io(mode, terminationType);
    
    lastStep.Serialize(mode, buff);

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

    if ((serializeFlags & SERIALIZE_INC_SEQ) && mode == MemStream::WRITE)
    {
        long seqCnt = sequenceCnt+1;
        buff.io(mode, seqCnt);
    }
    else
        buff.io(mode, sequenceCnt);

    if (DebugStream::Level5())
        debug5 << "DONE: avtIntegralCurve::Serialize. sz= "<<buff.buffLen() << endl;
}

// ****************************************************************************
//  Method: avtIntegralCurve::IdSeqCompare
//
//  Purpose:
//      Sort streamlines by id, then sequence number.
//
//  Programmer: Dave Pugmire
//  Creation:   September 24, 2009
//
//  Modifications:
//
//    Hank Childs, Fri Jun  4 19:58:30 CDT 2010
//    Move this method from avtStreamlineWrapper.
//
// ****************************************************************************
bool
avtIntegralCurve::IdSeqCompare(const avtIntegralCurve *icA,
                               const avtIntegralCurve *icB)
{
    if (icA->id == icB->id)
        return icA->sequenceCnt < icB->sequenceCnt;

    return icA->id < icB->id;
}

// ****************************************************************************
//  Method: avtIntegralCurve::IdRevSeqCompare
//
//  Purpose:
//      Sort streamlines by id, then reverse sequence number.
//
//  Programmer: Dave Pugmire
//  Creation:   September 24, 2009
//
//  Modifications:
//
//    Hank Childs, Fri Jun  4 19:58:30 CDT 2010
//    Move this method from avtStreamlineWrapper.
//
// ****************************************************************************
bool
avtIntegralCurve::IdRevSeqCompare(const avtIntegralCurve *icA,
                                  const avtIntegralCurve *icB)
{
    if (icA->id == icB->id)
        return icA->sequenceCnt > icB->sequenceCnt;

    return icA->id < icB->id;
}

bool
avtIntegralCurve::DomainCompare(const avtIntegralCurve *icA,
                                const avtIntegralCurve *icB)
{
    return icA->domain < icB->domain;
}


