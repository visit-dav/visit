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
// ****************************************************************************

avtIVPSolver::Result 
avtStreamline::Advance(const avtIVPField* field, double tEnd, bool vorticity, 
                       bool haveGhostZones)
{
    wantVorticity = vorticity;
    
    if (tEnd < TMin())
        return DoAdvance(_ivp_bwd, field, tEnd, haveGhostZones);
    else if (tEnd > TMax())
        return DoAdvance(_ivp_fwd, field, tEnd, haveGhostZones);
    
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
// ****************************************************************************

avtIVPSolver::Result
avtStreamline::DoAdvance(avtIVPSolver* ivp, const avtIVPField* field, 
                         double tEnd, bool haveGhostZones)
{
    avtIVPSolver::Result result;

    // catch cases where the start position is outside the 
    // domain of field
    if(! field->IsInside(ivp->GetCurrentT(), ivp->GetCurrentY()))
    {
        return avtIVPSolver::OUTSIDE_DOMAIN;
    }
    
    while (1)
    {
        // record state for later restore, if needed
        avtIVPState state;
        ivp->GetState( state );

        // create new step to be filled in by ivp
        avtIVPStep* step = new avtIVPStep;

        try
        {
            result = ivp->Step(field, tEnd, step);
        }
        catch( avtIVPField::Undefined& u )
        {
            // integrator left the domain, retry with smaller step
            // if step size is below given minimum, give up

            // restore old state to before failed step
            ivp->PutState( state );

            double h = ivp->GetNextStepSize();
            h = h/2;
            
            if( fabs(h) < 1e-9 )
            {
                delete step;
                HandleGhostZones(ivp, haveGhostZones);
                return avtIVPSolver::OUTSIDE_DOMAIN;
            }
            
            ivp->SetNextStepSize( h );

            // retry step
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
            //cout<<"Step. dist= "<<ivp->GetCurrentDist()
            //    <<" T= "<<ivp->GetCurrentT()<<endl;

            if (ivp->GetCurrentT() < TMin())
            {
                IntersectWithPlane( _steps.front(), step );
                _steps.push_front( step );

                if( ivp->GetCurrentT() <= tEnd )
                {
                    break;
                }
            }
            else 
            {
                IntersectWithPlane( _steps.back(), step );
                _steps.push_back( step );

                if( ivp->GetCurrentT() >= tEnd )
                {
                    break;
                }
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
// ****************************************************************************

void
avtStreamline::HandleGhostZones(avtIVPSolver* ivp, bool haveGhostZones)
{
    if ( haveGhostZones )
        return;
    
    //debug1 << "Try to jump out into a ghostzone. Size = " << size() << endl;
    if ( size() >= 2 )
    {
        static const double eps = 1e-3;

        if ( ivp == _ivp_fwd )
        {
            //debug1<< "Forward SL\n";
            iterator s1 = end(), s0 = end();
            
            s0--;
            s0--;
            s1--;
            for ( int i = size()-2; i > 0; i-- )
            {
                avtVec p0 = (*s0)->front();
                avtVec p1 = (*s1)->front();
                
                // Compute a vector between the last two points.
                double vec[3] = {  
                                   p1.values()[0]-p0.values()[0], 
                                   p1.values()[1]-p0.values()[1], 
                                   p1.values()[2]-p0.values()[2] 
                                };
                double lenSq = vec[0]*vec[0] + vec[1]*vec[1] + vec[2]*vec[2];
            
                //debug1 << i<<": Tangent: " << vec[0] << " " << vec[1] 
                //       << " " << vec[2] << endl;
                
                
                // If the vector is zero, advance Y a small distance 
                // in the tangent direction.
                if (lenSq > 0.0)
                {
                    double len = sqrt(lenSq);
                    vec[0] /= len;
                    vec[1] /= len;
                    vec[2] /= len;
                    
                    //debug1 << i<<": Tangent: " << vec[0] << " " 
                    //       << vec[1] << " " << vec[2] << endl;
                    vec[0] *= eps;
                    vec[1] *= eps;
                    vec[2] *= eps;
                    
                    avtVec newY = ivp->GetCurrentY();
                    newY.values()[0] += vec[0];
                    newY.values()[1] += vec[1];
                    newY.values()[2] += vec[2];
                    
                    ivp->SetCurrentY( newY );
                    ivp->SetCurrentT( ivp->GetCurrentT() + eps );
                    break;
                }
                s0--;
                s1--;
            }
        }
        else
        {
            iterator s1 = begin(), s0 = begin();
            //debug1<< "Backward SL\n";
            s0++;
            int i = 0;
            while ( s0 != end() )
            {
                avtVec p0 = (*s0)->front();
                avtVec p1 = (*s1)->front();
                
                // Compute a vector between the last two points.
                double vec[3] = {
                                   p1.values()[0]-p0.values()[0], 
                                   p1.values()[1]-p0.values()[1], 
                                   p1.values()[2]-p0.values()[2]
                                };
                double lenSq = vec[0]*vec[0] + vec[1]*vec[1] + vec[2]*vec[2];
            
                //debug1 << i<<": Tangent: " << vec[0] << " " << vec[1] 
                //       << " " << vec[2] << endl;
                //debug1 << "len = " << lenSq << endl;
                
                
                // If the vector is zero, advance Y a small distance in 
                // the tangent direction.
                if ( lenSq > 0.0 )
                {
                    double len = sqrt(lenSq);
                    vec[0] /= len;
                    vec[1] /= len;
                    vec[2] /= len;
                    
                    //debug1 << i<<": Tangent: " << vec[0] << " " 
                    //       << vec[1] << " " << vec[2] << endl;
                    vec[0] *= eps;
                    vec[1] *= eps;
                    vec[2] *= eps;
                    ///debug1 << i<<": Tangent: " << vec[0] << " " 
                    //       << vec[1] << " " << vec[2] << endl;
                    
                    avtVec newY = ivp->GetCurrentY();
                    //debug1 << "Y: " << newY.values()[0] <<", " 
                    //       << newY.values()[1]<<", "
                    //       << newY.values()[2]<< " --> ";
                    newY.values()[0] += vec[0];
                    newY.values()[1] += vec[1];
                    newY.values()[2] += vec[2];
                    //debug1 << newY.values()[0] <<", " <<newY.values()[1]
                    //       <<", "<<newY.values()[2]<< endl;
                    
                    ivp->SetCurrentY( newY );
                    ivp->SetCurrentT( ivp->GetCurrentT() - eps );
                    break;
                }
                s0++;
                s1++;
                i++;
            }
        }
    }
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
        size_t sz;
        buff.io( mode, sz );
        for ( int i = 0; i < sz; i++ )
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


