// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                              avtManagerWorkerICAlgorithm.C                  //
// ************************************************************************* //

#if 0

#include "avtManagerWorkerICAlgorithm.h"
#include <TimingsManager.h>
#include <iostream>
#include <iomanip>
#include <VisItStreamUtil.h>

using namespace std;

#ifdef PARALLEL
static int MAX_DOMAIN_PRINT = 30;
static bool MANAGER_BALANCING = false;
static bool WORKER_AUTO_LOAD_DOMS = false;
static int LATENCY_SEND_CNT = 2;

int avtManagerWorkerICAlgorithm::MSG_STATUS = 420003;
int avtManagerWorkerICAlgorithm::MSG_DONE = 420004;
int avtManagerWorkerICAlgorithm::MSG_SEND_IC = 420005;
int avtManagerWorkerICAlgorithm::MSG_LOAD_DOMAIN = 420006;
int avtManagerWorkerICAlgorithm::MSG_SEND_IC_HINT = 420007;
int avtManagerWorkerICAlgorithm::MSG_FORCE_SEND_STATUS = 420008;
int avtManagerWorkerICAlgorithm::MSG_MANAGER_STATUS = 420009;
int avtManagerWorkerICAlgorithm::MSG_OFFLOAD_IC = 420010;

// ****************************************************************************
//  Method: avtManagerWorkerICAlgorithm::Create
//
//  Purpose:
//      Static method to create and conifigure classes.
//
//  Programmer: Dave Pugmire
//  Creation:   January 27, 2009
//
//  Modifications:
//
//  Dave Pugmire, Wed Mar 18 17:17:40 EDT 2009
//  Allow managers to share work loads.
//
//  Dave Pugmire, Wed Mar 25 10:04:29 EDT 2009
//  Add flag that enables manager balancing.
//
// ****************************************************************************

avtManagerWorkerICAlgorithm*
avtManagerWorkerICAlgorithm::Create(avtPICSFilter *picsFilter,
                                  int maxCount,
                                  int rank,
                                  int nProcs,
                                  int workGroupSz)
{
    debug1<<"avtManagerWorkerICAlgorithm::Create\n";
    
    avtManagerWorkerICAlgorithm *algo = NULL;
    int numManagers = 1;
    int extra = 0;
    
    if (nProcs < workGroupSz)
        workGroupSz = nProcs;
    else
    {
        numManagers = nProcs/workGroupSz;
        extra = nProcs%workGroupSz;
    }
    
    int extraPerManager = extra / numManagers;
    int extraTill = extra % numManagers;

    debug1<<"numManagers= "<<numManagers<<endl;

    int nextWorker = numManagers;
    vector<vector<int> > managerList(numManagers);
    for (int m = 0; m < numManagers; m++)
    {
        managerList[m].resize(2);
        int N = workGroupSz-1 + extraPerManager;
        if (m < extraTill)
            N++;

        managerList[m][0] = nextWorker;
        managerList[m][1] = (nextWorker+N-1);
        debug1<<"Manager: "<<m<<" ["<<managerList[m][0]<<" "<<managerList[m][1]<<"]"<<endl;
        nextWorker += N;
    }

    //I am a manager.
    if (rank < numManagers)
    {
        vector<int> workers;
        for (int i = managerList[rank][0]; i <= managerList[rank][1]; i++)
            workers.push_back(i);
        
        vector<int> managers;
        int manager = -1;

        if (MANAGER_BALANCING)
        {
            if (rank == 0)
            {
                manager = -1;
                if (numManagers > 1)
                    for (int i = 0; i < numManagers; i++)
                        managers.push_back(i);
            }
            else
                manager = 0;
        }
        
        algo = new avtManagerICAlgorithm(picsFilter, maxCount, workGroupSz, workers, manager, managers);

        debug1<<"I am a manager. My workers are: "<<workers<<endl;
        debug1<<"My managerManager is "<<manager<<". Managers= "<<managers<<endl;
    }
    
    // I'm a worker. Look for my manager.
    else
    {
        for (int m = 0; m < numManagers; m++)
            if (rank >= managerList[m][0] && rank <= managerList[m][1])
            {
                debug1<<"I am a worker. My manager is "<<m<<endl;
                algo = new avtWorkerICAlgorithm(picsFilter, maxCount, m);
                break;
            }
    }

    if (algo == NULL)
        EXCEPTION0(ImproperUseException);
    
    return algo;
}


// ****************************************************************************
//  Method: avtManagerWorkerICAlgorithm::avtManagerWorkerICAlgorithm
//
//  Purpose:
//      avtManagerWorkerICAlgorithm constructor.
//
//  Programmer: Dave Pugmire
//  Creation:   January 27, 2009
//
//  Modifications:
//
//   Dave Pugmire, Tue Mar 10 12:41:11 EDT 2009
//   Generalized domain to include domain/time. Pathine cleanup.
//   
//  Dave Pugmire, Mon Mar 23 12:48:12 EDT 2009
//  Change how timings are reported/calculated.
//
//   Dave Pugmire, Wed Apr  1 11:21:05 EDT 2009
//   Message size and number of receives put in Initialize().
//  
//
//   Dave Pugmire, Fri Sep 25 15:35:32 EDT 2009
//   Initialize offloadCounter.
//  
// ****************************************************************************

avtManagerWorkerICAlgorithm::avtManagerWorkerICAlgorithm(avtPICSFilter *picsFilter,
                                                     int maxCount)
    : avtParICAlgorithm(picsFilter),
      SleepTime("sleepT"), LatencyTime("latT"), MaxLatencyTime("maxLatT"), SleepCnt("sleepC"),
      LatencySavingCnt("latSaveCnt"), OffloadCnt("offloadCnt")
{
    NUM_DOMAINS = numDomains * numTimeSteps;
    maxCnt = maxCount;
    case4AThreshold = 10*maxCnt;
    sleepMicroSec = 1;
    latencyTimer = -1;
}

// ****************************************************************************
//  Method: avtManagerWorkerICAlgorithm::~avtManagerWorkerICAlgorithm
//
//  Purpose:
//      avtManagerWorkerICAlgorithm destructor
//
//  Programmer: Dave Pugmire
//  Creation:   January 27, 2009
//
// ****************************************************************************

avtManagerWorkerICAlgorithm::~avtManagerWorkerICAlgorithm()
{
}

// ****************************************************************************
//  Method: avtManagerWorkerICAlgorithm::Initialize
//
//  Purpose:
//      Initializization.
//
//  Programmer: Dave Pugmire
//  Creation:   January 27, 2009
//
//  Modifications:
//
//   Dave Pugmire, Wed Apr  1 11:21:05 EDT 2009
//   Message size and number of receives put in Initialize().
//
//   Hank Childs, Mon Jun  7 14:57:13 CDT 2010
//   Reflect change in method name to InitializeBuffers.
//
//   Dave Pugmire, Fri Sep 10 14:03:45 EDT 2010
//   Send in number of recvs for msgs and ics.
//
// ****************************************************************************

void
avtManagerWorkerICAlgorithm::Initialize(std::vector<avtIntegralCurve *> &seedPts)
{
    int numRecvs = nProcs-1;
    if (numRecvs > 64)
        numRecvs = 64;
    
    avtParICAlgorithm::InitializeBuffers(seedPts, 2+NUM_DOMAINS, numRecvs, numRecvs);
}

void
avtManagerWorkerICAlgorithm::ResetIntegralCurvesForContinueExecute(int curTimeSlice)
{
    EXCEPTION0(ImproperUseException);
}

void
avtManagerWorkerICAlgorithm::AddIntegralCurves(std::vector<avtIntegralCurve*> &ics)
{
    EXCEPTION0(ImproperUseException);
}

// ****************************************************************************
// Method:  avtManagerWorkerICAlgorithm::ExchangeICs
//
// Purpose:
//   
// Programmer:  Dave Pugmire
// Creation:    September 10, 2010
//
// ****************************************************************************

bool
avtManagerWorkerICAlgorithm::ExchangeICs(list<avtIntegralCurve *> &ics,
                                       vector<vector< avtIntegralCurve *> > &sendICs)
{
    bool newIntegralCurves = false;
    
    // Do the IC sends.
    for (int i = 0; i < nProcs; i++)
    { 
        vector<avtIntegralCurve *> &ic = sendICs[i];
        if (i != rank)
            SendICs(i, ic);
        else // Pass them to myself....
        {
            for (int j = 0; j < ic.size(); j++)
                ics.push_back(ic[j]);
        }
    }

    // See if there are any recieves....
    int numNewICs = RecvICs(ics);
    newIntegralCurves = (numNewICs > 0);
    return newIntegralCurves;
}

// ****************************************************************************
//  Method: avtManagerWorkerICAlgorithm::NSleep
//
//  Purpose:
//      Sleep for a spell
//
//  Programmer: Dave Pugmire
//  Creation:   January 27, 2009
//
//  Modifications:
//    Kathleen Biagas, Mon Sep 26 07:15:32 MST 2011
//    Changed name to NSleep to prevent collision on Windows.
//
// ****************************************************************************

void
avtManagerWorkerICAlgorithm::NSleep()
{
    if (sleepMicroSec > 0)
    {
        //debug1<<"Sleep for "<<sleepMicroSec<<" microSec\n";
        int sleepTimer = visitTimer->StartTimer();
#ifdef _WIN32
        Sleep(0);
#else
        struct timespec ts = {0, 0};
        nanosleep(&ts, 0);
#endif
        SleepTime.value += visitTimer->StopTimer(sleepTimer, "SleepTimer");
        SleepCnt.value++;
    }
}

// ****************************************************************************
//  Method: avtManagerWorkerICAlgorithm::CompileTimingStatistics
//
//  Purpose:
//      Calculate statistics.
//
//  Programmer: Dave Pugmire
//  Creation:   January 27, 2009
//  
//  Dave Pugmire, Mon Mar 23 12:48:12 EDT 2009
//  Change how timings are reported/calculated.
//  
// ****************************************************************************

void
avtManagerWorkerICAlgorithm::CompileTimingStatistics()
{
    avtParICAlgorithm::CompileTimingStatistics();

    ComputeStatistic(SleepTime);
    ComputeStatistic(LatencyTime);
    ComputeStatistic(MaxLatencyTime);
}

