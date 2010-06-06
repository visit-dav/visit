/*****************************************************************************
*
* Copyright (c) 2000 - 2010, Lawrence Livermore National Security, LLC
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
//                              avtSLAlgorithm.h                             //
// ************************************************************************* //

#ifndef AVT_SL_ALGORITHM_H
#define AVT_SL_ALGORITHM_H

#include <avtPICSFilter.h>
#include "avtStreamline.h"

// ****************************************************************************
// Class: avtSLAlgorithm
//
// Purpose:
//    Abstract base class for streamline algorithms.
//
// Programmer: Dave Pugmire
// Creation:   Mon Jan 26 13:25:58 EST 2009
//
// Modifications:
//
//    Dave Pugmire, Fri Feb  6 08:43:00 EST 2009
//    Add a SortIntegralCurves that takes a vector.
//
//   Dave Pugmire, Fri Feb  6 14:42:07 EST 2009
//   Add histogram to the statistics.
//
//   Dave Pugmire, Tue Mar 10 12:41:11 EDT 2009
//   Generalized domain to include domain/time.
//   
//   Dave Pugmire, Mon Mar 23 12:48:12 EDT 2009
//   Change how timings are reported/calculated.
//
//   Dave Pugmire, Mon Mar 23 18:33:10 EDT 2009
//   Make changes for point decomposed domain databases.
//
//   Dave Pugmire, Thu Mar 26 12:02:27 EDT 2009
//   Add counters for domain loading
//
//   Dave Pugmire, Tue Aug 18 09:10:49 EDT 2009
//   Add ability to restart integration of streamlines.
//
//   Dave Pugmire, Thu Sep 24 13:52:59 EDT 2009
//   Replace Execute() with RunAlgorithm(). Add a Pre/Post RunAlgorithm.
//
//   Dave Pugmire, Tue Apr  6 08:24:44 EDT 2010
//   Make sure the avtStreamlineFilter baseclass gets called.
//
//   Dave Pugmire, Tue May 25 10:15:35 EDT 2010
//   Added DeleteIntegralCurves method.
//
//   Hank Childs, Fri Jun  4 19:58:30 CDT 2010
//   Use avtStreamlines, not avtStreamlineWrappers.
//
//   Hank Childs, Sun Jun  6 12:25:31 CDT 2010
//   Change the names of several methods to reflect the new emphasis in 
//   particle advection, as opposed to streamlines.  Also change reference
//   from avtStreamlineFilter to avtPICSFilter.
//
// ****************************************************************************

class avtSLAlgorithm
{
  public:
    avtSLAlgorithm( avtPICSFilter *slFilter );
    virtual ~avtSLAlgorithm();

    //Execution of the algorithm.
    virtual void              Initialize(std::vector<avtStreamline *> &);
    void                      Execute();
    virtual void              ResetIntegralCurvesForContinueExecute() = 0;
    virtual void              PostExecute();
    virtual void              GetTerminatedICs(vector<avtStreamline *> &v);
    virtual void              AddIntegralCurves(std::vector<avtStreamline*> &sls) = 0;
    virtual void              DeleteIntegralCurves(std::vector<int> &slIDs);

  protected:
    virtual void              RunAlgorithm() = 0;
    virtual void              PreRunAlgorithm() {}
    virtual void              PostRunAlgorithm() {}

    avtPICSFilter *picsFilter;
    std::list<avtStreamline *> terminatedICs;
    int                       numDomains, numTimeSteps, numSeedPoints;
    virtual const char*       AlgoName() const = 0;
    
    //Helper accessor funcstions to the filter.
    avtIVPSolver *            GetSolver() {return picsFilter->solver; }
    virtual bool              PointInDomain(avtVector &pt, DomainType &dom)
    { return picsFilter->avtPICSFilter::PointInDomain(pt, dom); }
    virtual void              AdvectParticle(avtStreamline *sl);
    vtkDataSet               *GetDomain(avtStreamline *sl);
    vtkDataSet               *GetDomain(const DomainType &dom,
                                        double X=0, double Y=0, double Z=0);
    virtual bool              DomainLoaded(DomainType &dom) const
    { return picsFilter->avtPICSFilter::DomainLoaded(dom); }
    
    bool                      OwnDomain(DomainType &dom)
    {return picsFilter->avtPICSFilter::OwnDomain(dom); }
    int                       DomainToRank(DomainType &dom)
    {return picsFilter->avtPICSFilter::DomainToRank(dom); }
    int                       DomCacheSize() const { return picsFilter->cacheQLen; }

    //Utility functions.
    virtual void              SortIntegralCurves(std::list<avtStreamline *> &);
    virtual void              SortIntegralCurves(std::vector<avtStreamline *> &);
    //Statistics and timers.
    class SLStatistics
    {
      public:
        SLStatistics(std::string s="")
        {
            nm = s;
            value = 0.0;
            min=max=mean=sigma=total=0.0;
            histogram.resize(0);
        }
        float min, max, mean, sigma, total;
        std::vector<float> histogram;
        float value;
        std::string nm;

        friend ostream& operator<<(std::ostream &out, const avtSLAlgorithm::SLStatistics &s)
        //friend ostream& operator<<(std::ostream &out)
        {
            out<<s.nm<<" V: "<<s.value<<" "<<s.total<<" ["<<s.min<<", "<<s.max<<", "<<s.mean<<" : "<<s.sigma<<"]";
            return out;
        }
    };

    virtual void              ReportStatistics();
    void                      CompileAlgorithmStatistics();
    virtual void              CompileTimingStatistics();
    virtual void              CompileCounterStatistics();
    virtual void              CalculateExtraTime();    
    virtual void              ComputeStatistic(SLStatistics &stats);
    virtual void              ComputeDomainLoadStatistic();
    virtual void              ReportStatistics(ostream &os);
    virtual void              ReportTimings(ostream &os, bool totals);
    virtual void              ReportCounters(ostream &os, bool totals);

    void                      PrintTiming(ostream &os,
                                          char *str, 
                                          const SLStatistics &s,
                                          const SLStatistics &t,
                                          bool total);
    void                      PrintCounter(ostream &os,
                                           char *str,
                                           const SLStatistics &s,
                                           bool total);

    //Timers.
    SLStatistics              TotalTime, IOTime, IntegrateTime, SortTime,
                              ExtraTime;
    //Counters.
    SLStatistics              IntegrateCnt, DomLoadCnt, DomPurgeCnt;

    //Special counters.
    int                       domainsUsed, totDomainsLoaded, domainLoadedMin, domainLoadedMax;
    int                       globalDomainsUsed, globalTotDomainsLoaded, globalDomainLoadedMin, globalDomainLoadedMax;
    float                     avgDomainLoaded, globalAvgDomainLoaded;
};

#endif
