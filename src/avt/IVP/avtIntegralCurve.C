/*****************************************************************************
*
* Copyright (c) 2000 - 2011, Lawrence Livermore National Security, LLC
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

const double avtIntegralCurve::minHFactor = std::numeric_limits<double>::epsilon() * 100.0;

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
//   Hank Childs, Tue Oct  5 18:41:35 PDT 2010
//   Remove references to "termination", which now go in derived type.
//
//   Dave Pugmire, Fri Nov  5 15:34:49 EDT 2010
//   Add counter to handle communication of ICs
//
//   Hank Childs, Sun Dec  5 11:43:46 PST 2010
//   Initialize encounteredNumericalProblems.
//
// ****************************************************************************

avtIntegralCurve::avtIntegralCurve( const avtIVPSolver* model, 
                                    Direction dir,
                                    const double& t_start,
                                    const avtVector &p_start, 
                                    const avtVector &v_start, 
                                    long ID )
    : status(STATUS_OK), direction(dir), domain(-1), sortKey(0), id(ID)
{
    ivp = model->Clone();
    ivp->Reset( t_start, p_start, v_start );
    counter = 0;
    encounteredNumericalProblems = false;
    postStepCallbackFunction = NULL;
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
//   Hank Childs, Tue Oct  5 18:41:35 PDT 2010
//   Remove references to "termination", which now go in derived type.
//
//   Dave Pugmire, Fri Nov  5 15:34:49 EDT 2010
//   Add counter to handle communication of ICs
//
//   Hank Childs, Sun Dec  5 11:43:46 PST 2010
//   Initialize encounteredNumericalProblems.
//
// ****************************************************************************

avtIntegralCurve::avtIntegralCurve()
{
    ivp = NULL;

    status = STATUS_OK;
    domain = -1;
    sortKey = 0;
    id = -1;
    counter = 0;
    encounteredNumericalProblems = false;
    postStepCallbackFunction = NULL;
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
    if( ivp )
        delete ivp;
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
//    Christoph Garth, Wed Jul 21, 09:27:01 PDT 2010
//    Rolled DoAdvance into this method since all it did was call it.
//
//    Hank Childs, Sun Dec  5 11:43:46 PST 2010
//    Indicate when we have a numerical problem.
//
//    David Camp, Tue Feb  1 09:45:41 PST 2011
//    Added a catch just incase an error ocurres.
//
//   Dave Pugmire, Fri Feb 18 14:52:18 EST 2011
//   Replaced minH with minHFactor for use when integrating upto a domain boundary.
//
// ****************************************************************************

void avtIntegralCurve::Advance( avtIVPField* field )
{
    // Catch cases where the start position is outside the
    // domain of field; in this case, mark the curve 
    if( !field->IsInside( ivp->GetCurrentT(), ivp->GetCurrentY() ) )
    {
        if( DebugStream::Level5() )
            debug5 << "avtIntegralCurve::Advance(): initial point is outside domain\n";
        return;
    }

    double range[2];
    field->GetTimeRange( range );

    // Determine the maximum integration time from the field's temporal
    // extent and the time termination criterion (if set).
    double tfinal;

    if( direction == DIRECTION_FORWARD )
    {
        tfinal = range[1];

        if (UseFixedTerminationTime() && FixedTerminationTime() < tfinal)
            tfinal = FixedTerminationTime();
    }
    else
    {
        tfinal = range[0];

        if (UseFixedTerminationTime() && FixedTerminationTime() > tfinal)
            tfinal = FixedTerminationTime();
    }

    // Loop doing integration steps.
    do
    {
        avtIVPStep           step;
        avtIVPSolver::Result result;

        try
        { 
            result = ivp->Step( field, tfinal, &step );
        }
        catch (avtIVPField::Undefined&)
        {
            result = avtIVPSolver::OUTSIDE_DOMAIN;
        }

        if( result == avtIVPSolver::OK || result == avtIVPSolver::TERMINATE )
        {
            // The step was successful, call AnalyzeStep() which will
            // determine((among other things) whether to terminate.
            AnalyzeStep( step, field );

            if( result == avtIVPSolver::TERMINATE )
                break;
        }
        else if( result == avtIVPSolver::OUTSIDE_DOMAIN )
        {
            // Last step took us outside the domain, see what can be done.

            // First, check if the current point is inside the domain.
            // If it is outside, there is nothing further we can do.
            if( !field->IsInside( ivp->GetCurrentT(), ivp->GetCurrentY() ) )
            {
                if( DebugStream::Level5() )
                    debug5 << "avtIntegralCurve::Advance(): "
                           << "current point outside domain\n";

                break;
            }

            // The point is inside, hence the step was too long. 
            // Try to reduce the step size.
            double h = ivp->GetNextStepSize();
            double t = ivp->GetCurrentT();

            if( h == 0.0 )
            {
                // In the case where h = 0, the integrator's initial 
                // guess failed; use the maximum stepsize in this case.
                // If that is zero, too, we cannot make any guess as to
                // a suitable stepsize, so we use 1.0, and rely on 
                // the integrator and boundary handling to adjust this 
                // to a more sensible value.

                // FIXME
                // h = ivp->GetMaximumStepSize();

                if( h == 0.0 )
                    h = 1.0;

                if( DebugStream::Level5() )
                    debug5 << "avtIntegralCurve::Advance(): step outside, "
                           << "retry with initial guess " << h << '\n';
            }
            else if (std::abs(h) <= std::abs(t) * minHFactor)
            {
                // If we are close enough to the boundary, stop the bisection.
                // Do a very small Euler step to move the integrator's current 
                // position just outside the domain so that it is inside the 
                // next domain.
                avtVector y = ivp->GetCurrentY();
                avtVector v;
                try
                { 
                    v = (*field)( t, y );
                }
                catch (avtIVPField::Undefined&)
                {
                    if( DebugStream::Level5() )
                        debug5 << "avtIntegralCurve::Advance(): bad step, "
                               << "Error with t: " << t << " y: " << y << endl;
                    break;
                }
                        
                // Determine the stepsize such that the push distance
                // in either coordinate direction is at least 1e-6
                // times the bounding box extent in one directions. 
                // That should get the point into the next domain even
                // if the dataset geometry is given as float data.
                double ext[6];
                field->GetExtents( ext );

                const double eps = 1e-6;

                ext[0] = std::abs( v.x ? eps * (ext[1] - ext[0]) / v.x : 0.0 );
                ext[1] = std::abs( v.y ? eps * (ext[3] - ext[2]) / v.y : 0.0 );
                ext[2] = std::abs( v.z ? eps * (ext[5] - ext[4]) / v.z : 0.0 );

                // TODO: max maybe to big of a step, min to small. Need to look at the middle value.
                double hmin = std::max( ext[0], std::max( ext[1], ext[2] ) );
                //double hmin = std::min( ext[0], std::min( ext[1], ext[2] ) );

                if( std::abs(h) < hmin )
                    h = h < 0 ? -hmin : hmin;

                t += h;
                y += h*v;

                if( DebugStream::Level5() )
                    debug5 << "avtIntegralCurve::Advance(): step outside, minimal "
                           << "stepsize reached, pushing to " << y << '\n';
                        
                ivp->SetCurrentT( t );
                ivp->SetCurrentY( y );

                break;
            }

            // Retry with halved stepsize.
            ivp->SetNextStepSize( h/2.0 );
        }
        else
        {
            // If we get here, the integration resulted in a (likely
            // numerical) error. Cannot continue the integration.
            if( DebugStream::Level5() )
                debug5 << "avtIntegralCurve::Advance(): "
                       << "error during step, finished\n";
            encounteredNumericalProblems = true;

            status = STATUS_FINISHED;
        }
        
        /*
        if (status == STATUS_OK && postStepCallbackFunction != NULL)
            postStepCallbackFunction();
        */
    }
    while( status == STATUS_OK );

    if( DebugStream::Level5() )
        debug5 << "avtIntegralCurve::Advance(): done, status is "
               << status << '\n';
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
    return ivp->GetCurrentT();
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
    end = ivp->GetCurrentY();
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
//    Hank Childs, Tue Jun  8 09:30:45 CDT 2010
//    Separate out portions related to sequence tracking into
//    avtStateRecorderIntegralCurve.
//
//   Hank Childs, Tue Oct  5 18:41:35 PDT 2010
//   Remove references to "termination", which now go in derived type.
//
//   Dave Pugmire, Fri Nov  5 15:34:49 EDT 2010
//   Add counter to handle communication of ICs
//
//   Hank Childs, Sun Dec  5 11:43:46 PST 2010
//   Send encounteredNumericalProblems.
//
// ****************************************************************************

