// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                            avtLCSIC.h                                    //
// ************************************************************************* //

#ifndef AVT_LCS_IC_H
#define AVT_LCS_IC_H

#include <avtIntegralCurve.h>

// ****************************************************************************
//  Class: avtLCSIC
//
//  Purpose:
//      A derived type of avtIntegralCurve. This class 
//      decides how to terminate a LCS.
//
//  Programmer: Allen Sanderson
//  Creation:   August 14, 2013
//
//  Modifications:
//
// ****************************************************************************

class IVP_API avtLCSIC : public avtIntegralCurve
{
public:

    avtLCSIC(int maxSteps, bool doDistance, double maxDistance,
              bool doTime, double maxTime,
              const avtIVPSolver* model, 
              Direction dir, const double& t_start, 
              const avtVector &p_start, const avtVector &v_start,
              int ID);

    avtLCSIC();
    virtual ~avtLCSIC();

  protected:
    avtLCSIC( const avtLCSIC& );
    avtLCSIC& operator=( const avtLCSIC& );
    
  public:
    virtual void  Finalize();
    virtual void  Serialize(MemStream::Mode mode, MemStream &buff, 
                            avtIVPSolver *solver, SerializeFlags serializeFlags);

    virtual void  PrepareForSend(void)
                           { _serializeFlags = (SerializeFlags)(_serializeFlags | avtIntegralCurve::SERIALIZE_INC_SEQ); };
    virtual void      ResetAfterSend(void) { _serializeFlags = SERIALIZE_NO_OPT;}
    virtual bool  SameCurve(avtIntegralCurve *ic);

    virtual avtIntegralCurve* MergeIntegralCurveSequence(
                              std::vector<avtIntegralCurve *> &v);
    static bool IdSeqCompare(const avtIntegralCurve *slA,
                             const avtIntegralCurve *slB);
    static bool IdRevSeqCompare(const avtIntegralCurve *slA,
                                const avtIntegralCurve *slB);
    virtual bool LessThan(const avtIntegralCurve *ic) const;
    virtual void PrepareForFinalCommunication(void)
                     { _serializeFlags = avtIntegralCurve::SERIALIZE_STEPS; };

    virtual bool    UseFixedTerminationTime(void) { return doTime; };
    virtual double  FixedTerminationTime(void)    { return maxTime; };
    virtual bool    UseFixedTerminationDistance(void) { return doDistance; };
    virtual double  FixedTerminationDistance(void)    { return maxDistance; };

    virtual bool   CheckForTermination(avtIVPStep& step, avtIVPField *);
    virtual bool   TerminatedBecauseOfMaxSteps(void) 
                            { return terminatedBecauseOfMaxSteps; };
    virtual void   SetMaxSteps( unsigned int ms ) { maxSteps = ms; };
    virtual int    GetMaxSteps() { return maxSteps; };

    virtual int    GetNumSteps()  { return numSteps; }
    virtual double GetTime()      { return time; }
    virtual double GetArcLength() { return arcLength; }
    virtual double GetDistance()  { return distance; }
    virtual double GetSummation0() { return summation0; }
    virtual double GetSummation1() { return summation1; }

    avtVector GetStartPoint() { return p_start; }
    avtVector GetEndPoint() { return p_end; }

  protected:
    virtual void   AnalyzeStep( avtIVPStep &step,
                                avtIVPField *field,
                                bool firstStep=false);

  protected:
    SerializeFlags   _serializeFlags;
    long             sequenceCnt;

    bool             doDistance;
    double           maxDistance;

    bool             doTime;
    double           maxTime;

    unsigned int     maxSteps;
    unsigned int     numSteps;
    bool             terminatedBecauseOfMaxSteps;

    double           time;
    double           arcLength;
    double           distance;
    double           summation0;
    double           summation1;

    avtVector p_start, p_end;
};

#endif