// ****************************************************************************
//  Method: avtManagerWorkerICAlgorithm::CompileCounterStatistics
//
//  Purpose:
//      Calculate statistics.
//
//  Programmer: Dave Pugmire
//  Creation:   January 27, 2009
//
// ****************************************************************************

void
avtManagerWorkerICAlgorithm::CompileCounterStatistics()
{
    avtParICAlgorithm::CompileCounterStatistics();
    ComputeStatistic(SleepCnt);
    ComputeStatistic(LatencySavingCnt);
}

// ****************************************************************************
//  Method: avtManagerWorkerICAlgorithm::CalculateExtraTime
//
//  Purpose:
//      Calculate ExtraTime.
//
//  Programmer: Dave Pugmire
//  Creation:   January 27, 2009
//  
//  Dave Pugmire, Mon Mar 23 12:48:12 EDT 2009
//  Change how timings are reported/calculated.
//
// ****************************************************************************

void
avtManagerWorkerICAlgorithm::CalculateExtraTime()
{
    avtParICAlgorithm::CalculateExtraTime();
    
    if (SleepTime.value > 0.0)
        ExtraTime.value -= SleepTime.value;
}

// ****************************************************************************
//  Method: avtManagerWorkerICAlgorithm::ReportTimings
//
//  Purpose:
//      Report timings.
//
//  Programmer: Dave Pugmire
//  Creation:   January 27, 2009
//
// ****************************************************************************

void
avtManagerWorkerICAlgorithm::ReportTimings(ostream &os, bool totals)
{
    avtParICAlgorithm::ReportTimings(os, totals);
    
    PrintTiming(os, "SleepTime", SleepTime, TotalTime, totals);
    PrintTiming(os, "LatencyTime", LatencyTime, TotalTime, totals);
    PrintTiming(os, "MaxLatTime", MaxLatencyTime, TotalTime, totals);
}

// ****************************************************************************
//  Method: avtManagerWorkerICAlgorithm::ReportCounters
//
//  Purpose:
//      Report counters.
//
//  Programmer: Dave Pugmire
//  Creation:   January 27, 2009
//  
//  Dave Pugmire, Mon Mar 23 12:48:12 EDT 2009
//  Change how timings are reported/calculated.
//
//   Dave Pugmire, Fri Sep 25 15:35:32 EDT 2009
//   Print new offload counter.
//
// ****************************************************************************

void
avtManagerWorkerICAlgorithm::ReportCounters(ostream &os, bool totals)
{
    avtParICAlgorithm::ReportCounters(os, totals);
    PrintCounter(os, "SleepCnt", SleepCnt, totals);
    PrintCounter(os, "LSaveCnt", LatencySavingCnt, totals);
    PrintCounter(os, "OffldCnt", OffloadCnt, totals);
    if (!totals)
        os<<"LatencyHistory: "<<latencyHistory<<endl;
}


// ****************************************************************************
//  Method: avtManagerICAlgorithm::avtManagerICAlgorithm
//
//  Purpose:
//      avtManagerICAlgorithm constructor.
//
//  Programmer: Dave Pugmire
//  Creation:   January 27, 2009
//
//  Modifications:
//
//   Dave Pugmire, Tue Mar 10 12:41:11 EDT 2009
//   Generalized domain to include domain/time. Pathine cleanup.
//   
//  Dave Pugmire, Wed Mar 18 17:17:40 EDT 2009
//  Allow managers to share work loads.
//
//   Dave Pugmire, Fri Sep 25 15:35:32 EDT 2009
//   Initialize new counters.
//
// ****************************************************************************

avtManagerICAlgorithm::avtManagerICAlgorithm(avtPICSFilter *picsFilter,
                                           int maxCount,
                                           int workGrpSz,
                                           vector<int> &workers,
                                           int mst,
                                           vector<int> &managers)
    : avtManagerWorkerICAlgorithm(picsFilter, maxCount)
{
    // Sleeping manager seems to increase latency.
    sleepMicroSec = 0;
    
    workGroupSz = workGrpSz;
    //Create my workers.
    for (int i = 0; i < workers.size(); i++)
        workerInfo.push_back(WorkerInfo(workers[i], NUM_DOMAINS));

    //Create any managers to manage.
    manager = mst;
    for (int i = 0; i < managers.size(); i++)
        managerInfo.push_back(WorkerInfo(managers[i], NUM_DOMAINS));
    
    icDomCnts.resize(NUM_DOMAINS,0);
    domLoaded.resize(NUM_DOMAINS,0);

    case1Cnt = 0;
    case2Cnt = 0;
    case3ACnt = case3BCnt = case3CCnt = 0;
    case4ACnt = case4BCnt = 0;
    case5ACnt = case5BCnt = 0;
    case6Cnt = 0;
}


// ****************************************************************************
//  Method: avtManagerICAlgorithm::~avtManagerICAlgorithm
//
//  Purpose:
//      avtManagerICAlgorithm destructor.
//
//  Programmer: Dave Pugmire
//  Creation:   January 27, 2009
//
// ****************************************************************************

avtManagerICAlgorithm::~avtManagerICAlgorithm()
{
}

// ****************************************************************************
//  Method: avtManagerICAlgorithm::Initialize
//
//  Purpose:
//      Initializization
//
//  Programmer: Dave Pugmire
//  Creation:   January 27, 2009
//
//  Modifications:
//  
//   Dave Pugmire, Wed Mar 18 17:17:40 EDT 2009
//   Allow managers to share work loads.
//
//   Dave Pugmire, Fri Sep 25 15:35:32 EDT 2009
//   Better initial SL distribution.
//
//   Dave Pugmire, Fri Feb 12 09:30:27 EST 2010
//   Wrong initial sizes for status and prevStatus.
//
// ****************************************************************************

void
avtManagerICAlgorithm::Initialize(std::vector<avtIntegralCurve *> &seedPts)
{
    SortIntegralCurves(seedPts);
    avtManagerWorkerICAlgorithm::Initialize(seedPts);
    int nSeeds = seedPts.size();

    int numManagers;
    if (nProcs < workGroupSz)
        numManagers = 1;
    else
        numManagers = nProcs/workGroupSz;
    
    int nSeedsPerProc = (nSeeds / numManagers);
    int oneExtraUntil = (nSeeds % numManagers);
    int i0 = 0, i1 = nSeeds;

    if (rank < oneExtraUntil)
    {
        i0 = (rank)*(nSeedsPerProc+1);
        i1 = (rank+1)*(nSeedsPerProc+1);
    }
    else
    {
        i0 = (rank)*(nSeedsPerProc) + oneExtraUntil;
        i1 = (rank+1)*(nSeedsPerProc) + oneExtraUntil;
    }

    debug1 << "I have "<<(i1-i0)<<" seeds: "<<i0<<" to "<<i1<<" of "<<nSeeds<<endl;
    
    //Delete the seeds I don't need.
    for (int i = 0; i < i0; i++)
        delete seedPts[i];
    for (int i = i1; i < nSeeds; i++)
        delete seedPts[i];
    
    for (int i = i0; i < i1; i++)
        activeICs.push_back(seedPts[i]);
    
    workGroupActiveICs = activeICs.size();
    done = false;
    workerUpdate = false;
    managerUpdate = false;
    status.resize(NUM_DOMAINS+1,0);
    prevStatus.resize(NUM_DOMAINS+1,0);
}

// ****************************************************************************
//  Method: avtManagerICAlgorithm::CompileTimingStatistics
//
//  Purpose:
//      Calculate statistics. For the manager, we need to be a little tricky.
//      The manager doesn't do a lot of things, so we don't want to skew the
//      statistics. So, set the timer/counters to -1 and then ignore these values
//      when reporting statistics.
//
//  Programmer: Dave Pugmire
//  Creation:   January 27, 2009
//  
//  Dave Pugmire, Mon Mar 23 12:48:12 EDT 2009
//  Change how timings are reported/calculated.
//  
// ****************************************************************************

void
avtManagerICAlgorithm::CompileTimingStatistics()
{
    LatencyTime.value = -1;
    MaxLatencyTime.value = -1;
    IOTime.value = -1;
    IntegrateTime.value = -1;
    SortTime.value = -1;
    avtManagerWorkerICAlgorithm::CompileTimingStatistics();
}

// ****************************************************************************
//  Method: avtManagerICAlgorithm::CompileCounterStatistics
//
//  Purpose:
//      Calculate statistics. For the manager, we need to be a little tricky.
//      The manager doesn't do a lot of things, so we don't want to skew the
//      statistics. So, set the timer/counters to -1 and then ignore these values
//      when reporting statistics.
//
//  Programmer: Dave Pugmire
//  Creation:   January 27, 2009
//
// ****************************************************************************

void
avtManagerICAlgorithm::CompileCounterStatistics()
{
    IntegrateCnt.value = -1;
    DomLoadCnt.value = -1;
    DomPurgeCnt.value = -1;
    avtManagerWorkerICAlgorithm::CompileCounterStatistics();
}


// ****************************************************************************
//  Method: avtManagerICAlgorithm::ReportCounters
//
//  Purpose:
//      Report counters.
//
//  Programmer: Dave Pugmire
//  Creation:   March 25, 2009
//
//  Modifications:
//
//   Dave Pugmire, Fri Sep 25 15:35:32 EDT 2009
//   Report new counters.
//
// ****************************************************************************

void
avtManagerICAlgorithm::ReportCounters(ostream &os, bool totals)
{
    avtManagerWorkerICAlgorithm::ReportCounters(os, totals);
    
    if (!totals)
    {
        double tot = case1Cnt+case2Cnt+case3ACnt+case3BCnt+case3CCnt+
                     case4ACnt+case4BCnt+case5ACnt+case5BCnt+case6Cnt;
        
        int case1 = case1Cnt;
        int case2 = case2Cnt;
        int case3 = case3ACnt+case3BCnt+case3CCnt;
        int case4 = case4ACnt+case4BCnt;
        int case5 = case5ACnt+case5BCnt;
        int case6 = case6Cnt;
        
        os<<"Cases:";
        os<<" C1: "<<case1;
        os<<"("<<setprecision(3)<<(double)case1/tot*100.0<<"%)";
        os<<" C2: "<<case2;
        os<<"("<<setprecision(3)<<(double)case2/tot*100.0<<"%)";
        os<<" C3: "<<case3;
        os<<"["<<case3ACnt<<" "<<case3BCnt<<" "<<case3CCnt<<"] ";
        os<<"("<<setprecision(3)<<(double)case3/tot*100.0<<"%)";
        os<<" C4: "<<case4;
        os<<"["<<case4ACnt<<" "<<case4BCnt<<"] ";
        os<<"("<<setprecision(3)<<(double)case4/tot*100.0<<"%)";
        os<<" C5: "<<case5;
        os<<"["<<case5ACnt<<" "<<case5BCnt<<"] ";
        os<<"("<<setprecision(3)<<(double)case5/tot*100.0<<"%)";
        os<<" C6: "<<case6;
        os<<"("<<setprecision(3)<<(double)case6/tot*100.0<<"%)";
        os<<endl;
    }
}

