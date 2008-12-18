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
// ****************************************************************************

avtStreamline::avtStreamline(const avtIVPSolver* model, const double& t_start,
                             const avtVec& p_start, int ID)
{
    _t0 = t_start;
    _p0 = p_start;
    id = ID;
    intersectPlaneSet = false;
    
    _ivp_fwd = model->Clone();
    _ivp_fwd->Reset(_t0, _p0);

    _ivp_bwd = model->Clone();
    _ivp_bwd->Reset(_t0, _p0);
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
    intersectPlaneSet = false;
    _ivp_fwd = NULL;
    _ivp_bwd = NULL;
    wantVorticity = false;
}


// ****************************************************************************
//  Method: avtStreamline destructor
//
//  Programmer: Christoph Garth
//  Creation:   February 25, 2008
//
// ****************************************************************************

avtStreamline::~avtStreamline()
{
    if ( _ivp_fwd )
        delete _ivp_fwd;
    if ( _ivp_bwd )
        delete _ivp_bwd;
    
    _ivp_fwd = NULL;
    _ivp_bwd = NULL;
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
// ****************************************************************************

avtIVPSolver::Result 
avtStreamline::Advance(const avtIVPField* field,
                       bool timeMode,
                       double end,
                       int maxSteps,
                       bool vorticity,
                       bool haveGhostZones,
                       double *extents)
{
    wantVorticity = vorticity;
    double tEnd, dEnd;
    
    if ( timeMode )
    {
        tEnd = end;
        dEnd = std::numeric_limits<double>::max();
    }
    else
    {
        dEnd = end;
        tEnd = std::numeric_limits<double>::max();
        if ( end < 0 )
        {
            tEnd = -tEnd;
            dEnd = fabs(dEnd);
        }
    }
    
    if (tEnd < TMin())
        return DoAdvance(_ivp_bwd, field, tEnd, dEnd, timeMode,
                         maxSteps, haveGhostZones, extents);
    else if (tEnd > TMax())
        return DoAdvance(_ivp_fwd, field, tEnd, dEnd, timeMode,
                         maxSteps, haveGhostZones, extents);
    
    return avtIVPSolver::OK;
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
// ****************************************************************************

avtIVPSolver::Result
avtStreamline::DoAdvance(avtIVPSolver* ivp,
                         const avtIVPField* field,
                         double tEnd,
                         double dEnd,
                         bool timeMode,
                         int maxSteps,
                         bool haveGhostZones,
                         double *extents)
{
    avtIVPSolver::Result result;

    // catch cases where the start position is outside the 
    // domain of field
    if(! field->IsInside(ivp->GetCurrentT(), ivp->GetCurrentY()))
    {
        //cout<<"Pt0 "<<ivp->GetCurrentY()<<" not in domain\n";
        return avtIVPSolver::OUTSIDE_DOMAIN;
    }
    
    int numSteps = 0;
    while (1)
    {
        // record state for later restore, if needed
        avtIVPState state;
        ivp->GetState( state );

        // create new step to be filled in by ivp
        avtIVPStep* step = new avtIVPStep;

        try
        {
            //debug1<< "Step( t= "<<tEnd<<", d= "<<dEnd<<" );\n";
            result = ivp->Step(field, timeMode, tEnd, dEnd, step);
            numSteps++;
        }
        catch( avtIVPField::Undefined& )
        {
            //debug1<<ivp->GetCurrentY()<<" not in domain\n";
            // integrator left the domain, retry with smaller step
            // if step size is below given minimum, give up

            // restore old state to before failed step
            ivp->PutState( state );

            double h = ivp->GetNextStepSize();

            h = h/2;
            
            if( fabs(h) < 1e-9 )
            {
                delete step;
                HandleGhostZones(ivp, haveGhostZones, extents);
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
        if (result == avtIVPSolver::OK)
        {
            if ( wantVorticity )
                step->ComputeVorticity( field );
            
            if (ivp->GetCurrentT() < TMin())
            {
                if (!_steps.empty())
                    IntersectWithPlane( _steps.front(), step );
                _steps.push_front( step );

                if( ivp->GetCurrentT() <= tEnd )
                {
                    break;
                }
            }
            else 
            {
                if (!_steps.empty())
                    IntersectWithPlane( _steps.back(), step );
                _steps.push_back( step );

                if( ivp->GetCurrentT() >= tEnd )
                {
                    break;
                }
            }
            
            // Max steps reached. Bail out.
            if (maxSteps != -1 && numSteps >= maxSteps)
            {
                break;
            }
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
// ****************************************************************************

void
avtStreamline::HandleGhostZones(avtIVPSolver *ivp,
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
    if ( ivp == _ivp_fwd )
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

    debug1<< "Leaping: "<<leapingDistance<< " dir = "<<dir<<endl;
    debug1<< "Leap: "<<pt;
    dir *= leapingDistance;
    avtVec newPt = pt + dir;
    debug1<<" ==> "<<newPt<<endl;

    ivp->SetCurrentY( newPt );
    ivp->SetCurrentT( ivp->GetCurrentT() + leapingDistance );
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
//  Method: avtStreamline::PtEnds
//
//  Purpose:
//      Sets the ending locations for the forward and backward integrations.
//
//  Programmer: Christoph Garth
//  Creation:   February 25, 2008
//
// ****************************************************************************

void
avtStreamline::PtEnds( avtVec &ptBwd, avtVec &ptFwd ) const
{
    ptBwd = _ivp_bwd->GetCurrentY();
    ptFwd = _ivp_fwd->GetCurrentY();
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
        //debug1 << "avtStreamline READ: listSz = " << _steps.size() <<endl;
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
        avtIVPState stateF, stateB;

        _ivp_fwd->GetState( stateF );
        stateF.Serialize( mode, buff );

        _ivp_bwd->GetState( stateB );
        stateB.Serialize( mode, buff );
    }
    else
    {
        // TODO:
        //_ivp_fwd->Serialize( mode, buff );
        //_ivp_bwd->Serialize( mode, buff );

        avtIVPState stateF, stateB;
        stateF.Serialize( mode, buff );
        stateB.Serialize( mode, buff );

        if ( _ivp_fwd )
            delete _ivp_fwd;
        if ( _ivp_bwd )
            delete _ivp_bwd;
        
        _ivp_fwd = solver->Clone();
        _ivp_bwd = solver->Clone();

        _ivp_fwd->PutState( stateF );
        _ivp_bwd->PutState( stateB );
    }
    buff.io( mode, intersectPlaneSet );
    buff.io( mode, intersectPlanePt );
    buff.io( mode, intersectPlaneNorm );
    if ( mode == MemStream::READ )
    {
        if ( intersectPlaneSet )
            SetIntersectPlane( intersectPlanePt, intersectPlaneNorm );
    }
    
    buff.io( mode, intersectPts );
    //debug1<<"IO: iPts = "<<intersectPts.size()<<endl;

    //debug1 << "DONE: avtStreamline::Serialize. sz= "<<buff.buffLen() << endl;
}

// ****************************************************************************
//  Method: avtStreamline::UnsetIntersectPlane
//
//  Purpose:
//      Indicate that we don't want to do processing that focuses on plane
//      intersections.
//
//  Programmer: Christoph Garth
//  Creation:   February 25, 2008
//
// ****************************************************************************

void
avtStreamline::UnsetIntersectPlane()
{
    intersectPlaneSet = false;
}


// ****************************************************************************
//  Method: avtStreamline::SetIntersectPlane
//
//  Purpose:
//      Sets a plane that we should focus on while integrating streamlines.
//
//  Programmer: Christoph Garth
//  Creation:   February 25, 2008
//
// ****************************************************************************

void
avtStreamline::SetIntersectPlane( const avtVec &pt, const avtVec &norm )
{
    intersectPlanePt = pt;
    intersectPlaneNorm = (norm / norm.length());

    planeEq[0] = intersectPlaneNorm.values()[0];
    planeEq[1] = intersectPlaneNorm.values()[1];
    planeEq[2] = intersectPlaneNorm.values()[2];
    planeEq[3] = -inner(intersectPlanePt,intersectPlaneNorm);
    intersectPlaneSet = true;
}


// ****************************************************************************
//  Method: avtStreamline::IntersectLinePlane
//
//  Purpose:
//      Determines if a plane and line intersect and where
//
//  Programmer: Christoph Garth
//  Creation:   February 25, 2008
//
// ****************************************************************************

static bool
IntersectLinePlane(avtVec &Lp0, avtVec &Lp1, avtVec &plnPt, 
                   avtVec &plnNorm, avtVec &intPt)
{
    avtVec lineDir = Lp1-Lp0;
    avtVec lineDirN = lineDir / lineDir.length();
    
    // No intersection!
    double dotProd = inner( lineDir, plnNorm );
    if ( dotProd <= 0.0 )
        return false;

    avtVec w = Lp0-plnPt;
    double s = -inner(plnNorm,w) / dotProd;
    intPt = Lp0 + s*(lineDir);
    
    return true;
}


// ****************************************************************************
//  Method: avtStreamline::IntersectWithPlane
//
//  Purpose:
//      Intersects a streamline with a plane.
//
//  Programmer: Christoph Garth
//  Creation:   February 25, 2008
//
// ****************************************************************************

void
avtStreamline::IntersectWithPlane(avtIVPStep *step0, avtIVPStep *step1)
{
    if ( !intersectPlaneSet )
        return;

    if ( step0 == NULL || step1 == NULL )
        return;
    
    avtVec p0(0.0,0.0,0.0), p1(0.0,0.0,0.0);
    for ( int i = 0; i < 3; i++ )
    {
        p0.values()[i] = step0->front().values()[i];
        p1.values()[i] = step1->front().values()[i];
    }

    double distP0 = p0.values()[0]*planeEq[0] + p0.values()[1]*planeEq[1] 
                  + p0.values()[2]*planeEq[2] + planeEq[3];
    double distP1 = p1.values()[0]*planeEq[0] + p1.values()[1]*planeEq[1] 
                  + p1.values()[2]*planeEq[2] + planeEq[3];

#define SIGN(x) ((x) < 0.0 ? -1 : 1)
    
    //    cout<<"P0: "<<p0<<endl;
    //    cout<<"P1: "<<p1<<endl;
    //    cout<<"IntersectWithPlane: "<<distP0<<" "<<distP1<<" ::==> "
    //        <<(SIGN(distP0) != SIGN(distP1))<<endl;
    //    cout<<endl;
    
    if (SIGN(distP0) != SIGN(distP1))
    {
        avtVec intPt;
        if (IntersectLinePlane(p0, p1, intersectPlanePt, 
                               intersectPlaneNorm, intPt))
        {
            intersectPts.push_back( intPt );
            /*
            debug1<<"Compute: iPts = "<<intersectPts.size()<<endl;
            cout << "*******************Intersected the plane\n";
            cout<<p0<<" "<<p1<<" ==> "<<intPt<<endl;
            */
        }
    }
}


