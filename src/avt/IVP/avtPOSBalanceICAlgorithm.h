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
//                              avtPOSBalanceICAlgorithm.h                 //
// ************************************************************************* //

#ifndef AVT_POS_BALANCE_IC_ALGORITHM_H
#define AVT_POS_BALANCE_IC_ALGORITHM_H

#if 0

#ifdef PARALLEL
#include "avtParICAlgorithm.h"


class avtPOSBalanceICAlgorithm : public avtParICAlgorithm
{
  public:
    avtPOSBalanceICAlgorithm(avtPICSFilter *picsFilter);
    virtual ~avtPOSBalanceICAlgorithm();
    virtual const char*       AlgoName() const {return "POS Balance";}

    static avtPOSBalanceICAlgorithm* Create(avtPICSFilter *picsFilter,
                                            int numToSend);

    virtual void              Initialize(std::vector<avtIntegralCurve *> &);
    virtual void              ResetIntegralCurvesForContinueExecute(int curTimeSlice = -1);
    virtual bool              CheckNextTimeStepNeeded(int curTimeSlice);

  protected:
    std::list<avtIntegralCurve *> activeICs, oobICs;
};

class avtPOSBalanceMasterICAlgorithm : public avtPOSBalanceICAlgorithm
{
  public:
    avtPOSBalanceMasterICAlgorithm(avtPICSFilter *picsFilter, int N);
    virtual ~avtPOSBalanceMasterICAlgorithm();
    virtual void              Initialize(std::vector<avtIntegralCurve *> &);
    virtual void              AddIntegralCurves(std::vector<avtIntegralCurve*> &sls);

  protected:
    virtual void              RunAlgorithm();
    
    std::list<avtIntegralCurve *> unassignedICs;
    int nToSend, numTerminated;
};


class avtPOSBalanceWorkerICAlgorithm : public avtPOSBalanceICAlgorithm
{
  public:
    avtPOSBalanceWorkerICAlgorithm(avtPICSFilter *picsFilter, int master);
    virtual ~avtPOSBalanceWorkerICAlgorithm();
    
    virtual void              Initialize(std::vector<avtIntegralCurve *> &);
    virtual void              AddIntegralCurves(std::vector<avtIntegralCurve*> &sls);
    
  protected:
    virtual void              RunAlgorithm();
    bool                      HandleMessages();
    int master;
};


#endif
#endif

#endif
