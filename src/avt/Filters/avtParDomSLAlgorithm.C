/*****************************************************************************
*
* Copyright (c) 2000 - 2009, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400124
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
//                              avtParDomSLAlgorithm.h                       //
// ************************************************************************* //

#include "avtParDomSLAlgorithm.h"
#include <TimingsManager.h>

using namespace std;

#ifdef PARALLEL

// ****************************************************************************
//  Method: avtParDomSLAlgorithm::avtParDomSLAlgorithm
//
//  Purpose:
//      avtParDomSLAlgorithm constructor.
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

avtParDomSLAlgorithm::avtParDomSLAlgorithm(avtStreamlineFilter *slFilter,
                                           int maxCount)
    : avtParSLAlgorithm(slFilter)
{
    numSLChange = 0;
    totalNumStreamlines = 0;
    maxCnt = maxCount;
}


// ****************************************************************************
//  Method: avtParDomSLAlgorithm::~avtParDomSLAlgorithm
//
//  Purpose:
//      avtParDomSLAlgorithm destructor
//
//  Programmer: Dave Pugmire
//  Creation:   January 27, 2009
//
// ****************************************************************************
avtParDomSLAlgorithm::~avtParDomSLAlgorithm()
{
}

// ****************************************************************************
//  Method: avtParDomSLAlgorithm::Initialize
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
// ****************************************************************************

void
avtParDomSLAlgorithm::Initialize(vector<avtStreamlineWrapper *> &seedPts)
{
    int numRecvs = nProcs-1;
    if (numRecvs > 64)
        numRecvs = 64;
    
    avtParSLAlgorithm::Initialize(seedPts, 1, numRecvs);
    numSLChange = 0;
    AddStreamlines(seedPts);
}

// ****************************************************************************
//  Method: avtParDomSLAlgorithm::AddStreamlines
//
//  Purpose:
//      Add streamlines
//
//  Programmer: Dave Pugmire
//  Creation:   December 3, 2009
//
//  Modifications:
//
// ****************************************************************************

void
avtParDomSLAlgorithm::AddStreamlines(std::vector<avtStreamlineWrapper*> &sls)
{
    //Get the SLs that I own.
    for (int i = 0; i < sls.size(); i++)
    {
        avtStreamlineWrapper *s = sls[i];
        if (OwnDomain(s->domain))
        {
            avtVector endPt;
            s->GetEndPoint(endPt);
            
            if (PointInDomain(endPt, s->domain))
                activeSLs.push_back(s);
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
    
    list<avtStreamlineWrapper *>::iterator s;
    for (s = activeSLs.begin(); s != activeSLs.end(); ++s)
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

    totalNumStreamlines = activeSLs.size();
    SumIntAcrossAllProcessors(totalNumStreamlines);
    /*
    debug5<<"Init_totalNumStreamlines= "<<totalNumStreamlines<<endl;
    debug5<<"My SLs: "<<endl;
    list<avtStreamlineWrapper *>::iterator s;
    for (s = activeSLs.begin(); s != activeSLs.end(); ++s)
        debug5<<"ID "<<(*s)->id<<" dom= "<<(*s)->domain<<endl;
    */

    debug1<<"My SLcount= "<<activeSLs.size()<<endl;
    debug1<<"I own: [";
    for (int i = 0; i < numDomains; i++)
    {
        DomainType d(i,0);
        if (OwnDomain(d))
            debug1<<i<<" ";
    }
    debug1<<"]\n";
}


// ****************************************************************************
//  Method: avtParDomSLAlgorithm::ExchangeTermination()
//
//  Purpose:
//      Send/recv terminations.
//
//  Programmer: Dave Pugmire
//  Creation:   January 27, 2009
//
//  Modifications:
//
//    Dave Pugmire, Fri Feb  6 08:43:00 EST 2009
//    Change numTerminated to numSLChange.
//
// ****************************************************************************

void
avtParDomSLAlgorithm::ExchangeTermination()
{
    // If I have terminations, send it out.
    if (numSLChange != 0)
    {
        vector<int> msg(1);
        msg[0] = numSLChange;
        SendAllMsg(msg);
        
        totalNumStreamlines += numSLChange;
        numSLChange = 0;
    }

    // Check to see if msgs are coming in.
    vector<vector<int> > msgs;
    RecvMsgs(msgs);
    for (int i = 0; i < msgs.size(); i++)
    {
        debug2<<msgs[i][1]<<" slChange= "<<msgs[i][1]<<endl;
        totalNumStreamlines += msgs[i][1];
    }
}