// ****************************************************************************
//  Method: avtManagerICAlgorithm::PrintStatus
//
//  Purpose:
//      Display the worker status
//
//  Programmer: Dave Pugmire
//  Creation:   January 27, 2009
//
//  Modifications:
//
//   Dave Pugmire, Tue Mar 10 12:41:11 EDT 2009
//   Generalized domain to include domain/time. Pathine cleanup.
//   
//  Dave Pugmire, Wed Mar 18 17:17:40 EDT 2009
//  Allow managers to share work loads.
//
//  Dave Pugmire, Wed Mar 25 10:04:29 EDT 2009
//  Control print information for large domain problems.
//  
// ****************************************************************************

void
avtManagerICAlgorithm::PrintStatus()
{
    if (managerInfo.size() > 0)
    {
        debug1<<"Managers:\n";
        for (int i = 0; i < managerInfo.size(); i++)
            managerInfo[i].Debug();
        debug1<<endl;
    }

    debug1<<"DOM:               [";
    if (NUM_DOMAINS < MAX_DOMAIN_PRINT)
    {
        for ( int i = 0; i < NUM_DOMAINS; i++)
            debug1<<setw(4)<<i<<" ";
    }
    debug1<<"]\n";
    debug1<<"Manager:            [";
    if (NUM_DOMAINS < MAX_DOMAIN_PRINT)
    {
        for ( int i = 0; i < NUM_DOMAINS; i++)
            debug1<<setw(4)<<icDomCnts[i]<<" ";
    }
    debug1<<"]\n";
    
    debug1<<" R:  T ( L, OOB)"<<endl;
    for (int i = 0; i < workerInfo.size(); i++)
        workerInfo[i].Debug();
    debug1<<"DCounts:           [";
    int cnt = 0;
    if (NUM_DOMAINS < MAX_DOMAIN_PRINT)
    {
        for ( int i = 0; i < NUM_DOMAINS; i++)
        {
            debug1<<setw(4)<<domLoaded[i]<<" ";
            cnt += domLoaded[i];
        }
    }
    debug1<<"] ("<<cnt<<")"<<endl;

    vector<int> workerICs(NUM_DOMAINS,0);
    for (int i = 0; i < workerInfo.size(); i++)
        for (int j = 0; j < NUM_DOMAINS; j++)
            workerICs[j] += workerInfo[i].domainCnt[j];
    debug1<<"SCounts:           [";
    if (NUM_DOMAINS < MAX_DOMAIN_PRINT)
    {
        for ( int i = 0; i < NUM_DOMAINS; i++)
        {
            debug1<<setw(4)<<workerICs[i]<<" ";
            cnt += workerICs[i];
        }
    }
    debug1<<"] ("<<cnt<<")"<<endl;
    debug1<<endl;
}


// ****************************************************************************
//  Method: avtManagerICAlgorithm::UpdateWorkerStatus
//
//  Purpose:
//      Update the worker status.
//
//  Programmer: Dave Pugmire
//  Creation:   January 27, 2009
//
//  Modifications:
//  
//  Dave Pugmire, Wed Mar 18 17:17:40 EDT 2009
//  Allow managers to share work loads.    
//
//  Dave Pugmire, Wed Mar 25 10:04:29 EDT 2009
//  Control print information for large domain problems.
//
//  Hank Childs, Sat Jun  5 11:29:13 CDT 2010
//  Change return type to void, since it wasn't returning anything and the
//  calling function wasn't looking for a return value.
//
// ****************************************************************************

void
avtManagerICAlgorithm::UpdateWorkerStatus(vector<int> &status)
{
    int src = status[0];
    int msg = status[1];
    int nTerm = status[2];
    
    workGroupActiveICs -= nTerm;
    if (workGroupActiveICs < 0)
    {
        debug1<<"HACK: Need to figure out how the count got messed up!"<<endl;
        workGroupActiveICs = 0;
    }

    debug1<<"WorkerStatus: "<<src<<" ";
    if (NUM_DOMAINS < MAX_DOMAIN_PRINT)  debug1<<status;
    debug1<<endl;
    
    for (int i = 0; i < workerInfo.size(); i++)
    {
        if (workerInfo[i].rank == src)
        {
            debug5<<"Update for rank= "<<src<<endl;
            vector<int> domStatus;
            for (int j = 3; j < status.size(); j++)
                domStatus.push_back(status[j]);
            workerInfo[i].Update(domStatus, false);
            break;
        }
    }
}

// ****************************************************************************
//  Method: avtManagerICAlgorithm::RunAlgorithm
//
//  Purpose:
//      Execute the manager loop of the manager/worker algorithm.
//
//  Programmer: Dave Pugmire
//  Creation:   January 27, 2009
//
//  Modifications:
//
//   Dave Pugmire, Mon Feb 23 13:38:49 EST 2009
//   Add call to check for sent buffers.
//
//  Dave Pugmire, Wed Mar 18 17:17:40 EDT 2009
//  Allow managers to share work loads.
//
//   Dave Pugmire, Thu Sep 24 13:52:59 EDT 2009
//   Change Execute to RunAlgorithm.
//
// ****************************************************************************

void
avtManagerICAlgorithm::RunAlgorithm()
{
    int timer = visitTimer->StartTimer();
    
    //Each worker has to send information. Don't proceed until they have.
    Barrier();

    while (!done)
    {
        debug1<<"Looping ICs= "<<workGroupActiveICs<<endl;
        ProcessMessages();
        ProcessNewIntegralCurves();
        ManageWorkgroup();
        
        CheckPendingSendRequests();

        //See if we are done.
        PostLoopProcessing();
    }

    // We are done, tell all workers to pack it up.
    SendAllWorkersMsg(MSG_DONE);
    TotalTime.value += visitTimer->StopTimer(timer, "Execute");
}

// ****************************************************************************
//  Method: avtManagerICAlgorithm::PostLoopProcessing()
//
//  Purpose:
//      See if we are done.
//
//  Programmer: Dave Pugmire
//  Creation:   March 18, 2009
//
//  Modifications:
//
//  Dave Pugmire, Sat Mar 28 10:04:01 EDT 2009
//  Temporary fix for a sporadic bug where it looks like messages are not
//  being delivered to the manager. Detect this case, and signal done.
//
// ****************************************************************************

void
avtManagerICAlgorithm::PostLoopProcessing()
{
    SendStatus();
    
    //This is a hack....
    if (activeICs.empty() && workGroupActiveICs > 0)
    {
        bool haveActiveWorkers = false;
        for (int i = 0; i < workerInfo.size(); i++)
            if (workerInfo[i].icCount != 0)
            {
                haveActiveWorkers = true;
                break;
            }
        if (!haveActiveWorkers)
        {
            debug1<<"HACK: Need to figure out how the count got messed up!"<<endl;
            //workGroupActiveICs = 0;
            //SendStatus(true);
        }
    }
    //end hack....

    //See if we are done.
    if (manager == -1) //I'm root manager.
    {
        debug1<<"See if we are done.\n";
        if (workGroupActiveICs == 0)
        {
            debug1<<"I'm done!\n";
            done = true;
            for (int i = 0; i < managerInfo.size(); i++)
                if (managerInfo[i].icCount != 0)
                {
                    done = false;
                    break;
                }
            
            debug1<<"Done= "<<done<<endl;
            if (done)
            {
                for (int i = 0; i < managerInfo.size(); i++)
                {
                    vector<int> msg(1, MSG_DONE);
                    SendMsg(managerInfo[i].rank, msg);
                }
            }
        }
    }
}

// ****************************************************************************
//  Method: avtManagerICAlgorithm::UpdateStatus
//
//  Purpose:
//      Update my status.
//
//  Programmer: Dave Pugmire
//  Creation:   March 18, 2009
//  
// ****************************************************************************

void
avtManagerICAlgorithm::UpdateStatus()
{
    for (int i = 0; i < status.size(); i++)
        status[i] = 0;
    
    status[0] = workGroupActiveICs;
    for (int i = 0; i < NUM_DOMAINS; i++)
        status[i+1] = domLoaded[i];
}


// ****************************************************************************
//  Method: avtManagerICAlgorithm::SendStatus
//
//  Purpose:
//      Send manager status to my manager.
//
//  Programmer: Dave Pugmire
//  Creation:   March 18, 2009
//
//  Modifications:
//
//  Dave Pugmire, Wed Mar 25 10:04:29 EDT 2009
//  Control print information for large domain problems.
//
// ****************************************************************************

void
avtManagerICAlgorithm::SendStatus(bool forceSend)
{
    UpdateStatus();

    bool statusChanged = false;
    
    //See if anything changed.
    if (forceSend)
        statusChanged = true;
    else
    {
        for (int i = 0; i < status.size(); i++)
            if (status[i] != prevStatus[i])
            {
                statusChanged = true;
                break;
            }
    }
    
    if (statusChanged)
    {
        vector<int> msg(NUM_DOMAINS+2);
        msg[0] = MSG_MANAGER_STATUS;
        msg[1] = workGroupActiveICs;
        for (int i = 0; i < NUM_DOMAINS; i++)
            msg[i+2] = domLoaded[i];

        debug1<<"ManagerStatusSend: ";
        if (NUM_DOMAINS < MAX_DOMAIN_PRINT) debug1<<msg;
        debug1<<endl;
        
        if (manager != -1)
            SendMsg(manager, msg);
        else
        {
            msg.insert(msg.begin(), rank);
            ProcessManagerUpdate(msg);
        }

        
        for (int i = 0; i < status.size(); i++)
            prevStatus[i] = status[i];
    }
}

