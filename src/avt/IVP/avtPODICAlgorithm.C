/*****************************************************************************
*
* Copyright (c) 2000 - 2015, Lawrence Livermore National Security, LLC
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
#include <VisItStreamUtil.h>

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
    for (size_t i = 0; i < ics.size(); i++)
    {
        avtIntegralCurve *ic = ics[i];
        
        if (DomainLoaded(ic->blockList.front()))
        {
            ic->originatingRank = rank;
            activeICs.push_back(ic);
            
#ifdef USE_IC_STATE_TRACKING
            ic->InitTrk();
#endif
        }
        else
            delete ic;
    }

    if (DebugStream::Level1())
    {
        debug1<<"My ICcount= "<<activeICs.size()<<endl;
        debug1<<"I own: [";
        for (int i = 0; i < numDomains; i++)
        {
            BlockIDType d(i,0);
            if (OwnDomain(d)) 
            {
                debug1<<i<<" ";
            }
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
// Modifications:
//
//   Hank Childs, Wed Mar 28 08:36:34 PDT 2012
//   Add support for terminated particle status.
//
// ****************************************************************************

void
avtPODICAlgorithm::RunAlgorithm()
{
    debug1<<"avtPODICAlgorithm::RunAlgorithm() activeICs: "<<activeICs.size()<<" inactiveICs: "<<inactiveICs.size()<<endl;
    
    int timer = visitTimer->StartTimer();
    
    bool done = HandleCommunication();
    while (!done)
    {
        int cnt = 0;
        while (cnt < maxCount && !activeICs.empty())
        {
            avtIntegralCurve *ic = activeICs.front();
            activeICs.pop_front();

            do
            {
                AdvectParticle(ic);
            }
            while (ic->status.Integrateable() &&
                   DomainLoaded(ic->blockList.front()));
            
            if (ic->status.EncounteredSpatialBoundary())
            {
                if (!ic->blockList.empty() && DomainLoaded(ic->blockList.front()))
                    activeICs.push_back(ic);
                else
                    inactiveICs.push_back(ic);
            }
            else
                terminatedICs.push_back(ic);
            
            cnt++;
        }
        
        done = HandleCommunication();
    }

    TotalTime.value += visitTimer->StopTimer(timer, "Execute");
}

// ****************************************************************************
// Method:  avtPODICAlgorithm::HandleCommunication
//
// Purpose: Process communication.
//
// Programmer:  Dave Pugmire
// Creation:    March 21, 2012
//
// Modifications:
//
//   Dave Pugmire, Fri Mar  8 15:49:14 EST 2013
//   Bug fix. Ensure that the same IC isn't sent to the same rank. Also, when
//   an IC is received, set the domain from the particle point.
//
// ****************************************************************************

bool
avtPODICAlgorithm::HandleCommunication()
{
    int numICs = inactiveICs.size() + activeICs.size();
    
    //See if we're done.
    SumIntAcrossAllProcessors(numICs);
    MsgCnt.value++;

    //debug1<<"avtPODICAlgorithm::HandleCommunication() numICs= "<<numICs<<endl;
    if (numICs == 0)
        return true;

    //Tell everyone how many ICs are coming their way.
    int *icCounts = new int[nProcs], *allCounts = new int[nProcs];
    for (int i = 0; i < nProcs; i++)
        icCounts[i] = 0;
    
    list<avtIntegralCurve*>::iterator s;
    map<int, vector<avtIntegralCurve *> > sendICs;
    map<int, vector<avtIntegralCurve *> >::iterator it;
    list<avtIntegralCurve*> tmp;
    for (s = inactiveICs.begin(); s != inactiveICs.end(); s++)
    {
        int domRank = DomainToRank((*s)->blockList.front());
        icCounts[domRank]++;
            
        //Add to sending map.
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
    inactiveICs.clear();
    
    SumIntArrayAcrossAllProcessors(icCounts, allCounts, nProcs);
    bool anyToSend = false;
    for (int i = 0; i < nProcs && !anyToSend; i++)
        anyToSend = (allCounts[i] > 0);
    
    int incomingCnt = allCounts[rank];
    
    //Send out my ICs.
    for (it = sendICs.begin(); it != sendICs.end(); it++)
        SendICs(it->first, it->second);

    //Wait till I get all my ICs.
    while (incomingCnt > 0)
    {
        list<ICCommData> ics;
        list<ICCommData>::iterator s;

        RecvAny(NULL, &ics, NULL, true);
        for (s = ics.begin(); s != ics.end(); s++)
        {
            avtIntegralCurve *ic = (*s).ic;

            //See if I have this block.
            BlockIDType blk;
            list<BlockIDType> tmp;
            bool blockFound = false;
            while (!ic->blockList.empty())
            {
                blk = ic->blockList.front();
                ic->blockList.pop_front();
                if (DomainLoaded(blk))
                {
                    if (picsFilter->ICInBlock(ic, blk))
                    {
                        ic->status.ClearSpatialBoundary();
                        ic->blockList.clear();
                        ic->blockList.push_back(blk);
                        blockFound = true;
                        activeICs.push_back(ic);
                        break;
                    }
                }
                else
                    tmp.push_back(blk);
            }

            //IC Not in my blocks.  Terminate if blockList empty, or send to
            //block owner of next block in list.
            if (!blockFound)
            {
                ic->blockList = tmp;
                if (ic->blockList.empty())
                    terminatedICs.push_back(ic);
                else
                    inactiveICs.push_back(ic);
            }
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
