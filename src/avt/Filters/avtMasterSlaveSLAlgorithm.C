/*****************************************************************************
*
* Copyright (c) 2000 - 2008, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400142
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
//                              avtMasterSlaveSLAlgorithm.C                  //
// ************************************************************************* //

#include "avtMasterSlaveSLAlgorithm.h"
#include <TimingsManager.h>
#include <iostream>
#include <iomanip>

using namespace std;

#ifdef PARALLEL

int avtMasterSlaveSLAlgorithm::MSG_STATUS = 420003;
int avtMasterSlaveSLAlgorithm::MSG_DONE = 420004;
int avtMasterSlaveSLAlgorithm::MSG_SEND_SL = 420005;
int avtMasterSlaveSLAlgorithm::MSG_LOAD_DOMAIN = 420006;
int avtMasterSlaveSLAlgorithm::MSG_SEND_SL_HINT = 420007;
int avtMasterSlaveSLAlgorithm::MSG_FORCE_SEND_STATUS = 420008;
int avtMasterSlaveSLAlgorithm::MSG_MASTER_STATUS = 420009;
int avtMasterSlaveSLAlgorithm::MSG_OFFLOAD_SL = 420010;

// ****************************************************************************
//  Method: avtMasterSlaveSLAlgorithm::Create
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
//  Allow masters to share work loads.
//
// ****************************************************************************

avtMasterSlaveSLAlgorithm*
avtMasterSlaveSLAlgorithm::Create(avtStreamlineFilter *slFilter,
                                  int maxCount,
                                  int rank,
                                  int nProcs,
                                  int workGroupSz)
{
    debug1<<"avtMasterSlaveSLAlgorithm::Create\n";
    
    avtMasterSlaveSLAlgorithm *algo = NULL;
    int numMasters = 1;
    int extra = 0;
    
    if (nProcs < workGroupSz)
        workGroupSz = nProcs;
    else
    {
        numMasters = nProcs/workGroupSz;
        extra = nProcs%workGroupSz;
    }
    
    int extraPerMaster = extra / numMasters;
    int extraTill = extra % numMasters;

    debug1<<"numMasters= "<<numMasters<<endl;

    int nextSlave = numMasters;
    vector<vector<int> > masterList(numMasters);
    for (int m = 0; m < numMasters; m++)
    {
        masterList[m].resize(2);
        int N = workGroupSz-1 + extraPerMaster;
        if (m < extraTill)
            N++;

        masterList[m][0] = nextSlave;
        masterList[m][1] = (nextSlave+N-1);
        debug1<<"Master: "<<m<<" ["<<masterList[m][0]<<" "<<masterList[m][1]<<"]"<<endl;
        nextSlave += N;
    }

    //I am a master.
    if (rank < numMasters)
    {
        vector<int> slaves;
        for (int i = masterList[rank][0]; i <= masterList[rank][1]; i++)
            slaves.push_back(i);
        
        vector<int> masters;
        int master;
        if (rank == 0)
        {
            master = -1;
            if (numMasters > 1)
                for (int i = 0; i < numMasters; i++)
                    masters.push_back(i);
        }
        else
        {
            master = 0;
        }
        
        algo = new avtMasterSLAlgorithm(slFilter, maxCount, workGroupSz, slaves, master, masters);

        debug1<<"I am a master. My slaves are: "<<slaves<<endl;
        debug1<<"My masterMaster is "<<master<<". Masters= "<<masters<<endl;
    }
    
    // I'm a slave. Look for my master.
    else
    {
        for (int m = 0; m < numMasters; m++)
            if (rank >= masterList[m][0] && rank <= masterList[m][1])
            {
                debug1<<"I am a slave. My master is "<<m<<endl;
                algo = new avtSlaveSLAlgorithm(slFilter, maxCount, m);
                break;
            }
    }

    if (algo == NULL)
        EXCEPTION0(ImproperUseException);
    
    return algo;
}


// ****************************************************************************
//  Method: avtMasterSlaveSLAlgorithm::avtMasterSlaveSLAlgorithm
//
//  Purpose:
//      avtMasterSlaveSLAlgorithm constructor.
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
// ****************************************************************************

avtMasterSlaveSLAlgorithm::avtMasterSlaveSLAlgorithm(avtStreamlineFilter *slFilter,
                                                     int maxCount)
    : avtParSLAlgorithm(slFilter),
      SleepTime("sleepT"), LatencyTime("latT"), MaxLatencyTime("maxLatT"), SleepCnt("sleepC"),
      LatencySavingCnt("latSaveCnt")
{
    NUM_DOMAINS = numDomains * numTimeSteps;
    maxCnt = maxCount;
    sleepMicroSec = 100;
    latencyTimer = -1;

    // Msg type, numTerminated, domain vector.
    statusMsgSz = 2 + NUM_DOMAINS;
}

// ****************************************************************************
//  Method: avtMasterSlaveSLAlgorithm::~avtMasterSlaveSLAlgorithm
//
//  Purpose:
//      avtMasterSlaveSLAlgorithm destructor
//
//  Programmer: Dave Pugmire
//  Creation:   January 27, 2009
//
// ****************************************************************************

avtMasterSlaveSLAlgorithm::~avtMasterSlaveSLAlgorithm()
{
}

// ****************************************************************************
//  Method: avtMasterSlaveSLAlgorithm::Initialize
//
//  Purpose:
//      Initializization.
//
//  Programmer: Dave Pugmire
//  Creation:   January 27, 2009
//
// ****************************************************************************

void
avtMasterSlaveSLAlgorithm::Initialize(std::vector<avtStreamlineWrapper *> &seedPts)
{
    avtParSLAlgorithm::Initialize(seedPts);
}


// ****************************************************************************
//  Method: avtMasterSlaveSLAlgorithm::Sleep
//
//  Purpose:
//      Sleep for a spell
//
//  Programmer: Dave Pugmire
//  Creation:   January 27, 2009
//
// ****************************************************************************

void
avtMasterSlaveSLAlgorithm::Sleep()
{
    if (sleepMicroSec > 0)
    {
        //debug1<<"Sleep for "<<sleepMicroSec<<" microSec\n";
        int sleepTimer = visitTimer->StartTimer();
        struct timespec ts = {0, sleepMicroSec*1000};
        nanosleep(&ts, 0);
        SleepTime.value += visitTimer->StopTimer(sleepTimer, "SleepTimer");
        SleepCnt.value++;
    }
}

// ****************************************************************************
//  Method: avtMasterSlaveSLAlgorithm::CompileTimingStatistics
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
avtMasterSlaveSLAlgorithm::CompileTimingStatistics()
{
    avtParSLAlgorithm::CompileTimingStatistics();

    ComputeStatistic(SleepTime);
    ComputeStatistic(LatencyTime);
    ComputeStatistic(MaxLatencyTime);
}

// ****************************************************************************
//  Method: avtMasterSlaveSLAlgorithm::CompileCounterStatistics
//
//  Purpose:
//      Calculate statistics.
//
//  Programmer: Dave Pugmire
//  Creation:   January 27, 2009
//
// ****************************************************************************

void
avtMasterSlaveSLAlgorithm::CompileCounterStatistics()
{
    avtParSLAlgorithm::CompileCounterStatistics();
    ComputeStatistic(SleepCnt);
    ComputeStatistic(LatencySavingCnt);
}

// ****************************************************************************
//  Method: avtMasterSlaveSLAlgorithm::CalculateExtraTime
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
avtMasterSlaveSLAlgorithm::CalculateExtraTime()
{
    avtParSLAlgorithm::CalculateExtraTime();
    
    if (SleepTime.value > 0.0)
        ExtraTime.value -= SleepTime.value;
}

// ****************************************************************************
//  Method: avtMasterSlaveSLAlgorithm::ReportTimings
//
//  Purpose:
//      Report timings.
//
//  Programmer: Dave Pugmire
//  Creation:   January 27, 2009
//
// ****************************************************************************

void
avtMasterSlaveSLAlgorithm::ReportTimings(ostream &os, bool totals)
{
    avtParSLAlgorithm::ReportTimings(os, totals);
    
    PrintTiming(os, "SleepTime", SleepTime, TotalTime, totals);
    PrintTiming(os, "LatencyTime", LatencyTime, TotalTime, totals);
    PrintTiming(os, "MaxLatTime", MaxLatencyTime, TotalTime, totals);
}

// ****************************************************************************
//  Method: avtMasterSlaveSLAlgorithm::ReportCounters
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
// ****************************************************************************

void
avtMasterSlaveSLAlgorithm::ReportCounters(ostream &os, bool totals)
{
    avtParSLAlgorithm::ReportCounters(os, totals);
    PrintCounter(os, "SleepCnt", SleepCnt, totals);
    PrintCounter(os, "LSaveCnt", LatencySavingCnt, totals);
}


// ****************************************************************************
//  Method: avtMasterSLAlgorithm::avtMasterSLAlgorithm
//
//  Purpose:
//      avtMasterSLAlgorithm constructor.
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
//  Allow masters to share work loads.
//  
// ****************************************************************************

avtMasterSLAlgorithm::avtMasterSLAlgorithm(avtStreamlineFilter *slFilter,
                                           int maxCount,
                                           int workGrpSz,
                                           vector<int> &slaves,
                                           int mst,
                                           vector<int> &masters)
    : avtMasterSlaveSLAlgorithm(slFilter, maxCount)
{
    workGroupSz = workGrpSz;
    //Create my slaves.
    for (int i = 0; i < slaves.size(); i++)
        slaveInfo.push_back(SlaveInfo(slaves[i], NUM_DOMAINS));

    //Create any masters to manage.
    master = mst;
    for (int i = 0; i < masters.size(); i++)
        masterInfo.push_back(SlaveInfo(masters[i], NUM_DOMAINS));
    
    slDomCnts.resize(NUM_DOMAINS,0);
    domLoaded.resize(NUM_DOMAINS,0);

    case1Cnt = 0;
    case2Cnt = 0;
    case3Cnt = 0;
    case4Cnt = 0;
    case5Cnt = 0;
}


// ****************************************************************************
//  Method: avtMasterSLAlgorithm::~avtMasterSLAlgorithm
//
//  Purpose:
//      avtMasterSLAlgorithm destructor.
//
//  Programmer: Dave Pugmire
//  Creation:   January 27, 2009
//
// ****************************************************************************

avtMasterSLAlgorithm::~avtMasterSLAlgorithm()
{
}

// ****************************************************************************
//  Method: avtMasterSLAlgorithm::Initialize
//
//  Purpose:
//      Initializization
//
//  Programmer: Dave Pugmire
//  Creation:   January 27, 2009
//
//  Modifications:
//  
//  Dave Pugmire, Wed Mar 18 17:17:40 EDT 2009
//  Allow masters to share work loads.  
//
// ****************************************************************************

void
avtMasterSLAlgorithm::Initialize(std::vector<avtStreamlineWrapper *> &seedPts)
{
    SortStreamlines(seedPts);
    avtMasterSlaveSLAlgorithm::Initialize(seedPts);
    int nSeeds = seedPts.size();

    int numMasters;
    if (nProcs < workGroupSz)
        numMasters = 1;
    else
        numMasters = nProcs/workGroupSz;
    int ptsPerMaster = nSeeds/numMasters;

    int i0 = rank*ptsPerMaster;
    int i1 = i0+ptsPerMaster;
    if (rank == (numMasters-1))
        i1 = nSeeds;

    debug1<<"I have seeds: "<<i0<<" --> "<<i1<<endl;
    
    //Delete the seeds I don't need.
    for (int i = 0; i < i0; i++)
        delete seedPts[i];
    for (int i = i1; i < nSeeds; i++)
        delete seedPts[i];
    
    for (int i = i0; i < i1; i++)
        activeSLs.push_back(seedPts[i]);
    
    workGroupActiveSLs = activeSLs.size();
    done = false;
    slaveUpdate = false;
    masterUpdate = false;
    status.resize(NUM_DOMAINS,0);
    prevStatus.resize(NUM_DOMAINS,0);
}

// ****************************************************************************
//  Method: avtMasterSLAlgorithm::CompileTimingStatistics
//
//  Purpose:
//      Calculate statistics. For the master, we need to be a little tricky.
//      The master doesn't do a lot of things, so we don't want to skew the
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
avtMasterSLAlgorithm::CompileTimingStatistics()
{
    LatencyTime.value = -1;
    MaxLatencyTime.value = -1;
    IOTime.value = -1;
    IntegrateTime.value = -1;
    SortTime.value = -1;
    avtMasterSlaveSLAlgorithm::CompileTimingStatistics();
}

// ****************************************************************************
//  Method: avtMasterSLAlgorithm::CompileCounterStatistics
//
//  Purpose:
//      Calculate statistics. For the master, we need to be a little tricky.
//      The master doesn't do a lot of things, so we don't want to skew the
//      statistics. So, set the timer/counters to -1 and then ignore these values
//      when reporting statistics.
//
//  Programmer: Dave Pugmire
//  Creation:   January 27, 2009
//
// ****************************************************************************

void
avtMasterSLAlgorithm::CompileCounterStatistics()
{
    IntegrateCnt.value = -1;
    DomLoadCnt.value = -1;
    DomPurgeCnt.value = -1;
    avtMasterSlaveSLAlgorithm::CompileCounterStatistics();
}

// ****************************************************************************
//  Method: avtMasterSLAlgorithm::PrintStatus
//
//  Purpose:
//      Display the slave status
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
//  Allow masters to share work loads.
//  
// ****************************************************************************

void
avtMasterSLAlgorithm::PrintStatus()
{
    if (masterInfo.size() > 0)
    {
        debug1<<"Masters:\n";
        for (int i = 0; i < masterInfo.size(); i++)
            masterInfo[i].Debug();
        debug1<<endl;
    }

    debug1<<"DOM:               [";
    for ( int i = 0; i < NUM_DOMAINS; i++)
        debug1<<setw(4)<<i<<" ";
    debug1<<"]\n";
    debug1<<"Master:            [";
    for ( int i = 0; i < NUM_DOMAINS; i++)
        debug1<<setw(4)<<slDomCnts[i]<<" ";
    debug1<<"]\n";
    
    for (int i = 0; i < slaveInfo.size(); i++)
        slaveInfo[i].Debug();
    debug1<<"DCounts:           [";
    for ( int i = 0; i < NUM_DOMAINS; i++)
        debug1<<setw(4)<<domLoaded[i]<<" ";
    debug1<<"]\n";

    vector<int> slaveSLs(NUM_DOMAINS,0);
    for (int i = 0; i < slaveInfo.size(); i++)
        for (int j = 0; j < NUM_DOMAINS; j++)
            slaveSLs[j] += slaveInfo[i].domainCnt[j];
    debug1<<"SCounts:           [";
    int cnt = 0;
    for ( int i = 0; i < NUM_DOMAINS; i++)
    {
        debug1<<setw(4)<<slaveSLs[i]<<" ";
        cnt += slaveSLs[i];
    }
    debug1<<"] ("<<cnt<<")"<<endl;
    debug1<<endl;
}


// ****************************************************************************
//  Method: avtMasterSLAlgorithm::UpdateSlaveStatus
//
//  Purpose:
//      Update the slave status.
//
//  Programmer: Dave Pugmire
//  Creation:   January 27, 2009
//
//  Modifications:
//  
//  Dave Pugmire, Wed Mar 18 17:17:40 EDT 2009
//  Allow masters to share work loads.    
//
// ****************************************************************************
bool
avtMasterSLAlgorithm::UpdateSlaveStatus(vector<int> &status)
{
    int src = status[0];
    int msg = status[1];
    int nTerm = status[2];
    
    workGroupActiveSLs -= nTerm;
    debug1<<"SlaveStatus: "<<src<<" "<<status<<endl;
    
    for (int i = 0; i < slaveInfo.size(); i++)
    {
        if (slaveInfo[i].rank == src)
        {
            debug5<<"Update for rank= "<<src<<endl;
            vector<int> domStatus;
            for (int j = 3; j < status.size(); j++)
                domStatus.push_back(status[j]);
            slaveInfo[i].Update(domStatus, false);
            break;
        }
    }
}

// ****************************************************************************
//  Method: avtMasterSLAlgorithm::Execute
//
//  Purpose:
//      Execute the master loop of the master/slave algorithm.
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
//  Allow masters to share work loads.       
//
// ****************************************************************************

void
avtMasterSLAlgorithm::Execute()
{
    int timer = visitTimer->StartTimer();
    
    //Each slave has to send information. Don't proceed until they have.
    Barrier();

    while (!done)
    {
        debug1<<"Looping SLs= "<<workGroupActiveSLs<<endl;
        ProcessMessages();
        ProcessNewStreamlines();
        ManageWorkgroup();
        
        CheckPendingSendRequests();

        //See if we are done.
        PostLoopProcessing();
    }

    // We are done, tell all slaves to pack it up.
    SendAllSlavesMsg(MSG_DONE);
    TotalTime.value += visitTimer->StopTimer(timer, "Execute");
}

// ****************************************************************************
//  Method: avtMasterSLAlgorithm::PostLoopProcessing()
//
//  Purpose:
//      See if we are done.
//
//  Programmer: Dave Pugmire
//  Creation:   March 18, 2009
//
// ****************************************************************************

void
avtMasterSLAlgorithm::PostLoopProcessing()
{
    SendStatus();

    //See if we are done.
    if (master == -1) //I'm root master.
    {
        debug1<<"See if we are done.\n";
        if (workGroupActiveSLs == 0)
        {
            debug1<<"I'm done!\n";
            done = true;
            for (int i = 0; i < masterInfo.size(); i++)
                if (masterInfo[i].slCount != 0)
                {
                    done = false;
                    break;
                }
            
            debug1<<"Done= "<<done<<endl;
            if (done)
            {
                for (int i = 0; i < masterInfo.size(); i++)
                {
                    vector<int> msg(1, MSG_DONE);
                    SendMsg(masterInfo[i].rank, msg);
                }
            }
        }
    }
}

// ****************************************************************************
//  Method: avtMasterSLAlgorithm::UpdateStatus
//
//  Purpose:
//      Update my status.
//
//  Programmer: Dave Pugmire
//  Creation:   March 18, 2009
//  
// ****************************************************************************

void
avtMasterSLAlgorithm::UpdateStatus()
{
    for (int i = 0; i < status.size(); i++)
        status[i] = 0;
    
    status[0] = workGroupActiveSLs;
    for (int i = 0; i < NUM_DOMAINS; i++)
        status[i+1] = domLoaded[i];
}


// ****************************************************************************
//  Method: avtMasterSLAlgorithm::SendStatus
//
//  Purpose:
//      Send master status to my master.
//
//  Programmer: Dave Pugmire
//  Creation:   March 18, 2009
//
// ****************************************************************************

void
avtMasterSLAlgorithm::SendStatus(bool forceSend)
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
    
    if (statusChanged)
    {
        vector<int> msg(NUM_DOMAINS+2);
        msg[0] = MSG_MASTER_STATUS;
        msg[1] = workGroupActiveSLs;
        for (int i = 0; i < NUM_DOMAINS; i++)
            msg[i+2] = domLoaded[i];

        debug1<<"MasterStatusSend: "<<msg<<endl;
        
        if (master != -1)
            SendMsg(master, msg);
        else
        {
            msg.insert(msg.begin(), rank);
            ProcessMasterUpdate(msg);
        }

        
        for (int i = 0; i < status.size(); i++)
            prevStatus[i] = status[i];
    }
}

// ****************************************************************************
//  Method: avtMasterSLAlgorithm::ProcessMessages
//
//  Purpose:
//      Handle incoming messages.
//
//  Programmer: Dave Pugmire
//  Creation:   March 18, 2009
//
// ****************************************************************************

void
avtMasterSLAlgorithm::ProcessMessages()
{
    debug1<<"avtMasterSLAlgorithm::ProcessMessages()\n";
    
    vector<vector<int> > msgs;
    RecvMsgs(msgs);

    for (int i = 0; i < msgs.size(); i++)
    {
        vector<int> &msg = msgs[i];
        int src = msg[0];
        int msgType = msg[1];

        if (msgType == MSG_DONE)
        {
            done = true;
            break;
        }
        else if (msgType == MSG_STATUS)
            ProcessSlaveUpdate(msg);
        else if (msgType == MSG_MASTER_STATUS)
            ProcessMasterUpdate(msg);
        else if (msgType == MSG_OFFLOAD_SL)
            ProcessOffloadSL(msg);
    }
}

// ****************************************************************************
//  Method: avtMasterSLAlgorithm::ProcessSlaveUpdate
//
//  Purpose:
//      Process status messages from slaves.
//
//  Programmer: Dave Pugmire
//  Creation:   March 18, 2009
//
// ****************************************************************************

void
avtMasterSLAlgorithm::ProcessSlaveUpdate(vector<int> &status)
{
    UpdateSlaveStatus(status);
    slaveUpdate = true;
}

// ****************************************************************************
//  Method: avtMasterSLAlgorithm::ProcessMasterUpdate
//
//  Purpose:
//      Process status messages from masters.
//
//  Programmer: Dave Pugmire
//  Creation:   March 18, 2009
//
// ****************************************************************************

void
avtMasterSLAlgorithm::ProcessMasterUpdate(vector<int> &status)
{
    int src = status[0];
    int msg = status[1];
    int nSLs = status[2];

    debug1<<"MasterUpdateStatus: "<<src<<status<<endl;
    
    for (int i = 0; i < masterInfo.size(); i++)
    {
        if (masterInfo[i].rank == src)
        {
            debug5<<"Update for rank= "<<src<<endl;
            vector<int> domStatus;
            for (int j = 3; j < status.size(); j++)
                domStatus.push_back(status[j]);
            masterInfo[i].Update(domStatus, false);
            masterInfo[i].slCount = nSLs;
            break;
        }
    }

    masterUpdate = true;
}

// ****************************************************************************
//  Method: avtMasterSLAlgorithm::ProcessOffloadSL
//
//  Purpose:
//      Offload work to another master.
//
//  Programmer: Dave Pugmire
//  Creation:   March 18, 2009
//
//  Modifications:
//
//  Dave Pugmire, Wed Mar 18 21:55:32 EDT 2009
//  Modify how masters handle offloading work to other masters.
//
// ****************************************************************************

void
avtMasterSLAlgorithm::ProcessOffloadSL(vector<int> &status)
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

    // Find slaves with domains send offload message.
    for (int i = 0; i < slaveInfo.size(); i++)
    {
        debug1<<" "<<slaveInfo[i].rank<<": "<<slaveInfo[i].slCount<<endl;
        if (slaveInfo[i].slCount > 0)
        {
            vector<int> domsToSend;
            for (int d = 0; d < doms.size(); d++)
            {
                if (slaveInfo[i].domainCnt[doms[d]] > 0)
                    domsToSend.push_back(doms[d]);
            }

            // This slave has some domains that this master has loaded.
            if (domsToSend.size() > 0)
            {
                vector<int> msg;
                msg.push_back(MSG_OFFLOAD_SL);
                msg.push_back(dst);
                msg.push_back(domsToSend.size());
                for (int j = 0; j < domsToSend.size(); j++)
                    msg.push_back(domsToSend[j]);
                //slaveInfo[i].RemoveSL(doms[d]);
                
                debug1<<"Send OFFLOAD MSG: "<<slaveInfo[i].rank<<" ==> "<<dst<<" "<<msg<<endl;
                SendMsg(slaveInfo[i].rank, msg);
            }
        }
    }

    masterUpdate = true;
}


// ****************************************************************************
//  Method: avtMasterSLAlgorithm::NewStreamlines
//
//  Purpose:
//      Handle incoming streamlines.
//
//  Programmer: Dave Pugmire
//  Creation:   March 18, 2009
//
// ****************************************************************************

void
avtMasterSLAlgorithm::ProcessNewStreamlines()
{
    list<avtStreamlineWrapper*> newSLs;
    RecvSLs(newSLs);
    if (!newSLs.empty())
    {
        debug1<<"avtMasterSLAlgorithm::ProcessNewStreamlines() cnt "<<workGroupActiveSLs<<" ==> ";
        workGroupActiveSLs += newSLs.size();
        debug1<<workGroupActiveSLs<<endl;

        activeSLs.splice(activeSLs.end(), newSLs);

        // We need to process for slaves now.
        slaveUpdate = true;
    }
}

// ****************************************************************************
//  Method: avtMasterSLAlgorithm::ManageWorkgroup
//
//  Purpose:
//      Manage workgroup.
//
//  Programmer: Dave Pugmire
//  Creation:   March 18, 2009
//
// ****************************************************************************

void
avtMasterSLAlgorithm::ManageWorkgroup()
{
    if (slaveUpdate)
    {
        //Update our counters.
        for (int i = 0; i < NUM_DOMAINS; i++)
        {
            slDomCnts[i] = 0;
            domLoaded[i] = 0;
        }
        
        list<avtStreamlineWrapper *>::const_iterator s;
        for (s = activeSLs.begin(); s != activeSLs.end(); ++s)
            slDomCnts[DomToIdx( (*s)->domain )] ++;
    
        for (int i = 0; i < slaveInfo.size(); i++)
            for ( int d = 0; d < NUM_DOMAINS; d++)
                if (slaveInfo[i].domainLoaded[d])
                    domLoaded[d]++;
    }
    
    PrintStatus();

    if (!slaveUpdate && !masterUpdate)
        Sleep();
    else
    {
        if (slaveUpdate)
            ManageSlaves();
        if (masterUpdate)
            ManageMasters();
    }

    debug1<<endl<<"Post-Mortem"<<endl;
    PrintStatus();

    slaveUpdate = false;
    masterUpdate = false;
}

// ****************************************************************************
//  Method: avtMasterSLAlgorithm::ManageSlaves
//
//  Purpose:
//      Manage slaves workgroup.
//
//  Programmer: Dave Pugmire
//  Creation:   March 18, 2009
//
// ****************************************************************************

void
avtMasterSLAlgorithm::ManageSlaves()
{
    /*
      Case1(case1Cnt);
      Case2(case2Cnt);
      int case3OverloadFactor = 10*maxCnt, case3NDomainFactor = 3*maxCnt;
      Case3(case3OverloadFactor, case3NDomainFactor, case3Cnt);
      Case4(0,case4Cnt);
    */
    
    Case4(3*maxCnt, case4Cnt);
    Case1(case1Cnt);
    //Case4(1*maxCnt, case4Cnt); //Remove this KILLED performance. ODD.
    Case2(case2Cnt);
    int case3OverloadFactor = 10*maxCnt, case3NDomainFactor = 3*maxCnt;
    Case3(case3OverloadFactor, case3NDomainFactor, case3Cnt);
    Case4(0, case4Cnt);
    
    /*
    Case5(maxCnt, true, case5Cnt);
    Case5(maxCnt, false, case5Cnt);

    //See who else still doesn't have work....
    FindSlackers();
    if (slackers.size() > 0)
        debug1<<"WE STILL HAVE SLACKERS\n";
    */
    
    //Resets.
    for (int i = 0; i < slaveInfo.size(); i++)
        slaveInfo[i].Reset();
}