void
avtIntegralCurve::Serialize(MemStream::Mode mode, MemStream &buff, 
                            avtIVPSolver *solver)
{
    if( DebugStream::Level5() )
        debug5 << "  avtIntegralCurve::Serialize "
           << (mode==MemStream::READ?"READ":"WRITE")<<endl;

    buff.io(mode, id);
    buff.io(mode, direction);
    buff.io(mode, domain);
    buff.io(mode, status);
    buff.io(mode, counter);
    buff.io(mode, encounteredNumericalProblems);
    
    if ( mode == MemStream::WRITE )
    {
        avtIVPState solverState;

        ivp->GetState(solverState);
        solverState.Serialize(mode, buff);
    }
    else
    {
        // TODO:
        //ivp->Serialize( mode, buff );

        avtIVPState solverState;
        solverState.Serialize(mode, buff);

        if( ivp )
            delete ivp;
        
        ivp = solver->Clone();
        ivp->PutState(solverState);
    }    

    if( DebugStream::Level5() )
        debug5 << "avtIntegralCurve::Serialize() size is " 
               << buff.len() << endl;
}

bool
avtIntegralCurve::DomainCompare(const avtIntegralCurve *icA,
                                const avtIntegralCurve *icB)
{
    return icA->domain < icB->domain;
}