// ****************************************************************************
//  Method: avtManagerICAlgorithm::ProcessMessages
//
//  Purpose:
//      Handle incoming messages.
//
//  Programmer: Dave Pugmire
//  Creation:   March 18, 2009
//
//   Dave Pugmire, Tue Jan 18 06:57:31 EST 2011
//   Regression fix. Message processing assumed that src was in msg[0].
//
// ****************************************************************************

void
avtManagerICAlgorithm::ProcessMessages()
{
    vector<MsgCommData> msgs;
    RecvMsg(msgs);

    for (int i = 0; i < msgs.size(); i++)
    {
        vector<int> &msg = msgs[i].message;
        int src = msgs[i].rank;
        int msgType = msg[0];

        if (msgType == MSG_DONE)
        {
            done = true;
            break;
        }
        else
        {
            vector<int> status(msg.size()+1);
            status[0] = src;
            for (int i = 1; i < status.size(); i++)
                status[i] = msg[i-1];
            
            if (msgType == MSG_STATUS)
                ProcessWorkerUpdate(status);
            else if (msgType == MSG_MANAGER_STATUS)
                ProcessManagerUpdate(status);
            else if (msgType == MSG_OFFLOAD_IC)
                ProcessOffloadIC(status);
        }
    }
}

// ****************************************************************************
//  Method: avtManagerICAlgorithm::ProcessWorkerUpdate
//
//  Purpose:
//      Process status messages from workers.
//
//  Programmer: Dave Pugmire
//  Creation:   March 18, 2009
//
// ****************************************************************************

void
avtManagerICAlgorithm::ProcessWorkerUpdate(vector<int> &status)
{
    UpdateWorkerStatus(status);
    workerUpdate = true;
}

// ****************************************************************************
//  Method: avtManagerICAlgorithm::ProcessManagerUpdate
//
//  Purpose:
//      Process status messages from managers.
//
//  Programmer: Dave Pugmire
//  Creation:   March 18, 2009
//
//  Modifications:
//
//  Dave Pugmire, Wed Mar 25 10:04:29 EDT 2009
//  Control print information for large domain problems.
//
// ****************************************************************************

void
avtManagerICAlgorithm::ProcessManagerUpdate(vector<int> &status)
{
    int src = status[0];
    int msg = status[1];
    int nICs = status[2];

    debug1<<"ManagerUpdateStatus: "<<src<<" "<<msg<<" "<<nICs<<" ";
    if (NUM_DOMAINS < MAX_DOMAIN_PRINT) debug1<<status;
    debug1<<endl;
    
    for (int i = 0; i < managerInfo.size(); i++)
    {
        if (managerInfo[i].rank == src)
        {
            debug5<<"Update for rank= "<<src<<endl;
            vector<int> domStatus;
            for (int j = 3; j < status.size(); j++)
                domStatus.push_back(status[j]);
            managerInfo[i].Update(domStatus, false);
            managerInfo[i].icCount = nICs;
            break;
        }
    }

    managerUpdate = true;
}

// ****************************************************************************
//  Method: avtManagerICAlgorithm::ProcessOffloadIC
//
//  Purpose:
//      Offload work to another manager.
//
//  Programmer: Dave Pugmire
//  Creation:   March 18, 2009
//
//  Modifications:
//
//  Dave Pugmire, Wed Mar 18 21:55:32 EDT 2009
//  Modify how managers handle offloading work to other managers.
//
// ****************************************************************************

void
avtManagerICAlgorithm::ProcessOffloadIC(vector<int> &status)
{
    int src = status[0];
    int msg = status[1];
    int dst = status[2];

    debug1<<"Offload some work to "<<dst<<" domInfo= "<<status<<endl;

    vector<int> doms;
    for (int i = 3; i < status.size(); i++)
        if (status[i] != 0)
            doms.push_back(i-3);

    debug1<<"Doms to consider: "<<doms<<endl;

    // Find workers with domains send offload message.
    for (int i = 0; i < workerInfo.size(); i++)
    {
        debug1<<" "<<workerInfo[i].rank<<": "<<workerInfo[i].icCount<<endl;
        if (workerInfo[i].icCount > 0)
        {
            vector<int> domsToSend;
            for (int d = 0; d < doms.size(); d++)
            {
                if (workerInfo[i].domainCnt[doms[d]] > 0)
                    domsToSend.push_back(doms[d]);
            }

            // This worker has some domains that this manager has loaded.
            if (domsToSend.size() > 0)
            {
                vector<int> msg;
                msg.push_back(MSG_OFFLOAD_IC);
                msg.push_back(dst);
                msg.push_back(domsToSend.size());
                for (int j = 0; j < domsToSend.size(); j++)
                    msg.push_back(domsToSend[j]);
                //workerInfo[i].RemoveIC(doms[d]);
                
                debug1<<"Send OFFLOAD MSG: "<<workerInfo[i].rank<<" ==> "<<dst<<" "<<msg<<endl;
                SendMsg(workerInfo[i].rank, msg);
            }
        }
    }

    managerUpdate = true;
}


// ****************************************************************************
//  Method: avtManagerICAlgorithm::NewIntegralCurves
//
//  Purpose:
//      Handle incoming integral curves.
//
//  Programmer: Dave Pugmire
//  Creation:   March 18, 2009
//
//  Modifications:
//
// ****************************************************************************

void
avtManagerICAlgorithm::ProcessNewIntegralCurves()
{
    list<avtIntegralCurve*> newICs;
    RecvICs(newICs);
    if (!newICs.empty())
    {
        debug1<<"avtManagerICAlgorithm::ProcessNewIntegralCurves() cnt "<<workGroupActiveICs<<" ==> ";
        workGroupActiveICs += newICs.size();
        debug1<<workGroupActiveICs<<endl;

        activeICs.splice(activeICs.end(), newICs);

        // We need to process for workers now.
        workerUpdate = true;
    }
}

// ****************************************************************************
//  Method: avtManagerICAlgorithm::ManageWorkgroup
//
//  Purpose:
//      Manage workgroup.
//
//  Programmer: Dave Pugmire
//  Creation:   March 18, 2009
//
//  Modifications:
//    Kathleen Biagas, Mon Sep 26 07:15:32 MST 2011
//    Changed Sleep call to NSleep to prevent collision on Windows.
//
// ****************************************************************************

void
avtManagerICAlgorithm::ManageWorkgroup()
{
    if (workerUpdate)
    {
        //Update our counters.
        for (int i = 0; i < NUM_DOMAINS; i++)
        {
            icDomCnts[i] = 0;
            domLoaded[i] = 0;
        }
        
        list<avtIntegralCurve *>::const_iterator s;
        for (s = activeICs.begin(); s != activeICs.end(); ++s)
            icDomCnts[DomToIdx( (*s)->domain )] ++;
    
        for (int i = 0; i < workerInfo.size(); i++)
            for ( int d = 0; d < NUM_DOMAINS; d++)
                if (workerInfo[i].domainLoaded[d])
                    domLoaded[d]++;
    }
    
    //PrintStatus();

    if (!workerUpdate && !managerUpdate)
    {
        NSleep();
    }
    else
    {
        if (workerUpdate)
            ManageWorkers();
        if (managerUpdate)
            ManageManagers();
    }

    debug1<<endl<<"Post-Mortem"<<endl;
    //PrintStatus();

    workerUpdate = false;
    managerUpdate = false;
}

// ****************************************************************************
//  Method: avtManagerICAlgorithm::ManageWorkers
//
//  Purpose:
//      Manage workers workgroup.
//
//  Programmer: Dave Pugmire
//  Creation:   March 18, 2009
//
//  Modifications:
//
//  Dave Pugmire, Sat Mar 28 10:04:01 EDT 2009
//  Modifications to the logic that made a big impact on domain loading.
//  After a domain is loaded, there is opportunity to send ICs to it (case 3)
//  Do this after each case 4.  Increase the case3 overload factor. Add case5.
//
// ****************************************************************************

void
avtManagerICAlgorithm::ManageWorkers()
{
    /*
      Case1(case1Cnt);
      Case2(case2Cnt);
      int case3OverloadFactor = 10*maxCnt, case3NDomainFactor = 3*maxCnt;
      Case3(case3OverloadFactor, case3NDomainFactor, case3Cnt);
      Case4(0,case4Cnt);
    */
    
    int case3OverloadFactor = 20*maxCnt, case3NDomainFactor = 3*maxCnt;
    Case3(case3OverloadFactor, case3NDomainFactor, case3ACnt);

    Case4(case4AThreshold, case4ACnt);
    Case1(case1Cnt);
    //Case2(case2Cnt);
    
    Case3(case3OverloadFactor, case3NDomainFactor, case3BCnt);
    
    Case2(case2Cnt);
    Case4(0, case4BCnt);
    Case3(case3OverloadFactor, case3NDomainFactor, case3CCnt);
    
    Case5(maxCnt, true, case5ACnt);
    Case5(maxCnt, false, case5ACnt);

    //See who else still doesn't have work....
    FindSlackers();
    if (slackers.size() > 0)
    {
        case6Cnt++;
        debug1<<"Case 6: "<<slackers<<endl;
    }
    
    //Resets.
    for (int i = 0; i < workerInfo.size(); i++)
        workerInfo[i].Reset();
}

// ****************************************************************************
//  Method: avtManagerICAlgorithm::ManageManagers
//
//  Purpose:
//      Manage workers workgroup.
//
//  Programmer: Dave Pugmire
//  Creation:   March 18, 2009
//
//  Modifications:
//
//  Dave Pugmire, Wed Mar 18 21:55:32 EDT 2009
//  Randomize the idle/busy pairings. This should provide better balancing.
//
// ****************************************************************************

