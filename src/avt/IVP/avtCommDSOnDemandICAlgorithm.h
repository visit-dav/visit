// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                              avtCommDSOnDemandICAlgorithm.h                 //
// ************************************************************************* //

#ifndef AVT_COMM_DS_ON_DEMAND_IC_ALGORITHM_H
#define AVT_COMM_DS_ON_DEMAND_IC_ALGORITHM_H

#if 0

#ifdef PARALLEL
#include <avtParICAlgorithm.h>

class domainCacheEntry
{
  public:
    domainCacheEntry(BlockIDType _dom, vtkDataSet *_ds){ds=_ds; dom=_dom; refCnt=0;}
    domainCacheEntry(){ds=NULL; refCnt=0;}

    BlockIDType dom;
    int refCnt;
    vtkDataSet *ds;
};

// ****************************************************************************
// Class: avtCommDSOnDemandICAlgorithm
//
// Purpose: Communicate domains on demand algorithm.
//   
//
// Programmer:  Dave Pugmire
// Creation:    December  6, 2010
//
// Modifications:
//
//   Dave Pugmire, Fri Dec 17 12:15:04 EST 2010
//   Override PostRunAlgorithm as a no-op.
//
//   Dave Pugmire, Fri Jan 14 11:06:09 EST 2011
//   Added PostStepCallback() method, numDone as member data.
//
// ****************************************************************************

class avtCommDSOnDemandICAlgorithm : public avtParICAlgorithm
{
  public:
    avtCommDSOnDemandICAlgorithm(avtPICSFilter *picsFilter, int cacheSize);
    virtual ~avtCommDSOnDemandICAlgorithm();

    virtual const char*       AlgoName() const {return "CommDSOnDemand";}
    virtual void              Initialize(std::vector<avtIntegralCurve *> &);
    virtual void              ResetIntegralCurvesForContinueExecute(int curTimeSlice=-1);
    virtual bool              CheckNextTimeStepNeeded(int curTimeSlice);
    virtual void              AddIntegralCurves(std::vector<avtIntegralCurve*> &ics);
    virtual bool              PostStepCallback();

  protected:
    vtkDataSet               *GetDataset(const BlockIDType &dom);

    virtual void              RunAlgorithm();
    virtual void              PostRunAlgorithm() {}
    virtual void              SortIntegralCurves(std::list<avtIntegralCurve *> &);
    virtual void              HandleOOBIC(avtIntegralCurve *s);
    virtual bool              RequestDataset(BlockIDType &d);
    virtual bool              HandleMessages(bool checkMsgs=true,
                                             bool checkDSs=true,
                                             bool allowBlockAndWait=true);

    void                      AddRef(const BlockIDType &dom);
    void                      DelRef(const BlockIDType &dom);
    void                      CheckCacheVacancy(bool makeReq);

    void Debug();
    
    std::list<avtIntegralCurve *> activeICs, oobICs;
    std::set<int>             pendingDomRequests;
    std::map<int, int>        pendingDomReqTimers;

    //Communicated domain cache.
    virtual vtkDataSet       *GetDSFromDomainCache(const BlockIDType &dom);
    virtual void              AddDSToDomainCache(std::vector<DSCommData> &ds);

    int                       numDone;
    std::list<domainCacheEntry> domainCache;
    int domainCacheSizeLimit;

    ICStatistics              DSLatencyTime;
    virtual void              CompileTimingStatistics()
    {
        avtParICAlgorithm::CompileTimingStatistics();
        ComputeStatistic(DSLatencyTime);
    }

    virtual void              ReportTimings(ostream &os, bool totals)
    {
        avtParICAlgorithm::ReportTimings(os, totals);
        PrintTiming(os, "DSLatTime", DSLatencyTime, TotalTime, totals);
    }
};

#endif

#endif
#endif
