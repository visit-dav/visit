/*****************************************************************************
*
* Copyright (c) 2000 - 2012, Lawrence Livermore National Security, LLC
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
//                              avtCommDSOnDemandICAlgorithm.C                 //
// ************************************************************************* //

#include <avtCommDSOnDemandICAlgorithm.h>
#include <TimingsManager.h>
#include <vtkDataSetWriter.h>

#ifdef PARALLEL

using namespace std;

static const int DONE = 0;
static const int DATASET_REQUEST = 1;

static bool cacheSort(const domainCacheEntry &cA,
                      const domainCacheEntry &cB)
{
    return cA.refCnt < cB.refCnt;
}

static bool icDomainCompare(const avtIntegralCurve *icA,
                            const avtIntegralCurve *icB)
{
    // If # of domain occurances is equal, sort on domain.
    if (icA->sortKey == icB->sortKey)
        return icA->domain.domain < icB->domain.domain;
    
    //Reverse sort, so that the largest is first.
    return icA->sortKey > icB->sortKey;
}

// ****************************************************************************
// Method:  avtCommDSOnDemandICAlgorithm::avtCommDSOnDemandICAlgorithm
//
// Purpose: ctor.
//   
// Programmer:  Dave Pugmire
// Creation:    December 15, 2010
//
// ****************************************************************************

avtCommDSOnDemandICAlgorithm::avtCommDSOnDemandICAlgorithm(avtPICSFilter *picsFilter,
                                                           int cacheSize)
    : avtParICAlgorithm(picsFilter), DSLatencyTime("dsLat")
{
    domainCacheSizeLimit = cacheSize;
    numDone = 0;
}

// ****************************************************************************
// Method:  avtCommDSOnDemandICAlgorithm::~avtCommDSOnDemandICAlgorithm
//
// Purpose: dtor.
//   
// Programmer:  Dave Pugmire
// Creation:    December 15, 2010
//
// ****************************************************************************

avtCommDSOnDemandICAlgorithm::~avtCommDSOnDemandICAlgorithm()
{
    list<domainCacheEntry>::iterator it;

    // Cleanup cache memory.
    for (it = domainCache.begin(); it != domainCache.end(); it++)
        it->ds->Delete();
    
    domainCache.resize(0);
}

// ****************************************************************************
// Method:  avtCommDSOnDemandICAlgorithm::Initialize
//
// Purpose: Initialize things.
//   
// Programmer:  Dave Pugmire
// Creation:    December 15, 2010
//
// ****************************************************************************

void
avtCommDSOnDemandICAlgorithm::Initialize(vector<avtIntegralCurve *> &seedPts)
{
    int numRecvs = nProcs-1;
    if (numRecvs > 64)
        numRecvs = 64;
    
    avtParICAlgorithm::InitializeBuffers(seedPts, 8, numRecvs, numRecvs, 2*numRecvs);
    
    AddIntegralCurves(seedPts);
}

// ****************************************************************************
// Method:  avtCommDSOnDemandICAlgorithm::AddIntegralCurves
//
// Purpose: Assign ICs.
//   
// Programmer:  Dave Pugmire
// Creation:    December 15, 2010
//
// ****************************************************************************

void
avtCommDSOnDemandICAlgorithm::AddIntegralCurves(vector<avtIntegralCurve *> &ics)
{
    int nSeeds = ics.size();
    int i0 = 0, i1 = nSeeds;

    int nSeedsPerProc = (nSeeds / nProcs);
    int oneExtraUntil = (nSeeds % nProcs);
    
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
    
    //Delete the seeds I don't need.
    for (int i = 0; i < i0; i++)
        delete ics[i];
    for (int i = i1; i < nSeeds; i++)
        delete ics[i];
    
    debug5 << "I have seeds: "<<i0<<" to "<<(i1-1)<<" of "<<nSeeds<<endl;
    
    // Filter the seeds for proper domain inclusion and fill the activeICs list.
    avtVector endPt;
    for ( int i = i0; i < i1; i++)
    {
        avtIntegralCurve *s = ics[i];
        debug5<<"  "<<s->id<<"  dom= "<<s->domain<<endl;
        /*
        s->CurrentLocation(endPt);
        if (PointInDomain(endPt, s->domain))
            activeICs.push_back(s);
        else
            delete s;
        */
        
        if (DomainToRank(s->domain) == rank)
            activeICs.push_back(s);
        else
            oobICs.push_back(s);
    }
    debug1<<"My seeds. Active: "<<activeICs.size()<<" OOB: "<<oobICs.size()<<endl;
}