void
avtManagerICAlgorithm::ManageManagers()
{
    // Don't bother if no managers...
    if (managerInfo.size() == 0)
        return;

    // Get all the managers with no work.
    vector<int> idleManagers, busyManagers;
    for (int i = 0; i < managerInfo.size(); i++)
    {
        if (managerInfo[i].initialized)
        {
            if (managerInfo[i].icCount == 0)
                idleManagers.push_back(managerInfo[i].rank);
            else
                busyManagers.push_back(managerInfo[i].rank);
        }
    }

    // Nothing to do!
    if (idleManagers.size() == 0 ||
        busyManagers.size() == 0)
        return;
    
    //Randomize things.
    random_shuffle(idleManagers.begin(), idleManagers.end());
    random_shuffle(busyManagers.begin(), busyManagers.end());
    
    debug1<<"IdleManagers: "<<idleManagers<<endl;
    debug1<<"BusyManagers: "<<busyManagers<<endl;

    // Tell each busyManager to offload to an idleManager.
    int N = idleManagers.size();
    if (busyManagers.size() < N)
        N = busyManagers.size();
    
    //TODO: Should we wrap around?
    for (int i = 0; i < N; i++)
    {
        // Have busy
        vector<int> msg(NUM_DOMAINS+2,0);
        msg[0] = MSG_OFFLOAD_IC;
        msg[1] = idleManagers[i];
        for (int j = 0; j < NUM_DOMAINS; j++)
            msg[2+j] = managerInfo[idleManagers[i]].domainLoaded[j];
        
        debug1<<busyManagers[i]<<": ManagerOffload to "<<idleManagers[i]<<" : "<<msg<<endl;
        if (busyManagers[i] == rank)
        {
            msg.insert(msg.begin(), rank);
            ProcessOffloadIC(msg);
        }
        else
            SendMsg(busyManagers[i], msg);
    }
}


// ****************************************************************************
//  Method: avtManagerICAlgorithm::SendAllWorkersMsg
//
//  Purpose:
//      Send a message to all workers.
//
//  Programmer: Dave Pugmire
//  Creation:   January 27, 2009
//
// ****************************************************************************

void
avtManagerICAlgorithm::SendAllWorkersMsg(int msg)
{
    vector<int> msgVec(1,msg);
    SendAllWorkersMsg(msgVec);
}

// ****************************************************************************
//  Method: avtManagerICAlgorithm::SendAllWorkersMsg
//
//  Purpose:
//      Send a message to all workers.
//
//  Programmer: Dave Pugmire
//  Creation:   January 27, 2009
//
// ****************************************************************************

void
avtManagerICAlgorithm::SendAllWorkersMsg(vector<int> &msg)
{
    for (int i = 0; i < workerInfo.size(); i++)
        SendMsg(workerInfo[i].rank, msg);
}

// ****************************************************************************
//  Method: avtManagerICAlgorithm::FindSlackers
//
//  Purpose:
//      Find workers with no work to do
//
//  Programmer: Dave Pugmire
//  Creation:   January 27, 2009
//
//  Modifications:
//
//  Dave Pugmire, Sat Mar 28 10:04:01 EDT 2009
//  Latency saving sends can leave us a count of 2 or less. So, consider these
//  as slackers.
//
//   Dave Pugmire, Wed Apr  1 11:21:05 EDT 2009
//   Use LATENCY_SEND_CNT to determine who is out of work.
//
// ****************************************************************************

void
avtManagerICAlgorithm::FindSlackers(int oobFactor,
                                   bool randomize,
                                   bool checkJustUpdated)
{
    slackers.resize(0);

    //if oobFactor != -1, find workers with between 0 and oobFactor OOB
    //integral curves.

    for (int i = 0; i < workerInfo.size(); i++)
        if (workerInfo[i].icLoadedCount <= LATENCY_SEND_CNT ||
            (workerInfo[i].justUpdated && checkJustUpdated))
        {
            if ( oobFactor != -1 &&
                 workerInfo[i].icOOBCount > 0 &&
                 workerInfo[i].icOOBCount < oobFactor)
                slackers.push_back(i);
            else
                slackers.push_back(i);
        }

    if (randomize)
        random_shuffle(slackers.begin(), slackers.end());
}


// ****************************************************************************
//  Method: avtManagerICAlgorithm::Case1
//
//  Purpose:
//      Case1 of managerworker algorithm. Give ICs to workers who have domain
//      loaded.
//
//  Programmer: Dave Pugmire
//  Creation:   January 27, 2009
//
//  Modifications:
//
//   Dave Pugmire, Tue Mar 10 12:41:11 EDT 2009
//   Generalized domain to include domain/time. Pathine cleanup.
//
//   Dave Pugmire, Sat Mar 28 10:04:01 EDT 2009
//   Bug fix. Case1 never happened. forgot the "not" empty.
//
// ****************************************************************************

void
avtManagerICAlgorithm::Case1(int &counter)
{
    if (activeICs.empty())
        return;
    
    FindSlackers();
    
    vector< vector< avtIntegralCurve *> > distributeICs(nProcs);
    bool icsToSend = false;
    
    for (int i = 0; i < slackers.size(); i++)
    {
        if (activeICs.empty())
            break;
        
        int cnt = 0;
        int slackerRank = workerInfo[slackers[i]].rank;
        list<avtIntegralCurve *>::iterator s = activeICs.begin();

        while ( !activeICs.empty() && cnt < maxCnt)
        {
            int sDom = DomToIdx( (*s)->domain );
            if (workerInfo[slackers[i]].domainLoaded[sDom])
            {
                distributeICs[slackerRank].push_back(*s);
                workerInfo[slackers[i]].AddIC(sDom, DomCacheSize());
                s = activeICs.erase(s);
                icsToSend = true;
                cnt++;
            }
            else
                ++s;
            if (s == activeICs.end())
                break;
        }
        
        if (cnt > 0)
        {
            debug1<<"Case 1: "<<slackerRank<<" Send "<<cnt<<" ICs [";
            for (int j = 0; j < distributeICs[slackerRank].size(); j++)
                debug1<<distributeICs[slackerRank][j]->domain<<" ";
            debug1<<"]\n";
            counter++;
        }
    }

    if (icsToSend)
        ExchangeICs(activeICs, distributeICs);
}


// ****************************************************************************
//  Method: avtManagerICAlgorithm::Case2
//
//  Purpose:
//      Case2 of managerworker algorithm. Give ICs to workers and force domain load
//
//  Programmer: Dave Pugmire
//  Creation:   January 27, 2009
//
//  Modifications:
//
//   Dave Pugmire, Tue Mar 10 12:41:11 EDT 2009
//   Generalized domain to include domain/time. Pathine cleanup.
//
//   Dave Pugmire, Fri Feb 12 09:30:27 EST 2010
//   Fix memory leak of domCnts.
//
// ****************************************************************************

static bool domCntCompare( const int *a, const int *b) { return a[1] > b[1]; }

void
avtManagerICAlgorithm::Case2(int &counter)
{
    if (activeICs.empty())
        return;
    
    FindSlackers();
    
    vector< vector< avtIntegralCurve *> > distributeICs(nProcs);
    bool icsToSend = false;

    for (int s = 0; s < slackers.size(); s++)
    {
        if (activeICs.empty())
            break;
        
        int slackerRank = workerInfo[slackers[s]].rank;
    
        vector<int*> domCnts;
        for (int i = 0; i < icDomCnts.size(); i++)
            if (icDomCnts[i] > 0)
            {
                int *entry = new int[2];
                entry[0] = i;
                entry[1] = icDomCnts[i];
                domCnts.push_back(entry);
            }

        if (icDomCnts.size() == 0)
            continue;
        //Sort on IC count per domain.
        sort(domCnts.begin(),domCnts.end(), domCntCompare);
        if (false)
        {
            debug1<<"IC sort: [";
            for (int i = 0; i < domCnts.size(); i++)
                debug1<<domCnts[i][0]<<" "<<domCnts[i][1]<<", ";
            debug1<<"]\n";
        }

        int domToLoad = -1;
        for (int i = 0; i < domCnts.size(); i++)
        {
            bool anyLoaded = false;
            for (int j = 0; j < workerInfo.size(); j++)
                if (workerInfo[j].domainLoaded[domCnts[i][0]])
                    anyLoaded = true;
            if (!anyLoaded)
            {
                domToLoad = domCnts[i][0];
                break;
            }
        }

        if (domToLoad == -1)
        {
            random_shuffle(domCnts.begin(), domCnts.end());
            domToLoad = domCnts[0][0];
            //debug1<<"All doms loaded. Random pick: "<<domToLoad<<endl;
        }

        int cnt = 0;

        list<avtIntegralCurve *>::iterator ic = activeICs.begin();
        
        while ( !activeICs.empty() && cnt < maxCnt)
        {
            int sDom = DomToIdx( (*ic)->domain );
            if (sDom == domToLoad && cnt < maxCnt)
            {
                distributeICs[slackerRank].push_back(*ic);
                workerInfo[slackers[s]].AddIC(sDom, DomCacheSize());
                cnt++;
                icDomCnts[domToLoad]--;
                ic = activeICs.erase(ic);
            }
            else
                ++ic;
            if (ic == activeICs.end())
                break;
        }
            
        for (int i = 0; i < domCnts.size(); i++)
            delete [] domCnts[i];

        if (cnt > 0)
        {
            icsToSend = true;
            if (distributeICs[slackerRank].size() > 0)
            {
                debug1<<"Case 2: "<<slackerRank<<" Send "<<cnt<<" ICs [";
                for (int j = 0; j < distributeICs[slackerRank].size(); j++)
                    debug1<<distributeICs[slackerRank][j]->domain<<" ";
                debug1<<"]\n";
                counter++;
            }
        }
    }
    
    if (icsToSend)
    {
        for (int i = 0; i < nProcs; i++)
        {
            if (distributeICs[i].size() > 0)
                SendICs(i, distributeICs[i]);
        }
    }    
}

// ****************************************************************************
//  Method: avtManagerICAlgorithm::Case3
//
//  Purpose:
//      Case3 of managerworker algorithm. Send ICs to another worker.
//
//  Programmer: Dave Pugmire
//  Creation:   January 27, 2009
//
//  Modifications:
//
//   Dave Pugmire, Tue Mar 10 12:41:11 EDT 2009
//   Generalized domain to include domain/time. Pathine cleanup.
//
//   Dave Pugmire, Mon Mar 16 15:05:14 EDT 2009
//   Bug fix. Didn't use new BlockIDType structure for MSG_SEND_IC.
//   
//   Dave Pugmire, Wed Mar 18 17:17:40 EDT 2009
//   Allow managers to share work loads.
//
// ****************************************************************************

