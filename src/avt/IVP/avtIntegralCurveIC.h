// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                            avtIntegralCurveIC.h                              //
// ************************************************************************* //

#ifndef AVT_INTEGRAL_CURVE_IC_H
#define AVT_INTEGRAL_CURVE_IC_H

#include <avtStateRecorderIntegralCurve.h>

// ****************************************************************************
//  Class: avtIntegralCurveIC
//
//  Purpose:
//      A derived type of avtStateRecorderIntegralCurve.  This class 
//      decides how to terminate a integral curve.
//
//  Programmer: Hank Childs
//  Creation:   October 4, 2010
//
//  Modifications:
//
//    Hank Childs, Sun Dec  5 10:18:13 PST 2010
//    Add avtIVPField argument to CheckForTermination.  Also add data members
//    to help determine if a warning should be issued.
//
//   David Camp, Wed Mar  7 10:43:07 PST 2012
//   Added a Serialize flag to the arguments. This is to support the restore
//   ICs code.
//
// ****************************************************************************

class IVP_API avtIntegralCurveIC : public avtStateRecorderIntegralCurve
{
public:
    avtIntegralCurveIC(int maxSteps, bool doDistance, double maxDistance,
                    bool doTime, double maxTime,
                    unsigned int mask, const avtIVPSolver* model, 
                    Direction dir, const double& t_start, 
                    const avtVector &p_start, const avtVector &v_start,
                    int ID);

    avtIntegralCurveIC();
    virtual ~avtIntegralCurveIC();

    virtual void    Serialize(MemStream::Mode mode,
                              MemStream &buff, 
                              avtIVPSolver *solver,
                              SerializeFlags serializeFlags);

    virtual void    MergeIntegralCurve(avtIntegralCurve *);

    virtual bool    UseFixedTerminationTime(void) { return doTime; };
    virtual double  FixedTerminationTime(void)    { return maxTime; };
    virtual bool    UseFixedTerminationDistance(void) { return doDistance; };
    virtual double  FixedTerminationDistance(void)    { return maxDistance; };

    virtual void    SetMaxSteps( int ms ) { maxSteps = ms; }
    virtual int     GetNumSteps() { return numSteps; }
    virtual bool    TerminatedBecauseOfMaxSteps(void)
                                 { return terminatedBecauseOfMaxSteps; };

  protected:
    avtIntegralCurveIC( const avtIntegralCurveIC& );
    avtIntegralCurveIC& operator=( const avtIntegralCurveIC& );
    
    virtual bool     CheckForTermination(avtIVPStep& step, avtIVPField *);

  protected:
    bool             doTime;
    double           maxTime;

    bool             doDistance;
    double           maxDistance;

    unsigned int     maxSteps;
    unsigned int     numSteps;
    bool             terminatedBecauseOfMaxSteps;
};

#endif 
