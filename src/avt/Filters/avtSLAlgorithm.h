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
//                              avtSLAlgorithm.h                             //
// ************************************************************************* //

#ifndef AVT_SL_ALGORITHM_H
#define AVT_SL_ALGORITHM_H

#include <avtStreamlineFilter.h>
#include "avtStreamlineWrapper.h"

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
//    Add a SortStreamlines that takes a vector.
//
//   Dave Pugmire, Fri Feb  6 14:42:07 EST 2009
//   Add histogram to the statistics.
//
// ****************************************************************************

class avtSLAlgorithm
{
  public:
    avtSLAlgorithm( avtStreamlineFilter *slFilter );
    virtual ~avtSLAlgorithm();

    //Execution of the algorithm.
    virtual void              Initialize(std::vector<avtStreamlineWrapper *> &);
    virtual void              Execute() = 0;
    virtual void              PostExecute();

  protected:
    avtStreamlineFilter *streamlineFilter;
    std::list<avtStreamlineWrapper *> terminatedSLs;
    int                       numDomains, numSeedPoints;
    virtual const char*       AlgoName() const = 0;
    
    //Helper accessor funcstions to the filter.
    avtIVPSolver *            GetSolver() {return streamlineFilter->solver; }
    virtual bool              PointInDomain(avtVector &pt, int dom)
                            { return streamlineFilter->PointInDomain(pt, dom); }
    virtual void              IntegrateStreamline(avtStreamlineWrapper *slSeg);
    vtkDataSet               *GetDomain(int dom);
    virtual bool              DomainLoaded(int dom) const
                                 { return streamlineFilter->DomainLoaded(dom); }
    
    bool                      OwnDomain(int dom)
                                     {return streamlineFilter->OwnDomain(dom); }
    int                       DomainToRank(int dom)
                                     {return streamlineFilter->DomainToRank(dom); }

    //Utility functions.
    virtual void              SortStreamlines(std::list<avtStreamlineWrapper *> &);
    virtual void              SortStreamlines(std::vector<avtStreamlineWrapper *> &);
    //Statistics and timers.
    class SLStatistics
    {
      public:
        SLStatistics()
        {
            value = 0.0;
            min=max=mean=sigma=total=0.0;
            histogram.resize(0);
        }
        float min, max, mean, sigma, total;
        std::vector<float> histogram;
        float value;
    };

    virtual void              ReportStatistics();
    virtual void              CalculateStatistics();
    virtual void              CalculateExtraTime();    
    virtual void              ComputeStatistics(SLStatistics &stats);
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
};

#endif
