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
//                               avtStreamline.h                             //
// ************************************************************************* //

#ifndef AVT_STREAMLINE_H
#define AVT_STREAMLINE_H

#include <avtIVPSolver.h>
#include <MemStream.h>
#include <string>
#include <list>
#include <avtVector.h>

class vtkObject;

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
//    Dave Pugmire, Wed Dec  3 08:33:42 EST 2008
//    Added maxSteps argument to Advance() to optionally control how many
//    integration steps are taken.
//
//    Dave Pugmire, Mon Feb 23, 09:11:34 EST 2009
//    Reworked the termination code. Added a type enum and value. Made num steps
//    a termination criterion. Code cleanup: We no longer need fwd/bwd solvers.
//    Removed the plane intersection code.
//
//   Dave Pugmire, Mon Jun 8 2009, 11:44:01 EDT 2009
//   Removed the wantVorticity, extents and ghostzone flags. Extents and ghost
//   zones are handled by the vtkDataSet itself. The wantVorticity was replaced
//   with a scalarValueType which can be 'or'-d together to specify what to
//   compute.
//
//   Dave Pugmire, Tue Aug 11 10:25:45 EDT 2009
//   Add new termination criterion: Number of intersections with an object.
//
//   Dave Pugmire, Tue Aug 18 08:47:40 EDT 2009
//   Don't record intersection points, just count them.
//
//   Dave Pugmire, Thu Sep 24 13:52:59 EDT 2009
//   Option to serialize steps.
//
//   Dave Pugmire, Tue Dec  1 11:50:18 EST 2009
//   Switch from avtVec to avtVector.
//
//   Dave Pugmire, Tue Dec 29 14:37:53 EST 2009
//   Generalize the compute scalar variable.
//
//   Dave Pugmire, Tue Feb 23 09:42:25 EST 2010
//   Use a vector instead of a list for the integration steps.
//
// ****************************************************************************

class IVP_API avtStreamline
{
  public:
    enum ScalarValueType {NONE=0, SPEED=1, VORTICITY=2, SCALAR_VARIABLE=4};

    typedef std::vector<avtIVPStep*>::const_iterator iterator;
    avtStreamline(const avtIVPSolver* model, const double& t_start, 
                  const avtVector &p_start, int ID=-1);
    avtStreamline();
    ~avtStreamline();

    avtIVPSolver::Result Advance(const avtIVPField* field,
                                 avtIVPSolver::TerminateType termType,
                                 double end);

    void      SetScalarValueType(ScalarValueType t) {scalarValueType = t;}
    void      SetIntersectionObject(vtkObject *obj);
    
    // Min/Max T of integrated streamlines.
    double    TMin() const;
    double    TMax() const;

    avtVector PtStart() const { return _p0; }
    void      PtEnd(avtVector &end);
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
                        avtIVPSolver *solver,
                        bool serializeSteps=false);

    int       GetVariableIdx(const std::string &var) const;

  protected:
    avtStreamline( const avtStreamline& );
    avtStreamline& operator=( const avtStreamline& );
    
    avtIVPSolver::Result DoAdvance(avtIVPSolver* ivp,
                                   const avtIVPField* field,
                                   avtIVPSolver::TerminateType termType,
                                   double end);

    void      HandleGhostZones(bool forward, double *extents);
    void      HandleIntersections(avtIVPStep *step,
                                  avtIVPSolver::TerminateType termType,
                                  double end,
                                  avtIVPSolver::Result *result);
    bool      IntersectPlane(const avtVector &p0, const avtVector &p1);

  public:
    // Integration steps.
    std::vector<avtIVPStep*> _steps;
  protected:

    // Intersection points.
    bool intersectionsSet;
    int numIntersections;
    double     intersectPlaneEq[4];

    // Initial T and seed point.
    double _t0;
    avtVector _p0;
    
    ScalarValueType    scalarValueType;

    // Solver.
    avtIVPSolver*       _ivpSolver;

  public:
    //Bookeeping
    int id;

    std::vector<std::string> scalars;
};

#endif // AVT_STREAMLINE_H


