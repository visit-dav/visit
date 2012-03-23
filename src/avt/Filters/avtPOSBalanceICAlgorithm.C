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
//                              avtPOSBalanceICAlgorithm.C                   //
// ************************************************************************* //

#include <avtPOSBalanceICAlgorithm.h>
#include <TimingsManager.h>

using namespace std;

#ifdef PARALLEL
static const int GIMME_PARTICLES = 0;
static const int TERMINATE = 1;


avtPOSBalanceICAlgorithm *
avtPOSBalanceICAlgorithm::Create(avtPICSFilter *picsFilter, int nToSend)
{
    debug1<<__FILE__<<" "<<__LINE__<<endl;
    if (PAR_Rank() == 0)
        return new avtPOSBalanceMasterICAlgorithm(picsFilter, nToSend);
    else
        return new avtPOSBalanceWorkerICAlgorithm(picsFilter, 0);
}

avtPOSBalanceICAlgorithm::avtPOSBalanceICAlgorithm(avtPICSFilter *picsFilter)
    : avtParICAlgorithm(picsFilter)
{
}

avtPOSBalanceICAlgorithm::~avtPOSBalanceICAlgorithm()
{
}

void
avtPOSBalanceICAlgorithm::Initialize(std::vector<avtIntegralCurve *> &seeds)
{
    int numRecvs = nProcs-1;
    if (numRecvs > 64)
        numRecvs = 64;
    
    avtParICAlgorithm::InitializeBuffers(seeds, 3, numRecvs, numRecvs);
}


void
avtPOSBalanceICAlgorithm::ResetIntegralCurvesForContinueExecute(int curTimeSlice)
{
    while (! terminatedICs.empty())
    {
        avtIntegralCurve *s = terminatedICs.front();
        terminatedICs.pop_front();
        
        activeICs.push_back(s);
        s->status = avtIntegralCurve::STATUS_OK;
    }
}

bool
avtPOSBalanceICAlgorithm::CheckNextTimeStepNeeded(int curTimeSlice)
{
    int val = 0;
    list<avtIntegralCurve *>::const_iterator it;
    for (it = terminatedICs.begin(); it != terminatedICs.end(); it++)
    {
        if ((*it)->domain.domain != -1 && (*it)->domain.timeStep > curTimeSlice)
        {
            val = 1;
            break;
        }
    }

    SumIntAcrossAllProcessors(val);

    return val > 0;
}

//*******************************************************************************
// Master
//*******************************************************************************

avtPOSBalanceMasterICAlgorithm::avtPOSBalanceMasterICAlgorithm(avtPICSFilter *picsFilter, int N) 
    : avtPOSBalanceICAlgorithm(picsFilter)
{
    numTerminated = 0;
    nToSend = N;
    if (nToSend == 0)
        nToSend = 5;
}

avtPOSBalanceMasterICAlgorithm::~avtPOSBalanceMasterICAlgorithm()
{
}

void
avtPOSBalanceMasterICAlgorithm::Initialize(std::vector<avtIntegralCurve *> &seeds)
{
    avtPOSBalanceICAlgorithm::Initialize(seeds);
    AddIntegralCurves(seeds);
}

void
avtPOSBalanceMasterICAlgorithm::AddIntegralCurves(std::vector<avtIntegralCurve*> &ics)
{
    vector<avtIntegralCurve *>::const_iterator it;
    for (it = ics.begin(); it != ics.end(); it++)
        unassignedICs.push_back(*it);

    SortIntegralCurves(unassignedICs);

    //If few seeds, and many procs, adjust nToSend.
    int nSeeds = unassignedICs.size();
    if (nToSend * nProcs > nSeeds)
    {
        nToSend = nSeeds/nProcs;
        if (nToSend == 0)
            nToSend = 1;
    }
    
    //Send out some ICs.
    for (int i = 0; i < nProcs; i++)
    {
        if (i == rank)
            continue;
        
        //No more ICs, tell the rest to terminate.
        if (unassignedICs.empty())
        {
            vector<int> msg(1);
            msg[0] = TERMINATE;
            SendMsg(i, msg);
            numTerminated++;
        }
        else
        {
            int cnt = 0;
            vector<avtIntegralCurve *> ics;
        
            while (!unassignedICs.empty() && cnt < nToSend)
            {
                avtIntegralCurve *ic = unassignedICs.front();
                unassignedICs.pop_front();
                ics.push_back(ic);
                cnt++;
            }
            if (! ics.empty())
                SendICs(i, ics);
        }
    }
}


