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

// ****************************************************************************
//  Method: avtMasterSlaveSLAlgorithm::Create
//
//  Purpose:
//      Static method to create and conifigure classes.
//
//  Programmer: Dave Pugmire
//  Creation:   January 27, 2009
//
// ****************************************************************************

avtMasterSlaveSLAlgorithm*
avtMasterSlaveSLAlgorithm::Create(avtStreamlineFilter *slFilter,
                                  int maxCount,
                                  int rank,
                                  int nProcs,
                                  int workGroupSz)
{
    avtMasterSlaveSLAlgorithm *algo = NULL;
    
    //Easy case, 1 working group.
    if (nProcs <= workGroupSz)
    {
        if (rank == 0)
        {
            vector<int> slaves;
            for (int i = 1; i < nProcs; i++)
                slaves.push_back(i);
            algo = new avtMasterSLAlgorithm(slFilter, maxCount, slaves);
        }
        else
        {
            int master = 0;
            algo = new avtSlaveSLAlgorithm(slFilter, maxCount, master);
        }
    }
    
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
// ****************************************************************************

avtMasterSlaveSLAlgorithm::avtMasterSlaveSLAlgorithm(avtStreamlineFilter *slFilter,
                                                     int maxCount)
    : avtParSLAlgorithm(slFilter)
{
    maxCnt = maxCount;
    sleepMicroSec = 300;
    
    // Msg type, numTerminated, domain vector.
    statusMsgSz = 2 + numDomains;
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
    latencyTimer = -1;
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
//  Method: avtMasterSlaveSLAlgorithm::CalculateStatistics
//
//  Purpose:
//      Calculate statistics.
//
//  Programmer: Dave Pugmire
//  Creation:   January 27, 2009
//
// ****************************************************************************

void
avtMasterSlaveSLAlgorithm::CalculateStatistics()
{
    avtParSLAlgorithm::CalculateStatistics();

    ComputeStatistics(SleepTime);
    ComputeStatistics(LatencyTime);
    ComputeStatistics(MaxLatencyTime);
    ComputeStatistics(SleepCnt);
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
// ****************************************************************************

void
avtMasterSlaveSLAlgorithm::CalculateExtraTime()
{
    avtParSLAlgorithm::CalculateExtraTime();
    ExtraTime.value -= SleepTime.value;
    ExtraTime.value -= LatencyTime.value;
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
    
    PrintTiming(os, "SleepTime = ", SleepTime, TotalTime, totals);
    PrintTiming(os, "LatencyTime = ", LatencyTime, TotalTime, totals);
    PrintTiming(os, "MaxLatTime  = ", MaxLatencyTime, TotalTime, totals);
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
// ****************************************************************************

void
avtMasterSlaveSLAlgorithm::ReportCounters(ostream &os, bool totals)
{
    avtParSLAlgorithm::ReportCounters(os, totals);
    PrintCounter(os, "SleepCnt = ", SleepCnt, totals);
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
// ****************************************************************************

avtMasterSLAlgorithm::avtMasterSLAlgorithm(avtStreamlineFilter *slFilter,
                                           int maxCount,
                                           vector<int> &slaves)
    : avtMasterSlaveSLAlgorithm(slFilter, maxCount)
{
    //Create my slaves.
    for (int i = 0; i < slaves.size(); i++)
        slaveInfo.push_back(SlaveInfo(slaves[i], numDomains));
    
    slDomCnts.resize(numDomains,0);
    domLoaded.resize(numDomains,0);

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
// ****************************************************************************

void
avtMasterSLAlgorithm::Initialize(std::vector<avtStreamlineWrapper *> &seedPts)
{
    avtMasterSlaveSLAlgorithm::Initialize(seedPts);
    case1Cnt = 0;
    case2Cnt = 0;
    case3Cnt = 0;
    case4Cnt = 0;
    case5Cnt = 0;
    
    totalNumStreamlines = seedPts.size();
    
    for (int i = 0; i < seedPts.size(); i++)
        activeSLs.push_back(seedPts[i]);
}

// ****************************************************************************
//  Method: avtMasterSLAlgorithm::CalculateStatistics
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
avtMasterSLAlgorithm::CalculateStatistics()
{
    LatencyTime.value = -1;
    MaxLatencyTime.value = -1;
    IOTime.value = -1;
    IntegrateTime.value = -1;
    SortTime.value = -1;
    IntegrateCnt.value = -1;
    DomLoadCnt.value = -1;
    DomPurgeCnt.value = -1;

    avtMasterSlaveSLAlgorithm::CalculateStatistics();
}

// ****************************************************************************
//  Method: avtMasterSLAlgorithm::UpdateStatus
//
//  Purpose:
//      Update the status of any slaves.
//
//  Programmer: Dave Pugmire
//  Creation:   January 27, 2009
//
// ****************************************************************************
bool
avtMasterSLAlgorithm::UpdateStatus()
{
    debug1<<"avtMasterSLAlgorithm::UpdateStatus()\n";
    
    //Clean and update SL/Domain counts.
    for (int i = 0; i < numDomains; i++)
    {
        slDomCnts[i] = 0;
        domLoaded[i] = 0;
    }
    list<avtStreamlineWrapper *>::const_iterator s;
    for (s = activeSLs.begin(); s != activeSLs.end(); ++s)
        slDomCnts[(*s)->domain] ++;

    // See if any slaves have sent a status.
    vector<vector<int> > msgs;
    RecvMsgs(msgs);
    bool newStatus = (msgs.size() > 0);
    
    for (int i = 0; i < msgs.size(); i++)
        UpdateSlaveStatus(msgs[i]);
    
    for (int i = 0; i < slaveInfo.size(); i++)
        for ( int d = 0; d < numDomains; d++)
            if (slaveInfo[i].domainLoaded[d])
                domLoaded[d]++;

    return newStatus;
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
// ****************************************************************************

void
avtMasterSLAlgorithm::PrintStatus()
{
    debug1<<"DOM:               [";
    for ( int i = 0; i < numDomains; i++)
        debug1<<setw(4)<<i<<" ";
    debug1<<"]\n";
    debug1<<"Master:            [";
    for ( int i = 0; i < numDomains; i++)
        debug1<<setw(4)<<slDomCnts[i]<<" ";
    debug1<<"]\n";
    
    for (int i = 0; i < slaveInfo.size(); i++)
        slaveInfo[i].Debug();
    debug1<<"DCounts:           [";
    for ( int i = 0; i < numDomains; i++)
        debug1<<setw(4)<<domLoaded[i]<<" ";
    debug1<<"]\n";

    vector<int> slaveSLs(numDomains,0);
    for (int i = 0; i < slaveInfo.size(); i++)
        for (int j = 0; j < numDomains; j++)
            slaveSLs[j] += slaveInfo[i].domainCnt[j];
    debug1<<"SCounts:           [";
    int cnt = 0;
    for ( int i = 0; i < numDomains; i++)
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
// ****************************************************************************
bool
avtMasterSLAlgorithm::UpdateSlaveStatus(vector<int> &status)
{
    int rank = status[0];
    int msg = status[1];
    int nTerm = status[2];
    
    totalNumStreamlines -= nTerm;
    
    for (int i = 0; i < slaveInfo.size(); i++)
    {
        if (slaveInfo[i].rank == rank)
        {
            debug1<<"Update for rank= "<<rank<<endl;
            vector<int> domStatus;
            for (int j = 3; j < status.size(); j++)
                domStatus.push_back(status[j]);
            slaveInfo[i].Update(domStatus, false);
            break;
        }
    }
    debug1<<"UpdateSlaveStatus() DONE\n";
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
// ****************************************************************************

void
avtMasterSLAlgorithm::Execute()
{
    int timer = visitTimer->StartTimer();
    
    //Each slave has to send information. Don't proceed until they have.
    Barrier();

    while (totalNumStreamlines > 0)
    {
        debug1<<"while ("<<totalNumStreamlines<<" > 0)\n";
        if (UpdateStatus())
        {
            PrintStatus();
            Case1(case1Cnt);
            Case2(case2Cnt);
            int case3OverloadFactor = 10*maxCnt, case3NDomainFactor = 3*maxCnt;
            Case3(case3OverloadFactor, case3NDomainFactor, case3Cnt);
            Case4(0,case4Cnt);

            /*
            Case4(3*maxCount, case4Bcnt);
            Case1(case1cnt);
            Case4(1*maxCount, case4Ccnt); //Remove this KILLED performance. ODD.
            Case2(case2cnt);
            int case3OverloadFactor = 10*maxCount, case3NDomainFactor = 3*maxCount;
            Case3(case3OverloadFactor, case3NDomainFactor, case3cnt);
            Case4(0, case4Acnt);
            Case5(2*maxCount, true, case5Acnt);
            Case5(2*maxCount, false, case5Bcnt);
            */

            debug1<<endl<<"Post-Mortem"<<endl;
            PrintStatus();
            
            for (int i = 0; i < slaveInfo.size(); i++)
                slaveInfo[i].Reset();
        }
        else
        {
            debug1<<"Nothing to do: "<<totalNumStreamlines<<endl;
            Sleep();
        }
    }
    
    SendAllSlavesMsg(MSG_DONE);
    TotalTime.value += visitTimer->StopTimer(timer, "Execute");
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
//  Modifications:
//
//
// ****************************************************************************

void
avtMasterSLAlgorithm::Case1(int &counter)
{
    if (activeSLs.empty())
        return;
    
    FindSlackers();
    debug1<<"Case1: slackers: "<<slackers.size()<<endl;
    
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
            int sDom = (*s)->domain;
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
//  Modifications:
//
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
        if (true)
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
            int sDom = (*sl)->domain;
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
        int earlyTerminations = 0;
        ExchangeSLs(activeSLs, distributeSLs, earlyTerminations);
        
        if (earlyTerminations != 0)
            EXCEPTION0(ImproperUseException);
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
//  Modifications:
//
//
// ****************************************************************************

void
avtMasterSLAlgorithm::Case3(int overloadFactor,
                            int NDomainFactor,
                            int &counter )
{
    FindSlackers(NDomainFactor);

    vector<int> sender, recv, dom;
    for (int i = 0; i < slackers.size(); i++)
    {
        int slackerIdx = slackers[i];
        debug1<<"Case 3: slackerRank="<<slaveInfo[slackerIdx].rank<<endl;
        
        for (int d = 0; d < numDomains; d++)
        {
            vector<int> domPartner;
            if ( !slaveInfo[slackerIdx].domainLoaded[d] &&
                 slaveInfo[slackerIdx].domainCnt[d] > 0)
            {
                debug1<<"   dom= "<<d<<endl;
                // Find a partner who has the domain and has fewer than overloadFactor SLs.
                for (int j = 0; j < slaveInfo.size(); j++)
                {
                    if (j != slackerIdx && slaveInfo[j].domainLoaded[d] &&
                        slaveInfo[j].slCount < overloadFactor)
                    {
                        debug1<<"      partner= "<<j<<endl;
                        domPartner.push_back(j);
                    }
                }
            }
            
            if (domPartner.size() > 0)
            {
                debug1<<"domPartner: [";
                for(int k=0; k<domPartner.size(); k++) debug1<<domPartner[k]<<" ";
                debug1<<"]\n";
                
                random_shuffle(domPartner.begin(), domPartner.end());
                
                debug1<<"random sort: [";
                for(int k=0; k<domPartner.size(); k++) debug1<<domPartner[k]<<" ";
                debug1<<"]\n";                
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
        
        debug1<<"c3: si = "<<sender[i]<<endl;
        debug1<<"c3: ri = "<<recv[i]<<endl;
        debug1<<"c3: sr = "<<sendSlave.rank<<endl;
        debug1<<"c3: rr = "<<recvSlave.rank<<endl;
        
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
//  Modifications:
//
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
            msg.push_back(domToLoad);
            SendMsg(slaveInfo[idx].rank, msg);
            debug1<<"Case 4: "<<slaveInfo[idx].rank<<" load dom= "<<domToLoad<<" oobThreshold: "<<oobThreshold<<endl;
            slaveInfo[idx].LoadDom(domToLoad);
            counter++;
        }
    }
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
// ****************************************************************************

avtSlaveSLAlgorithm::avtSlaveSLAlgorithm(avtStreamlineFilter *slFilter,
                                         int maxCount,
                                         int masterRank)
    : avtMasterSlaveSLAlgorithm(slFilter, maxCount)
{
    master = masterRank;
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
// ****************************************************************************

void
avtSlaveSLAlgorithm::Initialize(std::vector<avtStreamlineWrapper *> &seedPts)
{
    avtMasterSlaveSLAlgorithm::Initialize(seedPts);
    status.resize(numDomains,0);
    prevStatus.resize(numDomains,0);
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
    for (int i = 0; i < numDomains; i++)
        status[i] = (DomainLoaded(i) ? 1: 0);
    
    //Increment/decrement all the streamlines we have.
    list<avtStreamlineWrapper *>::const_iterator s;
    
    bool prevWorkToDo = workToDo;
    workToDo = false;
    for (s = activeSLs.begin(); s != activeSLs.end(); ++s)
        if (DomainLoaded((*s)->domain))
        {
            status[(*s)->domain] ++;
            workToDo = true;
        }
        else
            status[(*s)->domain] --;

    for (s = oobSLs.begin(); s != oobSLs.end(); ++s)
    {
        if (DomainLoaded((*s)->domain))
        {
            status[(*s)->domain] ++;
            workToDo = true;
        }
        else
            status[(*s)->domain] --;
    }


    
    //Start/stop the latency timer.
    if (prevWorkToDo && !workToDo)
    {
        latencyTimer = visitTimer->StartTimer();
    }
    else if (!prevWorkToDo && workToDo && latencyTimer != -1)
    {
        double t = visitTimer->StopTimer(latencyTimer, "Latency");
        LatencyTime.value += t;
        if (t > MaxLatencyTime.value)
            MaxLatencyTime.value = t;
    }
    
    if (!workToDo)
        debug1<<"Nothing to do.....\n";
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
// ****************************************************************************

void
avtSlaveSLAlgorithm::Execute()
{
    list<avtStreamlineWrapper *>::const_iterator si;
    int timer = visitTimer->StartTimer();

    //Send initial status.
    SendStatus(true);
    Barrier();

#if 1
    bool sendStatus = true;

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

        while (!activeSLs.empty())
        {
            avtStreamlineWrapper *s = activeSLs.front();
            activeSLs.pop_front();
            debug1<<"Actives= "<<activeSLs.size()<<endl;

            sendStatus = true;
            if (activeSLs.empty())
            {
                SendStatus();
                sendStatus = false;
            }
                
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
        }

        activeSLs.splice(activeSLs.end(), oobSLs);
        
        bool done, newMsgs;
        ProcessMessages(done, newMsgs);
        if (done)
            break;
        
        //See if we have any SLs.
        int earlyTerminations = 0;
        bool newSLs = RecvSLs(activeSLs, earlyTerminations);
        numTerminated += earlyTerminations;

        if (newSLs)
            debug1<<"Recv some SLs. sz= "<<activeSLs.size()<<endl;
        
        if (sendStatus || newSLs || newMsgs || numTerminated > 0)
            SendStatus();
    }
#endif

#if 0
    while ( 1 )
    {
        // Integrate while I have work to do.
        int cnt = 0;
        while (cnt < maxCnt && !activeSLs.empty())
        {
            avtStreamlineWrapper *s = activeSLs.front();
            activeSLs.pop_front();

            if (DomainLoaded(s->domain))
            {
                IntegrateStreamline(s);
                if (s->status == avtStreamlineWrapper::TERMINATE)
                {
                    terminatedSLs.push_back(s);
                    numTerminated++;
                }
                else
                    activeSLs.push_back(s);
                
                cnt++;
            }
            else
                oobSLs.push_back(s);
        }
        activeSLs.splice(activeSLs.end(), oobSLs);
        
        bool done, newMsgs;
        ProcessMessages(done, newMsgs);
        if (done)
            break;
        
        //See if we have any SLs.
        int earlyTerminations = 0;
        RecvSLs(activeSLs, earlyTerminations);
        numTerminated += earlyTerminations;
        
        SendStatus();
    }
#endif

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
// ****************************************************************************

void
avtSlaveSLAlgorithm::ProcessMessages(bool &done, bool &newMsgs)
{
    vector<vector<int> > msgs;
    RecvMsgs(msgs);
    
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
        
        else if (msgType == MSG_LOAD_DOMAIN)
        {
            int dom = msg[2];
            debug1<<"MSG: LoadDomain( "<<dom<<")\n";
            GetDomain(dom);
        }
        
        else if (msgType == MSG_SEND_SL)
        {
            int dst = msg[2];
            int dom = msg[3];
            int num = msg[4];

            debug1<<"MSG: Send dom= "<<dom<<" to "<<dst<<endl;
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
// ****************************************************************************

SlaveInfo::SlaveInfo( int r, int nDomains )
{
    justUpdated = false;
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
// ****************************************************************************

void
SlaveInfo::Update( vector<int> &status, bool debug )
{
    justUpdated = true;
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