// ****************************************************************************
// Method:  avtCommDSOnDemandICAlgorithm::RunAlgorithm
//
// Purpose: Run the algorithm.
//   
// Programmer:  Dave Pugmire
// Creation:    December 15, 2010
//
// ****************************************************************************

void
avtCommDSOnDemandICAlgorithm::RunAlgorithm()
{
    debug1<<"avtCommDSOnDemandICAlgorithm::RunAlgorithm()\n";
    int timer = visitTimer->StartTimer();

    //Sort the streamlines and load the first domain.
    SortIntegralCurves(activeICs);
    SortIntegralCurves(oobICs);

    numDone = 0;
    int numParticlesTotal = activeICs.size() + oobICs.size();
    int numParticlesResolved = 0;
    
    bool IamDone = false;
    CheckCacheVacancy(true);
    HandleMessages();
    
    while (numDone < nProcs)
    {
        //Do work, if we have it....
        if (!activeICs.empty())
        {
            avtIntegralCurve *s = activeICs.front();
            activeICs.pop_front();

            DomainType d = s->domain;
            vtkDataSet *ds = GetDataset(s->domain);
            if (ds != NULL)
            {
                //debug1<<"Advect("<<s->domain.domain<<") --> ";
                AdvectParticle(s, ds);
                if (s->status != avtIntegralCurve::STATUS_OK)
                {
                    //debug1<<" TERMINATE"<<endl;
                    terminatedICs.push_back(s);
                    numParticlesResolved++;
                    picsFilter->UpdateProgress(numParticlesResolved,
                                               numParticlesTotal);
                }
                else
                {
                    //debug1<<s->domain.domain<<endl;
                    HandleOOBIC(s);
                }
                DelRef(d);
            }
            else
            {
                //debug1<<"Opps. Dom disappared. "<<d.domain<<endl;
                HandleOOBIC(s);
            }
        }

        //See if we're done.
        if (!IamDone && activeICs.empty() && oobICs.empty())
        {
            numDone++;
            vector<int> msg(1);
            msg[0] = DONE;
            SendAllMsg(msg);
            IamDone = true;
        }
        
        CheckCacheVacancy(true);
        HandleMessages();
    }

    debug1<<"All done. terminated sz= "<<terminatedICs.size()<<endl;
    TotalTime.value += visitTimer->StopTimer(timer, "Execute");
}

// ****************************************************************************
// Method:  avtCommDSOnDemandICAlgorithm::HandleOOBIC
//
// Purpose: Handle ICs that exit a domain.
//   
// Programmer:  Dave Pugmire
// Creation:    December 15, 2010
//
// ****************************************************************************

void
avtCommDSOnDemandICAlgorithm::HandleOOBIC(avtIntegralCurve *s)
{
    //See if we already have the data.
    if (GetDataset(s->domain) != NULL)
    {
        activeICs.push_back(s);
        AddRef(s->domain);
        return;
    }
    
    oobICs.push_back(s);
}


// ****************************************************************************
// Method:  avtCommDSOnDemandICAlgorithm::RequestDataset
//
// Purpose: Post a request to the owner of the data.
//   
// Programmer:  Dave Pugmire
// Creation:    December 15, 2010
//
// ****************************************************************************

bool
avtCommDSOnDemandICAlgorithm::RequestDataset(DomainType &d)
{
    //See if request already made.
    set<int>::iterator it = pendingDomRequests.find(d.domain);
    if (it != pendingDomRequests.end())
        return false;
    
    //Make the request, and send it.
    vector<int> req(2);
    req[0] = DATASET_REQUEST;
    req[1] = d.domain;

    int domRank = DomainToRank(d);
    SendMsg(domRank, req);
    pendingDomRequests.insert(d.domain);
    pendingDomReqTimers[d.domain] = visitTimer->StartTimer();
    //debug1<<" ** RequestDS: "<<d.domain<<" from "<<domRank<<endl;
    return true;
}


