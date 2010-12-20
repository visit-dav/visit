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
//                              avtICAlgorithm.C                             //
// ************************************************************************* //

#include "avtICAlgorithm.h"
#include <TimingsManager.h>
#include <DebugStream.h>
#include <iostream>
#include <iomanip>

using namespace std;

static bool icDomainCompare(const avtIntegralCurve *icA, 
                            const avtIntegralCurve *icB)
{
    return icA->sortKey < icB->sortKey;
}

// ****************************************************************************
//  Method: avtICAlgorithm::avtICAlgorithm
//
//  Purpose:
//      avtICAlgorithm constructor.
//
//  Programmer: Dave Pugmire
//  Creation:   January 27, 2009
//
//  Modifications:
//
//   Dave Pugmire, Tue Mar 10 12:41:11 EDT 2009
//   Generalized domain to include domain/time. Pathine cleanup.
//
//   Dave Pugmire, Mon Mar 23 12:48:12 EDT 2009
//   Change how timings are reported/calculated.
//     
//   Hank Childs, Sun Jun  6 12:25:31 CDT 2010
//   Change reference from streamline filter to PICS filter.
//
// ****************************************************************************

avtICAlgorithm::avtICAlgorithm( avtPICSFilter *f ) :
    TotalTime("totT"), IOTime("ioT"), IntegrateTime("intT"), SortTime("sorT"), ExtraTime("extT"),
    IntegrateCnt("intC"), DomLoadCnt("domLC"), DomPurgeCnt("domPC")
{
    picsFilter = f;
    numDomains = picsFilter->numDomains;
    numTimeSteps = picsFilter->numTimeSteps;
}

// ****************************************************************************
//  Method: avtICAlgorithm::~avtICAlgorithm
//
//  Purpose:
//      avtICAlgorithm destructor.
//
//  Programmer: Dave Pugmire
//  Creation:   January 27, 2009
//
//  Modifications:
//
//   Hank Childs, Sun Jun  6 12:25:31 CDT 2010
//   Change reference from streamline filter to PICS filter.
//
// ****************************************************************************

avtICAlgorithm::~avtICAlgorithm()
{
    picsFilter = NULL;
}

// ****************************************************************************
//  Method: avtICAlgorithm::GetDomain
//
//  Purpose:
//      Retrieve a domain.
//
//  Programmer: Dave Pugmire
//  Creation:   January 27, 2009
//
//  Modifications:
//
//   Dave Pugmire, Tue Mar 10 12:41:11 EDT 2009
//   Generalized domain to include domain/time. Pathine cleanup.
//
//   Dave Pugmire, Mon Mar 23 18:33:10 EDT 2009
//   Make changes for point decomposed domain databases.
//
//   Hank Childs, Thu Jun  3 10:22:16 PDT 2010
//   Use new name "GetCurrentLocation".
//
//   Hank Childs, Fri Jun  4 19:58:30 CDT 2010
//   Use avtStreamlines, not avtStreamlineWrappers.
//
//   Hank Childs, Sun Jun  6 12:25:31 CDT 2010
//   Change reference from streamline filter to PICS filter.
//
// ****************************************************************************

vtkDataSet *
avtICAlgorithm::GetDomain(avtIntegralCurve *ic)
{
    avtVector pt;
    ic->CurrentLocation(pt);
    return GetDomain(ic->domain, pt.x, pt.y, pt.z);
}

// ****************************************************************************
//  Method: avtICAlgorithm::GetDomain
//
//  Purpose:
//      Retrieve a domain.
//
//  Programmer: Dave Pugmire
//  Creation:   March 23, 2009
//
// ****************************************************************************

vtkDataSet *
avtICAlgorithm::GetDomain(const DomainType &dom, double X, double Y, double Z)
{
    int timerHandle = visitTimer->StartTimer();
    vtkDataSet *ds = picsFilter->GetDomain(dom, X, Y, Z);
    IOTime.value += visitTimer->StopTimer(timerHandle, "GetDomain()");
    
    return ds;
    
}

