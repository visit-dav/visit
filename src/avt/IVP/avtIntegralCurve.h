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
//                             avtIntegralCurve.h                            //
// ************************************************************************* //

#ifndef AVT_INTEGRAL_CURVE_H
#define AVT_INTEGRAL_CURVE_H

#include <avtIVPSolver.h>
#include <MemStream.h>
#include <string>
#include <vector>
#include <avtVector.h>

//IC state tracking support.
//#define USE_IC_STATE_TRACKING

// ****************************************************************************
// Class: BlockIDType
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

class IVP_API BlockIDType
{
  public:
    BlockIDType() :domain(-1), timeStep(0) {}
    BlockIDType(const int &d) :domain(d), timeStep(0) {}
    BlockIDType(const int &d, const int &t) :domain(d), timeStep(t) {}
    ~BlockIDType() {}

    void operator=(const BlockIDType &dt)
    {
        domain=dt.domain;
        timeStep=dt.timeStep;
    }

    bool operator==(const BlockIDType &dt) const
    {
        return (domain == dt.domain &&
                timeStep == dt.timeStep);
    }
    bool operator<(const BlockIDType &dt) const
    {
        return (domain < dt.domain) ||
               ((domain == dt.domain) && timeStep < dt.timeStep);
    }

    //Members
    int domain, timeStep;

    friend std::ostream& operator<<(std::ostream &out, const BlockIDType &d)
    {
        out<<"["<<d.domain<<", "<<d.timeStep<<"]";
        return out;
    }
};

//****************************************************************************
// Class:  ICStatus
//
// Purpose:
//   Describes the status of an integral curve.
//
// 
// Programmer:  Dave Pugmire
// Creation:    April 15, 2013
//
// Modifications:
//
//****************************************************************************

class IVP_API ICStatus
{
 public:
    ICStatus() {Clear(); SetOK();}
    ICStatus(const ICStatus &s) {status=s.status;}
    ~ICStatus() {Clear();}

    //Get
    bool OK() const {return CheckBit(STATUS_OK);}
    bool Error() const {return !OK();}

    bool TerminatedOnly() const // i.e. terminated only due the maxSteps
    {
      if( Error() ||
          EncounteredSpatialBoundary() || EncounteredTemporalBoundary() ||
          ExitedSpatialBoundary() || ExitedTemporalBoundary() ||
          NumericalError() || BadStepError() )
        return false;
      else if( OK() && TerminationMet() )
        return true;
      else
        return false;
    }
    bool Terminated() const
    {
        return Error() || TerminationMet() ||
            ExitedSpatialBoundary()||ExitedTemporalBoundary();
    }
    bool Integrateable() const
    {
        return !Terminated() && !EncounteredSpatialBoundary() && !EncounteredTemporalBoundary();
    }
    bool OutsideBoundary() const {return EncounteredSpatialBoundary()||EncounteredTemporalBoundary();}
    bool TerminationMet() const {return CheckBit(TERMINATION_MET);}
    bool EncounteredSpatialBoundary() const {return CheckBit(ENCOUNTERED_SPATIAL_BOUNDARY);}
    bool EncounteredTemporalBoundary() const {return CheckBit(ENCOUNTERED_TEMPORAL_BOUNDARY);}
    bool ExitedSpatialBoundary() const {return CheckBit(EXITED_SPATIAL_BOUNDARY);}
    bool ExitedTemporalBoundary() const {return CheckBit(EXITED_TEMPORAL_BOUNDARY);}
    bool NumericalError() const {return CheckBit(NUMERICAL_ERROR);}
    bool BadStepError() const {return CheckBit(BAD_STEP_ERROR);}

