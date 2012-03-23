/*****************************************************************************
*
* Copyright (c) 2000 - 2011, Lawrence Livermore National Security, LLC
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
//                              avtPODICAlgorithm.C                          //
// ************************************************************************* //

#include <avtPODICAlgorithm.h>
#include <TimingsManager.h>

using namespace std;

#ifdef PARALLEL

// ****************************************************************************
// Method:  avtPODICAlgorithm::avtPODICAlgorithm
//
// Purpose: constructor
//
// Programmer:  Dave Pugmire
// Creation:    March 21, 2012
//
// ****************************************************************************


avtPODICAlgorithm::avtPODICAlgorithm(avtPICSFilter *picsFilter, int count)
    : avtParICAlgorithm(picsFilter)
{
    numICs = 0;
    maxCount = count;
}

// ****************************************************************************
// Method:  avtPODICAlgorithm::~avtPODICAlgorithm
//
// Purpose: destructor
//
// Programmer:  Dave Pugmire
// Creation:    March 21, 2012
//
// ****************************************************************************

avtPODICAlgorithm::~avtPODICAlgorithm()
{
}

// ****************************************************************************
// Method:  avtPODICAlgorithm::Initialize
//
// Purpose: Initialize
//
// Programmer:  Dave Pugmire
// Creation:    March 21, 2012
//
// ****************************************************************************

void
avtPODICAlgorithm::Initialize(vector<avtIntegralCurve *> &seeds)
{
    int numRecvs = 64;
    if (numRecvs > nProcs)
        numRecvs = nProcs-1;

    avtParICAlgorithm::InitializeBuffers(seeds, 1, 1, numRecvs);
    AddIntegralCurves(seeds);
}

// ****************************************************************************
// Method:  avtPODICAlgorithm::ResetIntegralCurvesForContinueExecute
//
// Purpose: Reset for continuation.
//
// Programmer:  Dave Pugmire
// Creation:    March 21, 2012
//
// ****************************************************************************

void
avtPODICAlgorithm::ResetIntegralCurvesForContinueExecute(int curTimeSlice)
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
// Method:  avtPODICAlgorithm::CheckNextTimeStepNeeded
//
// Purpose: See if more timesteps needed.
//
// Programmer:  Dave Pugmire
// Creation:    March 21, 2012
//
// ****************************************************************************

bool
avtPODICAlgorithm::CheckNextTimeStepNeeded(int curTimeSlice)
{
    int val = 0;
    list<avtIntegralCurve *>::const_iterator it;
    for (it = terminatedICs.begin(); it != terminatedICs.end(); it++)
    {
        if ((*it)->domain.domain != -1 && (*it)->domain.timeStep != curTimeSlice)
        {
            val = 1;
            break;
        }
    }

    SumIntAcrossAllProcessors(val);

    return val > 0;
}

// ****************************************************************************
// Method:  avtPODICAlgorithm::AddIntegralCurves
//
// Purpose: Add ICs.
//
// Programmer:  Dave Pugmire
// Creation:    March 21, 2012
//
// ****************************************************************************

void
avtPODICAlgorithm::AddIntegralCurves(vector<avtIntegralCurve*> &ics)
{
    //Get the ICs that I own.
    for (int i = 0; i < ics.size(); i++)
    {
        avtIntegralCurve *s = ics[i];
        
        if (OwnDomain(s->domain))
        {
            avtVector endPt;
            s->CurrentLocation(endPt);
            
            if (PointInDomain(endPt, s->domain))
            {
                activeICs.push_back(s);
                s->originatingRank = rank;
            }
            else
                delete s;
        }
        else
            delete s;
    }

    numICs = activeICs.size();
    SumIntAcrossAllProcessors(numICs);


    if (DebugStream::Level1())
    {
        debug1<<"My ICcount= "<<activeICs.size()<<endl;
        debug1<<"numICs= "<<numICs<<endl;
        debug1<<"I own: [";
        for (int i = 0; i < numDomains; i++)
        {
            DomainType d(i,0);
            if (OwnDomain(d))
                debug1<<i<<" ";
        }
        debug1<<"]\n";
    }
}

// ****************************************************************************
// Method:  avtPODICAlgorithm::PreRunAlgorithm
//
// Purpose:
//
// Programmer:  Dave Pugmire
// Creation:    March 21, 2012
//
// ****************************************************************************

void
avtPODICAlgorithm::PreRunAlgorithm()
{
    picsFilter->InitializeLocators();
}

// ****************************************************************************
// Method:  avtPODICAlgorithm::RunAlgorithm
//
// Purpose: Run algorithm.
//
// Programmer:  Dave Pugmire
// Creation:    March 21, 2012
//
// ****************************************************************************

void
avtPODICAlgorithm::RunAlgorithm()
{
    if (DebugStream::Level1())
        debug1<<"avtPODICAlgorithm::RunAlgorithm(): numICs: "<<numICs<< " activeICs.size(): " << activeICs.size() << endl;
    int timer = visitTimer->StartTimer();
    
    bool done = false;

    while (!done)
    {
        //Integrate upto maxCnt streamlines.
        list<avtIntegralCurve *>::iterator s;
        int cnt = 0;
        while (cnt < maxCount && !activeICs.empty())
        {
            avtIntegralCurve *s = activeICs.front();
            activeICs.pop_front();
            
            if (s->domain.domain == -1 && s->seedPtDomainList.size() > 0)
            {
                // We don't have the particle, but someone else might.
                // This happens with pathlines when a new time slice is
                // loaded and the domain that contains the particle is
                // on a different MPI task.
                HandleOOBICs(s);
                cnt++;
                continue;
            }
           
            AdvectParticle(s);
            if (s->status == avtIntegralCurve::STATUS_FINISHED)
                terminatedICs.push_back(s);
            else
                HandleOOBICs(s);
            
            cnt++;
        }
        done = HandleCommunication();
    }

    TotalTime.value += visitTimer->StopTimer(timer, "Execute");
}

// ****************************************************************************
// Method:  avtPODICAlgorithm::HandleOOBICs
//
// Purpose: Handle out of bounds ICs.
//
// Programmer:  Dave Pugmire
// Creation:    March 21, 2012
//
// ****************************************************************************

void
avtPODICAlgorithm::HandleOOBICs(avtIntegralCurve *ic)
{
    //debug1<<"Handle OOBIC: "<<ic->id<<": ";
    for (int i = 0; i < ic->seedPtDomainList.size(); i++)
    {
        int domRank = DomainToRank(ic->seedPtDomainList[i]);
        if (domRank == rank)
        {
            avtVector endPt;
            ic->CurrentLocation(endPt);
            
            if (PointInDomain(endPt, ic->domain))
            {
                //debug1<<" MINE"<<endl;
                activeICs.push_back(ic);
                return;
            }
        }
    }
    
    //Not my IC.
    //debug1<<" OTHER"<<endl;
    oobICs.push_back(ic);
}

// ****************************************************************************
// Method:  avtPODICAlgorithm::HandleCommunication
//
// Purpose: Process communication.
//
// Programmer:  Dave Pugmire
// Creation:    March 21, 2012
//
// ****************************************************************************

bool
avtPODICAlgorithm::HandleCommunication()
{
    debug1<<"avtPODICAlgorithm::HandleCommunication()"<<endl;
    
    int numActive = oobICs.size() + activeICs.size();
    
    //See if we're done.
    SumIntAcrossAllProcessors(numActive);
    MsgCnt.value++;
    if (numActive == 0)
        return true;

    //Exchange ICs.
    int *icCounts = new int[nProcs], *allCounts = new int[nProcs];
    for (int i = 0; i < nProcs; i++)
        icCounts[i] = 0;
    
    list<avtIntegralCurve*>::iterator s;
    for (s = oobICs.begin(); s != oobICs.end(); s++)
    {
        for (int i = 0; i < (*s)->seedPtDomainList.size(); i++)
        {
            int domRank = DomainToRank((*s)->seedPtDomainList[i]);
            icCounts[domRank]++;
        }
    }
    SumIntArrayAcrossAllProcessors(icCounts, allCounts, nProcs);
    int incomingCnt = allCounts[rank];

    /*
    debug1<<"allCounts[";
    for(int i = 0; i < nProcs; i++)
        debug1<<allCounts[i]<<" ";
    debug1<<endl;
    */

    //Send out ICs.
    map<int, vector<avtIntegralCurve *> > sendICs;
    map<int, vector<avtIntegralCurve *> >::iterator it;

    //Collect into vectors for each rank.
    vector<int> domainIndices;
    for (s = oobICs.begin(); s != oobICs.end(); s++)
    {
        for (int i = 0; i < (*s)->seedPtDomainList.size(); i++)
        {
            int domRank = DomainToRank((*s)->seedPtDomainList[i]);
            domainIndices.push_back(i);
            it = sendICs.find(domRank);
            if (it == sendICs.end())
            {
                vector<avtIntegralCurve *> v;
                v.push_back(*s);
                sendICs[domRank] = v;
            }
            else
                it->second.push_back(*s);
        }
    }
    oobICs.clear();

    //Send out ICs.
    for (it = sendICs.begin(); it != sendICs.end(); it++)
        SendICs(it->first, it->second, domainIndices);

    //Wait for all my ics.
    while (incomingCnt > 0)
    {
        list<ICCommData> ics;
        RecvAny(NULL, &ics, NULL, true);
        
        list<ICCommData>::iterator s;
        for (s = ics.begin(); s != ics.end(); s++)
        {
            avtIntegralCurve *ic = (*s).ic;
            avtVector endPt;
            debug1<<"received :ic.id="<<ic->id<<" from "<<(*s).rank<<endl;

            ic->CurrentLocation(endPt);
            if (PointInDomain(endPt, ic->domain))
                activeICs.push_back(ic);
            else
                delete ic;
        }

        incomingCnt -= ics.size();
        CheckPendingSendRequests();
    }
    
    CheckPendingSendRequests();
    
    delete [] icCounts;
    delete [] allCounts;
    return false;
}

#endif