// ****************************************************************************
//  Method: avtMasterSLAlgorithm::ManageMasters
//
//  Purpose:
//      Manage slaves workgroup.
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
avtMasterSLAlgorithm::ManageMasters()
{
    // Don't bother if no masters...
    if (masterInfo.size() == 0)
        return;

    // Get all the masters with no work.
    vector<int> idleMasters, busyMasters;
    for (int i = 0; i < masterInfo.size(); i++)
    {
        if (masterInfo[i].initialized)
        {
            if (masterInfo[i].slCount == 0)
                idleMasters.push_back(masterInfo[i].rank);
            else
                busyMasters.push_back(masterInfo[i].rank);
        }
    }

    // Nothing to do!
    if (idleMasters.size() == 0 ||
        busyMasters.size() == 0)
        return;
    
    //Randomize things.
    random_shuffle(idleMasters.begin(), idleMasters.end());
    random_shuffle(busyMasters.begin(), busyMasters.end());
    
    debug1<<"IdleMasters: "<<idleMasters<<endl;
    debug1<<"BusyMasters: "<<busyMasters<<endl;

    // Tell each busyMaster to offload to an idleMaster.
    int N = idleMasters.size();
    if (busyMasters.size() < N)
        N = busyMasters.size();
    
    //TODO: Should we wrap around?
    for (int i = 0; i < N; i++)
    {
        // Have busy
        vector<int> msg(NUM_DOMAINS+2,0);
        msg[0] = MSG_OFFLOAD_SL;
        msg[1] = idleMasters[i];
        for (int j = 0; j < NUM_DOMAINS; j++)
            msg[2+j] = masterInfo[idleMasters[i]].domainLoaded[j];
        
        debug1<<busyMasters[i]<<": MasterOffload to "<<idleMasters[i]<<" : "<<msg<<endl;
        if (busyMasters[i] == rank)
        {
            msg.insert(msg.begin(), rank);
            ProcessOffloadSL(msg);
        }
        else
            SendMsg(busyMasters[i], msg);
    }
}


