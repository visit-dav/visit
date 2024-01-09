// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                              avtManagerWorkerICAlgorithm.h                  //
// ************************************************************************* //

#ifndef AVT_MANAGER_WORKER_IC_ALGORITHM_H
#define AVT_MANAGER_WORKER_IC_ALGORITHM_H

#if 0

#include "avtParICAlgorithm.h"

#include <vector>

#ifdef PARALLEL

class WorkerInfo;

// ****************************************************************************
// Class: avtManagerWorkerICAlgorithm
//
// Purpose:
//    Abstract base class for manager-worker algorithm.
//
// Programmer: Dave Pugmire
// Creation:   Mon Jan 26 13:25:58 EST 2009
//
// Modifications:
//
//   Dave Pugmire, Tue Mar 10 12:41:11 EDT 2009
//   Generalized domain to include domain/time. Pathine cleanup.
//
//   Dave Pugmire, Wed Mar 18 17:17:40 EDT 2009
//   Allow managers to share work loads.
//   
//   Dave Pugmire, Mon Mar 23 12:48:12 EDT 2009
//   Change how timings are reported/calculated.
//
//   Dave Pugmire, Fri Sep 25 15:35:32 EDT 2009
//   New counters.
//
//   Dave Pugmire, Thu Dec  2 11:21:06 EST 2010
//   Add CheckNextTimeStepNeeded.
//
//   Kathleen Biagas, Mon Sep 26 7:15:32 MST 2011
//   Change Sleep to NSleep to prevent collision on Windows.
//
// ****************************************************************************

class avtManagerWorkerICAlgorithm : public avtParICAlgorithm
{
  public:
    avtManagerWorkerICAlgorithm(avtPICSFilter *picsFilter,
                              int maxCount);
    virtual ~avtManagerWorkerICAlgorithm();
    
    virtual void              Initialize(std::vector<avtIntegralCurve *> &);
    virtual const char*       AlgoName() const {return "ManagerWorker";}
    
    virtual void              ResetIntegralCurvesForContinueExecute(int curTimeSlice=-1);
    virtual bool              CheckNextTimeStepNeeded(int curTimeSlice) { return true; }
    virtual void              AddIntegralCurves(std::vector<avtIntegralCurve*> &ics);

    static avtManagerWorkerICAlgorithm* Create(avtPICSFilter *picsFilter,
                                             int maxCount,
                                             int rank,
                                             int nProcs,
                                             int workGroupSz);

  protected:
    bool                      ExchangeICs(std::list<avtIntegralCurve *> &ics,
                                          std::vector<std::vector< avtIntegralCurve *> > &sendICs);
    int                        maxCnt, case4AThreshold;
    int                        NUM_DOMAINS;
    int                        DomToIdx(const BlockIDType &dom) const
    {
        int n = dom.domain/numTimeSteps + dom.timeStep;
        //debug5<<"numTS= "<<numTimeSteps<<endl;
        //debug5<<"dom "<<dom<<" ==> "<<n<<endl;
        return n;
    }

    BlockIDType                 IdxToDom(const int &num) const
    {
        BlockIDType d(num/numTimeSteps + num%numTimeSteps);
        //debug5<<"idx "<<num<<" ==> "<<d<<endl;
        return d;
    }
    
    int                        sleepMicroSec;
    void                       NSleep();
    
    static int                 MSG_STATUS, MSG_DONE, MSG_SEND_IC,
                               MSG_LOAD_DOMAIN, MSG_SEND_IC_HINT,
                               MSG_FORCE_SEND_STATUS, MSG_MANAGER_STATUS,
                               MSG_OFFLOAD_IC;

    //Statistics and coutners.
    int                       latencyTimer;
    std::vector<double>       latencyHistory;
    ICStatistics              SleepTime, LatencyTime, MaxLatencyTime;
    ICStatistics              SleepCnt, LatencySavingCnt, OffloadCnt;
    virtual void              CompileTimingStatistics();
    virtual void              CompileCounterStatistics();
    virtual void              CalculateExtraTime();
    virtual void              ReportTimings(ostream &os, bool totals);
    virtual void              ReportCounters(ostream &os, bool totals);
};


// ****************************************************************************
// Class: avtManagerICAlgorithm
//
// Purpose:
//    Manager portion of the manager-worker algorithm.
//
// Programmer: Dave Pugmire
// Creation:   Mon Jan 26 13:25:58 EST 2009
//
// Modifications:
//
//  Dave Pugmire, Sat Mar 28 10:04:01 EDT 2009
//  Report case counter information. Temporary fix for a sporadic bug where
//  it looks like messages are not being delivered to the manager. Manager will
//  detect when worker set is done and mark done. Modify logic in how managers
//  make decisions. Add domainOffloading (case 5).
//
//  Dave Pugmire, Thu Sep 24 13:52:59 EDT 2009
//  Change Execute to RunAlgorithm.
//
//  Hank Childs, Sat Jun  5 11:29:13 CDT 2010
//  Change return value of UpdateWorkerStatus.
//
// ****************************************************************************