    //Set
    void Clear()    {status = 0;}
    void SetOK()    {SetBit(STATUS_OK);}
    void SetError() {ClearBit(STATUS_OK);}
    void SetInsideBlock() {SetBit(INSIDE_BLOCK);}
    void SetTerminationMet() {SetBit(TERMINATION_MET);}
    void SetAtSpatialBoundary() {SetBit(ENCOUNTERED_SPATIAL_BOUNDARY);}
    void SetAtTemporalBoundary() {SetBit(ENCOUNTERED_TEMPORAL_BOUNDARY);}
    void SetExitSpatialBoundary() {SetBit(EXITED_SPATIAL_BOUNDARY);}
    void SetExitTemporalBoundary() {SetBit(EXITED_TEMPORAL_BOUNDARY);}
    void SetNumericalError() {SetError(); SetBit(NUMERICAL_ERROR);}
    void SetBadStepError() {SetError(); SetBit(BAD_STEP_ERROR);}

    //Clear
    void ClearInsideBlock() {ClearBit(INSIDE_BLOCK);}
    void ClearTerminationMet() {ClearBit(TERMINATION_MET);}
    void ClearAtSpatialBoundary() {ClearBit(ENCOUNTERED_SPATIAL_BOUNDARY);}
    void ClearExitSpatialBoundary() {ClearBit(EXITED_SPATIAL_BOUNDARY);}
    void ClearSpatialBoundary(){ ClearAtSpatialBoundary(); ClearExitSpatialBoundary();}
    void ClearAtTemporalBoundary() {ClearBit(ENCOUNTERED_TEMPORAL_BOUNDARY);}
    void ClearExitTemporalBoundary() {ClearBit(EXITED_TEMPORAL_BOUNDARY);}
    void ClearTemporalBoundary(){ ClearAtTemporalBoundary(); ClearExitTemporalBoundary();}
    void ClearNumericalError() {ClearBit(NUMERICAL_ERROR);}
    void ClearBadStepError() {ClearBit(BAD_STEP_ERROR);}

    unsigned long GetStatus() const {return status;};

 private:
    //bit assignments:
    //0:   OK
    //1:   inside Block
    //2:   termination met
    //3,4: at spatial/temporal boundary
    //5,6: exit spatial/temporal
    //7:   numerical error
    //8:   bad step error
    unsigned long status;

    enum ICStatusBits
    {
        STATUS_OK                     = 0x0001,
        INSIDE_BLOCK                  = 0x0002,
        TERMINATION_MET               = 0x0004,
        ENCOUNTERED_SPATIAL_BOUNDARY  = 0x0008,
        ENCOUNTERED_TEMPORAL_BOUNDARY = 0x0010,
        EXITED_SPATIAL_BOUNDARY       = 0x0020,
        EXITED_TEMPORAL_BOUNDARY      = 0x0040,
        NUMERICAL_ERROR               = 0x0080,
        BAD_STEP_ERROR                = 0x0100
    };

    void SetBit(const ICStatusBits &b) {status |= b;}
    void ClearBit(const ICStatusBits &b) {status &= ~b;}
    bool CheckBit(const ICStatusBits &b) const {return status & b;}
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
//   Hank Childs, Tue Jun  8 09:30:45 CDT 2010
//   Put sequence tracking code into avtStateRecorderIntegralCurve.
//
//   Hank Childs, Mon Oct  4 15:03:43 PDT 2010
//   Remove termination code.  It now goes in derived types.
//
//   Dave Pugmire, Fri Nov  5 15:34:49 EDT 2010
//   Add counter to handle communication of ICs
//
//   Hank Childs, Sun Dec  5 11:43:46 PST 2010
//   Added data member for tracking when we encounter numerical problems.
//
//   Dave Pugmire, Fri Feb 18 14:52:18 EST 2011
//   Replaced minH with minHFactor for use when integrating upto a domain boundary.
//
//   Hank Childs, Tue Dec  6 16:23:47 PST 2011
//   Add virtual methods LessThan (for sorting) and 
//   PrepareForFinalCommunication.
//
//   David Camp, Wed Mar  7 10:43:07 PST 2012
//   Added a Serialize flag to the arguments. This is to support the restore
//   ICs code.
//
//   Dave Pugmire, Mon Dec 15 11:00:23 EST 2014
//   Return number of steps taken.
//
// ****************************************************************************

class IVP_API avtIntegralCurve
{
  public:

