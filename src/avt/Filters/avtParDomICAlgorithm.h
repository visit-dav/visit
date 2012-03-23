/*****************************************************************************
*
* Copyright (c) 2000 - 2012, Lawrence Livermore National Security, LLC
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
//                              avtParDomICAlgorithm.h                       //
// ************************************************************************* //

#ifndef AVT_PAR_DOM_IC_ALGORITHM_H
#define AVT_PAR_DOM_IC_ALGORITHM_H

#ifdef PARALLEL
#include "avtParICAlgorithm.h"

// ****************************************************************************
// Class: avtParDomICAlgorithm
//
// Purpose:
//    A streamline algorithm that parallelizes over domains.
//
// Programmer: Dave Pugmire
// Creation:   Mon Jan 26 13:25:58 EST 2009
//
//  Modifications:
//
//   Dave Pugmire, Wed Feb  4 16:17:40 EST 2009
//   Regression fix. Handling streamlines that lie in multiple domains after
//   integration was not handled correctly after the code refactor. Added
//   HandleOOBSL().
//
//   Dave Pugmire, Fri Feb  6 08:43:00 EST 2009
//   Change numTerminated to numSLChange.
//
//   Dave Pugmire, Wed Apr  1 11:21:05 EDT 2009
//   Remove ExchangeSLs.
//
//   Dave Pugmire, Thu Sep 24 13:52:59 EDT 2009
//   Change Execute to RunAlgorithm.
//
//   Dave Pugmire, Thu Dec  3 13:28:08 EST 2009
//   Add AddStreamlines and ResetStreamlinesForContinueExecute
//
//   Hank Childs, Fri Feb 19 16:20:45 CST 2010
//   Add PreRunAlgorithm.
//
//   Hank Childs, Fri Jun  4 19:58:30 CDT 2010
//   Use avtStreamlines, not avtStreamlineWrappers.
//
//   Hank Childs, Sun Jun  6 12:25:31 CDT 2010
//   Change the names of several methods to reflect the new emphasis in
//   particle advection, as opposed to streamlines.  Also change reference
//   from avtStreamlineFilter to avtPICSFilter.
//
//   Hank Childs, Sun Jun  6 14:54:08 CDT 2010
//   Rename class "IC" from "SL", to reflect the emphasis on integral curves,
//   as opposed to streamlines.
//
//   Dave Pugmire, Tue Oct 19 10:53:51 EDT 2010
//   Fix for unstructured meshes.
//
//   Hank Childs, Fri Nov 26 14:39:43 PST 2010
//   Add data member (origNumIntegralCurves) to keep track of the starting
//   number of integral curves.  This is for progress updates.
//
//   Dave Pugmire, Thu Dec  2 11:21:06 EST 2010
//   Add CheckNextTimeStepNeeded.
//
// ****************************************************************************

class avtParDomICAlgorithm : public avtParICAlgorithm
{
  public:
    avtParDomICAlgorithm(avtPICSFilter *slFilter, int maxCount);
    virtual ~avtParDomICAlgorithm();

    virtual void              Initialize(std::vector<avtIntegralCurve *> &);
    virtual void              AddIntegralCurves(std::vector<avtIntegralCurve*> &sls);
    virtual void              ResetIntegralCurvesForContinueExecute(int curTimeSlice = -1);
    virtual bool              CheckNextTimeStepNeeded(int curTimeSlice);
    virtual const char*       AlgoName() const {return "ParallelStaticDomains";}

  protected:
    virtual void              RunAlgorithm();
    virtual void              PreRunAlgorithm();
    void                      HandleCommunication();
    void                      ProcessICs(std::list<ICCommData> &ics);
    void                      ProcessMsgs(std::vector<MsgCommData> &msgs);

    void                      HandleOOBIC(avtIntegralCurve *s);
    
    int                       numICChange, totalNumIntegralCurves;
    int                       origNumIntegralCurves;
    int                       sentICCounter;

    std::list<avtIntegralCurve *> activeICs;
    int                       maxCnt;

    //Tracks who uses ICs that are communicated.
    // pair<int,int> = (ic ID, sendICCounter).  Tracks a unique send of an IC to others.
    // pair<int, list<int> > = (numRanksThatUsedIC, list_of_ranks_send_IC).  This is bookeeping
    // to keep track of who used the IC (if any), and update IC count. If nobody used the IC, its
    // marked as terminated.
    std::map<std::pair<int,int>, std::pair<int, std::list<int> > > sendICInfo;
    typedef std::map<std::pair<int,int>, std::pair<int, std::list<int> > >::iterator sendICInfoIterator;
};


#endif
#endif