// ****************************************************************************
// Method:  avtCommDSOnDemandICAlgorithm::HandleMessages
//
// Purpose: Process incoming messages.
//   
// Programmer:  Dave Pugmire
// Creation:    December 15, 2010
//
// Modifications:
//   Dave Pugmire, Fri Dec 17 12:15:04 EST 2010
//   Fix memory leaks by adding CheckPendingSendRequests().
//
//   Dave Pugmire, Wed Jan  5 07:57:21 EST 2011
//   New datastructures for msg/ic/ds.
//
//   Dave Pugmire, Fri Jan  7 14:21:59 EST 2011
//   Allow blockAndWait if no work to do.
//
// ****************************************************************************

bool
avtCommDSOnDemandICAlgorithm::HandleMessages(bool checkMsgs,
                                             bool checkDSs,
                                             bool allowBlockAndWait)

{
    vector<MsgCommData> msgs, *msgPtr = NULL;
    vector<DSCommData> ds, *dsPtr = NULL;

    bool blockAndWait = allowBlockAndWait && (activeICs.empty() && (numDone < nProcs));
    if (checkMsgs)
        msgPtr = &msgs;
    if (checkDSs)
        dsPtr = &ds;
    
    RecvAny(msgPtr, NULL, dsPtr, blockAndWait);

    bool retVal = false;
    if (checkMsgs && !msgs.empty())
    {
        for (int i = 0; i < msgs.size(); i++)
        {
            int sendRank = msgs[i].rank;
            vector<int> &m = msgs[i].message;
            int msgType = m[0];

            if (msgType == DONE)
                numDone++;
            
            else if (msgType == DATASET_REQUEST)
            {
                int dom = m[1];
                vtkDataSet *d = GetDomain(dom);
                if (d)
                {
                    vector<vtkDataSet *> ds;
                    vector<DomainType> doms;
                    doms.push_back(DomainType(dom, 0));
                    ds.push_back(d);
                    SendDS(sendRank, ds, doms);
                }
                else
                {
                    //Why am I being asked for a domain I don't own?
                    EXCEPTION0(ImproperUseException);
                }
            }
        }
        retVal = true;
    }

    //Check for incoming Datasets.
    if (checkDSs && !ds.empty())
    {
        AddDSToDomainCache(ds);
        
        for (int i = 0; i < ds.size(); i++)
        {
            ds[i].ds->Delete();
            set<int>::iterator it = pendingDomRequests.find(ds[i].dom.domain);
            pendingDomRequests.erase(it);
        }

        //See if we can move any ICs into the active pool.
        list<avtIntegralCurve *> tmp;
        while (!oobICs.empty())
        {
            avtIntegralCurve *ic = oobICs.front();
            oobICs.pop_front();
            if (GetDataset(ic->domain))
            {
                //debug5<<"Activate "<<ic->id<<" dom= "<<ic->domain<<endl;
                activeICs.push_back(ic);
                AddRef(ic->domain);
            }
            else
                tmp.push_back(ic);
        }

        oobICs.insert(oobICs.begin(), tmp.begin(), tmp.end());
        retVal = true;
    }

    CheckPendingSendRequests();
    
    return retVal;
}


// ****************************************************************************
// Method:  avtCommDSOnDemandICAlgorithm::ResetIntegralCurvesForContinueExecute
//
// Purpose: Reset ICs.
//   
// Programmer:  Dave Pugmire
// Creation:    December 15, 2010
//
// ****************************************************************************

void
avtCommDSOnDemandICAlgorithm::ResetIntegralCurvesForContinueExecute(int curTimeSlice)
{
    while (! terminatedICs.empty())
    {
        avtIntegralCurve *s = terminatedICs.front();
        terminatedICs.pop_front();
        
        activeICs.push_back(s);
        s->status = avtIntegralCurve::STATUS_OK;
    }
}

// ****************************************************************************
// Method:  avtCommDSOnDemandICAlgorithm::CheckNextTimeStepNeeded
//
// Purpose: See if next time slice is needed.
//   
// Programmer:  Dave Pugmire
// Creation:    December 15, 2010
//
// ****************************************************************************

bool
avtCommDSOnDemandICAlgorithm::CheckNextTimeStepNeeded(int curTimeSlice)
{
    list<avtIntegralCurve *>::const_iterator it;
    for (it = terminatedICs.begin(); it != terminatedICs.end(); it++)
    {
        if ((*it)->domain.domain != -1 && (*it)->domain.timeStep > curTimeSlice)
        {
            return true;
        }
    }
    
    return false;

}