void
avtPOSBalanceMasterICAlgorithm::RunAlgorithm()
{
    debug1<<"avtPOSBalanceMasterICAlgorithm::RunAlgorithm()"<<endl;
    int timer = visitTimer->StartTimer();
    
    //Sort the streamlines and load the first domain.
    SortIntegralCurves(activeICs);

    while (1)
    {
        vector<MsgCommData> msgs;
        RecvAny(&msgs, NULL, NULL, true);
        for (int i = 0; i < msgs.size(); i++)
        {
            int fromRank = msgs[i].rank;
            if (msgs[i].message[0] == GIMME_PARTICLES)
            {
                if (unassignedICs.empty())
                {
                    vector<int> msg(1);
                    msg[0] = TERMINATE;
                    debug1<<"Send Terminate to "<<fromRank<<endl;
                    SendMsg(fromRank, msg);
                    numTerminated++;
                }
                else
                {
                    int cnt = 0;
                    vector<avtIntegralCurve *> ics;
                    
                    while (!unassignedICs.empty() && cnt < nToSend)
                    {
                        avtIntegralCurve *ic = unassignedICs.front();
                        unassignedICs.pop_front();
                        ics.push_back(ic);
                        cnt++;
                    }
                    
                    if (! ics.empty())
                    {
                        debug1<<"Sending "<<ics.size()<<" to "<<fromRank<<endl;
                        SendICs(fromRank, ics);
                    }
                }
            }
        }
        //Everyone has terminated, done.
        if (numTerminated == (nProcs-1))
            break;
        
        CheckPendingSendRequests();
        
#if 0

         while (! activeICs.empty())
         {
            avtIntegralCurve *ic = activeICs.front();
            activeICs.pop_front();
            
             if (DomainLoaded(ic->domain))
             {
                 GetDomain(ic); //FIX THIS!!!
                 AdvectParticle(ic);

                 if( ic->status != avtIntegralCurve::STATUS_OK )
                 {
                     terminatedICs.push_back(ic);
                     /*
                     numParticlesResolved++;
                     picsFilter->UpdateProgress(numParticlesResolved,
                                                numParticlesTotal);
                     */
                 }
                 else
                 {
                     if( ic->domain.domain == -1 && ic->domain.timeStep == -1 )
                         terminatedICs.push_back(ic);
                     else
                         oobICs.push_back(ic);
                 }
             }
             else
             {
                 if (ic->domain.domain == -1 && ic->domain.timeStep == -1)
                     terminatedICs.push_back(ic);
                 else
                     oobICs.push_back(ic);
             }

         }

         if (oobICs.empty())
         {
             if (rank == 0)
             {
             }
             if (rank != 0)
             {
                 //Get some more..
                 vector<int> msg(1);
                 msg[0] = GIMME_PARTICLES;
                 SendMsg(master, msg);
                 
                 //Wait for something.
                 vector<MsgCommData> msgs;
                 list<ICCommData> ics;
                 RecvAny(&msgs, &ics, NULL, true);
                 for (int i = 0; i < msgs.size(); i++)
                 {
                     if (msgs[i].message[0] == TERMINATE)
                         break;
                 }
                 if (!ics.empty())
                 {
                     list<ICCommData>::iterator s;
                     for (s = ics.begin(); s != ics.end(); s++)
                         oobICs.push_back((*s).ic);
                 }
             }
         }
         
        //Sort the remaining streamlines, get the next domain, continue.
        activeICs = oobICs;
        oobICs.clear();
        
        SortIntegralCurves(activeICs);
        avtIntegralCurve *s = activeICs.front();
        GetDomain(s);
#endif
    }

    CheckPendingSendRequests();
    TotalTime.value += visitTimer->StopTimer(timer, "Execute");
}


//*******************************************************************************
// Worker
//*******************************************************************************


