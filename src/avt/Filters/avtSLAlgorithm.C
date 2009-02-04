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
//                              avtSLAlgorithm.C                             //
// ************************************************************************* //

#include "avtSLAlgorithm.h"
#include <TimingsManager.h>
#include <DebugStream.h>
#include <iostream>

using namespace std;

static bool slDomainCompare(const avtStreamlineWrapper *slA, 
                            const avtStreamlineWrapper *slB)
{
    return slA->sortKey < slB->sortKey;
}

// ****************************************************************************
//  Method: avtSLAlgorithm::avtSLAlgorithm
//
//  Purpose:
//      avtSLAlgorithm constructor.
//
//  Programmer: Dave Pugmire
//  Creation:   January 27, 2009
//
// ****************************************************************************

avtSLAlgorithm::avtSLAlgorithm( avtStreamlineFilter *slFilter )
{
    streamlineFilter = slFilter;
    numDomains = streamlineFilter->numDomains;
}

// ****************************************************************************
//  Method: avtSLAlgorithm::~avtSLAlgorithm
//
//  Purpose:
//      avtSLAlgorithm destructor.
//
//  Programmer: Dave Pugmire
//  Creation:   January 27, 2009
//
// ****************************************************************************

avtSLAlgorithm::~avtSLAlgorithm()
{
    streamlineFilter = NULL;
}

// ****************************************************************************
//  Method: avtSLAlgorithm::GetDomain
//
//  Purpose:
//      Retrieve a domain.
//
//  Programmer: Dave Pugmire
//  Creation:   January 27, 2009
//
// ****************************************************************************

vtkDataSet *
avtSLAlgorithm::GetDomain(int dom)
{
    int timerHandle = visitTimer->StartTimer();

    vtkDataSet *ds = streamlineFilter->GetDomain(dom);

    IOTime.value += visitTimer->StopTimer(timerHandle, "GetDomain()");
    return ds;
}

// ****************************************************************************
//  Method: avtSLAlgorithm::IntegrateStreamline
//
//  Purpose:
//      Integrate a streamline through one domain.
//
//  Programmer: Dave Pugmire
//  Creation:   January 27, 2009
//
// ****************************************************************************

void
avtSLAlgorithm::IntegrateStreamline(avtStreamlineWrapper *s)
{
    int timerHandle = visitTimer->StartTimer();

    streamlineFilter->IntegrateStreamline(s);

    IntegrateTime.value += visitTimer->StopTimer(timerHandle, "IntegrateStreamline()");
    IntegrateCnt.value++;
}

// ****************************************************************************
//  Method: avtSLAlgorithm::Initialize
//
//  Purpose:
//      Do any post Execuction processing.
//
//  Programmer: Dave Pugmire
//  Creation:   January 27, 2009
//
// ****************************************************************************

void
avtSLAlgorithm::Initialize(vector<avtStreamlineWrapper *> &seedPts)
{
    numSeedPoints = seedPts.size();
}

// ****************************************************************************
//  Method: avtSLAlgorithm::PostExecute
//
//  Purpose:
//      Do any post Execuction processing.
//
//  Programmer: Dave Pugmire
//  Creation:   January 27, 2009
//
// ****************************************************************************

void
avtSLAlgorithm::PostExecute()
{
    debug1<<"avtSLAlgorithm::PostExecute()\n";

    vector<avtStreamlineWrapper *> v;

    while (! terminatedSLs.empty())
    {
        v.push_back(terminatedSLs.front());
        terminatedSLs.pop_front();
    }

    streamlineFilter->CreateStreamlineOutput(v);

    for (int i = 0; i < v.size(); i++)
        delete v[i];
    
    //ReportStatistics();
}

// ****************************************************************************
//  Method: avtSLAlgorithm::SortStreamlines
//
//  Purpose:
//      Sort streamlines based on the domains they span.
//
//  Programmer: Dave Pugmire
//  Creation:   June 16, 2008
//
//  Modifications:
//
//   Dave Pugmire, Fri Aug 22 14:47:11 EST 2008
//   Fix memory leak. domainCnt not delete if allSameDomain is true.
//
// ****************************************************************************