// ****************************************************************************
//  Method: avtMasterSLAlgorithm::SendAllSlavesMsg
//
//  Purpose:
//      Send a message to all slaves.
//
//  Programmer: Dave Pugmire
//  Creation:   January 27, 2009
//
// ****************************************************************************

void
avtMasterSLAlgorithm::SendAllSlavesMsg(int msg)
{
    vector<int> msgVec(1,msg);
    SendAllSlavesMsg(msgVec);
}

// ****************************************************************************
//  Method: avtMasterSLAlgorithm::SendAllSlavesMsg
//
//  Purpose:
//      Send a message to all slaves.
//
//  Programmer: Dave Pugmire
//  Creation:   January 27, 2009
//
// ****************************************************************************

void
avtMasterSLAlgorithm::SendAllSlavesMsg(vector<int> &msg)
{
    for (int i = 0; i < slaveInfo.size(); i++)
        SendMsg(slaveInfo[i].rank, msg);
}

// ****************************************************************************
//  Method: avtMasterSLAlgorithm::FindSlackers
//
//  Purpose:
//      Find slaves with no work to do
//
//  Programmer: Dave Pugmire
//  Creation:   January 27, 2009
//
// ****************************************************************************

void
avtMasterSLAlgorithm::FindSlackers(int oobFactor,
                                   bool randomize,
                                   bool checkJustUpdated)
{
    slackers.resize(0);

    for (int i = 0; i < slaveInfo.size(); i++)
        if (slaveInfo[i].slLoadedCount == 0 ||
            (slaveInfo[i].justUpdated && checkJustUpdated))
        {
            if ( oobFactor != -1 &&
                 slaveInfo[i].slOOBCount > 0 &&
                 slaveInfo[i].slOOBCount < oobFactor)
                slackers.push_back(i);
            else
                slackers.push_back(i);
        }

    if (randomize)
        random_shuffle(slackers.begin(), slackers.end());
}


