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
#include <iomanip>

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
//  Modifications:
//
//   Dave Pugmire, Tue Mar 10 12:41:11 EDT 2009
//   Generalized domain to include domain/time. Pathine cleanup.
//
//  Dave Pugmire, Mon Mar 23 12:48:12 EDT 2009
//  Change how timings are reported/calculated.
//     
// ****************************************************************************

avtSLAlgorithm::avtSLAlgorithm( avtStreamlineFilter *slFilter ) :
    TotalTime("totT"), IOTime("ioT"), IntegrateTime("intT"), SortTime("sorT"), ExtraTime("extT"),
    IntegrateCnt("intC"), DomLoadCnt("domLC"), DomPurgeCnt("domPC")
{
    streamlineFilter = slFilter;
    numDomains = streamlineFilter->numDomains;
    numTimeSteps = streamlineFilter->numTimeSteps;
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
//  Modifications:
//
//   Dave Pugmire, Tue Mar 10 12:41:11 EDT 2009
//   Generalized domain to include domain/time. Pathine cleanup.
//
//   Dave Pugmire, Mon Mar 23 18:33:10 EDT 2009
//   Make changes for point decomposed domain databases.
//
// ****************************************************************************

vtkDataSet *
avtSLAlgorithm::GetDomain(avtStreamlineWrapper *slSeg)
{
    avtVector pt;
    slSeg->GetEndPoint(pt);
    return GetDomain(slSeg->domain, pt.x, pt.y, pt.z);
}

// ****************************************************************************
//  Method: avtSLAlgorithm::GetDomain
//
//  Purpose:
//      Retrieve a domain.
//
//  Programmer: Dave Pugmire
//  Creation:   March 23, 2009
//
// ****************************************************************************

vtkDataSet *
avtSLAlgorithm::GetDomain(const DomainType &dom, double X, double Y, double Z)
{
    int timerHandle = visitTimer->StartTimer();
    vtkDataSet *ds = streamlineFilter->GetDomain(dom, X, Y, Z);
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
//  Modifications:
//  
//   Dave Pugmire, Mon Feb 23 13:38:49 EST 2009
//   Initialize the initial domain load count and timer.  
//
// ****************************************************************************

void
avtSLAlgorithm::Initialize(vector<avtStreamlineWrapper *> &seedPts)
{
    numSeedPoints = seedPts.size();

    IOTime.value = streamlineFilter->InitialIOTime;
    TotalTime.value = streamlineFilter->InitialIOTime;
    DomLoadCnt.value = streamlineFilter->InitialDomLoads;
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
//  Modifications:
//
//   Dave Pugmire, Tue Mar 24 08:15:04 EDT 2009
//   Report stats if timer is enabled.
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
    
    if (visitTimer->Enabled())
        ReportStatistics();
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
//   Dave Pugmire, Tue Mar 10 12:41:11 EDT 2009
//   Generalized domain to include domain/time. Pathine cleanup.
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
        long d = (*s)->domain.domain, t = (*s)->domain.timeStep;
        long long key = (d<<32) + t;
        key = (*s)->domain.domain;
        if ( DomainLoaded((*s)->domain))
            (*s)->sortKey = -key;
        else
            (*s)->sortKey = key;
    }

    sl.sort(slDomainCompare);
    
    SortTime.value += visitTimer->StopTimer(timerHandle, "SortStreamlines()");
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
//   Dave Pugmire, Tue Mar 10 12:41:11 EDT 2009
//   Generalized domain to include domain/time. Pathine cleanup.
//
// ****************************************************************************

void
avtSLAlgorithm::SortStreamlines(vector<avtStreamlineWrapper *> &sl)
{
    int timerHandle = visitTimer->StartTimer();
    vector<avtStreamlineWrapper*>::iterator s;

    //Set sortkey to -domain. (So that loaded domains sort first).
    for (s=sl.begin(); s != sl.end(); ++s)
    {
        if ( DomainLoaded((*s)->domain))
            (*s)->sortKey = -(*s)->domain.domain;
        else
            (*s)->sortKey = (*s)->domain.domain;
    }

    sort(sl.begin(), sl.end(), slDomainCompare);
    
    SortTime.value += visitTimer->StopTimer(timerHandle, "SortStreamlines()");
}


// ****************************************************************************
//  Method: avtStreamlineFilter::CalculateTimingStatistics
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
avtSLAlgorithm::CompileTimingStatistics()
{
    ComputeStatistic(TotalTime);
    ComputeStatistic(IOTime);
    ComputeStatistic(IntegrateTime);
    ComputeStatistic(SortTime);
}

// ****************************************************************************
//  Method: avtStreamlineFilter::CalculateCounterStatistics
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
// ****************************************************************************

void
avtSLAlgorithm::CompileCounterStatistics()
{
    ComputeStatistic(IntegrateCnt);
    DomLoadCnt.value += streamlineFilter->GetLoadDSCount();
    DomPurgeCnt.value += streamlineFilter->GetPurgeDSCount();
    ComputeStatistic(DomLoadCnt);
    ComputeStatistic(DomPurgeCnt);

    ComputeDomainLoadStatistic();
}


// ****************************************************************************
//  Method: avtStreamlineFilter::CompileAlgorithmStatistics
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
avtSLAlgorithm::CompileAlgorithmStatistics()
{
    CompileTimingStatistics();
    CompileCounterStatistics();
    
    //Once all the timings have been computed, calc the extra time.
    CalculateExtraTime();
    ComputeStatistic(ExtraTime);
}

// ****************************************************************************
//  Method: avtStreamlineFilter::ComputeStatistic
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
avtSLAlgorithm::ComputeStatistic(SLStatistics &stats)
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
//  Method: avtStreamlineFilter::ComputeDomainLoadStatistic
//
//  Purpose:
//      Compute domain loading statistics.
//
//  Programmer: Dave Pugmire
//  Creation:   March 26, 2009
//
// ****************************************************************************

void
avtSLAlgorithm::ComputeDomainLoadStatistic()
{
    //Figure out the domain loads...
    std::map<unsigned long, int>::iterator it;
    int *domLoads = new int[numDomains];
    for (int i = 0; i < numDomains; i++)
        domLoads[i] = 0;

    domainsUsed = streamlineFilter->domainLoadCount.size();
    totDomainsLoaded = 0;
    domainLoadedMin = 0;
    domainLoadedMax = 0;
    avgDomainLoaded = 0.0;
    
    for (it = streamlineFilter->domainLoadCount.begin(); it != streamlineFilter->domainLoadCount.end(); it++)
    {
        domLoads[it->first] = it->second;
        totDomainsLoaded += it->second;
        
        if (it == streamlineFilter->domainLoadCount.begin())
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
        avgDomainLoaded = (float)domainsUsed / (float)totDomainsLoaded;

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
        globalAvgDomainLoaded = (float)globalDomainsUsed / (float)globalTotDomainsLoaded;
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
    if (IOTime.value > 0.0)
        ExtraTime.value -= IOTime.value;
    if (IntegrateTime.value > 0.0)
        ExtraTime.value -= IntegrateTime.value;
    if (SortTime.value > 0.0)
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
    os<<" maxCount= "<<streamlineFilter->maxCount;
    os<<" domCache= "<<streamlineFilter->cacheQLen;
    os<<" workGrp=  "<<streamlineFilter->workGroupSz<<endl;
    os<<endl;

    ReportTimings(os, true);

    os<<endl;
    ReportCounters(os, true);

    os<<endl<<"Per Proccess:"<<endl;
    ReportTimings(os, false);
    ReportCounters(os, false);
    os<<endl;
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
//  Modifications:
//  
//   Dave Pugmire, Mon Feb 23 13:38:49 EST 2009
//   Print total TOTAL time.
//
// ****************************************************************************
void
avtSLAlgorithm::ReportTimings(ostream &os, bool totals)
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
//  Method: avtSLAlgorithm::ReportCounters
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
avtSLAlgorithm::ReportCounters(ostream &os, bool totals)
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
//  Method: avtSLAlgorithm::PrintTiming
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
avtSLAlgorithm::PrintTiming(ostream &os, 
                            char *str, 
                            const SLStatistics &s,
                            const SLStatistics &t,
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
//  Method: avtSLAlgorithm::PrintCounter
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
avtSLAlgorithm::PrintCounter(ostream &os, 
                             char *str, 
                             const SLStatistics &s,
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