void
avtSLAlgorithm::SortStreamlines(list<avtStreamlineWrapper *> &sl)
{
    int timerHandle = visitTimer->StartTimer();
    list<avtStreamlineWrapper*>::iterator s;

    //Set sortkey to -domain. (So that loaded domains sort first).
    for (s=sl.begin(); s != sl.end(); ++s)
    {
        if ( DomainLoaded((*s)->domain))
            (*s)->sortKey = -(*s)->domain;
        else
            (*s)->sortKey = (*s)->domain;
    }

    sl.sort(slDomainCompare);
    
    SortTime.value += visitTimer->StopTimer(timerHandle, "SortStreamlines()");
}

// ****************************************************************************
//  Method: avtStreamlineFilter::CalculateStatistics
//
//  Purpose:
//      Calculate the statistics
//
//  Programmer: Dave Pugmire
//  Creation:   January 27, 2009
//
//
// ****************************************************************************

void
avtSLAlgorithm::CalculateStatistics()
{
    ComputeStatistics(TotalTime);
    ComputeStatistics(IOTime);
    ComputeStatistics(IntegrateTime);
    ComputeStatistics(SortTime);
    ComputeStatistics(IntegrateCnt);

    DomLoadCnt.value = streamlineFilter->GetLoadDSCount();
    DomPurgeCnt.value = streamlineFilter->GetPurgeDSCount();
    ComputeStatistics(DomLoadCnt);
    ComputeStatistics(DomPurgeCnt);
    
    ComputeStatistics(DomPurgeCnt);

    //Make sure this is last!
    CalculateExtraTime();
    ComputeStatistics(ExtraTime);
}

// ****************************************************************************
//  Method: avtStreamlineFilter::ComputeStatistics
//
//  Purpose:
//      Compute statistics over a value.
//
//  Programmer: Dave Pugmire
//  Creation:   Dec 12, 2008
//
// Modifications:
//
//   Dave Pugmire, Thu Dec 18 13:24:23 EST 2008
//   Overhaul how statistics are computed. Add mean and std deviation.
//
// ****************************************************************************

void
avtSLAlgorithm::ComputeStatistics(SLStatistics &stats)
{
    stats.min = stats.value;
    stats.max = stats.value;
    stats.mean = stats.value;
    stats.sigma = 0.0;
    stats.total = stats.value;
}


// ****************************************************************************
//  Method: avtStreamlineFilter::CalculateExtraTime
//
//  Purpose:
//      Calculate extra time.
//
//  Programmer: Dave Pugmire
//  Creation:   January 28, 2009
//
// ****************************************************************************

void
avtSLAlgorithm::CalculateExtraTime()
{
    ExtraTime.value = TotalTime.value;
    ExtraTime.value -= IOTime.value;
    ExtraTime.value -= IntegrateTime.value;
    ExtraTime.value -= SortTime.value;
}

// ****************************************************************************
//  Method: avtSLAlgorithm::ReportStatistics
//
//  Purpose:
//      Report stats.
//
//  Programmer: Dave Pugmire
//  Creation:   January 28, 2009
//
// ****************************************************************************

void
avtSLAlgorithm::ReportStatistics()
{
    CalculateStatistics();

    char f[128];
    int rank = 0;
#ifdef PARALLEL
    rank = PAR_Rank();
#endif
    sprintf(f, "timings%03d.txt", rank);
    ofstream os;
    os.open(f, ios::out);

    ReportStatistics(os);
    os.close();
    if (rank == 0)
        ReportStatistics(cout);
}

// ****************************************************************************
//  Method: avtSLAlgorithm::ReportStatistics
//
//  Purpose:
//      Output timings to a stream.
//
//  Programmer: Dave Pugmire
//  Creation:   April 4, 2008
// Modifications:
//
//   Dave Pugmire, Thu Dec 18 13:24:23 EST 2008
//   Overhaul how timings are reported.
//
// ****************************************************************************