class avtManagerICAlgorithm : public avtManagerWorkerICAlgorithm
{
  public:
    avtManagerICAlgorithm(avtPICSFilter *picsFilter,
                         int maxCount,
                         int workGrpSz,
                         std::vector<int> &workers,
                         int manager,
                         std::vector<int> &managers);
    virtual ~avtManagerICAlgorithm();

    virtual const char*       AlgoName() const {return "ManagerWorker";}
    virtual void              Initialize(std::vector<avtIntegralCurve *> &);

  protected:
    virtual void              RunAlgorithm();
    virtual void              ProcessMessages();
    virtual void              PostLoopProcessing();    
    std::vector<int>          status, prevStatus;
    virtual void              UpdateStatus();
    virtual void              SendStatus(bool forceSend=false);
    virtual void              ProcessWorkerUpdate(std::vector<int> &);
    virtual void              ProcessManagerUpdate(std::vector<int> &);
    virtual void              ProcessOffloadIC(std::vector<int> &);
    virtual void              ProcessNewIntegralCurves();
    virtual void              ManageWorkgroup();
    virtual void              ManageWorkers();
    virtual void              ManageManagers();
    virtual void              CompileTimingStatistics();
    virtual void              CompileCounterStatistics();
    virtual void              ReportCounters(ostream &os, bool totals);

    int                       workGroupActiveICs, workGroupSz;
    bool                      done, workerUpdate, managerUpdate;
    int                       case1Cnt, case2Cnt, case3ACnt, case3BCnt, case3CCnt, case4ACnt, case4BCnt,
                              case5ACnt, case5BCnt, case6Cnt;
    int                       manager;
    std::vector<WorkerInfo>    workerInfo, managerInfo;
    std::vector<int>          icDomCnts, domLoaded, slackers;
    std::list<avtIntegralCurve *> activeICs;

    void                      SendAllWorkersMsg(int msg);
    void                      SendAllWorkersMsg(std::vector<int> &msg);
    void                      SendWorkerMsg(int worker, std::vector<int> &msg);
    void                      FindSlackers(int oobFactor=-1,
                                           bool randomize= true,
                                           bool checkJustUpdated=false);
    void                      UpdateWorkerStatus(std::vector<int> &);
    void                      PrintStatus();

    void                      Case1(int &counter);
    void                      Case2(int &counter);
    void                      Case3(int overloadFactor,
                                    int NDomainFactor,
                                    int &counter );
    void                      Case4(int oobThreshold,
                                    int &counter);
    void                      Case5(int overworkThreshold,
                                    bool domainCheck,
                                    int &counter);
};


// ****************************************************************************
// Class: avtWorkerICAlgorithm
//
// Purpose:
//    Worker portion of the manager-worker algorithm.
//
// Programmer: Dave Pugmire
// Creation:   Mon Jan 26 13:25:58 EST 2009
//
// Modifications:
//
//   Dave Pugmire, Mon Feb 23 13:38:49 EST 2009
//   Add timeout counter for workers.
//   
//   Dave Pugmire, Wed Apr  1 11:21:05 EDT 2009
//   Add HandleLatencyTimer method.
//
//   Dave Pugmire, Thu Sep 24 13:52:59 EDT 2009
//   Change Execute to RunAlgorithm.
//
// ****************************************************************************

class avtWorkerICAlgorithm : public avtManagerWorkerICAlgorithm
{
  public:
    avtWorkerICAlgorithm(avtPICSFilter *picsFilter,
                        int maxCount,
                        int managerRank);
    virtual ~avtWorkerICAlgorithm();

    virtual void              Initialize(std::vector<avtIntegralCurve *> &);
    virtual void              SendStatus(bool forceSend=false);
    virtual void              UpdateStatus();

  protected:
    virtual void              RunAlgorithm();

    int                       manager, numTerminated, timeout;
    bool                      workToDo;
    std::vector<int>          status, prevStatus;
    std::list<avtIntegralCurve *> activeICs, oobICs;

    void                      ProcessMessages(bool &done, bool &newMsg);
    void                      HandleLatencyTimer(int activeICCnt,
                                                 bool checkMaxLatency=true);
};


// ****************************************************************************
// Class: WorkerInfo
//
// Purpose:
//    Class to keep track of worker information.
//
// Programmer: Dave Pugmire
// Creation:   Mon Jan 26 13:25:58 EST 2009
//
// Modifications:
// 
//   Dave Pugmire, Wed Mar 18 17:17:40 EDT 2009
//   Allow managers to share work loads.
//
// ****************************************************************************
class WorkerInfo
{
  public:
    WorkerInfo( int r, int nDomains );
    ~WorkerInfo() {}

    void AddIC(int icDomain, int domCache);
    void LoadDom( int icDomain );
    void RemoveIC( int dom );
    void Update( std::vector<int> &status, bool debug=false );
    void Reset() { justUpdated = false; }
    void Debug();

    bool justUpdated, initialized;
    int canGive, canAccept, icCount, icLoadedCount, icOOBCount, rank;
    int domLoadedCount;
    std::vector<int> domainCnt;
    std::vector<bool> domainLoaded;
    std::vector<int> domainHistory;
};

#endif

#endif


#endif