// ****************************************************************************
// Method:  avtCommDSOnDemandICAlgorithm::GetDataset
//
// Purpose: Get dataset, if possible. Could come from static assignment, or cache.
//   
// Programmer:  Dave Pugmire
// Creation:    December 15, 2010
//
// ****************************************************************************

vtkDataSet *
avtCommDSOnDemandICAlgorithm::GetDataset(const DomainType &dom)
{
    //See if the PICS filter has it.
    vtkDataSet *ds = avtICAlgorithm::GetDomain(dom);
    if (ds)
        return ds;

    //See if the cache has it.
    ds = GetDSFromDomainCache(dom);
    if (ds)
        return ds;

    return NULL;
}


// ****************************************************************************
// Method:  avtCommDSOnDemandICAlgorithm::GetDSFromDomainCache
//
// Purpose: Get dataset from cache.
//   
// Programmer:  Dave Pugmire
// Creation:    December 15, 2010
//
// ****************************************************************************

vtkDataSet *
avtCommDSOnDemandICAlgorithm::GetDSFromDomainCache(const DomainType &dom)
{
    list<domainCacheEntry>::iterator it;

    for (it = domainCache.begin(); it != domainCache.end(); it++)
    {
        if (it->dom == dom)
        {
            vtkDataSet *ds = it->ds;
            
            //Move it to the front of the list.
            domainCacheEntry entry = *it;
            domainCache.erase(it);
            domainCache.push_front(entry);
            
            return ds;
        }
    }
    return NULL;
}


// ****************************************************************************
// Method:  avtCommDSOnDemandICAlgorithm::AddDSToDomainCache
//
// Purpose: Add new datset to cache.
//   
// Programmer:  Dave Pugmire
// Creation:    December 15, 2010
//
// ****************************************************************************

void
avtCommDSOnDemandICAlgorithm::AddDSToDomainCache(vector<DSCommData> &dss)
{
    int newDoms = dss.size();
    
    // Purge cache, as needed.
    if (domainCache.size() + newDoms > domainCacheSizeLimit)
    {
        int purgeCnt = 0;
        domainCache.sort(cacheSort);
        
        for (int i = 0; i < newDoms; i++)
        {
            domainCacheEntry &entry = domainCache.front();
            /*
            debug1<<" ** PURGE "<<entry.dom.domain<<":"<<entry.refCnt;
            if (entry.refCnt > 0)
                debug1<<" BAD PURGE";
            debug1<<endl;
            */
            
            entry.ds->Delete();
            
            domainCache.pop_front();
            DomPurgeCnt.value++;
        }
    }

    for (int i = 0; i < newDoms; i++)
    {
        domainCacheEntry entry(dss[i].dom.domain, dss[i].ds);

        domainCache.push_front(entry);
        dss[i].ds->Register(NULL);

        double latency = 0.0;
        map<int,int>::iterator itt = pendingDomReqTimers.find(dss[i].dom.domain);
        if (itt != pendingDomReqTimers.end())
        {
            latency = visitTimer->StopTimer(itt->second, "recvDS");
            pendingDomReqTimers.erase(itt);
        }
        DSLatencyTime.value += latency;
        
        //debug1<<" ** RECV DS "<<dss[i].dom.domain<<" latency= "<<latency<<endl;
    }
}


// ****************************************************************************
// Method:  avtCommDSOnDemandICAlgorithm::SortIntegralCurves
//
// Purpose: Sort on domain.
//   
//
// Programmer:  Dave Pugmire
// Creation:    December 17, 2010
//
// ****************************************************************************

void
avtCommDSOnDemandICAlgorithm::SortIntegralCurves(list<avtIntegralCurve *> &ics)
{
    int timerHandle = visitTimer->StartTimer();

    //We want to sort these by 'number of domains' in the whole list.
    list<avtIntegralCurve*>::iterator it;

    map<int,int> domCounts;
    map<int,int>::iterator mapIt;
    for (it = ics.begin(); it != ics.end(); it++)
    {
        mapIt = domCounts.find((*it)->domain.domain);
        if (mapIt == domCounts.end())
            domCounts[(*it)->domain.domain] = 1;
        else
            mapIt->second++;
    }

    for (it = ics.begin(); it != ics.end(); it++)
    {
        (*it)->sortKey = domCounts[(*it)->domain.domain];
    }
    
    ics.sort(icDomainCompare);

    SortTime.value += visitTimer->StopTimer(timerHandle, "SortIntegralCurves()");
}

