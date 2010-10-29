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
//                              avtSerialICAlgorithm.C                       //
// ************************************************************************* //

#include "avtSerialICAlgorithm.h"
#include <TimingsManager.h>

using namespace std;

// ****************************************************************************
//  Method: avtSerialICAlgorithm::avtSerialICAlgorithm
//
//  Purpose:
//      avtSerialICAlgorithm constructor.
//
//  Programmer: Dave Pugmire
//  Creation:   January 27, 2009
//
//  Modifications:
//
//    Hank Childs, Sun Jun  6 12:21:30 CDT 2010
//    Remove reference to avtStreamlineFilter, add reference to avtPICSFilter.
//
// ****************************************************************************

avtSerialICAlgorithm::avtSerialICAlgorithm( avtPICSFilter *picsFilter )
    : avtICAlgorithm(picsFilter)
{
}

// ****************************************************************************
//  Method: avtSerialICAlgorithm::~avtSerialICAlgorithm
//
//  Purpose:
//      avtSerialICAlgorithm destructor.
//
//  Programmer: Dave Pugmire
//  Creation:   January 27, 2009
//
// ****************************************************************************

avtSerialICAlgorithm::~avtSerialICAlgorithm()
{
}

// ****************************************************************************
//  Method: avtSerialICAlgorithm::Initialize
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
//   Hank Childs, Sun Jun  6 12:21:30 CDT 2010
//   Change name of method called to AddIntegralCurves.
//
// ****************************************************************************

void
avtSerialICAlgorithm::Initialize(vector<avtIntegralCurve *> &seedPts)
{
    avtICAlgorithm::Initialize(seedPts);

    AddIntegralCurves(seedPts);
}

// ****************************************************************************
//  Method: avtSerialICAlgorithm::AddIntegralCurves
//
//  Purpose:
//      Add streamlines
//
//  Programmer: Dave Pugmire
//  Creation:   December 3, 2009
//
//  Modifications:
//
//   Hank Childs, Thu Jun  3 10:22:16 PDT 2010
//   Use new name "GetCurrentLocation".
//
//   Hank Childs, Fri Jun  4 19:58:30 CDT 2010
//   Use avtStreamlines, not avtStreamlineWrappers.
//
//   Hank Childs, Sun Jun  6 12:21:30 CDT 2010
//   Rename method to AddIntegralCurves.
//
// ****************************************************************************

void
avtSerialICAlgorithm::AddIntegralCurves(vector<avtIntegralCurve *> &ics)
{
    int nSeeds = ics.size();
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
        delete ics[i];
    for (int i = i1; i < nSeeds; i++)
        delete ics[i];
#endif
    
    debug5 << "I have seeds: "<<i0<<" to "<<i1<<" of "<<nSeeds<<endl;
    
    // Filter the seeds for proper domain inclusion and fill the activeICs list.
    avtVector endPt;
    for ( int i = i0; i < i1; i++)
    {
        avtIntegralCurve *s = ics[i];
        s->CurrentLocation(endPt);
        if (PointInDomain(endPt, s->domain))
            activeICs.push_back(s);
        else
            delete s;
    }
}

// ****************************************************************************
//  Method: avtSerialICAlgorithm::RunAlgorithm
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
//   Make an explicit call to GetDomain before calling AdvectParticle.
//   If we don't make this call, AdvectParticle will call GetDomain for 
//   us.  But by calling it explicitly, it goes through the avtICAlgorithm
//   bookkeeping logic, meaning that I/O will correctly be counted as I/O,
//   instead of being rolled in with integration time.
//
//   Dave Pugmire, Thu Sep 24 13:52:59 EDT 2009
//   Change Execute to RunAlgorithm.
//
//   Dave Pugmire, Thu Dec  3 13:28:08 EST 2009
//   Move some initialization into RunAlgorithm.
//
//   Hank Childs, Fri Jun  4 19:58:30 CDT 2010
//   Use avtStreamlines, not avtStreamlineWrappers.
//
//   Hank Childs, Sun Jun  6 12:21:30 CDT 2010
//   Rename several methods that reflect the new emphasis in particle 
//   advection, as opposed to streamlines.
//
// ****************************************************************************

void
avtSerialICAlgorithm::RunAlgorithm()
{
    debug1<<"avtSerialICAlgorithm::RunAlgorithm()\n";
    int timer = visitTimer->StartTimer();

    //Sort the streamlines and load the first domain.
    SortIntegralCurves(activeICs);
    if (!activeICs.empty())
    {
        avtIntegralCurve *s = activeICs.front();
        GetDomain(s);
    }

    while (1)
    {
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
                GetDomain(s);
#endif
                AdvectParticle(s);

                if( s->status != avtIntegralCurve::STATUS_OK )
                {
                    terminatedICs.push_back(s);
                }
                else
                {
                    oobICs.push_back(s);
                }
            }
            else
                oobICs.push_back(s);
        }

        //We are done!
        if (oobICs.empty())
            break;

        //Sort the remaining streamlines, get the next domain, continue.
        activeICs = oobICs;
        oobICs.clear();
        
        SortIntegralCurves(activeICs);
        avtIntegralCurve *s = activeICs.front();
        GetDomain(s);
    }

    TotalTime.value += visitTimer->StopTimer(timer, "Execute");
}

// ****************************************************************************
//  Method: avtSerialICAlgorithm::ResetIntegralCurvesForContinueExecute
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
// ****************************************************************************

void
avtSerialICAlgorithm::ResetIntegralCurvesForContinueExecute()
{
    while (! terminatedICs.empty())
    {
        avtIntegralCurve *s = terminatedICs.front();
        terminatedICs.pop_front();
        
        activeICs.push_back(s);
    }
}