void
avtSLAlgorithm::ReportStatistics(ostream &os)
{
    int nCPUs = 1;
#ifdef PARALLEL
    nCPUs = PAR_Size();
#endif
    os<<endl;
    os<<"Totals: ***********************************************"<<endl;
    os<<"Method= "<<AlgoName()<<" nCPUs= "<<nCPUs<<" nDom= "<<numDomains;
    os<<" nPts= "<<numSeedPoints<<endl;
    os<<endl;

    ReportTimings(os, true);
    os<<endl;
    ReportCounters(os, true);

    os<<endl<<"Per Proccess:"<<endl;
    ReportTimings(os, false);
    ReportCounters(os, false);
}

// ****************************************************************************
//  Method: avtSLAlgorithm::ReportTimings
//
//  Purpose:
//      Print timing data.
//
//  Programmer: Dave Pugmire
//  Creation:   January 28, 2009
//
// ****************************************************************************
void
avtSLAlgorithm::ReportTimings(ostream &os, bool totals)
{
    os<<"Timings: *********************************************"<<endl;
    if (totals)
        os<<"Time      = "<<TotalTime.max<<endl;
    else
        os<<"Time      = "<<TotalTime.value<<endl;

    PrintTiming(os, "ExtraTime = ", ExtraTime, TotalTime, totals);
    PrintTiming(os, "IntgTime  = ", IntegrateTime, TotalTime, totals);
    PrintTiming(os, "IOTime    = ", IOTime, TotalTime, totals);
    PrintTiming(os, "SortTime  = ", SortTime, TotalTime, totals);
}


// ****************************************************************************
//  Method: avtSLAlgorithm::ReportCounters
//
//  Purpose:
//      Print timing data.
//
//  Programmer: Dave Pugmire
//  Creation:   January 28, 2009
//
// ****************************************************************************
void
avtSLAlgorithm::ReportCounters(ostream &os, bool totals)
{
    os<<"Counters: ********************************************"<<endl;
    
    PrintCounter(os, "DomLoad   = ", DomLoadCnt, totals);
    PrintCounter(os, "DomPurge  = ", DomPurgeCnt, totals);
    PrintCounter(os, "IntgrCnt  = ", IntegrateCnt, totals);
}

// ****************************************************************************
//  Method: avtSLAlgorithm::PrintTiming
//
//  Purpose:
//      Print timing data.
//
//  Programmer: Dave Pugmire
//  Creation:   January 28, 2009
//
// ****************************************************************************
void
avtSLAlgorithm::PrintTiming(ostream &os, 
                            char *str, 
                            const SLStatistics &s,
                            const SLStatistics &t,
                            bool total)
{
    if (total)
    {
        os<<str;
        os<<s.total;
        os<<" ["<<100.0*(s.total/t.total)<<"%] ";
        os<<" ["<<s.min<<", "<<s.max<<", "<<s.mean<<" : "<<s.sigma<<"]";

        if (s.mean != 0.0)
        {
            float v = s.sigma / s.mean;
            os<<" ["<<v<<"]";
        }
        os<<endl;
    }
    else
    {
        os<<str;
        float v = s.value;
        if (s.value < 0.0)
            v = 0.0;
        
        os<<v;
        os<<" ["<<100.0*(v/t.value)<<"%] ";
        os<<endl;
    }
}

// ****************************************************************************
//  Method: avtSLAlgorithm::PrintCounter
//
//  Purpose:
//      Print counter data.
//
//  Programmer: Dave Pugmire
//  Creation:   January 28, 2009
//
// ****************************************************************************
void
avtSLAlgorithm::PrintCounter(ostream &os, 
                             char *str, 
                             const SLStatistics &s,
                             bool totals)
{
    if (totals)
    {
        os<<str;
        os<<s.total;
        os<<" ["<<s.min<<", "<<s.max<<", "<<s.mean<<" : "<<s.sigma<<"]";

        if (s.mean != 0.0)
        {
            float v = s.sigma / s.mean;
            os<<" ["<<v<<"]";
        }
        os<<endl;
    }
    else
    {
        os<<str;
        float v = s.value;
        if (s.value < 0.0)
            v = 0.0;
        os<<v;
        float p = 0.0;
        if (s.total > 0.0)
            p = s.value/s.total * 100.0;

        float sd = 0.0;
        if (s.sigma != 0.0)
            sd = (v-s.mean) / s.sigma;
        os<<" ["<<p<<"%] ["<<sd<<"] ";
        os<<endl;
    }
}