// ****************************************************************************
// Method:  avtCommDSOnDemandICAlgorithm::Debug
//
// Purpose: Debug...
//
// Programmer:  Dave Pugmire
// Creation:    January  4, 2011
//
// ****************************************************************************

void
avtCommDSOnDemandICAlgorithm::Debug()
{
    list<avtIntegralCurve*>::iterator it;
    debug1<<endl;
    debug1<<"======================================================"<<endl;
    debug1<<"ActiveICs: "<<activeICs.size()<<" [";
    for (it = activeICs.begin(); it != activeICs.end(); it++)
        debug1<<(*it)->domain.domain<<" ";
    debug1<<"]"<<endl;
    debug1<<"OOB   ICs: "<<oobICs.size()<<" [";
    for (it = oobICs.begin(); it != oobICs.end(); it++)
        debug1<<(*it)->domain.domain<<" ";
    debug1<<"]"<<endl;

    list<domainCacheEntry>::iterator itt;
    debug1<<"Cache    : "<<domainCache.size()<<" [";
    for (itt = domainCache.begin(); itt != domainCache.end(); itt++)
        debug1<<itt->dom.domain<<":"<<itt->refCnt<<" ";
    debug1<<"]"<<endl;


    set<int>::iterator s;
    debug1<<"Reqs     : "<<pendingDomRequests.size()<<" [";
    for (s = pendingDomRequests.begin(); s != pendingDomRequests.end(); s++)
        debug1<<*s<<" ";
    debug1<<"]"<<endl;

    CheckCacheVacancy(false);

    debug1<<"======================================================"<<endl;
    debug1<<endl;
}


// ****************************************************************************
// Method:  avtCommDSOnDemandICAlgorithm::AddRef
//
// Purpose: Increase the domain ref count.
//
// Programmer:  Dave Pugmire
// Creation:    January  4, 2011
//
// ****************************************************************************

void
avtCommDSOnDemandICAlgorithm::AddRef(const DomainType &dom)
{
    list<domainCacheEntry>::iterator it;
    for (it = domainCache.begin(); it != domainCache.end(); it++)
        if (it->dom == dom)
        {
            it->refCnt++;
            break;
        }
}

// ****************************************************************************
// Method:  avtCommDSOnDemandICAlgorithm::DelRef
//
// Purpose: Decrease the domain ref count.
//
// Programmer:  Dave Pugmire
// Creation:    January  4, 2011
//
// ****************************************************************************

void
avtCommDSOnDemandICAlgorithm::DelRef(const DomainType &dom)
{
    list<domainCacheEntry>::iterator it;
    for (it = domainCache.begin(); it != domainCache.end(); it++)
        if (it->dom == dom)
        {
            it->refCnt--;
            break;
        }
}

// ****************************************************************************
// Method:  avtCommDSOnDemandICAlgorithm::CheckCacheVacancy
//
// Purpose: Check how much room is available in the cache.
//
// Programmer:  Dave Pugmire
// Creation:    January  4, 2011
//
// ****************************************************************************

void
avtCommDSOnDemandICAlgorithm::CheckCacheVacancy(bool makeReq)
{
    int cacheVacancy =  (domainCacheSizeLimit-1) - domainCache.size() - pendingDomRequests.size();

    list<domainCacheEntry>::iterator it;
    for (it = domainCache.begin(); it != domainCache.end(); it++)
        if (it->refCnt == 0)
            cacheVacancy++;

    //if (!makeReq)
    //    debug1<<"Cache Vacancy: "<<cacheVacancy<<endl;

    if (cacheVacancy <= 0 || !makeReq)
        return;

    SortIntegralCurves(oobICs);

    int reqs = 0;
    list<avtIntegralCurve*>::iterator itt;
    for (itt = oobICs.begin(); itt != oobICs.end(); itt++)
    {
        if (RequestDataset((*itt)->domain))
            reqs++;
        
        if (reqs == cacheVacancy)
            break;
    }
}

// ****************************************************************************
// Method:  avtCommDSOnDemandICAlgorithm::PostStepCallback
//
// Purpose: Do any processing needed after integration step has taken place.
//   
//
// Programmer:  Dave Pugmire
// Creation:    January 14, 2011
//
// ****************************************************************************

bool
avtCommDSOnDemandICAlgorithm::PostStepCallback()
{
    return HandleMessages(true, false, false);
}

#endif