// ****************************************************************************
//  Method: avtMasterSLAlgorithm::Case1
//
//  Purpose:
//      Case1 of masterslave algorithm. Give SLs to slaves who have domain
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
// ****************************************************************************

void
avtMasterSLAlgorithm::Case1(int &counter)
{
    if (activeSLs.empty())
        return;
    
    FindSlackers();
    
    vector< vector< avtStreamlineWrapper *> > distributeSLs(nProcs);
    bool streamlinesToSend = false;
    
    for (int i = 0; i < slackers.size(); i++)
    {
        if (activeSLs.empty())
            break;
        
        int cnt = 0;
        int slackerRank = slaveInfo[slackers[i]].rank;
        list<avtStreamlineWrapper *>::iterator s = activeSLs.begin();

        while ( activeSLs.empty() && cnt < maxCnt)
        {
            int sDom = DomToIdx( (*s)->domain );
            if (slaveInfo[slackers[i]].domainLoaded[sDom])
            {
                distributeSLs[slackerRank].push_back(*s);
                slaveInfo[slackers[i]].AddSL(sDom);
                s = activeSLs.erase(s);
                streamlinesToSend = true;
                cnt++;
            }
            else
                ++s;
            if (s == activeSLs.end())
                break;
        }
        
        if (cnt > 0)
        {
            debug1<<"Case 1: "<<slackerRank<<" Send "<<cnt<<" SLs [";
            for (int j = 0; j < distributeSLs[slackerRank].size(); j++)
                debug1<<distributeSLs[slackerRank][j]->domain<<" ";
            debug1<<"]\n";
            counter++;
        }
    }

    if (streamlinesToSend)
    {
        int earlyTerminations = 0;
        ExchangeSLs(activeSLs, distributeSLs, earlyTerminations);
        
        if (earlyTerminations != 0)
            EXCEPTION0(ImproperUseException);
    }    
}