// ****************************************************************************
//  Method: avtICAlgorithm::AdvectParticle
//
//  Purpose:
//      Advects a particle through one domain.
//
//  Programmer: Dave Pugmire
//  Creation:   January 27, 2009
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
avtICAlgorithm::AdvectParticle(avtIntegralCurve *s)
{
    int timerHandle = visitTimer->StartTimer();

    picsFilter->AdvectParticle(s);

    IntegrateTime.value += visitTimer->StopTimer(timerHandle, "AdvectParticle()");
    IntegrateCnt.value++;
}


void
avtICAlgorithm::AdvectParticle(avtIntegralCurve *s, vtkDataSet *ds)
{
    int timerHandle = visitTimer->StartTimer();

    picsFilter->AdvectParticle(s, ds);
    IntegrateTime.value += visitTimer->StopTimer(timerHandle, "AdvectParticle()");
    IntegrateCnt.value++;
}

// ****************************************************************************
//  Method: avtICAlgorithm::Initialize
//
//  Purpose:
//      Do any post Execuction processing.
//
//  Programmer: Dave Pugmire
//  Creation:   January 27, 2009
//
//  Modifications:
//  
//   Dave Pugmire, Mon Feb 23 13:38:49 EST 2009
//   Initialize the initial domain load count and timer.  
//
//   Hank Childs, Fri Jun  4 19:58:30 CDT 2010
//   Use avtStreamlines, not avtStreamlineWrappers.
//
//   Hank Childs, Sun Jun  6 12:25:31 CDT 2010
//   Change reference from streamline filter to PICS filter.
//
// ****************************************************************************

void
avtICAlgorithm::Initialize(vector<avtIntegralCurve *> &seedPts)
{
    numSeedPoints = seedPts.size();

    IOTime.value = picsFilter->InitialIOTime;
    TotalTime.value = picsFilter->InitialIOTime;
    DomLoadCnt.value = picsFilter->InitialDomLoads;
}


// ****************************************************************************
//  Method: avtICAlgorithm::Execute
//
//  Purpose:
//      Execute the algorithm.
//
//  Programmer: Dave Pugmire
//  Creation:   September 24, 2009
//
//  Modifications:
//  
// ****************************************************************************

void
avtICAlgorithm::Execute()
{
    PreRunAlgorithm();
    RunAlgorithm();
    PostRunAlgorithm();
}

// ****************************************************************************
//  Method: avtICAlgorithm::PostExecute
//
//  Purpose:
//      Do any post Execuction processing.
//
//  Programmer: Dave Pugmire
//  Creation:   January 27, 2009
//
//  Modifications:
//
//   Dave Pugmire, Tue Mar 24 08:15:04 EDT 2009
//   Report stats if timer is enabled.
//
//   Hank Childs, Fri Jun  4 19:58:30 CDT 2010
//   Use avtStreamlines, not avtStreamlineWrappers.
//
// ****************************************************************************

void
avtICAlgorithm::PostExecute()
{
    debug1<<"avtICAlgorithm::PostExecute()\n";

    vector<avtIntegralCurve *> v;
    
    while (! terminatedICs.empty())
    {
        v.push_back(terminatedICs.front());
        terminatedICs.pop_front();
    }

    picsFilter->CreateIntegralCurveOutput(v);

    for (int i = 0; i < v.size(); i++)
        delete v[i];
    
    if (visitTimer->Enabled())
        ReportStatistics();
}

