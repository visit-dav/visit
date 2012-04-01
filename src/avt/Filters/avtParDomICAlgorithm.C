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
//                              avtParDomICAlgorithm.C                       //
// ************************************************************************* //

#include <avtParDomICAlgorithm.h>
#include <TimingsManager.h>

using namespace std;

#ifdef PARALLEL

static const int PARTICLE_TERMINATE_COUNT = 0;
static const int PARTICLE_USED = 1;
static const int PARTICLE_NOT_USED = 2;

// ****************************************************************************
//  Method: avtParDomICAlgorithm::avtParDomICAlgorithm
//
//  Purpose:
//      avtParDomICAlgorithm constructor.
//
//  Programmer: Dave Pugmire
//  Creation:   January 27, 2009
//
//  Modifications:
//
//    Dave Pugmire, Fri Feb  6 08:43:00 EST 2009
//    Change numTerminated to numSLChange.
//
//   Dave Pugmire, Wed Apr  1 11:21:05 EDT 2009
//   Message size and number of receives moved to Initialize().
//
// ****************************************************************************

avtParDomICAlgorithm::avtParDomICAlgorithm(avtPICSFilter *icFilter,
                                           int maxCount)
    : avtParICAlgorithm(icFilter)
{
    numICChange = 0;
    totalNumIntegralCurves = 0;
    maxCnt = maxCount;
    sentICCounter = 0;
}

// ****************************************************************************
//  Method: avtParDomICAlgorithm::~avtParDomICAlgorithm
//
//  Purpose:
//      avtParDomICAlgorithm destructor
//
//  Programmer: Dave Pugmire
//  Creation:   January 27, 2009
//
// ****************************************************************************

avtParDomICAlgorithm::~avtParDomICAlgorithm()
{
}

// ****************************************************************************
//  Method: avtParDomICAlgorithm::Initialize
//
//  Purpose:
//      Initialization.
//
//  Programmer: Dave Pugmire
//  Creation:   January 27, 2009
//
//  Modifications:
//
//    Dave Pugmire, Fri Feb  6 08:43:00 EST 2009
//    Change numTerminated to numSLChange.
//
//   Dave Pugmire, Tue Mar 10 12:41:11 EDT 2009
//   Generalized domain to include domain/time. Pathine cleanup.
//
//   Dave Pugmire, Wed Apr  1 11:21:05 EDT 2009
//   Message size and number of receives moved to Initialize().
//
//   Dave Pugmire, Tue Nov  3 09:15:41 EST 2009
//   Establish how many streamlines there are once each proc determines if a
//   seed is in the domain.
//
//   Hank Childs, Fri Jun  4 19:58:30 CDT 2010
//   Use avtStreamlines, not avtStreamlineWrappers.
//
//   Hank Childs, Sun Jun  6 12:21:30 CDT 2010
//   Rename method called in this function to reflect the new emphasis 
//   in particle advection, as opposed to streamlines.
//
//   Hank Childs, Mon Jun  7 14:57:13 CDT 2010
//   Reflect change in name to InitializeBuffers method.
//
//   Dave Pugmire, Fri Nov  5 15:39:58 EDT 2010
//   Fix for unstructured meshes. Need to account for particles that are sent to domains
//   that based on bounding box, and the particle does not lay in any cells.
//
// ****************************************************************************

void
avtParDomICAlgorithm::Initialize(vector<avtIntegralCurve *> &seedPts)
{
    int numRecvs = nProcs-1;
    if (numRecvs > 64)
        numRecvs = 64;
    
    avtParICAlgorithm::InitializeBuffers(seedPts, 3, numRecvs, numRecvs);
    numICChange = 0;
    AddIntegralCurves(seedPts);
}

// ****************************************************************************
//  Method: avtParDomICAlgorithm::AddIntegralCurves
//
//  Purpose:
//      Add streamlines
//
//  Programmer: Dave Pugmire
//  Creation:   December 3, 2009
//
//  Modifications:
//
//   Hank Childs, Fri Jun  4 19:58:30 CDT 2010
//   Use avtStreamlines, not avtStreamlineWrappers.
//
//   Hank Childs, Sun Jun  6 12:21:30 CDT 2010
//   Rename this method to reflect the new emphasis in particle advection, as
//   opposed to streamlines.
//
// ****************************************************************************