avtPOSBalanceWorkerICAlgorithm::avtPOSBalanceWorkerICAlgorithm(avtPICSFilter *picsFilter, int m)
    : avtPOSBalanceICAlgorithm(picsFilter)
{
    master = m;
}

avtPOSBalanceWorkerICAlgorithm::~avtPOSBalanceWorkerICAlgorithm()
{
}

void
avtPOSBalanceWorkerICAlgorithm::Initialize(std::vector<avtIntegralCurve *> &seeds)
{
    avtPOSBalanceICAlgorithm::Initialize(seeds);
    AddIntegralCurves(seeds);
}

void
avtPOSBalanceWorkerICAlgorithm::AddIntegralCurves(std::vector<avtIntegralCurve*> &ics)
{
    vector<avtIntegralCurve *>::const_iterator it;
    for (it = ics.begin(); it != ics.end(); it++)
        delete *it;
}

bool
avtPOSBalanceWorkerICAlgorithm::HandleMessages()
{
    vector<MsgCommData> msgs;
    list<ICCommData> ics;
    bool block = activeICs.empty() && oobICs.empty();
    debug1<<__FILE__<<" "<<__LINE__<<" block= "<<block<<endl;
    RecvAny(&msgs, &ics, NULL, block);
    
    if (!ics.empty())
    {
        list<ICCommData>::iterator s;
        for (s = ics.begin(); s != ics.end(); s++)
            activeICs.push_back((*s).ic);
        SortIntegralCurves(activeICs);
    }
        
    //debug1<<"Recvd: msgs= "<<msgs.size()<<" ics= "<<ics.size()<<endl;
    for (int i = 0; i < msgs.size(); i++)
    {
        if (msgs[i].message[0] == TERMINATE)
            return true;
    }
    
    return false;
}


void
avtPOSBalanceWorkerICAlgorithm::RunAlgorithm()
{
    debug1<<"avtPOSBalanceWorkerICAlgorithm::RunAlgorithm()"<<endl;

    int timer = visitTimer->StartTimer();
    while (1)
    {
        bool terminate = HandleMessages();
        if (terminate)
            break;
        
        // Integrate all loaded domains.
        while (! activeICs.empty())
        {
            avtIntegralCurve *s = activeICs.front();
            activeICs.pop_front();

            if (DomainLoaded(s->domain))
            {
#ifndef PARALLEL
                // In serial, we return that every domain is loaded.
                // That's basically okay.
                // But it screws up time reporting, because the domain isn't
                // loaded until AdvectParticle calls GetDomain.
                // So call it explicitly first, so I/O can be correctly 
                // counted.
                debug1<<"LOAD: "<<s->domain<<endl;
                GetDomain(s);  //FIX THIS!!!
#endif
                debug1<<"Advecting "<<s->id<<endl;
                AdvectParticle(s);
                debug1<<"DONE!!! Advecting "<<s->id<<endl;

                if( s->status != avtIntegralCurve::STATUS_OK )
                {
                    terminatedICs.push_back(s);
                    /*
                    numParticlesResolved++;
                    picsFilter->UpdateProgress(numParticlesResolved,
                                               numParticlesTotal);
                    */
                }
                else
                {
                    if( s->domain.domain == -1 && s->domain.timeStep == -1 )
                        terminatedICs.push_back(s);
                    else
                        oobICs.push_back(s);
                }
            }
            else
            {
                if( s->domain.domain == -1 && s->domain.timeStep == -1 )
                    terminatedICs.push_back(s);
                else
                    oobICs.push_back(s);
            }
        }

        //Ask for more.
        if (oobICs.empty())
        {
            vector<int> msg(1);
            msg[0] = GIMME_PARTICLES;
            SendMsg(master, msg);
        }
        else
        {
            //Sort the remaining streamlines, get the next domain, continue.
            activeICs = oobICs;
            oobICs.clear();
        
            SortIntegralCurves(activeICs);
            avtIntegralCurve *s = activeICs.front();
            debug1<<__FILE__<<" "<<__LINE__<<endl;
            debug1<<"LOAD: "<<s->domain<<endl;
            GetDomain(s);
        }
        CheckPendingSendRequests();
    }
    
    CheckPendingSendRequests();
    TotalTime.value += visitTimer->StopTimer(timer, "Execute");
}

#endif
