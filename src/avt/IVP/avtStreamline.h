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
//                               avtStreamline.h                             //
// ************************************************************************* //

#ifndef AVT_STREAMLINE_H
#define AVT_STREAMLINE_H

#include <avtIVPSolver.h>
#include <MemStream.h>
#include <list>

// ****************************************************************************
//  Class: avtStreamline
//
//  Purpose:
//      avtStreamline is a straightforward implementation of streamlines, based
//      on avtIVPSolver. It currently keeps two avtIVPSolver instances for 
//      forward and backward integration, cloned from a model instance passed 
//      to it. Through this model instance, a user of avtStreamline is able to 
//      select any IVP scheme to be used in the streamline integration.
//
//      A streamline is advanced through the advance() method, and 
//      avtStreamline keeps track of the current integration length in both 
//      forward and backward time as well as the avtIVPSteps corresponding to 
//      the streamline. Based the list of accumulated steps, a simple 
//      interpolation facility is provided that allows querying the 
//      streamline points and derivatives at arbitrary parameters, independent 
//      of the actual nature of the representation (be it a high-order 
//      polynomial or a simple polyline). In the future, avtStreamline 
//      could be augmented by methods that take care of typical tasks such as 
//      e.g. generating a piecewise linear representation for rendering. 
//      Furthermore, avtStreamline provides boundary detection.
//
//      It is not necessarily intended that other streamline implementation 
//      (such as the planned AMR streamlines) derive from avtStreamline, but 
//      this is certainly possible if enough functionality can be reused. A 
//      restructuring of avtStreamline is anticipated once more experience has 
//      been gathered.
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
//    Chagned how distanced based termination is computed.
//
// ****************************************************************************

class IVP_API avtStreamline
{
  public:
    typedef std::list<avtIVPStep*>::const_iterator iterator;

              avtStreamline(const avtIVPSolver* model, const double& t_start, 
                            const avtVec& p_start, int ID=-1);
              avtStreamline();
              ~avtStreamline();

    //Set, for instance, a plane for doing poincare plots ??
    void      UnsetIntersectPlane();
    void      SetIntersectPlane(const avtVec &pt, const avtVec &norm);
   
    avtIVPSolver::Result Advance(const avtIVPField* field,
                                 bool timeMode,
                                 double end,
                                 bool vorticity=false,
                                 bool haveGhostZones=false,
                                 double *extents=NULL);

    // Min/Max T of integrated streamlines.
    double    TMin() const;
    double    TMax() const;

    avtVec    PtStart() const { return _p0; }
    void      PtEnds( avtVec &pBwd, avtVec &pFwd ) const;
    double    TStart() const { return _t0; }

    bool      IsForward() const { return true; }
    bool      IsBackward() const { return false; }
    bool      IsBothDir() const { return false; }

    // Integration steps.
    size_t    size()  const;
    iterator  begin() const;
    iterator  end()   const;

    void      Debug() const;
    
    void      Serialize(MemStream::Mode mode, MemStream &buff, 
                        avtIVPSolver *solver);

  protected:
    avtStreamline( const avtStreamline& );
    avtStreamline& operator=( const avtStreamline& );
    
    avtIVPSolver::Result DoAdvance(avtIVPSolver* ivp,
                                   const avtIVPField* field,
                                   double tEnd,
                                   double dEnd,
                                   bool timeMode,
                                   bool haveGhostZones=false,
                                   double *extents=NULL);

    void      HandleGhostZones(avtIVPSolver* ivp, bool haveGhostZones,
                               double *extents);

    // Integration steps.
    //std::list<const avtIVPStep*> _steps;
    std::list<avtIVPStep*> _steps;

    // Initial T and seed point.
    double _t0;
    avtVec _p0;
    
    bool wantVorticity;

    // Solvers.
    avtIVPSolver*       _ivp_fwd;
    avtIVPSolver*       _ivp_bwd;

  public:
    bool intersectPlaneSet;
    double planeEq[4];
    avtVec intersectPlanePt, intersectPlaneNorm;
    std::vector<avtVec> intersectPts;
    void IntersectWithPlane( avtIVPStep *step0, avtIVPStep *step1 );

    //Bookeeping
    int id;
};

#endif // AVT_STREAMLINE_H