    enum Direction
    {
        DIRECTION_FORWARD  = 0,
        DIRECTION_BACKWARD = 1
    };

    enum SerializeFlags
    {
        SERIALIZE_ALL     = -1,
        SERIALIZE_NO_OPT  = 0,
        SERIALIZE_STEPS   = 1,
        SERIALIZE_INC_SEQ = 2
    };

    avtIntegralCurve(const avtIVPSolver* model,
                     Direction dir,
                     const double& t_start,
                     const avtVector &p_start,
                     const avtVector &v_start,
                     long ID);

    avtIntegralCurve();
    virtual ~avtIntegralCurve();

    int Advance(avtIVPField* field);

    double    CurrentTime()     const {return ivp->GetCurrentT();}
    avtVector CurrentLocation() const {return ivp->GetCurrentY();}

    virtual void      Serialize(MemStream::Mode mode, MemStream &buff, 
                                avtIVPSolver *solver, SerializeFlags serializeFlags);

    virtual void      PrepareForSend() {}
    virtual void      ResetAfterSend() {}

    virtual bool      SameCurve(avtIntegralCurve *ic) {return id==ic->id;}

    static bool       DomainCompare(const avtIntegralCurve *slA,
                                    const avtIntegralCurve *slB);

    bool              EncounteredNumericalProblems() {return status.NumericalError();}

    virtual avtIntegralCurve* MergeIntegralCurveSequence(std::vector<avtIntegralCurve *> &v) = 0;
    virtual void      PrepareForFinalCommunication() {}

    // This is used for sorting, particularly for parallel communication
    virtual bool LessThan(const avtIntegralCurve *ic) const;

    ICStatus status;
    Direction direction;

    // Helpers needed for figuring out which domain to use next
    std::list<BlockIDType> blockList;
    long long sortKey;

    long id;
    int counter, originatingRank;

  protected:
    avtIntegralCurve(const avtIntegralCurve&);
    avtIntegralCurve& operator=(const avtIntegralCurve&);
    
    virtual void AnalyzeStep(avtIVPStep &step,
                             avtIVPField *field) = 0;
    virtual bool    UseFixedTerminationTime() {return false;}
    virtual double  FixedTerminationTime()    {return 0;}

    avtIVPSolver*       ivp;
    static const double minHFactor;

 public:

    //IC state tracking support.
#ifdef USE_IC_STATE_TRACKING
    std::ofstream trk;
    
    void InitTrk()
    {
        char tmp[64];
        sprintf(tmp, "IC_%d.txt", (int)id);
        trk.open(tmp, ofstream::out);
        trk<<"Init: ID= "<<id<<endl;
    }
#endif
    
};


inline std::ostream& operator<<(std::ostream& out, 
                                avtIntegralCurve::Direction dir)
{
    switch( dir )
    {
    case avtIntegralCurve::DIRECTION_FORWARD: 
        return out << "FORWARD";
    case avtIntegralCurve::DIRECTION_BACKWARD:
        return out << "BACKWARD";
    default:
        return out << "UNKNOWN";
    }
}

// ostream operators for avtICStatus
inline std::ostream& operator<<(std::ostream& out, 
                                ICStatus status)
{
    if (status.OK())
        out<<"OK: {";
    else
        out<<"ERROR: {";
    if (status.TerminationMet())
        out<<"Term ";
    if (status.EncounteredSpatialBoundary())
        out<<"AtSpatialBoundary ";
    if (status.EncounteredTemporalBoundary())
        out<<"AtTemporalBoundary ";
    if (status.ExitedSpatialBoundary())
        out<<"ExitSpatialBoundary ";
    if (status.ExitedTemporalBoundary())
        out<<"ExitTemporalBoundary ";
    if (status.NumericalError())
        out<<"NumericalError ";
    if (status.BadStepError())
        out<<"BadStepError ";
    
    out<<"}";
    return out;
}

#endif //  AVT_INTEGRAL_CURVE_H