// ****************************************************************************
//  Method: avtMasterSLAlgorithm::Case2
//
//  Purpose:
//      Case2 of masterslave algorithm. Give SLs to slaves and force domain load
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

static bool domCntCompare( const int *a, const int *b) { return a[1] > b[1]; }

void
avtMasterSLAlgorithm::Case2(int &counter)
{
    if (activeSLs.empty())
        return;
    
    FindSlackers();
    
    debug1<<"Case2: slackers: "<<slackers.size()<<endl;
    vector< vector< avtStreamlineWrapper *> > distributeSLs(nProcs);
    bool streamlinesToSend = false;

    for (int s = 0; s < slackers.size(); s++)
    {
        if (activeSLs.empty())
            break;
        
        int slackerRank = slaveInfo[slackers[s]].rank;
    
        vector<int*> domCnts;
        for (int i = 0; i < slDomCnts.size(); i++)
            if (slDomCnts[i] > 0)
            {
                int *entry = new int[2];
                entry[0] = i;
                entry[1] = slDomCnts[i];
                domCnts.push_back(entry);
            }

        if (slDomCnts.size() == 0)
            continue;
        //Sort on SL count per domain.
        sort(domCnts.begin(),domCnts.end(), domCntCompare);
        if (false)
        {
            debug1<<"SL sort: [";
            for (int i = 0; i < domCnts.size(); i++)
                debug1<<domCnts[i][0]<<" "<<domCnts[i][1]<<", ";
            debug1<<"]\n";
        }

        int domToLoad = -1;
        for (int i = 0; i < domCnts.size(); i++)
        {
            bool anyLoaded = false;
            for (int j = 0; j < slaveInfo.size(); j++)
                if (slaveInfo[j].domainLoaded[domCnts[i][0]])
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

        list<avtStreamlineWrapper *>::iterator sl = activeSLs.begin();
        
        while ( !activeSLs.empty() && cnt < maxCnt)
        {
            int sDom = DomToIdx( (*sl)->domain );
            if (sDom == domToLoad && cnt < maxCnt)
            {
                distributeSLs[slackerRank].push_back(*sl);
                slaveInfo[slackers[s]].AddSL(sDom);
                cnt++;
                slDomCnts[domToLoad]--;
                sl = activeSLs.erase(sl);
            }
            else
                ++sl;
            if (sl == activeSLs.end())
                break;
        }
            
        for (int i = 0; i < domCnts.size(); i++)
            delete domCnts[i];

        if (cnt > 0)
        {
            streamlinesToSend = true;
            if (distributeSLs[slackerRank].size() > 0)
            {
                debug1<<"Case 2: "<<slackerRank<<" Send "<<cnt<<" SLs [";
                for (int j = 0; j < distributeSLs[slackerRank].size(); j++)
                    debug1<<distributeSLs[slackerRank][j]->domain<<" ";
                debug1<<"]\n";
                counter++;
            }
        }
    }
    
    if (streamlinesToSend)
    {
        for (int i = 0; i < nProcs; i++)
        {
            if (distributeSLs[i].size() > 0)
                SendSLs(i, distributeSLs[i]);
        }
    }    
}

// ****************************************************************************
//  Method: avtMasterSLAlgorithm::Case3
//
//  Purpose:
//      Case3 of masterslave algorithm. Send SLs to another slave.
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
//   Bug fix. Didn't use new DomainType structure for MSG_SEND_SL.
//   
//   Dave Pugmire, Wed Mar 18 17:17:40 EDT 2009
//   Allow masters to share work loads.
//
// ****************************************************************************

void
avtMasterSLAlgorithm::Case3(int overloadFactor,
                            int NDomainFactor,
                            int &counter )
{
    FindSlackers(NDomainFactor, true, true);

    vector<int> sender, recv, dom;
    for (int i = 0; i < slackers.size(); i++)
    {
        int slackerIdx = slackers[i];
        //debug1<<"Case 3: slackerRank="<<slaveInfo[slackerIdx].rank<<endl;
        
        for (int d = 0; d < NUM_DOMAINS; d++)
        {
            vector<int> domPartner;
            if ( !slaveInfo[slackerIdx].domainLoaded[d] &&
                 slaveInfo[slackerIdx].domainCnt[d] > 0)
            {
                //debug1<<"   dom= "<<d<<endl;
                // Find a partner who has the domain and has fewer than overloadFactor SLs.
                for (int j = 0; j < slaveInfo.size(); j++)
                {
                    if (j != slackerIdx && slaveInfo[j].domainLoaded[d] &&
                        slaveInfo[j].slCount < overloadFactor)
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

    int maxSLsToSend = 5*maxCnt;
    int maxDestSLs = overloadFactor;

    // Send messages out.
    for (int i = 0; i < sender.size(); i++)
    {
        SlaveInfo &recvSlave = slaveInfo[recv[i]];
        SlaveInfo &sendSlave = slaveInfo[sender[i]];
        int d = dom[i];
        int n = sendSlave.domainCnt[d];

        if (n > maxSLsToSend)
            n = maxSLsToSend;

        //Dest already has enough work.
        if (recvSlave.slCount > maxDestSLs)
            continue;
    
        // Cap it.
        if (recvSlave.slCount + n > maxDestSLs)
            n = maxDestSLs - recvSlave.slCount;

        vector<int> msg;
        msg.push_back(MSG_SEND_SL);
        msg.push_back(recvSlave.rank);

        DomainType dd = IdxToDom(d);
        msg.push_back(d);
        msg.push_back(n);
        
        for (int i = 0; i < n; i++)
        {
            recvSlave.AddSL(d);
            sendSlave.RemoveSL(d);
        }

        if (n > 0)
        {
            debug1<<"Case 3: "<<sendSlave.rank<<" ==["<<n<<"]==> "<<recvSlave.rank<<"  d= "<<d;
            debug1<<" ***   "<<recvSlave.rank<<" now has "<<recvSlave.slCount<<" cap= "<<maxDestSLs<<endl;
            SendMsg(sendSlave.rank, msg);
            counter++;
        }
    }
}

// ****************************************************************************
//  Method: avtMasterSLAlgorithm::Case4
//
//  Purpose:
//      Case4 of masterslave algorithm. Tell a slave to load a domain.
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
avtMasterSLAlgorithm::Case4(int oobThreshold,
                            int &counter )
{
    slackers.resize(0);
    
    for (int i = 0; i < slaveInfo.size(); i++)
        if (slaveInfo[i].slLoadedCount == 0 && slaveInfo[i].slOOBCount >= oobThreshold)
            slackers.push_back(i);
    random_shuffle(slackers.begin(), slackers.end());

    
    for (int i = 0; i < slackers.size(); i++)
    {
        int idx = slackers[i];
        int domToLoad = -1, maxCnt=-1;
        for (int j = 0; j < slaveInfo[idx].domainCnt.size(); j++)
        {
            if (slaveInfo[idx].domainCnt[j] > 0 && slaveInfo[idx].domainCnt[j] > maxCnt)
            {
                domToLoad = j;
                maxCnt = slaveInfo[idx].domainCnt[j];
            }
        }
        
        if (domToLoad != -1)
        {
            vector<int> msg;
            msg.push_back(MSG_LOAD_DOMAIN);
            DomainType dom = IdxToDom(domToLoad);
            msg.push_back(dom.domain);
            msg.push_back(dom.timeStep);
            SendMsg(slaveInfo[idx].rank, msg);
            debug1<<"Case 4: "<<slaveInfo[idx].rank<<" load dom= "<<domToLoad<<" oobThreshold: "<<oobThreshold<<endl;
            slaveInfo[idx].LoadDom(domToLoad);
            counter++;
        }
    }
}


// ****************************************************************************
//  Method: avtMasterSLAlgorithm::Case5
//
//  Purpose:
//      Case5 of masterslave algorithm. Advise slave to send SLs to other slave.
//  
//
//  Programmer: Dave Pugmire
//  Creation:   Mar 23, 2009
//
//  Modifications:
//
//
// ****************************************************************************

void
avtMasterSLAlgorithm::Case5(int overworkThreshold, bool domainCheck, int &counter)
{
    vector<int> slackers, overWorked;
    vector<int *> overWorkedCnt;
    //Get folks with too much work to do.
    for (int i = 0; i < slaveInfo.size(); i++)
        if (slaveInfo[i].slCount > overworkThreshold)
        {
            int *v = new int[2];
            v[0] = i;
            v[1] = slaveInfo[i].slCount;
            overWorkedCnt.push_back(v);
        }

    if (overWorkedCnt.size() == 0)
        return;
    sort(overWorkedCnt.begin(), overWorkedCnt.end(), domCntCompare);
    for (int i = 0; i < overWorkedCnt.size(); i++)
        overWorked.push_back(overWorkedCnt[i][0]);

    //Get slackers with no work and no unloaded domains.
    for (int i = 0; i < slaveInfo.size(); i++)
        if (!slaveInfo[i].justUpdated && slaveInfo[i].slCount == 0)
            slackers.push_back(i);
    if (slackers.size() == 0)
        return;

    random_shuffle(slackers.begin(), slackers.end());

    debug1<<"C5: dc= "<<domainCheck<<endl;
    debug1<<"C5 Slackers: "<<slackers<<endl;
    debug1<<"C5 Overworked: "<<overWorked<<endl;

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
                    if (slaveInfo[overWorked[w]].domainCnt[d] > 0 &&
                        slaveInfo[slackers[s]].domainLoaded[d])
                        commonDoms.push_back(d);
                }
                
                if (commonDoms.size() > 0)
                {
                    senders.push_back(slaveInfo[overWorked[w]].rank);
                    receivers.push_back(slaveInfo[slackers[s]].rank);
                    doms.push_back(commonDoms);
                }
            }
            else
            {
                senders.push_back(slaveInfo[overWorked[w]].rank);
                receivers.push_back(slaveInfo[slackers[s]].rank);
            }
        }
    }

    //Nobody to send....
    if (senders.size() == 0 || receivers.size() == 0)
    {
        debug1<<"... No matches.\n";
        return;
    }

    for (int i = 0; i < senders.size(); i++)
    {
        debug1<<"Case5: "<<senders[i]<<" ===> "<<receivers[i];
        if (doms.size() > 0)
            debug1<<" doms= "<<doms[i];
        debug1<<endl;
    }

#if 0

    // Tell overWorked: Send SLs to slacker.
    vector<int> info(1);
    info[0] = recv;
    if (domainCheck)
    {
        for ( int i = 0; i < numDomains; i++)
            if (slaveInfo[recv].domainLoaded[i])
                info.push_back(i);
        info.push_back(-1);
    }
    else
        info.push_back(-1);

    debug1<<"Case 5: "<<sender<<" ==[";
    for (int i = 1; i < info.size()-1; i++) debug1<<info[i]<<" ";
    debug1<<"] ==> "<<recv<<endl;
    AsyncSendSlaveMsgs(sender, MSG_SEND_SL_HINT, info);

    // Update status. We're not sure which domains will be sent.
    // We need to update status with something, so use 'dom'.
    for (int i = 0; i < maxCount; i++)
    {
        slaveInfo[recv].AddSL(dom);
        if (slaveInfo[sender].domainCnt[dom] > 0)
            slaveInfo[sender].RemoveSL(dom);
    }
#endif
}


// ****************************************************************************
//  Method: avtSlaveSLAlgorithm::avtSlaveSLAlgorithm
//
//  Purpose:
//      avtSlaveSLAlgorithm constructor.
//
//  Programmer: Dave Pugmire
//  Creation:   January 27, 2009
//
//  Modifications:
//
//   Dave Pugmire, Mon Feb 23 13:38:49 EST 2009
//   Add timeout counter for slaves.
//
// ****************************************************************************

avtSlaveSLAlgorithm::avtSlaveSLAlgorithm(avtStreamlineFilter *slFilter,
                                         int maxCount,
                                         int masterRank)
    : avtMasterSlaveSLAlgorithm(slFilter, maxCount)
{
    master = masterRank;
    timeout = 0;
}

// ****************************************************************************
//  Method: avtSlaveSLAlgorithm::avtSlaveSLAlgorithm
//
//  Purpose:
//      avtSlaveSLAlgorithm destructor
//
//  Programmer: Dave Pugmire
//  Creation:   January 27, 2009
//
// ****************************************************************************

avtSlaveSLAlgorithm::~avtSlaveSLAlgorithm()
{
}

// ****************************************************************************
//  Method: avtSlaveSLAlgorithm::Initialize
//
//  Purpose:
//      Initialize the slave
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
avtSlaveSLAlgorithm::Initialize(std::vector<avtStreamlineWrapper *> &seedPts)
{
    avtMasterSlaveSLAlgorithm::Initialize(seedPts);
    status.resize(NUM_DOMAINS,0);
    prevStatus.resize(NUM_DOMAINS,0);
    numTerminated = 0;
    workToDo = false;
}


// ****************************************************************************
//  Method: avtSlaveSLAlgorithm::UpdateStatus
//
//  Purpose:
//      Update status vectors.
//
//  Programmer: Dave Pugmire
//  Creation:   January 27, 2009
//
// ****************************************************************************

void
avtSlaveSLAlgorithm::UpdateStatus()
{
    for (int i = 0; i < status.size(); i++)
        status[i] = 0;

    // Set 0,1 for domains loaded.
    for (int i = 0; i < NUM_DOMAINS; i++)
    {
        DomainType d = IdxToDom(i);
        status[i] = (DomainLoaded(d) ? 1: 0);
    }
    
    //Increment/decrement all the streamlines we have.
    list<avtStreamlineWrapper *>::const_iterator s;
    
    bool prevWorkToDo = workToDo;
    workToDo = false;
    for (s = activeSLs.begin(); s != activeSLs.end(); ++s)
        if (DomainLoaded((*s)->domain))
        {
            status[DomToIdx( (*s)->domain )] ++;
            workToDo = true;
        }
        else
            status[DomToIdx( (*s)->domain )] --;

    for (s = oobSLs.begin(); s != oobSLs.end(); ++s)
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
//  Method: avtSlaveSLAlgorithm::SendStatus
//
//  Purpose:
//      Send status to master.
//
//  Programmer: Dave Pugmire
//  Creation:   January 27, 2009
//
// ****************************************************************************

void
avtSlaveSLAlgorithm::SendStatus(bool forceSend)
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
        //Send the status message to master.
        vector<int> msg;
        msg.push_back(MSG_STATUS);
        msg.push_back(numTerminated);
        for (int i = 0; i < status.size(); i++)
            msg.push_back(status[i]);

        debug1<<"Slave SendStatus: "<<msg<<endl;
        SendMsg(master, msg);
        
        //Status just sent, reset.
        numTerminated = 0;
        for (int i = 0; i < status.size(); i++)
            prevStatus[i] = status[i];
    }
}