// ****************************************************************************
//  Method: avtICAlgorithm::SortIntegralCurves
//
//  Purpose:
//      Sort streamlines based on the domains they span.
//
//  Programmer: Dave Pugmire
//  Creation:   June 16, 2008
//
//  Modifications:
//
//   Dave Pugmire, Tue Mar 10 12:41:11 EDT 2009
//   Generalized domain to include domain/time. Pathine cleanup.
//
//   Dave Pugmire, Tue Aug 11 13:44:44 EDT 2009
//   Fix compiler warning.
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
avtICAlgorithm::SortIntegralCurves(list<avtIntegralCurve *> &ic)
{
    int timerHandle = visitTimer->StartTimer();
    list<avtIntegralCurve*>::iterator s;

    //Set sortkey to -domain. (So that loaded domains sort first).
    for (s=ic.begin(); s != ic.end(); ++s)
    {
        long long d = (*s)->domain.domain, t = (*s)->domain.timeStep;
        long long key = (d<<32) + t;
        key = (*s)->domain.domain;
        if ( DomainLoaded((*s)->domain))
            (*s)->sortKey = -key;
        else
            (*s)->sortKey = key;
    }

    ic.sort(icDomainCompare);
    
    SortTime.value += visitTimer->StopTimer(timerHandle, "SortIntegralCurves()");
}


// ****************************************************************************
//  Method: avtICAlgorithm::SortIntegralCurves
//
//  Purpose:
//      Sort streamlines based on the domains they span.
//
//  Programmer: Dave Pugmire
//  Creation:   June 16, 2008
//
//  Modifications:
//
//   Dave Pugmire, Tue Mar 10 12:41:11 EDT 2009
//   Generalized domain to include domain/time. Pathine cleanup.
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
avtICAlgorithm::SortIntegralCurves(vector<avtIntegralCurve *> &ic)
{
    int timerHandle = visitTimer->StartTimer();
    vector<avtIntegralCurve*>::iterator s;

    //Set sortkey to -domain. (So that loaded domains sort first).
    for (s=ic.begin(); s != ic.end(); ++s)
    {
        if ( DomainLoaded((*s)->domain))
            (*s)->sortKey = -(*s)->domain.domain;
        else
            (*s)->sortKey = (*s)->domain.domain;
    }

    sort(ic.begin(), ic.end(), icDomainCompare);
    
    SortTime.value += visitTimer->StopTimer(timerHandle, "SortIntegralCurves()");
}

// ****************************************************************************
// Method:  avtICAlgorithm::Sleep
//
// Purpose: Sleep for a spell.
//   
// Programmer:  Dave Pugmire
// Creation:    December 20, 2010
//
// ****************************************************************************

void
avtICAlgorithm::Sleep(long nanoSec) const
{
    struct timespec ts = {0, nanoSec};
    nanosleep(&ts, 0);
}

// ****************************************************************************
//  Method: avtICAlgorithm::GetTerminatedICs
//
//  Purpose:
//      Return an array of terminated streamlines.
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
avtICAlgorithm::GetTerminatedICs(vector<avtIntegralCurve *> &v)
{
    list<avtIntegralCurve *>::const_iterator s;
    
    for (s=terminatedICs.begin(); s != terminatedICs.end(); ++s)
        v.push_back(*s);
}


// ****************************************************************************
//  Method: avtICAlgorithm::DeleteIntegralCurves
//
//  Purpose:
//      Delete streamlines.
//
//  Programmer: Dave Pugmire
//  Creation:   Tue May 25 10:15:35 EDT 2010
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
avtICAlgorithm::DeleteIntegralCurves(std::vector<int> &icIDs)
{
    list<avtIntegralCurve *>::iterator s;
    vector<int>::const_iterator i;

    for (s=terminatedICs.begin(); s != terminatedICs.end(); ++s)
    {
        for (i=icIDs.begin(); i != icIDs.end(); i++)
            if ((*s)->id == (*i))
            {
                terminatedICs.erase(s);
                delete *s;
                break;
            }
    }
}


// ****************************************************************************
//  Method: avtICAlgorithm::CalculateTimingStatistics
//
//  Purpose:
//      Calculate the timings.
//
//  Programmer: Dave Pugmire
//  Creation:   March 23, 2009
//
//
// ****************************************************************************

