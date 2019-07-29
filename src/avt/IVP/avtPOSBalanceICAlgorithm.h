// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                              avtPOSBalanceICAlgorithm.h                 //
// ************************************************************************* //

#ifndef AVT_POS_BALANCE_IC_ALGORITHM_H
#define AVT_POS_BALANCE_IC_ALGORITHM_H

#if 0

#ifdef PARALLEL
#include "avtParICAlgorithm.h"


class avtPOSBalanceICAlgorithm : public avtParICAlgorithm
{
  public:
    avtPOSBalanceICAlgorithm(avtPICSFilter *picsFilter);
    virtual ~avtPOSBalanceICAlgorithm();
    virtual const char*       AlgoName() const {return "POS Balance";}

    static avtPOSBalanceICAlgorithm* Create(avtPICSFilter *picsFilter,
                                            int numToSend);

    virtual void              Initialize(std::vector<avtIntegralCurve *> &);
    virtual void              ResetIntegralCurvesForContinueExecute(int curTimeSlice = -1);
    virtual bool              CheckNextTimeStepNeeded(int curTimeSlice);

  protected:
    std::list<avtIntegralCurve *> activeICs, oobICs;
};

class avtPOSBalanceMasterICAlgorithm : public avtPOSBalanceICAlgorithm
{
  public:
    avtPOSBalanceMasterICAlgorithm(avtPICSFilter *picsFilter, int N);
    virtual ~avtPOSBalanceMasterICAlgorithm();
    virtual void              Initialize(std::vector<avtIntegralCurve *> &);
    virtual void              AddIntegralCurves(std::vector<avtIntegralCurve*> &sls);

  protected:
    virtual void              RunAlgorithm();
    
    std::list<avtIntegralCurve *> unassignedICs;
    int nToSend, numTerminated;
};


class avtPOSBalanceWorkerICAlgorithm : public avtPOSBalanceICAlgorithm
{
  public:
    avtPOSBalanceWorkerICAlgorithm(avtPICSFilter *picsFilter, int master);
    virtual ~avtPOSBalanceWorkerICAlgorithm();
    
    virtual void              Initialize(std::vector<avtIntegralCurve *> &);
    virtual void              AddIntegralCurves(std::vector<avtIntegralCurve*> &sls);
    
  protected:
    virtual void              RunAlgorithm();
    bool                      HandleMessages();
    int master;
};


#endif
#endif

#endif