// ****************************************************************************
//  Method: avtSlaveSLAlgorithm::Execute
//
//  Purpose:
//      Execute the slave loop of the master/slave algorithm.
//
//  Programmer: Dave Pugmire
//  Creation:   January 27, 2009
//
//   Dave Pugmire, Wed Mar 18 17:17:40 EDT 2009
//   Allow masters to share work loads.
//
//   Dave Pugmire, Mon Mar 23 12:48:12 EDT 2009
//   Changes related to trying to hide latency with slaves.
//
//   Dave Pugmire, Wed Mar 25 09:10:52 EDT 2009
//   Enable latency.
//   
// ****************************************************************************

void
avtSlaveSLAlgorithm::Execute()
{
    list<avtStreamlineWrapper *>::const_iterator si;
    int timer = visitTimer->StartTimer();

    //Send initial status.
    SendStatus(true);
    Barrier();

    while ( 1 )
    {
        //Fill oobSLs list.
        list<avtStreamlineWrapper *>::iterator si = activeSLs.begin();
        while (si != activeSLs.end())
        {
            if (!DomainLoaded((*si)->domain))
            {
                oobSLs.push_back(*si);
                si = activeSLs.erase(si);
            }
            else
                ++si;
        }
        
        if (latencyTimer != -1 && !activeSLs.empty())
        {
            double t = visitTimer->StopTimer(latencyTimer, "Latency");
            debug1<<"End latency: time= "<<t<<endl;
            LatencyTime.value += t;
            if (t > MaxLatencyTime.value)
                MaxLatencyTime.value = t;
            latencyTimer = -1;
        }
        
        bool done = false, newMsgs = false;
        while (!activeSLs.empty() && !done)
        {
            avtStreamlineWrapper *s = activeSLs.front();
            activeSLs.pop_front();

            bool doThis = true;
            if (doThis && activeSLs.empty())
            {
                debug1<<"Latency saving sendStatus"<<endl;
                LatencySavingCnt.value++;
                SendStatus();
            }
            
            debug1<<"Integrate "<<s->domain<<endl;
            IntegrateStreamline(s);
            if (s->status == avtStreamlineWrapper::TERMINATE)
            {
                terminatedSLs.push_back(s);
                numTerminated++;
            }
            else
            {
                if (DomainLoaded(s->domain))
                    activeSLs.push_back(s);
                else
                    oobSLs.push_back(s);
            }
            
            ProcessMessages(done, newMsgs);
        }
        
        if (done)
            break;

        activeSLs.splice(activeSLs.end(), oobSLs);
        oobSLs.clear();
        
        //See if we have any SLs.
        int earlyTerminations = 0;
        bool newSLs = RecvSLs(activeSLs, earlyTerminations);
        numTerminated += earlyTerminations;
        ProcessMessages(done, newMsgs);
        CheckPendingSendRequests();

        if (done)
            break;
        SendStatus();
        
        //Nothing to do, take a snooze....
        if (!workToDo)
        {
            if (latencyTimer == -1)
            {
                latencyTimer = visitTimer->StartTimer();
                debug1<<"Begin latency!\n";
            }
            Sleep();
        }
    }

    if (latencyTimer != -1)
    {
        double t = visitTimer->StopTimer(latencyTimer, "Latency");
        debug1<<"End latency: time= "<<t<<endl;
        LatencyTime.value += t;
        latencyTimer = -1;
    }

    TotalTime.value += visitTimer->StopTimer(timer, "Execute");
}