void
avtManagerICAlgorithm::Case3(int overloadFactor,
                            int NDomainFactor,
                            int &counter )
{
    FindSlackers(NDomainFactor, true, true);
    if (slackers.size() == 0)
        return;
    
    vector<int> sender, recv, dom;
    for (int i = 0; i < slackers.size(); i++)
    {
        int slackerIdx = slackers[i];
        //debug1<<"Case 3: slackerRank="<<workerInfo[slackerIdx].rank<<endl;
        
        for (int d = 0; d < NUM_DOMAINS; d++)
        {
            vector<int> domPartner;
            if ( !workerInfo[slackerIdx].domainLoaded[d] &&
                 workerInfo[slackerIdx].domainCnt[d] > 0)
            {
                //debug1<<"   dom= "<<d<<endl;
                // Find a partner who has the domain and has fewer than overloadFactor ICs.
                for (int j = 0; j < workerInfo.size(); j++)
                {
                    if (j != slackerIdx && workerInfo[j].domainLoaded[d] &&
                        workerInfo[j].icCount < overloadFactor)
                    {
                        //debug1<<"      partner= "<<j<<endl;
                        domPartner.push_back(j);
                    }
                }
            }
            
            if (domPartner.size() > 0)
            {
                //debug1<<"domPartner: [";
                //for(int k=0; k<domPartner.size(); k++) debug1<<domPartner[k]<<" ";
                //debug1<<"]\n";
                
                random_shuffle(domPartner.begin(), domPartner.end());
                
                //debug1<<"random sort: [";
                //for(int k=0; k<domPartner.size(); k++) debug1<<domPartner[k]<<" ";
                //debug1<<"]\n";                
                for (int j = 0; j < domPartner.size(); j++)
                {
                    sender.push_back(slackerIdx);
                    recv.push_back(domPartner[j]);
                    dom.push_back(d);
                    
                }
            }
        }
    }

    int maxICsToSend = 5*maxCnt;
    int maxDestICs = overloadFactor;

    // Send messages out.
    for (int i = 0; i < sender.size(); i++)
    {
        WorkerInfo &recvWorker = workerInfo[recv[i]];
        WorkerInfo &sendWorker = workerInfo[sender[i]];
        int d = dom[i];
        int n = sendWorker.domainCnt[d];

        if (n > maxICsToSend)
            n = maxICsToSend;

        //Dest already has enough work.
        if (recvWorker.icCount > maxDestICs)
            continue;
    
        // Cap it.
        if (recvWorker.icCount + n > maxDestICs)
            n = maxDestICs - recvWorker.icCount;

        vector<int> msg;
        msg.push_back(MSG_SEND_IC);
        msg.push_back(recvWorker.rank);

        BlockIDType dd = IdxToDom(d);
        msg.push_back(d);
        msg.push_back(n);
        
        for (int i = 0; i < n; i++)
        {
            recvWorker.AddIC(d, DomCacheSize());
            sendWorker.RemoveIC(d);
        }

        if (n > 0)
        {
            debug1<<"Case 3: "<<sendWorker.rank<<" ==["<<n<<"]==> "<<recvWorker.rank<<"  d= "<<d;
            debug1<<" ***   "<<recvWorker.rank<<" now has "<<recvWorker.icCount<<" cap= "<<maxDestICs<<endl;
            SendMsg(sendWorker.rank, msg);
            counter++;
        }
    }
}

// ****************************************************************************
//  Method: avtManagerICAlgorithm::Case4
//
//  Purpose:
//      Case4 of managerworker algorithm. Tell a worker to load a domain.
//
//  Programmer: Dave Pugmire
//  Creation:   January 27, 2009
//
//  Modifications:
//
//   Dave Pugmire, Tue Mar 10 12:41:11 EDT 2009
//   Generalized domain to include domain/time. Pathine cleanup.
//
//  Dave Pugmire, Sat Mar 28 10:04:01 EDT 2009
//  Look for domains with max oob count.
//
// ****************************************************************************

void
avtManagerICAlgorithm::Case4(int oobThreshold,
                            int &counter )
{
    slackers.resize(0);
    
    for (int i = 0; i < workerInfo.size(); i++)
        if (workerInfo[i].icLoadedCount == 0 && workerInfo[i].icOOBCount >= oobThreshold)
            slackers.push_back(i);
    random_shuffle(slackers.begin(), slackers.end());

    
    for (int i = 0; i < slackers.size(); i++)
    {
        int idx = slackers[i];
        int domToLoad = -1, maxOOBCnt=-1;
        for (int j = 0; j < workerInfo[idx].domainCnt.size(); j++)
        {
            if (workerInfo[idx].domainCnt[j] > 0 && workerInfo[idx].domainCnt[j] > maxOOBCnt &&
                workerInfo[idx].domainCnt[j] > oobThreshold)
            {
                domToLoad = j;
                maxOOBCnt = workerInfo[idx].domainCnt[j];
            }
        }
        
        if (domToLoad != -1)
        {
            vector<int> msg;
            msg.push_back(MSG_LOAD_DOMAIN);
            BlockIDType dom = IdxToDom(domToLoad);
            msg.push_back(dom.domain);
            msg.push_back(dom.timeStep);
            SendMsg(workerInfo[idx].rank, msg);
            debug1<<"Case 4: "<<workerInfo[idx].rank<<" load dom= "<<domToLoad<<" oobThreshold: "<<oobThreshold<<endl;
            workerInfo[idx].LoadDom(domToLoad);
            counter++;
        }
    }
}


// ****************************************************************************
//  Method: avtManagerICAlgorithm::Case5
//
//  Purpose:
//      Case5 of managerworker algorithm. Advise worker to send ICs to other worker.
//  
//
//  Programmer: Dave Pugmire
//  Creation:   Mar 23, 2009
//
//  Modifications:
//
//  Dave Pugmire, Sat Mar 28 10:04:01 EDT 2009
//  Finish implementation.
//
//   Dave Pugmire, Wed Apr  1 11:21:05 EDT 2009
//   Limit the number of case5 messages that are sent.
//
//   Dave Pugmire, Fri Feb 12 09:30:27 EST 2010
//   Fix memory leak of overWorkedCnt.
//
// ****************************************************************************

void
avtManagerICAlgorithm::Case5(int overworkThreshold, bool domainCheck, int &counter)
{
    vector<int> slackers, overWorked;
    vector<int *> overWorkedCnt;
    //Get folks with too much work to do.
    for (int i = 0; i < workerInfo.size(); i++)
        if (workerInfo[i].icCount > overworkThreshold)
        {
            int *v = new int[2];
            v[0] = i;
            v[1] = workerInfo[i].icCount;
            overWorkedCnt.push_back(v);
        }

    if (overWorkedCnt.size() == 0)
        return;
    sort(overWorkedCnt.begin(), overWorkedCnt.end(), domCntCompare);
    for (int i = 0; i < overWorkedCnt.size(); i++)
    {
        overWorked.push_back(overWorkedCnt[i][0]);
        delete [] overWorkedCnt[i];
    }

    //Get slackers with no work and no unloaded domains.
    for (int i = 0; i < workerInfo.size(); i++)
        if (!workerInfo[i].justUpdated && workerInfo[i].icCount == 0)
            slackers.push_back(i);
    if (slackers.size() == 0)
        return;

    random_shuffle(slackers.begin(), slackers.end());

    vector<int> senders, receivers;
    vector<vector<int> > doms;
    // Find the first send w/
    for (int w = 0; w < overWorked.size(); w++)
    {
        for (int s = 0; s < slackers.size(); s++)
        {
            if (domainCheck)
            {
                vector<int> commonDoms;
                for (int d = 0; d < NUM_DOMAINS; d++)
                {
                    if (workerInfo[overWorked[w]].domainCnt[d] > 0 &&
                        workerInfo[slackers[s]].domainLoaded[d])
                        commonDoms.push_back(d);
                }
                
                if (commonDoms.size() > 0)
                {
                    senders.push_back(workerInfo[overWorked[w]].rank);
                    receivers.push_back(workerInfo[slackers[s]].rank);
                    doms.push_back(commonDoms);
                }
            }
            else
            {
                senders.push_back(workerInfo[overWorked[w]].rank);
                receivers.push_back(workerInfo[slackers[s]].rank);
                vector<int> commonDoms;
                doms.push_back(commonDoms);
            }
        }
    }

    //Nobody to send....
    if (senders.size() == 0 || receivers.size() == 0)
    {
        debug1<<"... No matches.\n";
        return;
    }

    /*
    debug1<<"C5: dc= "<<domainCheck<<endl;
    debug1<<"C5 Slackers: "<<slackers<<endl;
    debug1<<"C5 Overworked: "<<overWorked<<endl;
    */

    for (int i = 0; i < senders.size(); i++)
    {
        vector<int> msg;
        msg.push_back(MSG_SEND_IC_HINT);
        msg.push_back(receivers[i]);
        msg.push_back(doms[i].size());
        for (int j = 0; j < doms[i].size(); j++)
            msg.push_back(doms[i][j]);

        debug1<<"Case 5: "<<senders[i]<<" ===> "<<receivers[i]<<" "<<doms[i]<<endl;
        SendMsg(senders[i], msg);
        counter++;
        if (i > 1)
            break;
    }
}


// ****************************************************************************
//  Method: avtWorkerICAlgorithm::avtWorkerICAlgorithm
//
//  Purpose:
//      avtWorkerICAlgorithm constructor.
//
//  Programmer: Dave Pugmire
//  Creation:   January 27, 2009
//
//  Modifications:
//
//   Dave Pugmire, Mon Feb 23 13:38:49 EST 2009
//   Add timeout counter for workers.
//
// ****************************************************************************

avtWorkerICAlgorithm::avtWorkerICAlgorithm(avtPICSFilter *picsFilter,
                                         int maxCount,
                                         int managerRank)
    : avtManagerWorkerICAlgorithm(picsFilter, maxCount)
{
    manager = managerRank;
    timeout = 0;
}

// ****************************************************************************
//  Method: avtWorkerICAlgorithm::avtWorkerICAlgorithm
//
//  Purpose:
//      avtWorkerICAlgorithm destructor
//
//  Programmer: Dave Pugmire
//  Creation:   January 27, 2009
//
// ****************************************************************************

avtWorkerICAlgorithm::~avtWorkerICAlgorithm()
{
}

