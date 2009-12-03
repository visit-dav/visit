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
//                              avtSerialSLAlgorithm.C                       //
// ************************************************************************* //

#include "avtSerialSLAlgorithm.h"
#include <TimingsManager.h>

using namespace std;

// ****************************************************************************
//  Method: avtSerialSLAlgorithm::avtSerialSLAlgorithm
//
//  Purpose:
//      avtSerialSLAlgorithm constructor.
//
//  Programmer: Dave Pugmire
//  Creation:   January 27, 2009
//
// ****************************************************************************

avtSerialSLAlgorithm::avtSerialSLAlgorithm( avtStreamlineFilter *slFilter )
    : avtSLAlgorithm(slFilter)
{
}

// ****************************************************************************
//  Method: avtSerialSLAlgorithm::~avtSerialSLAlgorithm
//
//  Purpose:
//      avtSerialSLAlgorithm destructor.
//
//  Programmer: Dave Pugmire
//  Creation:   January 27, 2009
//
// ****************************************************************************

avtSerialSLAlgorithm::~avtSerialSLAlgorithm()
{
}

// ****************************************************************************
//  Method: avtSerialSLAlgorithm::Initialize
//
//  Purpose:
//      Initialization.
//
//  Programmer: Dave Pugmire
//  Creation:   January 27, 2009
//
//  Modifications:
//
//   Dave Pugmire, Mon Mar 23 18:33:10 EDT 2009
//   Make changes for point decomposed domain databases.
//
//   Hank Childs, Fri Apr  3 16:26:24 PDT 2009
//   Change parallelization strategy, since it was loading up on the last
//   processor and we want it to be more spread out.
//
// ****************************************************************************

void
avtSerialSLAlgorithm::Initialize(vector<avtStreamlineWrapper *> &seedPts)
{
    avtSLAlgorithm::Initialize(seedPts);

    AddStreamlines(seedPts);
}

// ****************************************************************************
//  Method: avtSerialSLAlogrithm::AddStreamlines
//
//  Purpose:
//      Add streamlines
//
//  Programmer: Dave Pugmire
//  Creation:   December 3, 2009
//
// ****************************************************************************

void
avtSerialSLAlgorithm::AddStreamlines(vector<avtStreamlineWrapper *> &sls)
{
    int nSeeds = sls.size();
    int i0 = 0, i1 = nSeeds;

 #ifdef PARALLEL
    int rank = PAR_Rank();
    int nProcs = PAR_Size();

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
        delete sls[i];
    for (int i = i1; i < nSeeds; i++)
        delete sls[i];
#endif
    
    debug5 << "I have seeds: "<<i0<<" to "<<i1<<" of "<<nSeeds<<endl;
    
    // Filter the seeds for proper domain inclusion and fill the activeSLs list.
    avtVector endPt;
    for ( int i = i0; i < i1; i++)
    {
        avtStreamlineWrapper *s = sls[i];
        s->GetEndPoint(endPt);
        if (PointInDomain(endPt, s->domain))
            activeSLs.push_back(s);
        else
            delete s;
    }
}

// ****************************************************************************
//  Method: avtSerialSLAlgorithm::RunAlgorithm
//
//  Purpose:
//      Execute the serial streamline algorithm.
//
//  Programmer: Dave Pugmire
//  Creation:   January 27, 2009
//
//  Modifications:
//
//   Dave Pugmire, Mon Mar 23 18:33:10 EDT 2009
//   Make changes for point decomposed domain databases.
//
//   Hank Childs, Sat Apr 11 23:18:32 CDT 2009
//   Make an explicit call to GetDomain before calling IntegrateStreamline.
//   If we don't make this call, IntegrateStreamline will call GetDomain for 
//   us.  But by calling it explicitly, it goes through the avtSLAlgorithm
//   bookkeeping logic, meaning that I/O will correctly be counted as I/O,
//   instead of being rolled in with integration time.
//
//   Dave Pugmire, Thu Sep 24 13:52:59 EDT 2009
//   Change Execute to RunAlgorithm.
//
//   Dave Pugmire, Thu Dec  3 13:28:08 EST 2009
//   Move some initialization into RunAlgorithm.
//
// ****************************************************************************

void
avtSerialSLAlgorithm::RunAlgorithm()
{
    debug1<<"avtSerialSLAlgorithm::RunAlgorithm()\n";
    int timer = visitTimer->StartTimer();

    //Sort the streamlines and load the first domain.
    SortStreamlines(activeSLs);
    if (!activeSLs.empty())
    {
        avtStreamlineWrapper *s = activeSLs.front();
        GetDomain(s);
    }

    while (1)
    {
        // Integrate all loaded domains.
        while (! activeSLs.empty())
        {
            avtStreamlineWrapper *s = activeSLs.front();
            activeSLs.pop_front();

            if (DomainLoaded(s->domain))
            {
#ifndef PARALLEL
                // In serial, we return that every domain is loaded.
                // That's basically okay.
                // But it screws up time reporting, because the domain isn't
                // loaded until IntegrateStreamline calls GetDomain.
                // So call it explicitly first, so I/O can be correctly 
                // counted.
                GetDomain(s);
#endif
                IntegrateStreamline(s);
                if (s->status == avtStreamlineWrapper::TERMINATE)
                    terminatedSLs.push_back(s);
                else
                    oobSLs.push_back(s);
            }
            else
                oobSLs.push_back(s);
        }

        //We are done!
        if (oobSLs.empty())
            break;

        //Sort the remaining streamlines, get the next domain, continue.
        activeSLs = oobSLs;
        oobSLs.clear();
        
        SortStreamlines(activeSLs);
        avtStreamlineWrapper *s = activeSLs.front();
        GetDomain(s);
    }

    TotalTime.value += visitTimer->StopTimer(timer, "Execute");
}

// ****************************************************************************
//  Method: avtSerialSLAlgorithm::ResetStreamlinesForContinueExecute
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
avtSerialSLAlgorithm::ResetStreamlinesForContinueExecute()
{
    while (! terminatedSLs.empty())
    {
        avtStreamlineWrapper *s = terminatedSLs.front();
        terminatedSLs.pop_front();
        
        activeSLs.push_back(s);
    }
}
