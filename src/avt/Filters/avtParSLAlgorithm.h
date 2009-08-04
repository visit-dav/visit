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
//                              avtParSLAlgorithm.h                          //
// ************************************************************************* //

#ifndef AVT_PAR_SL_ALGORITHM_H
#define AVT_PAR_SL_ALGORITHM_H
#ifdef PARALLEL

#include "avtSLAlgorithm.h"

// ****************************************************************************
// Class: avtParSLAlgorithm
//
// Purpose:
//    Abstract base class for parallel streamline algorithms.
//
// Programmer: Dave Pugmire
// Creation:   Mon Jan 26 13:25:58 EST 2009
//
// Modifications:
//
//   Dave Pugmire, Thu Feb 12 08:43:01 EST 2009
//   Removed ComputeStatistics. (Moved to avtSLAlgorithm)
//
//   Dave Pugmire, Tue Mar 17 12:02:10 EDT 2009
//   Create a new RecvSLs method that doesn't check for domain inclusion.
//   
//  Dave Pugmire, Mon Mar 23 12:48:12 EDT 2009
//  Change how timings are reported/calculated.
//
//   Dave Pugmire, Wed Apr  1 11:21:05 EDT 2009
//   Message size and number of receives as member data. Add msgID to track msgs.
//
// ****************************************************************************

class avtParSLAlgorithm : public avtSLAlgorithm
{
  public:
    avtParSLAlgorithm(avtStreamlineFilter *slFilter);
    virtual ~avtParSLAlgorithm();

    virtual void              Initialize(std::vector<avtStreamlineWrapper *> &,
                                         int, int);
    virtual void              PostExecute();

  protected:
    void                      InitRequests();
    void                      CheckPendingSendRequests();
    void                      CleanupAsynchronous();
    void                      PostRecvStatusReq(int idx);
    void                      PostRecvSLReq(int idx);
    void                      SendMsg(int dest, std::vector<int> &msg);
    void                      SendAllMsg(std::vector<int> &msg);
    void                      RecvMsgs(std::vector<std::vector<int> > &msgs);
    void                      SendSLs(int dst,
                                      std::vector<avtStreamlineWrapper*> &);
    int                       RecvSLs(std::list<avtStreamlineWrapper*> &);
    int                       RecvSLs(std::list<avtStreamlineWrapper*> &,
                                      int &earlyTerminations);
    bool                      ExchangeSLs( std::list<avtStreamlineWrapper *> &,
                                           std::vector<std::vector<avtStreamlineWrapper *> >&,
                                           int &earlyTerminations );
    
    int                       rank, nProcs;
    std::map<MPI_Request, unsigned char*> sendSLBufferMap, recvSLBufferMap;
    std::map<MPI_Request, int *> sendIntBufferMap, recvIntBufferMap;

    std::vector<MPI_Request>  statusRecvRequests, slRecvRequests;

    virtual void              CompileTimingStatistics();
    virtual void              CompileCounterStatistics();
    virtual void              CalculateExtraTime();
    virtual void              ReportTimings(ostream &os, bool totals);
    virtual void              ReportCounters(ostream &os, bool totals);

    //Timers.
    SLStatistics              CommTime;
    //Counters.
    SLStatistics              MsgCnt, SLCommCnt, BytesCnt;

  private:
    static int                STATUS_TAG, STREAMLINE_TAG;
    int                       statusMsgSz, slMsgSz, numAsyncRecvs, msgID;
};

#endif
#endif
