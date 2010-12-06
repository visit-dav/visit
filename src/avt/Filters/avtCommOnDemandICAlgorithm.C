/*****************************************************************************
*
* Copyright (c) 2000 - 2010, Lawrence Livermore National Security, LLC
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
//                              avtCommOnDemandICAlgorithm.C                 //
// ************************************************************************* //

#include <avtCommOnDemandICAlgorithm.h>
#include <TimingsManager.h>

#ifdef PARALLEL

using namespace std;

static const int DONE = 0;
static const int DATASET_REQUEST = 1;

avtCommOnDemandICAlgorithm::avtCommOnDemandICAlgorithm(avtPICSFilter *picsFilter)
    : avtParICAlgorithm(picsFilter)
{
}

avtCommOnDemandICAlgorithm::~avtCommOnDemandICAlgorithm()
{
}

void
avtCommOnDemandICAlgorithm::Initialize(vector<avtIntegralCurve *> &seedPts)
{
    int numRecvs = nProcs-1;
    if (numRecvs > 64)
        numRecvs = 64;
    
    avtParICAlgorithm::InitializeBuffers(seedPts, 8, numRecvs, numRecvs, 2*numRecvs);
    
    AddIntegralCurves(seedPts);
}

void
avtCommOnDemandICAlgorithm::AddIntegralCurves(vector<avtIntegralCurve *> &ics)
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
    debug5<<"My seeds. Active: "<<activeICs.size()<<" OOB: "<<oobICs.size()<<endl;
}


void
avtCommOnDemandICAlgorithm::RunAlgorithm()
{
    debug1<<"avtCommOnDemandICAlgorithm::RunAlgorithm()\n";
    int timer = visitTimer->StartTimer();

    //Sort the streamlines and load the first domain.
    SortIntegralCurves(activeICs);
    SortIntegralCurves(oobICs);

    int numDone = 0;
    int numParticlesTotal = activeICs.size() + oobICs.size();
    int numParticlesResolved = 0;
    
    bool IamDone = false;
    
    while (numDone < nProcs)
    {
        //Do work, if we have it....
        if (!activeICs.empty())
        {
            debug5<<"activeICs.size()= "<<activeICs.size()<<endl;
            avtIntegralCurve *s = activeICs.front();
            activeICs.pop_front();

            if (picsFilter->dataSets[s->domain.domain] != NULL)
            {
                AdvectParticle(s);
                if (s->status != avtIntegralCurve::STATUS_OK)
                {
                    terminatedICs.push_back(s);
                    numParticlesResolved++;
                    picsFilter->UpdateProgress(numParticlesResolved,
                                               numParticlesTotal);
                }
                else
                    HandleOOBIC(s);
            }
        }

        HandleMessages(numDone);

        //No actives, let's activate an oob IC.
        if (activeICs.empty() && !oobICs.empty())
        {
            SortIntegralCurves(oobICs);
            RequestDataset(oobICs.front()->domain);
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
    }

    TotalTime.value += visitTimer->StopTimer(timer, "Execute");
}

void
avtCommOnDemandICAlgorithm::HandleOOBIC(avtIntegralCurve *s)
{
    //See if we already have the data.
    if (picsFilter->dataSets[s->domain.domain] != NULL)
    {
        activeICs.push_back(s);
        return;
    }

    //Otherwise, request it, and put it in the OOB list.
    RequestDataset(s->domain);
    oobICs.push_back(s);
}

void
avtCommOnDemandICAlgorithm::RequestDataset(DomainType &d)
{
    //See if request already made.
    set<int>::iterator it = pendingDomRequests.find(d.domain);
    if (it != pendingDomRequests.end())
        return;
    
    //Make the request, and send it.
    vector<int> req(2);
    req[0] = DATASET_REQUEST;
    req[1] = d.domain;

    int domRank = DomainToRank(d);
    SendMsg(domRank, req);
    pendingDomRequests.insert(d.domain);
}

void
avtCommOnDemandICAlgorithm::HandleMessages(int &numDone)
{
    vector<vector<int> > msgs;
    if (RecvMsg(msgs))
    {
        for (int i = 0; i < msgs.size(); i++)
        {
            int sendRank = msgs[i][0];
            int msgType = msgs[i][1];

            if (msgType == DONE)
                numDone++;
            
            else if (msgType == DATASET_REQUEST)
            {
                int dom = msgs[i][2];
                vtkDataSet *d = picsFilter->dataSets[dom];
                vector<vtkDataSet *> ds;
                vector<DomainType> doms;
                doms.push_back(DomainType(dom, 0));
                ds.push_back(d);
                SendDS(sendRank, ds, doms);
            }
        }
    }

    //Check for incoming Datasets.
    vector<vtkDataSet *> ds;
    vector<DomainType> doms;
    if (RecvDS(ds, doms))
    {
        for (int i = 0; i < ds.size(); i++)
        {
            debug2<<"RECV a DS "<<doms[i]<<endl;
            
            vtkDataSet *d = ds[i];
            DomainType dom = doms[i];
            picsFilter->dataSets[dom.domain] = d;

            set<int>::iterator it = pendingDomRequests.find(dom.domain);
            pendingDomRequests.erase(it);
        }

        //See if we can move any ICs into the active pool.
        list<avtIntegralCurve *> tmp;
        debug5<<"Check activation: "<<oobICs.size()<<endl;
        while (!oobICs.empty())
        {
            avtIntegralCurve *ic = oobICs.front();
            oobICs.pop_front();
            if (picsFilter->dataSets[ic->domain.domain] != NULL)
            {
                debug5<<"Activate "<<ic->id<<" dom= "<<ic->domain<<endl;
                activeICs.push_back(ic);
            }
            else
                tmp.push_back(ic);
        }

        oobICs.insert(oobICs.begin(), tmp.begin(), tmp.end());
    }
}

void
avtCommOnDemandICAlgorithm::ResetIntegralCurvesForContinueExecute()
{
    EXCEPTION0(ImproperUseException);
    
    /*
    while (! terminatedICs.empty())
    {
        avtIntegralCurve *s = terminatedICs.front();
        terminatedICs.pop_front();
        
        activeICs.push_back(s);
        s->status = avtIntegralCurve::STATUS_OK;
    }
    */
}

bool
avtCommOnDemandICAlgorithm::CheckNextTimeStepNeeded(int curTimeSlice)
{
    EXCEPTION0(ImproperUseException);

    /*
    list<avtIntegralCurve *>::const_iterator it;
    for (it = terminatedICs.begin(); it != terminatedICs.end(); it++)
    {
        if ((*it)->domain.domain != -1 && (*it)->domain.timeStep > curTimeSlice)
        {
            return true;
        }
    }
    */
    
    return false;

}

#endif