// ****************************************************************************
//  Method: avtSlaveSLAlgorithm::ProcessMessages
//
//  Purpose:
//      Processes messages from master.
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
//   Bug fix. Didn't use new DomainType structure for MSG_SEND_SL.
//
//  Dave Pugmire, Wed Mar 18 21:55:32 EDT 2009
//  Improve the logic for streamline offloading. Only offload streamlines
//  in unloaded domains.
//
// ****************************************************************************

void
avtSlaveSLAlgorithm::ProcessMessages(bool &done, bool &newMsgs)
{
    vector<vector<int> > msgs;
    RecvMsgs(msgs);

    debug5<<"avtSlave::ProcessMessages()  msgs= "<<msgs.size()<<endl;
    
    done = false;
    newMsgs = (msgs.size() > 0);
    
    for (int i = 0; i < msgs.size(); i++)
    {
        vector<int> &msg = msgs[i];
        int src = msg[0];
        int msgType = msg[1];
        
        if (msgType == MSG_DONE)
        {
            done = true;
            break;
        }
        
        //Load this domain.
        else if (msgType == MSG_LOAD_DOMAIN)
        {
            DomainType dom(msg[2], msg[3]);
            debug1<<"MSG: LoadDomain( "<<dom<<")\n";
            GetDomain(dom);
        }

        //Offload unloaded domains.
        else if (msgType == MSG_OFFLOAD_SL)
        {
            debug1<<"Slave: MSG_OFFLOAD_SL I have "<<activeSLs.size()<<" to offer"<<endl;
            debug1<<msg<<endl;
            
            int dst = msg[2];
            int numDoms = msg[3];
            int num = 10*maxCnt;

            vector< avtStreamlineWrapper *> sendSLs;

            for (int d = 0; d < numDoms; d++)
            {
                int domIdx = msg[4+d];
                DomainType dom = IdxToDom(domIdx);
                
                list<avtStreamlineWrapper *>::iterator s = activeSLs.begin();
                while (s != activeSLs.end() &&
                       sendSLs.size() < num)
                {
                    if ((*s)->domain == dom &&
                        !DomainLoaded(dom))
                    {
                        sendSLs.push_back(*s);
                        s = activeSLs.erase(s);
                        numTerminated++;
                    }
                    else
                        s++;
                }
            }
            
            if (sendSLs.size() > 0)
            {
                debug1<<"OFFLOAD: Send "<<sendSLs.size()<<" to "<<dst<<endl;
                SendSLs(dst, sendSLs);
            }
        }
        
        //Send streamlines to another slave.
        else if (msgType == MSG_SEND_SL)
        {
            int dst = msg[2];
            DomainType dom = IdxToDom(msg[3]);
            int num = msg[4];

            debug1<<"MSG: Send "<<num<<" x dom= "<<dom<<" to "<<dst;
            list<avtStreamlineWrapper *>::iterator s = activeSLs.begin();
            vector< avtStreamlineWrapper *> sendSLs;
            while (s != activeSLs.end() &&
                   sendSLs.size() < num)
            {
                if ((*s)->domain == dom)
                {
                    sendSLs.push_back(*s);
                    s = activeSLs.erase(s);
                }
                else
                    ++s;
            }

            debug1<<" sent "<<sendSLs.size()<<endl;
            if (sendSLs.size() > 0)
                SendSLs(dst, sendSLs);
        }
    }
}