void
avtICAlgorithm::CompileTimingStatistics()
{
    ComputeStatistic(TotalTime);
    ComputeStatistic(IOTime);
    ComputeStatistic(IntegrateTime);
    ComputeStatistic(SortTime);
}

// ****************************************************************************
//  Method: avtICAlgorithm::CalculateCounterStatistics
//
//  Purpose:
//      Calculate the statistics
//
//  Programmer: Dave Pugmire
//  Creation:   March 23, 2009
//
//  Modifications:
//
//   Dave Pugmire, Thu Mar 26 12:02:27 EDT 2009
//   Add counters for domain loading.
//
//   Hank Childs, Sun Jun  6 12:25:31 CDT 2010
//   Change reference from streamline filter to PICS filter.
//
// ****************************************************************************

void
avtICAlgorithm::CompileCounterStatistics()
{
    ComputeStatistic(IntegrateCnt);
    DomLoadCnt.value += picsFilter->GetLoadDSCount();
    DomPurgeCnt.value += picsFilter->GetPurgeDSCount();
    ComputeStatistic(DomLoadCnt);
    ComputeStatistic(DomPurgeCnt);

    ComputeDomainLoadStatistic();
}


// ****************************************************************************
//  Method: avtICAlgorithm::CompileAlgorithmStatistics
//
//  Purpose:
//      Calculate the statistics
//
//  Programmer: Dave Pugmire
//  Creation:   March 19, 2009
//
//
// ****************************************************************************

void
avtICAlgorithm::CompileAlgorithmStatistics()
{
    CompileTimingStatistics();
    CompileCounterStatistics();
    
    //Once all the timings have been computed, calc the extra time.
    CalculateExtraTime();
    ComputeStatistic(ExtraTime);
}

// ****************************************************************************
//  Method: avtICAlgorithm::ComputeStatistic
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
//   Dave Pugmire, Fri Feb  6 14:42:07 EST 2009
//   Add histogram to the statistics. Move parallel version here.
//
// ****************************************************************************

void
avtICAlgorithm::ComputeStatistic(ICStatistics &stats)
{
#ifndef PARALLEL

    stats.min = stats.value;
    stats.max = stats.value;
    stats.mean = stats.value;
    stats.sigma = 0.0;
    stats.total = stats.value;

#else

    int rank = PAR_Rank();
    int nProcs = PAR_Size();
    float *input = new float[nProcs], *output = new float[nProcs];

    for (int i = 0; i < nProcs; i++)
        input[i] = 0.0;
    input[rank] = stats.value;
    
    SumFloatArrayAcrossAllProcessors(input, output, nProcs);
    
    // A value of -1 means that there is no data to be calculated.
    // We need to remove these from the min/max/mean computation.
    stats.total = 0.0;
    int nVals = 0;
    for (int i = 0; i < nProcs; i++)
    {
        if (output[i] >= 0.0)
        {
            stats.total += output[i];
            nVals++;
        }
    }
    stats.mean = stats.total / (float)nVals;

    float sum = 0.0;
    for (int i = 0; i < nProcs; i++)
    {
        if (output[i] >= 0.0)
        {
            float x = output[i] - stats.mean;
            sum += (x*x);
        }
    }
    sum /= (float)nVals;
    stats.sigma = sqrt(sum);

    stats.histogram.resize(nVals);
    int i, j;
    for (i = 0, j = 0; i < nProcs; i++)
        if (output[i] >= 0.0)
        {
            stats.histogram[j] = output[i];
            j++;
        }
    if (stats.histogram.size() > 0)
    {
        sort(stats.histogram.begin(), stats.histogram.end());
        stats.min = stats.histogram[0];
        stats.max = stats.histogram[nVals-1];
    }

    delete [] input;
    delete [] output;
#endif
}