// ****************************************************************************
//  Method: avtParDomSLAlgorithm::RunAlgorithm
//
//  Purpose:
//      Execute the static domain SL algorithm.
//
//  Programmer: Dave Pugmire
//  Creation:   January 27, 2009
//
//  Modifications:
//
//   Dave Pugmire, Wed Feb  4 16:17:40 EST 2009
//   Regression fix. Handling streamlines that lie in multiple domains after
//   integration was not handled correctly after the code refactor. Added
//   HandleOOBSL().
//
//    Dave Pugmire, Fri Feb  6 08:43:00 EST 2009
//    Change numTerminated to numSLChange.
//
//   Dave Pugmire, Wed Apr  1 11:21:05 EDT 2009
//   Remove ExchangeSLs() method.
//
//   Dave Pugmire, Thu Sep 24 13:52:59 EDT 2009
//   Replace Execute() with RunAlgorithm().
//
// ****************************************************************************

void
avtParDomSLAlgorithm::RunAlgorithm()
{
    debug1<<"avtParDomSLAlgorithm::RunAlgorithm()\n";
    int timer = visitTimer->StartTimer();
    
    while (totalNumStreamlines > 0)
    {
        //Integrate upto maxCnt streamlines.
        list<avtStreamlineWrapper *>::iterator s;
        int cnt = 0;
        while (cnt < maxCnt && !activeSLs.empty())
        {
            avtStreamlineWrapper *s = activeSLs.front();
            activeSLs.pop_front();
            
            IntegrateStreamline(s);
            if (s->status == avtStreamlineWrapper::TERMINATE)
            {
                debug5<<"TerminatedSL: "<<s->id<<endl;
                terminatedSLs.push_back(s);
                numSLChange--;
            }
            else
                HandleOOBSL(s);
            
            cnt++;
        }

        //Check for new SLs.
        int earlyTerminations = 0;
        RecvSLs(activeSLs, earlyTerminations);
        //numSLChange -= earlyTerminations;

        ExchangeTermination();
        CheckPendingSendRequests();
    }

    CheckPendingSendRequests();
    TotalTime.value += visitTimer->StopTimer(timer, "Execute");
}


// ****************************************************************************
//  Method: avtParDomSLAlgorithm::HandleOOBSL
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
// ****************************************************************************

void
avtParDomSLAlgorithm::HandleOOBSL(avtStreamlineWrapper *s)
{
    // The integrated streamline could lie in multiple domains.
    // Duplicate the SL and send to the proper owner.
    for (int i = 0; i < s->seedPtDomainList.size(); i++)
    {
        // if i > 0, we create new streamlines.
        //        if (i > 0)
        //            numSLChange++;

        int domRank = DomainToRank(s->seedPtDomainList[i]);
        s->domain = s->seedPtDomainList[i];
        if (domRank == rank)
        {
            activeSLs.push_back(s);
            //debug5<<"Handle OOB: id= "<<s->id<<" "<<s->domain<<" --> me"<<endl;
        }
        else
        {
            vector<avtStreamlineWrapper *> sls;
            sls.push_back(s);
            SendSLs(domRank, sls);
            //debug5<<"Handle OOB: id= "<<s->id<<" "<<s->domain<<" --> "<<domRank<<endl;
        }
    }
}

// ****************************************************************************
//  Method: avtParDomSLAlgorithm::ResetStreamlinesForContinueExecute
//
//  Purpose:
//      Reset for continued streamline integration.
//
//  Programmer: Dave Pugmire
//  Creation:   Tue Aug 18 08:59:40 EDT 2009
//
//  Modifications:
//
//
// ****************************************************************************

void
avtParDomSLAlgorithm::ResetStreamlinesForContinueExecute()
{
    while (! terminatedSLs.empty())
    {
        avtStreamlineWrapper *s = terminatedSLs.front();
        terminatedSLs.pop_front();
        
        activeSLs.push_back(s);
        numSLChange++;
    }

    ExchangeTermination();
}

#endif