// ****************************************************************************
//  Method: SlaveInfo::SlaveInfo
//
//  Purpose:
//      SlaveInfo constructor
//
//  Programmer: Dave Pugmire
//  Creation:   January 27, 2009
//
//   Dave Pugmire, Wed Mar 18 17:17:40 EDT 2009
//   Allow masters to share work loads.
//   
// ****************************************************************************

SlaveInfo::SlaveInfo( int r, int nDomains )
{
    justUpdated = false;
    initialized = false;
    canGive = canAccept = slCount = slLoadedCount = slOOBCount = 0;
    domLoadedCount = 0;
    domainCnt.resize(nDomains, 0);
    domainLoaded.resize(nDomains, false);
    rank = r;
}

// ****************************************************************************
//  Method: SlaveInfo::AddSL
//
//  Purpose:
//      Update when passing a SL.
//
//  Programmer: Dave Pugmire
//  Creation:   January 27, 2009
//
// ****************************************************************************

void
SlaveInfo::AddSL( int slDomain)
{
    bool underPurgeLimit = (domLoadedCount <= 3);
    //We assume that it will get loaded..
    if (domainLoaded[slDomain] == false)
        domLoadedCount++;
    domainLoaded[slDomain] = true;
    domainCnt[slDomain]++;
    slCount++;
    slLoadedCount++;
    justUpdated = false;
    
    if (underPurgeLimit && domLoadedCount > 3)
        debug1<<"WARNING: "<<rank<<" Purge is coming!\n";
    if (domainHistory.size() == 0 ||
        (domainHistory.size() > 0 && slDomain != domainHistory[domainHistory.size()-1]))
        domainHistory.push_back(slDomain);      
}

// ****************************************************************************
//  Method: SlaveInfo::LoadDom
//
//  Purpose:
//      Update when loading a domain.
//
//  Programmer: Dave Pugmire
//  Creation:   January 27, 2009
//
// ****************************************************************************

void
SlaveInfo::LoadDom( int slDomain )
{
    bool underPurgeLimit = (domLoadedCount <= 3);
    if (domainLoaded[slDomain] == false)
        domLoadedCount++;
    
    domainLoaded[slDomain] = true;
    slLoadedCount += domainCnt[slDomain];
    slOOBCount -= domainCnt[slDomain];
    
    if (underPurgeLimit && domLoadedCount > 3)
        debug1<<"WARNING: "<<rank<<" Purge is coming!\n";
    
    if (domainHistory.size() == 0 ||
        (domainHistory.size() > 0 && slDomain != domainHistory[domainHistory.size()-1]))
        domainHistory.push_back(slDomain);
}


// ****************************************************************************
//  Method: SlaveInfo::RemoveSL
//
//  Purpose:
//      Update when removing a domain.
//
//  Programmer: Dave Pugmire
//  Creation:   January 27, 2009
//
// ****************************************************************************
    
void
SlaveInfo::RemoveSL( int dom )
{
    domainCnt[dom]--;
    //We assume that it will get loaded..
    slCount--;
    if (domainLoaded[dom])
        slLoadedCount--;
    else
        slOOBCount--;
    justUpdated = false;
}

// ****************************************************************************
//  Method: SlaveInfo::Update
//
//  Purpose:
//      Update with a new status from the slave.
//
//  Programmer: Dave Pugmire
//  Creation:   January 27, 2009
//
//   Dave Pugmire, Wed Mar 18 17:17:40 EDT 2009
//   Allow masters to share work loads.
//   
// ****************************************************************************

void
SlaveInfo::Update( vector<int> &status, bool debug )
{
    justUpdated = true;
    initialized = true;
    slCount = 0;
    slLoadedCount = 0;
    slOOBCount = 0;
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
            slCount += (cnt-1);
            slLoadedCount += (cnt-1);
        }
        else if (cnt < 0)
        {
            domainCnt[i] = -cnt;
            domainLoaded[i] = false;
            slCount += (-cnt);
            slOOBCount += (-cnt);
        }
    }
    
    for (int i = 0; i < domainLoaded.size(); i++)
        domLoadedCount += (domainLoaded[i] ? 1 : 0);
        
    if (debug)
        Debug();  
}

// ****************************************************************************
//  Method: SlaveInfo::Debug
//
//  Purpose:
//      Print your self out.
//
//  Programmer: Dave Pugmire
//  Creation:   January 27, 2009
//
// ****************************************************************************

void
SlaveInfo::Debug()
{
    bool slacker = (slLoadedCount == 0);
    debug1<<setw(2)<<rank;
    debug1<<": "<<setw(3)<<slCount<<" ("<<setw(3)<<slLoadedCount<<", "<<setw(3)<<slOOBCount<<") [";
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
    debug1<<"] ("<<domLoadedCount<<")";
    debug1<< (slacker ? "S" : " ");
    if (justUpdated)
    {
        debug1<<" ***";
        if (slLoadedCount == 0)
            debug1<<" SLACKER: "<<rank;
        else
            debug1<<" UPDATE: "<<rank;              
    }
    debug1<<endl;
}


#endif