// ****************************************************************************
//  Method: avtICAlgorithm::ComputeDomainLoadStatistic
//
//  Purpose:
//      Compute domain loading statistics.
//
//  Programmer: Dave Pugmire
//  Creation:   March 26, 2009
//
//  Modifications:
//
//   Dave Pugmire, Wed Apr  1 11:21:05 EDT 2009
//   Compute avgDomainLoaded, instead of 1.0/avgDomainLoaded.
//
//   Hank Childs, Sun Jun  6 12:25:31 CDT 2010
//   Change reference from streamline filter to PICS filter.
//
// ****************************************************************************

void
avtICAlgorithm::ComputeDomainLoadStatistic()
{
    //Figure out the domain loads...
    std::map<unsigned long, int>::iterator it;
    int *domLoads = new int[numDomains];
    for (int i = 0; i < numDomains; i++)
        domLoads[i] = 0;

    domainsUsed = picsFilter->domainLoadCount.size();
    totDomainsLoaded = 0;
    domainLoadedMin = 0;
    domainLoadedMax = 0;
    avgDomainLoaded = 0.0;
    
    for (it = picsFilter->domainLoadCount.begin(); it != picsFilter->domainLoadCount.end(); it++)
    {
        domLoads[it->first] = it->second;
        totDomainsLoaded += it->second;
        
        if (it == picsFilter->domainLoadCount.begin())
        {
            domainLoadedMin = it->second;
            domainLoadedMax = it->second;
        }
        else
        {
            if (it->second < domainLoadedMin)
                domainLoadedMin = it->second;
            if (it->second > domainLoadedMax)
                domainLoadedMax = it->second;
        }
    }

    if (totDomainsLoaded > 0)
        avgDomainLoaded = (float)totDomainsLoaded / (float)domainsUsed;

    debug1<<"Local Dom report:"<<endl;
    for (int i = 0; i < numDomains; i++) debug1<<setw(3)<<i<<": "<<domLoads[i]<<endl;

#if PARALLEL
    globalDomainsUsed = 0;
    globalTotDomainsLoaded = 0;
    globalDomainLoadedMin = 0;
    globalDomainLoadedMax = 0;
    globalAvgDomainLoaded = 0.0;

    int *sums = new int[numDomains];
    SumIntArrayAcrossAllProcessors(domLoads, sums, numDomains);
    
    debug1<<"Global Dom report:"<<endl;
    for (int i = 0; i < numDomains; i++) debug1<<setw(3)<<i<<": "<<sums[i]<<endl;
        
    for (int i = 0; i < numDomains; i++)
    {
        if (sums[i] != 0)
        {
            globalDomainsUsed++;
            globalTotDomainsLoaded += sums[i];
            
            if (globalDomainLoadedMin == 0) //First one.
            {
                globalDomainLoadedMin = sums[i];
                globalDomainLoadedMax = sums[i];
            }
            else
            {
                if (sums[i] < globalDomainLoadedMin)
                    globalDomainLoadedMin = sums[i];
                if (sums[i] > globalDomainLoadedMax)
                    globalDomainLoadedMax = sums[i];
            }
        }
    }
        
    if (globalTotDomainsLoaded > 0)
        globalAvgDomainLoaded = (float)globalTotDomainsLoaded / (float)globalDomainsUsed;
    delete [] sums;
#else
    globalDomainsUsed = domainsUsed;
    globalDomainLoadedMin = domainLoadedMin;
    globalDomainLoadedMax = domainLoadedMax;
    globalAvgDomainLoaded = avgDomainLoaded;
#endif
    
    delete [] domLoads;
}


// ****************************************************************************
//  Method: avtICAlgorithm::CalculateExtraTime
//
//  Purpose:
//      Calculate extra time.
//
//  Programmer: Dave Pugmire
//  Creation:   January 28, 2009
//
// ****************************************************************************

