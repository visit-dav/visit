// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                              avtMasterSlaveICAlgorithm.h                  //
// ************************************************************************* //

#ifndef AVT_MASTER_SLAVE_IC_ALGORITHM_H
#define AVT_MASTER_SLAVE_IC_ALGORITHM_H

#if 0

#include "avtParICAlgorithm.h"

#include <vector>

#ifdef PARALLEL

class SlaveInfo;

// ****************************************************************************
// Class: avtMasterSlaveICAlgorithm
//
// Purpose:
//    Abstract base class for master-slave algorithm.
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
//   Allow masters to share work loads.
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

class avtMasterSlaveICAlgorithm : public avtParICAlgorithm
{
  public:
    avtMasterSlaveICAlgorithm(avtPICSFilter *picsFilter,
                              int maxCount);
    virtual ~avtMasterSlaveICAlgorithm();
    
    virtual void              Initialize(std::vector<avtIntegralCurve *> &);
    virtual const char*       AlgoName() const {return "MasterSlave";}
    
    virtual void              ResetIntegralCurvesForContinueExecute(int curTimeSlice=-1);
    virtual bool              CheckNextTimeStepNeeded(int curTimeSlice) { return true; }
    virtual void              AddIntegralCurves(std::vector<avtIntegralCurve*> &ics);

    static avtMasterSlaveICAlgorithm* Create(avtPICSFilter *picsFilter,
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
                               MSG_FORCE_SEND_STATUS, MSG_MASTER_STATUS,
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
// Class: avtMasterICAlgorithm
//
// Purpose:
//    Master portion of the master-slave algorithm.
//
// Programmer: Dave Pugmire
// Creation:   Mon Jan 26 13:25:58 EST 2009
//
// Modifications:
//
//  Dave Pugmire, Sat Mar 28 10:04:01 EDT 2009
//  Report case counter information. Temporary fix for a sporadic bug where
//  it looks like messages are not being delivered to the master. Master will
//  detect when slave set is done and mark done. Modify logic in how masters
//  make decisions. Add domainOffloading (case 5).
//
//  Dave Pugmire, Thu Sep 24 13:52:59 EDT 2009
//  Change Execute to RunAlgorithm.
//
//  Hank Childs, Sat Jun  5 11:29:13 CDT 2010
//  Change return value of UpdateSlaveStatus.
//
// ****************************************************************************

class avtMasterICAlgorithm : public avtMasterSlaveICAlgorithm
{
  public:
    avtMasterICAlgorithm(avtPICSFilter *picsFilter,
                         int maxCount,
                         int workGrpSz,
                         std::vector<int> &slaves,
                         int master,
                         std::vector<int> &masters);
    virtual ~avtMasterICAlgorithm();

    virtual const char*       AlgoName() const {return "MasterSlave";}
    virtual void              Initialize(std::vector<avtIntegralCurve *> &);

  protected:
    virtual void              RunAlgorithm();
    virtual void              ProcessMessages();
    virtual void              PostLoopProcessing();    
    std::vector<int>          status, prevStatus;
    virtual void              UpdateStatus();
    virtual void              SendStatus(bool forceSend=false);
    virtual void              ProcessSlaveUpdate(std::vector<int> &);
    virtual void              ProcessMasterUpdate(std::vector<int> &);
    virtual void              ProcessOffloadIC(std::vector<int> &);
    virtual void              ProcessNewIntegralCurves();
    virtual void              ManageWorkgroup();
    virtual void              ManageSlaves();
    virtual void              ManageMasters();
    virtual void              CompileTimingStatistics();
    virtual void              CompileCounterStatistics();
    virtual void              ReportCounters(ostream &os, bool totals);

    int                       workGroupActiveICs, workGroupSz;
    bool                      done, slaveUpdate, masterUpdate;
    int                       case1Cnt, case2Cnt, case3ACnt, case3BCnt, case3CCnt, case4ACnt, case4BCnt,
                              case5ACnt, case5BCnt, case6Cnt;
    int                       master;
    std::vector<SlaveInfo>    slaveInfo, masterInfo;
    std::vector<int>          icDomCnts, domLoaded, slackers;
    std::list<avtIntegralCurve *> activeICs;

    void                      SendAllSlavesMsg(int msg);
    void                      SendAllSlavesMsg(std::vector<int> &msg);
    void                      SendSlaveMsg(int slave, std::vector<int> &msg);
    void                      FindSlackers(int oobFactor=-1,
                                           bool randomize= true,
                                           bool checkJustUpdated=false);
    void                      UpdateSlaveStatus(std::vector<int> &);
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
// Class: avtSlaveICAlgorithm
//
// Purpose:
//    Slave portion of the master-slave algorithm.
//
// Programmer: Dave Pugmire
// Creation:   Mon Jan 26 13:25:58 EST 2009
//
// Modifications:
//
//   Dave Pugmire, Mon Feb 23 13:38:49 EST 2009
//   Add timeout counter for slaves.
//   
//   Dave Pugmire, Wed Apr  1 11:21:05 EDT 2009
//   Add HandleLatencyTimer method.
//
//   Dave Pugmire, Thu Sep 24 13:52:59 EDT 2009
//   Change Execute to RunAlgorithm.
//
// ****************************************************************************

class avtSlaveICAlgorithm : public avtMasterSlaveICAlgorithm
{
  public:
    avtSlaveICAlgorithm(avtPICSFilter *picsFilter,
                        int maxCount,
                        int masterRank);
    virtual ~avtSlaveICAlgorithm();

    virtual void              Initialize(std::vector<avtIntegralCurve *> &);
    virtual void              SendStatus(bool forceSend=false);
    virtual void              UpdateStatus();

  protected:
    virtual void              RunAlgorithm();

    int                       master, numTerminated, timeout;
    bool                      workToDo;
    std::vector<int>          status, prevStatus;
    std::list<avtIntegralCurve *> activeICs, oobICs;

    void                      ProcessMessages(bool &done, bool &newMsg);
    void                      HandleLatencyTimer(int activeICCnt,
                                                 bool checkMaxLatency=true);
};


// ****************************************************************************
// Class: SlaveInfo
//
// Purpose:
//    Class to keep track of slave information.
//
// Programmer: Dave Pugmire
// Creation:   Mon Jan 26 13:25:58 EST 2009
//
// Modifications:
// 
//   Dave Pugmire, Wed Mar 18 17:17:40 EDT 2009
//   Allow masters to share work loads.
//
// ****************************************************************************
class SlaveInfo
{
  public:
    SlaveInfo( int r, int nDomains );
    ~SlaveInfo() {}

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