void
avtParDomICAlgorithm::AddIntegralCurves(std::vector<avtIntegralCurve*> &ics)
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

    //Check for seeds owned by multiple procs.
    /*
    int *idBuffer = new int[numSeedPoints], *idBuffer2 = new int[numSeedPoints];
    for (int i = 0; i < numSeedPoints; i++)
        idBuffer[i] = 0;
    
    list<avtIntegralCurve *>::iterator s;
    for (s = activeICs.begin(); s != activeICs.end(); ++s)
        idBuffer[(*s)->id]++;

    SumIntArrayAcrossAllProcessors(idBuffer, idBuffer2, numSeedPoints);
    for (int i = 0; i < numSeedPoints; i++)
    {
        debug5<<"SEED_COUNT: id= "<<i<<" cnt= "<<idBuffer2[i]<<endl;
        if (idBuffer2[i] > 1)
        {
            debug5<<"DUPLICATE SEED: "<<i<<endl;
        }
    }

    delete [] idBuffer;
    delete [] idBuffer2;
    */

    totalNumIntegralCurves = activeICs.size();
    SumIntAcrossAllProcessors(totalNumIntegralCurves);
    origNumIntegralCurves = totalNumIntegralCurves;
    numSeedPoints = totalNumIntegralCurves;
    /*
    debug5<<"Init_totalNumIntegralCurves= "<<totalNumIntegralCurves<<endl;
    debug5<<"My ICs: "<<endl;
    list<avtIntegralCurve *>::iterator s;
    for (s = activeICs.begin(); s != activeICs.end(); ++s)
        debug5<<"ID "<<(*s)->id<<" dom= "<<(*s)->domain<<endl;
    */

    if (DebugStream::Level1())
    {
        debug1<<"My ICcount= "<<activeICs.size()<<endl;
        debug1<<"totalNumIntegralCurves= "<<totalNumIntegralCurves<<endl;
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
//  Method: avtParDomICAlgorithm::PreRunAlgorithm
//
//  Purpose:
//      "PreRun" for static domain IC algorithm.  In this case, initialize
//      the cell locators.  If we don't do it now, we will have processors
//      busy waiting, and then doing the initialization when they finally
//      get work ... meaning unnecessary delays.
//  
//  Programmer: Hank Childs
//  Creation:   February 19, 2010
//
// ****************************************************************************

void
avtParDomICAlgorithm::PreRunAlgorithm()
{
    picsFilter->InitializeLocators();
}

// ****************************************************************************
//  Method: avtParDomICAlgorithm::RunAlgorithm
//
//  Purpose:
//      Execute the static domain IC algorithm.
//
//  Programmer: Dave Pugmire
//  Creation:   January 27, 2009
//
//  Modifications:
//
//   Dave Pugmire, Wed Feb  4 16:17:40 EST 2009
//   Regression fix. Handling streamlines that lie in multiple domains after
//   integration was not handled correctly after the code refactor. Added
//   HandleOOBIC().
//
//   Dave Pugmire, Fri Feb  6 08:43:00 EST 2009
//   Change numTerminated to numICChange.
//
//   Dave Pugmire, Wed Apr  1 11:21:05 EDT 2009
//   Remove ExchangeICs() method.
//
//   Dave Pugmire, Thu Sep 24 13:52:59 EDT 2009
//   Replace Execute() with RunAlgorithm().
//
//   Hank Childs, Fri Jun  4 19:58:30 CDT 2010
//   Use avtStreamlines, not avtStreamlineWrappers.
//
//   Hank Childs, Sun Jun  6 12:21:30 CDT 2010
//   Rename data members to reflect the new emphasis in particle advection, as
//   opposed to streamlines.
//
//   Dave Pugmire, Fri Nov  5 15:39:58 EDT 2010
//   Fix for unstructured meshes. Need to account for particles that are sent to domains
//   that based on bounding box, and the particle does not lay in any cells.
//
// ****************************************************************************

void
avtParDomICAlgorithm::RunAlgorithm()
{
    if (DebugStream::Level1())
        debug1<<"avtParDomICAlgorithm::RunAlgorithm() - totalNumIntegralCurves: "<<totalNumIntegralCurves<< " activeICs.size(): " << activeICs.size() << endl;
    int timer = visitTimer->StartTimer();
    
    while (totalNumIntegralCurves > 0)
    {
        //Integrate upto maxCnt streamlines.
        list<avtIntegralCurve *>::iterator s;
        int cnt = 0;
        while (cnt < maxCnt && !activeICs.empty())
        {
            avtIntegralCurve *s = activeICs.front();
            activeICs.pop_front();
            
            AdvectParticle(s);
            if (s->status != avtIntegralCurve::STATUS_OK)
            {
                if (DebugStream::Level5())
                    debug5<<"TerminatedIC: "<<s->id<<endl;
                terminatedICs.push_back(s);
                numICChange--;
            }
            else
                HandleOOBIC(s);
            
            cnt++;
        }

        CheckPendingSendRequests();
        HandleCommunication();
    }

    CheckPendingSendRequests();
    TotalTime.value += visitTimer->StopTimer(timer, "Execute");
}

// ****************************************************************************
// Method:  avtParDomICAlgorithm::HandleCommunication
//
// Purpose:
//   Send termination msgs, and recv any incoming msgs/ICs.
//
// Programmer:  Dave Pugmire
// Creation:    February  1, 2011
//
// ****************************************************************************

void
avtParDomICAlgorithm::HandleCommunication()
{
    // Send terminations, if any.
    if (activeICs.size() == 0 && numICChange != 0)
    {
        vector<int> msg(2);
        msg[0] = PARTICLE_TERMINATE_COUNT;
        msg[1] = numICChange;
        SendAllMsg(msg);
        //debug5<<rank<<": SEND TERMINATE "<<numICChange<<endl;
        
        totalNumIntegralCurves += numICChange;
        picsFilter->UpdateProgress(
                       origNumIntegralCurves-totalNumIntegralCurves,
                       origNumIntegralCurves);
        numICChange = 0;
    }
    
    vector<MsgCommData> msgs;
    list<ICCommData> ics;
    bool blockAndWait = activeICs.empty() && (totalNumIntegralCurves > 0);

    RecvAny(&msgs, &ics, NULL, blockAndWait);

    if (!ics.empty())
        ProcessICs(ics);
    if (!msgs.empty())
        ProcessMsgs(msgs);
}

// ****************************************************************************
// Method:  avtParDomICAlgorithm::ProcessICs
//
// Purpose:
//   Handle incoming ICs, and see if they belong to me.
//
// Programmer:  Dave Pugmire
// Creation:    February  1, 2011
//
// David Camp, Mon Aug 15 12:55:54 PDT 2011
// Problem when sending IC that has 2 or more domains it could be in and if 2 
// or more are owned by 1 process, we only send the IC one time. So we need to
// test all possible domains when we recieved the IC.
//
// ****************************************************************************

void
avtParDomICAlgorithm::ProcessICs(list<ICCommData> &ics)
{
    list<ICCommData>::iterator s;
    for (s = ics.begin(); s != ics.end(); s++)
    {
        avtIntegralCurve *ic = (*s).ic;
        int sendRank = (*s).rank;
        
        vector<int> msg(3);
        msg[0] = PARTICLE_NOT_USED;
        msg[1] = ic->id;
        msg[2] = ic->counter;
        
        // fastest way to get the seedPtDomainList filled (see above)
        SetDomain( ic );

        avtVector endPt;
        ic->CurrentLocation(endPt);
        for (int i = 0; i < ic->seedPtDomainList.size(); i++)
        {
            DomainType dom = ic->seedPtDomainList[i];
            if (OwnDomain(dom))
            {
                // If point is inside domain, we are done.
                if (PointInDomain(endPt, dom))
                {
                    msg[0] = PARTICLE_USED;
                    ic->domain = ic->seedPtDomainList[i];
                    activeICs.push_back(ic);
                    break;
                }
            }
        }
        if( msg[0] == PARTICLE_NOT_USED )
        {
            delete ic;
        }

        SendMsg(sendRank, msg);
    }
}

// ****************************************************************************
// Method:  avtParDomICAlgorithm::ProcessMsgs
//
// Purpose:
//   Handle incoming messages.
//
// Programmer:  Dave Pugmire
// Creation:    February  1, 2011
//
// ****************************************************************************

void
avtParDomICAlgorithm::ProcessMsgs(vector<MsgCommData> &msgs)
{
    for (int i = 0; i < msgs.size(); i++)
    {
        int fromRank = msgs[i].rank;
        vector<int> &msg = msgs[i].message;
        int msgType = msg[0];
        
        if (msgType == PARTICLE_TERMINATE_COUNT)
        {
            totalNumIntegralCurves += msg[1];
            //debug5<<fromRank<<": RECV DECR"<<endl;
        }
        else
        {
            //Find the right entry.
            int icID = msg[1], icCnt = msg[2];
            pair<int,int> key(icID, icCnt);
            
            sendICInfoIterator i = sendICInfo.find(key);
            if (i == sendICInfo.end())
            {
                //debug5<<icID<<", "<<icCnt<<" not found in sendICInfo!!"<<endl;
                continue;
            }
            
            list<int>::iterator li;
            for (li = i->second.second.begin(); li != i->second.second.end(); li++)
                if (*li == fromRank)
                {
                    i->second.second.erase(li);
                    if (msgType == PARTICLE_USED)
                        i->second.first++;
                    break;
                }
            
            //Everyone has reported back.
            if (i->second.second.empty())
            {
                //Nobody used it
                if (i->second.first == 0)
                {
                    numICChange--;
                }
                else if (i->second.first > 1)
                {
                    numICChange += (i->second.first-1);
                }
                sendICInfo.erase(i);
            }
        }
    }
}

// ****************************************************************************
//  Method: avtParDomICAlgorithm::HandleOOBIC
//
//  Purpose:
//      Handle an out of bounds streamline.
//
//  Programmer: Dave Pugmire
//  Creation:   Febuary 4 2009
//
//  Modifications:
//
//    Dave Pugmire, Fri Feb  6 08:43:00 EST 2009
//    Change numTerminated to numSLChange. Account for SLs 'created' when
//    passing one SL multiple times.
//
//   Dave Pugmire, Wed Apr  1 11:21:05 EDT 2009
//   Send SLs using SendSLs.
//
//   Dave Pugmire, Thu Sep 24 13:52:59 EDT 2009
//   SLs are exchanged after processing, simplifying this method.
//
//   Dave Pugmire, Tue Nov  3 09:15:41 EST 2009
//   Bug fix. Set the new domain before communicating SL.
//
//   Hank Childs, Fri Jun  4 19:58:30 CDT 2010
//   Use avtStreamlines, not avtStreamlineWrappers.
//
//   Dave Pugmire, Fri Sep 24 12:42:49 EDT 2010
//   Handle case where single IC goes to multiple places. Increase number of ICs.
//
//   Dave Pugmire, Fri Nov  5 15:39:58 EDT 2010
//   Fix for unstructured meshes. Need to account for particles that are sent to domains
//   that based on bounding box, and the particle does not lay in any cells.
//
// ****************************************************************************

void
avtParDomICAlgorithm::HandleOOBIC(avtIntegralCurve *s)
{
    // The integrated streamline could lie in multiple domains.
    // Duplicate the IC and send to the proper owner.
    set<int> sentRanks;
    
    for (int i = 0; i < s->seedPtDomainList.size(); i++)
    {
        // if i > 0, we create new streamlines.
        //        if (i > 0)
        //            numICChange++;
        
        int domRank = DomainToRank(s->seedPtDomainList[i]);
        if (sentRanks.find(domRank) != sentRanks.end())
            continue;
        
        s->domain = s->seedPtDomainList[i];
        if (domRank == rank)
        {
            activeICs.push_back(s);
            //debug5<<"Handle OOB: id= "<<s->id<<" "<<s->domain<<" --> me"<<endl;
        }
        else
        {
            vector<avtIntegralCurve *> ics;
            s->counter = sentICCounter;
            ics.push_back(s);
            SendICs(domRank, ics);
            sentRanks.insert(domRank);
            
            //debug5<<"Handle OOB: id= "<<s->id<<" "<<s->domain<<" --> "<<domRank<<endl;

            //Add it to the sendICInfo.
            pair<int, int> key(s->id, s->counter);
            sendICInfoIterator i = sendICInfo.find(key);

            pair<int,list<int> > entry;
            if (i == sendICInfo.end())
            {
                entry.first = 0;
                entry.second.push_back(domRank);
                sendICInfo[key] = entry;
            }
            else
                i->second.second.push_back(domRank);
        }
    }

    sentICCounter++;
}

// ****************************************************************************
//  Method: avtParDomICAlgorithm::ResetIntegralCurvesForContinueExecute
//
//  Purpose:
//      Reset for continued streamline integration.
//
//  Programmer: Dave Pugmire
//  Creation:   Tue Aug 18 08:59:40 EDT 2009
//
//  Modifications:
//
//   Hank Childs, Fri Jun  4 19:58:30 CDT 2010
//   Use avtStreamlines, not avtStreamlineWrappers.
//
//   Hank Childs, Sun Jun  6 12:21:30 CDT 2010
//   Rename this method to reflect the new emphasis in particle advection, as
//   opposed to streamlines.
//
//   Dave Pugmire, Mon Nov 29 09:28:07 EST 2010
//   Need to synchronize to get number of total streamlines after continuation.
//
//   Dave Pugmire, Tue Nov 30 13:24:26 EST 2010
//   Change IC status when ic to not-terminated.
//
//   David Camp, Mon Aug 15 12:55:54 PDT 2011
//   For pathlines we need to call HandleOOBIC to handle changes to the mesh
//   that can happen in AMR data.
//
// ****************************************************************************

void
avtParDomICAlgorithm::ResetIntegralCurvesForContinueExecute(int curTimeSlice)
{
    std::list<avtIntegralCurve *> saveICs;

    // We may send IC to a new process, so we should just count before sending.
    totalNumIntegralCurves = terminatedICs.size();
    while (! terminatedICs.empty())
    {
        avtIntegralCurve *s = terminatedICs.front();
        terminatedICs.pop_front();
        
        if( s->domain.domain == -1 )
        {
            saveICs.push_back(s);
            --totalNumIntegralCurves;   // will not be processing this IC again.
        }
        else
        {
            s->status = avtIntegralCurve::STATUS_OK;
            // The IC may need to move to another process.
            SetDomain(s);
            if( s->domain.domain == -1 && s->seedPtDomainList.empty() )
            {
                saveICs.push_back(s);
                --totalNumIntegralCurves;   // will not be processing this IC again.
            }
            else
                HandleOOBIC(s);
        }
    }

    // Move the IC that are out of bounds to the terminated list. 
    // No reason to process them anymore.
    terminatedICs = saveICs;

    SumIntAcrossAllProcessors(totalNumIntegralCurves);
}

// ****************************************************************************
// Method:  avtParDomICAlgorithm::CheckNextTimeStepNeeded
//
// Purpose: Is the next time slice required to continue?
//   
//
// Programmer:  Dave Pugmire
// Creation:    December  2, 2010
//
// Modifications:
//
//   Hank Childs, Fri Mar  9 16:49:06 PST 2012
//   Add support for reverse pathlines.
//
//   Hank Childs, Sun Apr  1 10:32:00 PDT 2012
//   Fix recently introduced error with bad logic about what has been terminated.
//
// ****************************************************************************

bool
avtParDomICAlgorithm::CheckNextTimeStepNeeded(int curTimeSlice)
{
    int val = 0;
    list<avtIntegralCurve *>::const_iterator it;
    for (it = terminatedICs.begin(); it != terminatedICs.end(); it++)
    {
        bool itsDone = false;
        if ((*it)->domain.domain == -1 || (*it)->domain.timeStep == curTimeSlice)
)
            itsDone = true;
        if ((*it)->status == avtIntegralCurve::STATUS_TERMINATED)
            itsDone = true;
        if (! itsDone)
        {
            val = 1;
            break;
        }
    }
    
    SumIntAcrossAllProcessors(val);

    return val > 0;
}

#endif