void
avtICAlgorithm::CalculateExtraTime()
{
    ExtraTime.value = TotalTime.value;
    if (IOTime.value > 0.0)
        ExtraTime.value -= IOTime.value;
    if (IntegrateTime.value > 0.0)
        ExtraTime.value -= IntegrateTime.value;
    if (SortTime.value > 0.0)
        ExtraTime.value -= SortTime.value;
}

// ****************************************************************************
//  Method: avtICAlgorithm::ReportStatistics
//
//  Purpose:
//      Report stats.
//
//  Programmer: Dave Pugmire
//  Creation:   January 28, 2009
//
// ****************************************************************************

void
avtICAlgorithm::ReportStatistics()
{
    CompileAlgorithmStatistics();

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
//  Method: avtICAlgorithm::ReportStatistics
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
//   Dave Pugmire, Fri Apr  3 11:46:10 EDT 2009
//   Add tags to the timings reports that make them easier to parse.
//
// ****************************************************************************

void
avtICAlgorithm::ReportStatistics(ostream &os)
{
    int nCPUs = 1;
#ifdef PARALLEL
    nCPUs = PAR_Size();
#endif
    os<<endl;
    os<<"ReportBegin: ***********************************************"<<endl;
    string db = picsFilter->GetInput()->GetInfo().GetAttributes().GetFullDBName();
    os<<"File= "<<db<<endl;
    os<<"Method= "<<AlgoName()<<" nCPUs= "<<nCPUs<<" nDom= "<<numDomains;
    os<<" nPts= "<<numSeedPoints<<endl;
    os<<"maxCount= "<<picsFilter->maxCount;
    os<<" domCache= "<<picsFilter->cacheQLen;
    os<<" workGrp=  "<<picsFilter->workGroupSz<<endl;
    os<<endl;

    ReportTimings(os, true);

    os<<endl;
    ReportCounters(os, true);

    os<<endl<<"Per Proccess:"<<endl;
    ReportTimings(os, false);
    ReportCounters(os, false);
    os<<endl;
    os<<"ReportEnd: ***********************************************"<<endl;
}

// ****************************************************************************
//  Method: avtICAlgorithm::ReportTimings
//
//  Purpose:
//      Print timing data.
//
//  Programmer: Dave Pugmire
//  Creation:   January 28, 2009
//
//  Modifications:
//  
//   Dave Pugmire, Mon Feb 23 13:38:49 EST 2009
//   Print total TOTAL time.
//
// ****************************************************************************
void
avtICAlgorithm::ReportTimings(ostream &os, bool totals)
{
    os<<"Timings: *********************************************"<<endl;
    if (totals)
        os<<"t_Time       = "<<TotalTime.max<<endl;
    else
        os<<"l_Time      = "<<TotalTime.value<<endl;

    PrintTiming(os, "TotalTime", TotalTime, TotalTime, totals);
    PrintTiming(os, "IntgTime", IntegrateTime, TotalTime, totals);
    PrintTiming(os, "IOTime", IOTime, TotalTime, totals);
    PrintTiming(os, "SortTime", SortTime, TotalTime, totals);
    PrintTiming(os, "ExtraTime", ExtraTime, TotalTime, totals);
}


// ****************************************************************************
//  Method: avtICAlgorithm::ReportCounters
//
//  Purpose:
//      Print timing data.
//
//  Programmer: Dave Pugmire
//  Creation:   January 28, 2009
//
//  Modifications:
//
//   Dave Pugmire, Thu Mar 26 12:02:27 EDT 2009
//   Add counters for domain loading.
//
// ****************************************************************************
void
avtICAlgorithm::ReportCounters(ostream &os, bool totals)
{
    os<<"Counters: ********************************************"<<endl;
    
    PrintCounter(os, "DomLoad", DomLoadCnt, totals);
    PrintCounter(os, "DomPurge", DomPurgeCnt, totals);
    PrintCounter(os, "IntgrCnt", IntegrateCnt, totals);

    //Report domain loads.
    if (totals)
    {
        os<<"t_DomUsed    = #Dom: "<<globalDomainsUsed<<" TLoads: "<<globalTotDomainsLoaded<<" ["<<globalDomainLoadedMin<<", "<<globalDomainLoadedMax<<", "<<globalAvgDomainLoaded<<"]"<<endl;
    }
    else
    {
        os<<"l_DomUsed    = #Dom: "<<domainsUsed<<" TLoads: "<<totDomainsLoaded<<" ["<<domainLoadedMin<<", "<<domainLoadedMax<<", "<<avgDomainLoaded<<"]"<<endl;
    }

}

// ****************************************************************************
//  Method: avtICAlgorithm::PrintTiming
//
//  Purpose:
//      Print timing data.
//
//  Programmer: Dave Pugmire
//  Creation:   January 28, 2009
//
//  Modifications:
//
//    Dave Pugmire, Thu Feb 12 08:47:29 EST 2009
//    Better formatting for stats output.
//
// ****************************************************************************
void
avtICAlgorithm::PrintTiming(ostream &os, 
                            const char *str, 
                            const ICStatistics &s,
                            const ICStatistics &t,
                            bool total)
{
    string strFmt = str;
    strFmt.resize(10, ' ');
    os << (total ? "t_" : "l_");
    os<<strFmt<<" = ";

    if (total)
    {
        os<<s.total;
        os<<" ["<<100.0*(s.total/t.total)<<"%] ";
        os<<" ["<<s.min<<", "<<s.max<<", "<<s.mean<<" : "<<s.sigma<<"]";

        if (s.mean != 0.0)
        {
            float v = s.sigma / s.mean;
            os<<" [s/m"<<v<<"]";
        }
        os<<endl;
    }
    else
    {
        float v = s.value;
        if (s.value < 0.0)
            v = 0.0;
        
        os<<v;
        os<<" ["<<100.0*(v/t.value)<<"%] ";
        os<<endl;
    }
}

// ****************************************************************************
//  Method: avtICAlgorithm::PrintCounter
//
//  Purpose:
//      Print counter data.
//
//  Programmer: Dave Pugmire
//  Creation:   January 28, 2009
//
//  Modifications:
//
//    Dave Pugmire, Thu Feb 12 08:47:29 EST 2009
//    Better formatting for stats output.
//
// ****************************************************************************
void
avtICAlgorithm::PrintCounter(ostream &os, 
                             const char *str, 
                             const ICStatistics &s,
                             bool total)
{
    string strFmt = str;
    strFmt.resize(10, ' ');
    os << (total ? "t_" : "l_");
    os<<strFmt<<" = ";

    if (total)
    {
        os<<s.total;
        os<<" ["<<s.min<<", "<<s.max<<", "<<s.mean<<" : "<<s.sigma<<"]";

        if (s.mean != 0.0)
        {
            float v = s.sigma / s.mean;
            os<<" ["<<v<<"]";
        }
        os<<endl;
#ifdef PARALLEL
        //Print histogram.
        if (PAR_Rank() == 0)
        {
            char f[128];
            sprintf(f, "%s_histogram.txt", str);
            ofstream hos;
            hos.open(f, ios::out);
            for (int i = 0; i < s.histogram.size(); i++)
                hos<<s.histogram[i]<<endl;
            hos.close();
        }
#endif
    }
    else
    {
        float v = s.value;
        if (s.value < 0.0)
            v = 0.0;
        float p = 0.0;
        if (s.total > 0.0)
            p = s.value/s.total * 100.0;

        float sd = 0.0;
        if (s.sigma != 0.0)
            sd = (v-s.mean) / s.sigma;

        //Print the value, % of all total, how many sigmas away.
        os<<v;
        os<<" ["<<p<<"%] ["<<sd<<"] ";
        os<<endl;
    }
}
