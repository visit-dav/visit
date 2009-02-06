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
// ****************************************************************************

avtParDomSLAlgorithm::avtParDomSLAlgorithm(avtStreamlineFilter *slFilter,
                                           int maxCount)
    : avtParSLAlgorithm(slFilter)
{
    numSLChange = 0;
    totalNumStreamlines = 0;
    statusMsgSz = 1;
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
// ****************************************************************************


void
avtParDomSLAlgorithm::Initialize(vector<avtStreamlineWrapper *> &seedPts)
{
    avtParSLAlgorithm::Initialize(seedPts);
    totalNumStreamlines = seedPts.size();
    numSLChange = 0;

    //Get the SLs that I own.
    for (int i = 0; i < seedPts.size(); i++)
    {
        avtStreamlineWrapper *s = seedPts[i];
        avtVector endPt;
        s->GetEndPoint(endPt);
        if (OwnDomain(s->domain))
        {
            if (PointInDomain(endPt, s->domain))
                activeSLs.push_back(s);
            else
            {
                numSLChange--;
                delete s;
            }
        }
        else
            delete s;
    }
    ExchangeTermination();

    debug1<<"My SLcount= "<<activeSLs.size()<<endl;
    debug1<<"I own: [";
    for (int i = 0; i < numDomains; i++)
        if (OwnDomain(i))
            debug1<<i<<" ";
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
//  Method: avtParDomSLAlgorithm::ExchangeSLs
//
//  Purpose:
//      Send/recv streamlines.
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
avtParDomSLAlgorithm::ExchangeSLs(
               vector<vector<avtStreamlineWrapper *> > &distributeSLs)
{
    int earlyTerminations = 0;
    avtParSLAlgorithm::ExchangeSLs(activeSLs, distributeSLs, earlyTerminations);
    numSLChange -= earlyTerminations;
}


// ****************************************************************************
//  Method: avtParDomSLAlgorithm::Execute
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
// ****************************************************************************

void
avtParDomSLAlgorithm::Execute()
{
    debug1<<"avtParDomSLAlgorithm::Execute()\n";
    int timer = visitTimer->StartTimer();
    
    vector< vector< avtStreamlineWrapper *> > distributeStreamlines(nProcs);

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
                terminatedSLs.push_back(s);
                numSLChange--;
            }
            else
                HandleOOBSL(s, distributeStreamlines);
            
            cnt++;
        }

        ExchangeSLs(distributeStreamlines);
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
// ****************************************************************************

void
avtParDomSLAlgorithm::HandleOOBSL(avtStreamlineWrapper *s,
                         std::vector< std::vector< avtStreamlineWrapper *> > &d)
{
    MemStream buff;
    bool deleteSL = true;

    // The integrated streamline could lie in multiple domains.
    // Duplicate the SL and send to the proper owner.
    for (int i = 0; i < s->seedPtDomainList.size(); i++)
    {
        // if i > 0, we create new streamlines.
        if (i > 0)
            numSLChange++;

        int domRank = DomainToRank(s->seedPtDomainList[i]);
        if (domRank == rank)
        {
            activeSLs.push_back(s);
            deleteSL = false;
        }
        else
        {
            // First time, create the buffer.
            if (buff.buffLen() == 0)
                s->Serialize(MemStream::WRITE, buff, GetSolver());

            // Create a copy of the SL and add it to the distribute array.
            avtStreamlineWrapper *newS = new avtStreamlineWrapper;
            buff.rewind();
            newS->Serialize(MemStream::READ, buff, GetSolver());
            newS->domain = s->seedPtDomainList[i];
            d[domRank].push_back(newS);
        }
    }

    if (deleteSL)
        delete s;
}

#endif