// ****************************************************************************
//  Method: avtWorkerICAlgorithm::HandleLatencyTimer
//
//  Purpose:
//      Handle latency timer.
//
//  Programmer: Dave Pugmire
//  Creation:   April 2, 2009
//
//
//  Modifications:
//   Dave Pugmire, Fri Sep 25 15:35:32 EDT 2009
//   Record latency.
//
// ****************************************************************************

void
avtWorkerICAlgorithm::HandleLatencyTimer(int activeICCnt, bool checkMaxLatency)
{
    if (latencyTimer == -1 && activeICCnt == 0)
    {
        latencyTimer = visitTimer->StartTimer();
        debug1<<"++++++++++++++++++++++++++++++++++++++++++Begin latency!\n";
    }
    else if (latencyTimer != -1 && activeICCnt > 0)
    {
        double t = visitTimer->StopTimer(latencyTimer, "Latency");
        latencyHistory.push_back(t);
        debug1<<"------------------------------------------End latency: time= "<<t<<endl;
        LatencyTime.value += t;
        
        if (checkMaxLatency && t > MaxLatencyTime.value)
            MaxLatencyTime.value = t;
        latencyTimer = -1;
    }
}



// ****************************************************************************
//  Method: avtWorkerICAlgorithm::Initialize
//
//  Purpose:
//      Initialize the worker
//
//  Programmer: Dave Pugmire
//  Creation:   January 27, 2009
//
//  Modifications:
//
//   Dave Pugmire, Tue Mar 10 12:41:11 EDT 2009
//   Generalized domain to include domain/time. Pathine cleanup.
//
// ****************************************************************************

void
avtWorkerICAlgorithm::Initialize(std::vector<avtIntegralCurve *> &seedPts)
{
    avtManagerWorkerICAlgorithm::Initialize(seedPts);
    status.resize(NUM_DOMAINS,0);
    prevStatus.resize(NUM_DOMAINS,0);
    numTerminated = 0;
    workToDo = false;
}


// ****************************************************************************
//  Method: avtWorkerICAlgorithm::UpdateStatus
//
//  Purpose:
//      Update status vectors.
//
//  Programmer: Dave Pugmire
//  Creation:   January 27, 2009
//
//  Modifications:
//
// ****************************************************************************

void
avtWorkerICAlgorithm::UpdateStatus()
{
    for (int i = 0; i < status.size(); i++)
        status[i] = 0;

    // Set 0,1 for domains loaded.
    for (int i = 0; i < NUM_DOMAINS; i++)
    {
        BlockIDType d = IdxToDom(i);
        status[i] = (DomainLoaded(d) ? 1: 0);
    }
    
    //Increment/decrement all the integral curves we have.
    list<avtIntegralCurve *>::const_iterator s;
    
    bool prevWorkToDo = workToDo;
    workToDo = false;
    for (s = activeICs.begin(); s != activeICs.end(); ++s)
        if (DomainLoaded((*s)->domain))
        {
            status[DomToIdx( (*s)->domain )] ++;
            workToDo = true;
        }
        else
            status[DomToIdx( (*s)->domain )] --;

    for (s = oobICs.begin(); s != oobICs.end(); ++s)
    {
        if (DomainLoaded((*s)->domain))
        {
            status[DomToIdx( (*s)->domain )] ++;
            workToDo = true;
        }
        else
            status[DomToIdx( (*s)->domain )] --;
    }

    if (!workToDo)
    {
        debug1<<"Nothing to do.....\n";
    }
}


// ****************************************************************************
//  Method: avtWorkerICAlgorithm::SendStatus
//
//  Purpose:
//      Send status to manager.
//
//  Programmer: Dave Pugmire
//  Creation:   January 27, 2009
//
//  Modifications:
//
//  Dave Pugmire, Wed Mar 25 10:04:29 EDT 2009
//  Control print information for large domain problems.
//
// ****************************************************************************

void
avtWorkerICAlgorithm::SendStatus(bool forceSend)
{
    UpdateStatus();
    
    bool statusChanged = false;
    
    //See if anything changed.
    if (!forceSend)
    {
        for (int i = 0; i < status.size(); i++)
            if (status[i] != prevStatus[i])
            {
                statusChanged = true;
                break;
            }
    }

    if (forceSend || (numTerminated > 0) || statusChanged)
    {
        //Send the status message to manager.
        vector<int> msg;
        msg.push_back(MSG_STATUS);
        msg.push_back(numTerminated);
        for (int i = 0; i < status.size(); i++)
            msg.push_back(status[i]);

        debug1<<"Worker SendStatus: "<<numTerminated;
        if (NUM_DOMAINS < MAX_DOMAIN_PRINT) debug1<<msg;
        debug1<<endl;
        SendMsg(manager, msg);
        
        //Status just sent, reset.
        numTerminated = 0;
        for (int i = 0; i < status.size(); i++)
            prevStatus[i] = status[i];
    }
}


// ****************************************************************************
//  Method: avtWorkerICAlgorithm::RunAlgorithm
//
//  Purpose:
//      Execute the worker loop of the manager/worker algorithm.
//
//  Programmer: Dave Pugmire
//  Creation:   January 27, 2009
//
//   Dave Pugmire, Wed Mar 18 17:17:40 EDT 2009
//   Allow managers to share work loads.
//
//   Dave Pugmire, Mon Mar 23 12:48:12 EDT 2009
//   Changes related to trying to hide latency with workers.
//
//   Dave Pugmire, Wed Mar 25 09:10:52 EDT 2009
//   Enable latency.
//
//   Dave Pugmire, Sat Mar 28 10:04:01 EDT 2009
//   Resend status if getting no work from manager. This is related to the tmp
//   fix above in the manager.
//  
//   Dave Pugmire, Wed Apr  1 11:21:05 EDT 2009
//   Latency time was not always being reported accurately. Only send the 
//   latency saving status update once.
//
//   Dave Pugmire, Thu Sep 24 13:52:59 EDT 2009
//   Change Execute to RunAlgorithm.
//   
//   Dave Pugmire, Fri Sep 25 15:35:32 EDT 2009
//   Add auto-load code for workers.
//
//   Dave Pugmire, Tue Jan 18 06:58:40 EST 2011
//   Regression fix. The old recvICs code was doing a domain load, as a side effect.
//
// ****************************************************************************

void
avtWorkerICAlgorithm::RunAlgorithm()
{
    list<avtIntegralCurve *>::const_iterator si;
    int timer = visitTimer->StartTimer();

    //Send initial status.
    SendStatus(true);
    Barrier();
    HandleLatencyTimer(activeICs.size());
    
    while (1)
    {
        //Fill oobICs list.
        list<avtIntegralCurve *>::iterator si = activeICs.begin();
        while (si != activeICs.end())
        {
            if (!DomainLoaded((*si)->domain))
            {
                oobICs.push_back(*si);
                si = activeICs.erase(si);
            }
            else
                ++si;
        }
                
        bool done = false, newMsgs = false, forceSend = false;
        bool sentLatencySavingStatus = false;
        
        //Check for a case4A overide...
        if (WORKER_AUTO_LOAD_DOMS && activeICs.empty() && (oobICs.size() > case4AThreshold))
        {
            debug1<<"Checking for 4A.... "<<endl;
            int candidate = -1;
            for (int i = 0; i < NUM_DOMAINS; i++)
                if (status[i] < 0 && status[i] <= case4AThreshold)
                {
                    if (candidate == -1 || (status[i] < status[candidate]))
                        candidate = i;
                }

            if (candidate != -1)
            {
                //Force send the status with this new domain loaded.
                status[candidate] = abs(status[candidate])+1;
                SendStatus(true);
                
                //Get the domain, and return to the top of the while loop.
                BlockIDType dom = IdxToDom(candidate);
                GetDomain(dom);
                activeICs.splice(activeICs.end(), oobICs);
                oobICs.clear();
                continue; //Drop out of loop, recompute active/oobs.
            }
        }

        HandleLatencyTimer(activeICs.size());
        while (!activeICs.empty() && !done)
        {
            avtIntegralCurve *s = activeICs.front();

            forceSend = false;
            if (activeICs.size() <= LATENCY_SEND_CNT)
            {
                if( !sentLatencySavingStatus)
                {
                    SendStatus(true);
                    LatencySavingCnt.value++;
                    debug1<<"Latency saving sendStatus"<<endl;
                }
                forceSend = true;
                sentLatencySavingStatus = true;
            }
            else
                sentLatencySavingStatus = false;
            
            activeICs.pop_front();
            debug1<<"Integrate "<<s->domain<<".....";
            AdvectParticle(s);
            if (s->status != avtIntegralCurve::STATUS_OK)
            {
                terminatedICs.push_back(s);
                numTerminated++;
                debug1<<"TERM. nT= "<<numTerminated<<endl;
            }
            else
            {
                debug1<<"OOB. dom= "<<s->domain<<endl;
                if (DomainLoaded(s->domain))
                    activeICs.push_back(s);
                else
                    oobICs.push_back(s);
            }
            
            ProcessMessages(done, newMsgs);
        }
        
        if (done)
            break;

        activeICs.splice(activeICs.end(), oobICs);
        oobICs.clear();
        
        //See if we have any ICs.
        list<avtIntegralCurve*> newICs;
        if (RecvICs(newICs))
        {
            list<avtIntegralCurve*>::iterator it;
            for (it = newICs.begin(); it != newICs.end(); it++)
                if (! DomainLoaded((*it)->domain))
                    GetDomain((*it)->domain);
            activeICs.splice(activeICs.end(), newICs);
        }
        ProcessMessages(done, newMsgs);
        CheckPendingSendRequests();

        if (done)
            break;
        SendStatus(forceSend);
        
        //Nothing to do, take a snooze....
        if (!workToDo)
        {
            NSleep();
        }
    }

    HandleLatencyTimer(0,false);
    CheckPendingSendRequests();

    debug1<<"Worker done: activeICs= "<<activeICs.size()<<" oobICs= "<<oobICs.size()<<endl;
    if (!activeICs.empty())
    {
        debug1<<"activeproblem "<<endl;
        terminatedICs.splice(terminatedICs.end(), activeICs);
    }
    if (!oobICs.empty())
    {
        debug1<<"oobproblem "<<endl;
        terminatedICs.splice(terminatedICs.end(), oobICs);
    }

    TotalTime.value += visitTimer->StopTimer(timer, "Execute");
}


