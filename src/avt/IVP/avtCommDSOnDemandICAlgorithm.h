/*****************************************************************************
*
* Copyright (c) 2000 - 2015, Lawrence Livermore National Security, LLC
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
//                              avtCommDSOnDemandICAlgorithm.h                 //
// ************************************************************************* //

#ifndef AVT_COMM_DS_ON_DEMAND_IC_ALGORITHM_H
#define AVT_COMM_DS_ON_DEMAND_IC_ALGORITHM_H

#if 0

#ifdef PARALLEL
#include <avtParICAlgorithm.h>

class domainCacheEntry
{
  public:
    domainCacheEntry(BlockIDType _dom, vtkDataSet *_ds){ds=_ds; dom=_dom; refCnt=0;}
    domainCacheEntry(){ds=NULL; refCnt=0;}

    BlockIDType dom;
    int refCnt;
    vtkDataSet *ds;
};

// ****************************************************************************
// Class: avtCommDSOnDemandICAlgorithm
//
// Purpose: Communicate domains on demand algorithm.
//   
//
// Programmer:  Dave Pugmire
// Creation:    December  6, 2010
//
// Modifications:
//
//   Dave Pugmire, Fri Dec 17 12:15:04 EST 2010
//   Override PostRunAlgorithm as a no-op.
//
//   Dave Pugmire, Fri Jan 14 11:06:09 EST 2011
//   Added PostStepCallback() method, numDone as member data.
//
// ****************************************************************************

class avtCommDSOnDemandICAlgorithm : public avtParICAlgorithm
{
  public:
    avtCommDSOnDemandICAlgorithm(avtPICSFilter *picsFilter, int cacheSize);
    virtual ~avtCommDSOnDemandICAlgorithm();

    virtual const char*       AlgoName() const {return "CommDSOnDemand";}
    virtual void              Initialize(std::vector<avtIntegralCurve *> &);
    virtual void              ResetIntegralCurvesForContinueExecute(int curTimeSlice=-1);
    virtual bool              CheckNextTimeStepNeeded(int curTimeSlice);
    virtual void              AddIntegralCurves(std::vector<avtIntegralCurve*> &ics);
    virtual bool              PostStepCallback();

  protected:
    vtkDataSet               *GetDataset(const BlockIDType &dom);

    virtual void              RunAlgorithm();
    virtual void              PostRunAlgorithm() {}
    virtual void              SortIntegralCurves(std::list<avtIntegralCurve *> &);
    virtual void              HandleOOBIC(avtIntegralCurve *s);
    virtual bool              RequestDataset(BlockIDType &d);
    virtual bool              HandleMessages(bool checkMsgs=true,
                                             bool checkDSs=true,
                                             bool allowBlockAndWait=true);

    void                      AddRef(const BlockIDType &dom);
    void                      DelRef(const BlockIDType &dom);
    void                      CheckCacheVacancy(bool makeReq);

    void Debug();
    
    std::list<avtIntegralCurve *> activeICs, oobICs;
    std::set<int>             pendingDomRequests;
    std::map<int, int>        pendingDomReqTimers;

    //Communicated domain cache.
    virtual vtkDataSet       *GetDSFromDomainCache(const BlockIDType &dom);
    virtual void              AddDSToDomainCache(std::vector<DSCommData> &ds);

    int                       numDone;
    std::list<domainCacheEntry> domainCache;
    int domainCacheSizeLimit;

    ICStatistics              DSLatencyTime;
    virtual void              CompileTimingStatistics()
    {
        avtParICAlgorithm::CompileTimingStatistics();
        ComputeStatistic(DSLatencyTime);
    }

    virtual void              ReportTimings(ostream &os, bool totals)
    {
        avtParICAlgorithm::ReportTimings(os, totals);
        PrintTiming(os, "DSLatTime", DSLatencyTime, TotalTime, totals);
    }
};

#endif

#endif
#endif
