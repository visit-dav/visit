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
//                             avtIntegralCurve.h                            //
// ************************************************************************* //

#ifndef AVT_INTEGRAL_CURVE_H
#define AVT_INTEGRAL_CURVE_H

#include <avtIVPSolver.h>
#include <MemStream.h>
#include <string>
#include <vector>
#include <avtVector.h>

class vtkObject;

// ****************************************************************************
// Class: DomainType
//
// Purpose:
//    Encapsulate the a domain/timestep.
//    
//
// Programmer: Dave Pugmire
// Creation:   Tue Mar 10 12:41:11 EDT 2009
//
// Modifications:
//
//   Dave Pugmire, Mon May 11 12:41:51 EDT 2009
//   Fix operator< so that that std::map works.
//
// ****************************************************************************

class IVP_API DomainType
{
  public:
    DomainType() :domain(-1), timeStep(0) {}
    DomainType(const int &d) :domain(d), timeStep(0) {}
    DomainType(const int &d, const int &t) :domain(d), timeStep(t) {}
    ~DomainType() {}

    void operator=(const DomainType &dt)
    {
        domain=dt.domain;
        timeStep=dt.timeStep;
    }

    bool operator==(const DomainType &dt) const
    {
        return (domain == dt.domain &&
                timeStep == dt.timeStep);
    }
    bool operator<(const DomainType &dt) const
    {
        return (domain < dt.domain) ||
               (!(domain < dt.domain) && timeStep < dt.timeStep);
    }

    //Members
    int domain, timeStep;

    friend std::ostream& operator<<(std::ostream &out, const DomainType &d)
    {
        out<<"["<<d.domain<<", "<<d.timeStep<<"]";
        return out;
    }
};


// ****************************************************************************
//  Class: avtIntegralCurve
//
//  Purpose:
//      avtIntegralCurve is a straightforward implementation of integral curves,
//      based on avtIVPSolver.  Through this model instance, a user of 
//      avtIntegralCurve is able to select any IVP scheme to be used in the 
//      integration.
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
//   Dave Pugmire, Wed May 26 13:48:24 EDT 2010
//   New return enum.
//
//   Hank Childs, Thu Jun  3 10:44:46 PDT 2010
//   Remove TMin, PtStart, TStart, IsForward, IsBackward, and IsBothDir.
//   Rename TMax to GetCurrentTime, PtEnd to GetCurrentLocation.
//
//   Hank Childs, Fri Jun  4 15:45:39 CDT 2010
//   Combine this class with the contents of avtStreamlineWrapper.
//
//   Hank Childs, Fri Jun  4 21:30:18 CDT 2010
//   Separate out portions specific to Poincare and Streamline into
//   avtStateRecorderIntegralCurve.
//
// ****************************************************************************

class IVP_API avtStreamline
{
  public:
    enum Result
    {
        RESULT_TERMINATE,
        RESULT_POINT_OUTSIDE_DOMAIN,
        RESULT_EXIT_DOMAIN,
        RESULT_ERROR,
    };

    enum Status
    {
        STATUS_UNSET=-1,
        STATUS_TERMINATE,
        STATUS_OUTOFBOUNDS
    };

    enum SerializeFlags
    {
        SERIALIZE_STEPS = 1,
        SERIALIZE_INC_SEQ = 2,
    };

    avtStreamline(const avtIVPSolver* model, const double& t_start, 
                  const avtVector &p_start, int ID);
    avtStreamline();
    ~avtStreamline();

    avtStreamline::Result Advance(const avtIVPField* field,
                                  avtIVPSolver::TerminateType termType,
                                  double end);

    double    CurrentTime() const;
    void      CurrentLocation(avtVector &end);

    void      Debug() const;
    
    virtual void      Serialize(MemStream::Mode mode, MemStream &buff, 
                        avtIVPSolver *solver);

    // Could be static ... only virtual to get to the right method.
    // (Temporary solution until we build a communicator class.)
    virtual avtStreamline*
                      MergeIntegralCurveSequence(
                              std::vector<avtStreamline *> &v) {;};
    static bool IdSeqCompare(const avtStreamline *slA,
                             const avtStreamline *slB);
    static bool IdRevSeqCompare(const avtStreamline *slA,
                                const avtStreamline *slB);
    static bool DomainCompare(const avtStreamline *slA,
                              const avtStreamline *slB);

  protected:
    avtStreamline( const avtStreamline& );
    avtStreamline& operator=( const avtStreamline& );
    
    avtStreamline::Result DoAdvance(avtIVPSolver* ivp,
                                    const avtIVPField* field,
                                    avtIVPSolver::TerminateType termType,
                                    double end);
    virtual void AnalyzeStep(avtIVPStep *step,
                         const avtIVPField* field,
                         avtIVPSolver::TerminateType termType,
                         double end, avtIVPSolver::Result *result) = 0;

    void      HandleGhostZones(bool forward, double *extents);

  public:
    double termination;
    avtIVPSolver::TerminateType terminationType;
    bool terminated;

    // needed for ghost cells
    avtIVPStep lastStep;
    bool lastStepValid;

    // Helpers needed for figuring out which domain to use next
    std::vector<DomainType> seedPtDomainList;
    DomainType domain;
    Status status;

    unsigned long serializeFlags;

    long sequenceCnt;
    long long sortKey;

    long id;

  protected:

    // Solver.
    avtIVPSolver*       _ivpSolver;
    static const double minH;
};

inline std::ostream& operator<<( std::ostream& out, const avtStreamline::Result &res )
{
    switch (res)
    {
      case avtStreamline::RESULT_TERMINATE: out<<"TERMINATE"; break;
      case avtStreamline::RESULT_POINT_OUTSIDE_DOMAIN: out<<"POINTOUTSIDE_DOMAIN"; break;
      case avtStreamline::RESULT_EXIT_DOMAIN: out<<"EXIT_DOMAIN"; break;
      case avtStreamline::RESULT_ERROR: out<<"ERROR"; break;
      default:
        out<<"UNKNOWN_RESULT"; break;
    }
    return out;
}


#endif // AVT_STREAMLINE_H