// ****************************************************************************
//  Method: avtWorkerICAlgorithm::ProcessMessages
//
//  Purpose:
//      Processes messages from manager.
//
//  Programmer: Dave Pugmire
//  Creation:   January 27, 2009
//
//  Modifications:
//
//   Dave Pugmire, Tue Mar 10 12:41:11 EDT 2009
//   Generalized domain to include domain/time. Pathine cleanup.
//
//   Dave Pugmire, Mon Mar 16 15:05:14 EDT 2009
//   Bug fix. Didn't use new BlockIDType structure for MSG_SEND_IC.
//
//   Dave Pugmire, Wed Mar 18 21:55:32 EDT 2009
//   Improve the logic for integral curve offloading. Only offload
//   integral curves in unloaded domains.
//
// ****************************************************************************

void
avtWorkerICAlgorithm::ProcessMessages(bool &done, bool &newMsgs)
{
    vector<MsgCommData> msgs;
    RecvMsg(msgs);
    
    done = false;
    newMsgs = (msgs.size() > 0);
    
    for (int i = 0; i < msgs.size(); i++)
    {
        vector<int> &msg = msgs[i].message;
        int src = msgs[i].rank;
        int msgType = msg[0];
        
        if (msgType == MSG_DONE)
        {
            done = true;
            break;
        }
        
        //Load this domain.
        else if (msgType == MSG_LOAD_DOMAIN)
        {
            BlockIDType dom(msg[1], msg[2]);
            debug1<<"MSG: LoadDomain( "<<dom<<")\n";
            GetDomain(dom);
        }

        //Offload unloaded domains.
        else if (msgType == MSG_OFFLOAD_IC ||
                 msgType == MSG_SEND_IC_HINT)
        {
            debug1<<"Worker: MSG_OFFLOAD_IC I have "<<activeICs.size()<<" to offer"<<endl;
            debug1<<msg<<endl;
            
            int dst = msg[1];
            int numDoms = msg[2];
            int num = 10*maxCnt;
            if (msgType == MSG_OFFLOAD_IC)
                num = 10*maxCnt;
            else
                num = maxCnt;

            vector< avtIntegralCurve *> sendICs;

            for (int d = 0; d < numDoms; d++)
            {
                int domIdx = msg[3+d];
                BlockIDType dom = IdxToDom(domIdx);
                
                list<avtIntegralCurve *>::iterator s = activeICs.begin();
                while (s != activeICs.end() &&
                       sendICs.size() < num)
                {
                    if (msgType == MSG_OFFLOAD_IC)
                    {
                        if ((*s)->domain == dom &&
                            !DomainLoaded(dom))
                        {
                            sendICs.push_back(*s);
                            s = activeICs.erase(s);
                            numTerminated++;
                        }
                        else
                            s++;
                    }
                    else
                    {
                        if ((*s)->domain == dom)
                        {
                            sendICs.push_back(*s);
                            s = activeICs.erase(s);
                        }
                        else
                            s++;
                    }
                }
            }
            
            if (sendICs.size() > 0)
            {
                debug1<<"OFFLOAD: Send "<<sendICs.size()<<" to "<<dst<<endl;
                SendICs(dst, sendICs);
                OffloadCnt.value += sendICs.size();
            }
        }
        
        //Send integral curves to another worker.
        else if (msgType == MSG_SEND_IC)
        {
            int dst = msg[1];
            BlockIDType dom = IdxToDom(msg[2]);
            int num = msg[3];

            debug1<<"MSG: Send "<<num<<" x dom= "<<dom<<" to "<<dst;
            list<avtIntegralCurve *>::iterator s = activeICs.begin();
            vector< avtIntegralCurve *> sendICs;
            while (s != activeICs.end() &&
                   sendICs.size() < num)
            {
                if ((*s)->domain == dom)
                {
                    sendICs.push_back(*s);
                    s = activeICs.erase(s);
                }
                else
                    ++s;
            }

            debug1<<" sent "<<sendICs.size()<<endl;
            if (sendICs.size() > 0)
                SendICs(dst, sendICs);
        }
    }
}


// ****************************************************************************
//  Method: WorkerInfo::WorkerInfo
//
//  Purpose:
//      WorkerInfo constructor
//
//  Programmer: Dave Pugmire
//  Creation:   January 27, 2009
//
//   Dave Pugmire, Wed Mar 18 17:17:40 EDT 2009
//   Allow managers to share work loads.
//   
// ****************************************************************************

WorkerInfo::WorkerInfo( int r, int nDomains )
{
    justUpdated = false;
    initialized = false;
    canGive = canAccept = icCount = icLoadedCount = icOOBCount = 0;
    domLoadedCount = 0;
    domainCnt.resize(nDomains, 0);
    domainLoaded.resize(nDomains, false);
    rank = r;
}

// ****************************************************************************
//  Method: WorkerInfo::AddIC
//
//  Purpose:
//      Update when passing a IC.
//
//  Programmer: Dave Pugmire
//  Creation:   January 27, 2009
//
//  Modifications:
//
//  Dave Pugmire, Sat Mar 28 10:04:01 EDT 2009
//  Use the domainCache to report coming purges.
//
// ****************************************************************************

void
WorkerInfo::AddIC(int icDomain, int domCache)
{
    bool underPurgeLimit = (domLoadedCount <= domCache);
    //We assume that it will get loaded..
    if (domainLoaded[icDomain] == false)
        domLoadedCount++;
    domainLoaded[icDomain] = true;
    domainCnt[icDomain]++;
    icCount++;
    icLoadedCount++;
    justUpdated = false;
    
    if (underPurgeLimit && domLoadedCount > domCache)
        debug1<<"WARNING: "<<rank<<" Purge is coming. "<<domLoadedCount<<endl;
    if (domainHistory.size() == 0 ||
        (domainHistory.size() > 0 && icDomain != domainHistory[domainHistory.size()-1]))
        domainHistory.push_back(icDomain);      
}

// ****************************************************************************
//  Method: WorkerInfo::LoadDom
//
//  Purpose:
//      Update when loading a domain.
//
//  Programmer: Dave Pugmire
//  Creation:   January 27, 2009
//
// ****************************************************************************

void
WorkerInfo::LoadDom( int icDomain )
{
    bool underPurgeLimit = (domLoadedCount <= 3);
    if (domainLoaded[icDomain] == false)
        domLoadedCount++;
    
    domainLoaded[icDomain] = true;
    icLoadedCount += domainCnt[icDomain];
    icOOBCount -= domainCnt[icDomain];
    
    if (underPurgeLimit && domLoadedCount > 3)
        debug1<<"WARNING: "<<rank<<" Purge is coming!\n";
    
    if (domainHistory.size() == 0 ||
        (domainHistory.size() > 0 && icDomain != domainHistory[domainHistory.size()-1]))
        domainHistory.push_back(icDomain);
}


// ****************************************************************************
//  Method: WorkerInfo::RemoveIC
//
//  Purpose:
//      Update when removing a domain.
//
//  Programmer: Dave Pugmire
//  Creation:   January 27, 2009
//
// ****************************************************************************
    
void
WorkerInfo::RemoveIC( int dom )
{
    domainCnt[dom]--;
    //We assume that it will get loaded..
    icCount--;
    if (domainLoaded[dom])
        icLoadedCount--;
    else
        icOOBCount--;
    justUpdated = false;
}

// ****************************************************************************
//  Method: WorkerInfo::Update
//
//  Purpose:
//      Update with a new status from the worker.
//
//  Programmer: Dave Pugmire
//  Creation:   January 27, 2009
//
//   Dave Pugmire, Wed Mar 18 17:17:40 EDT 2009
//   Allow managers to share work loads.
//   
// ****************************************************************************

void
WorkerInfo::Update( vector<int> &status, bool debug )
{
    justUpdated = true;
    initialized = true;
    icCount = 0;
    icLoadedCount = 0;
    icOOBCount = 0;
    domLoadedCount = 0;
    
    for (int i = 0; i < domainCnt.size(); i++)
        domainCnt[i] = 0;
    for (int i = 0; i < domainLoaded.size(); i++)
        domainLoaded[i] = false;

    for (int i = 0; i < domainCnt.size(); i++)
    {
        int cnt = status[i];
        if (cnt > 0)
        {
            domainCnt[i] = (cnt-1);
            domainLoaded[i] = true;
            icCount += (cnt-1);
            icLoadedCount += (cnt-1);
        }
        else if (cnt < 0)
        {
            domainCnt[i] = -cnt;
            domainLoaded[i] = false;
            icCount += (-cnt);
            icOOBCount += (-cnt);
        }
    }
    
    for (int i = 0; i < domainLoaded.size(); i++)
        domLoadedCount += (domainLoaded[i] ? 1 : 0);
        
    if (debug)
        Debug();  
}

// ****************************************************************************
//  Method: WorkerInfo::Debug
//
//  Purpose:
//      Print your self out.
//
//  Programmer: Dave Pugmire
//  Creation:   January 27, 2009
//
//  Modifications:
//
//  Dave Pugmire, Wed Mar 25 10:04:29 EDT 2009
//  Control print information for large domain problems.
//  
//   Dave Pugmire, Wed Apr  1 11:21:05 EDT 2009
//   Print worker status based on LATENCY_SEND_CNT.
//   
// ****************************************************************************

void
WorkerInfo::Debug()
{
    bool slacker = (icLoadedCount == 0);
    debug1<<setw(2)<<rank;
    debug1<<": "<<setw(3)<<icCount<<" ("<<setw(3)<<icLoadedCount<<", "<<setw(3)<<icOOBCount<<") [";
    if (domainCnt.size() < MAX_DOMAIN_PRINT)
    {
        for (int i = 0; i < domainCnt.size(); i++)
        {
            int N = 0;
            int cnt = domainCnt[i];
            if (domainLoaded[i])
                N = (cnt > 0 ? cnt+1 : 1);
            else
                N = -cnt;
            
            debug1<<setw(4)<<N<<" ";
        }
    }
    debug1<<"] ("<<domLoadedCount<<")";
    debug1<< (slacker ? "S" : " ");
    if (justUpdated)
    {
        debug1<<" ***";
        if (icLoadedCount <= LATENCY_SEND_CNT)
            debug1<<" SLACKER: "<<rank;
        else
            debug1<<" UPDATE: "<<rank;              
    }
    debug1<<endl;
}


#endif

#endif
