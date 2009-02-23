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
//                              avtMasterSlaveSLAlgorithm.h                  //
// ************************************************************************* //

#ifndef AVT_MASTER_SLAVE_SL_ALGORITHM_H
#define AVT_MASTER_SLAVE_SL_ALGORITHM_H

#include "avtParSLAlgorithm.h"

#ifdef PARALLEL

class SlaveInfo;

// ****************************************************************************
// Class: avtMasterSlaveSLAlgorithm
//
// Purpose:
//    Abstract base class for master-slave algorithm.
//
// Programmer: Dave Pugmire
// Creation:   Mon Jan 26 13:25:58 EST 2009
//
// ****************************************************************************

class avtMasterSlaveSLAlgorithm : public avtParSLAlgorithm
{
  public:
    avtMasterSlaveSLAlgorithm(avtStreamlineFilter *slFilter,
                              int maxCount);
    virtual ~avtMasterSlaveSLAlgorithm();
    
    virtual void              Initialize(std::vector<avtStreamlineWrapper *> &);
    virtual const char*       AlgoName() const {return "MasterSlave";}

    static avtMasterSlaveSLAlgorithm* Create(avtStreamlineFilter *slFilter,
                                             int maxCount,
                                             int rank,
                                             int nProcs,
                                             int workGroupSz);

  protected:
    int                        maxCnt;
    
    int                        sleepMicroSec;
    void                       Sleep();
    
    static int                 MSG_STATUS, MSG_DONE, MSG_SEND_SL, MSG_LOAD_DOMAIN,
                               MSG_SEND_SL_HINT, MSG_FORCE_SEND_STATUS;

    //Statistics and coutners.
    int                       latencyTimer;
    SLStatistics              SleepTime, LatencyTime, MaxLatencyTime;
    SLStatistics              SleepCnt;
    virtual void              CalculateStatistics();
    virtual void              CalculateExtraTime();
    virtual void              ReportTimings(ostream &os, bool totals);
    virtual void              ReportCounters(ostream &os, bool totals);
};


// ****************************************************************************
// Class: avtMasterSLAlgorithm
//
// Purpose:
//    Master portion of the master-slave algorithm.
//
// Programmer: Dave Pugmire
// Creation:   Mon Jan 26 13:25:58 EST 2009
//
// ****************************************************************************

class avtMasterSLAlgorithm : public avtMasterSlaveSLAlgorithm
{
  public:
    avtMasterSLAlgorithm(avtStreamlineFilter *slFilter,
                         int maxCount,
                         int workGrpSz,
                         std::vector<int> &slaves);
    virtual ~avtMasterSLAlgorithm();

    virtual const char*       AlgoName() const {return "MasterSlave";}
    virtual void              Initialize(std::vector<avtStreamlineWrapper *> &);
    virtual void              Execute();

  protected:
    virtual void              CalculateStatistics();

    int                       totalNumStreamlines, workGroupSz;
    int                       case1Cnt, case2Cnt, case3Cnt, case4Cnt, case5Cnt;
    std::vector<SlaveInfo>    slaveInfo;
    std::vector<int>          slDomCnts, domLoaded, slackers;
    std::list<avtStreamlineWrapper *> activeSLs;

    void                      SendAllSlavesMsg(int msg);
    void                      SendAllSlavesMsg(std::vector<int> &msg);
    void                      SendSlaveMsg(int slave, std::vector<int> &msg);
    void                      FindSlackers(int oobFactor=-1,
                                           bool randomize= true,
                                           bool checkJustUpdated=false);
    bool                      UpdateStatus();
    bool                      UpdateSlaveStatus(std::vector<int> &);
    void                      PrintStatus();

    void                      Case1(int &counter);
    void                      Case2(int &counter);
    void                      Case3(int overloadFactor,
                                    int NDomainFactor,
                                    int &counter );
    void                      Case4(int oobThreshold,
                                    int &counter);
};


// ****************************************************************************
// Class: avtSlaveSLAlgorithm
//
// Purpose:
//    Slave portion of the master-slave algorithm.
//
// Programmer: Dave Pugmire
// Creation:   Mon Jan 26 13:25:58 EST 2009
//
// Modifications:
//
//   Dave Pugmire, Mon Feb 23 13:38:49 EST 2009
//   Add timeout counter for slaves. 
//
// ****************************************************************************

class avtSlaveSLAlgorithm : public avtMasterSlaveSLAlgorithm
{
  public:
    avtSlaveSLAlgorithm(avtStreamlineFilter *slFilter,
                        int maxCount,
                        int masterRank);
    virtual ~avtSlaveSLAlgorithm();

    virtual void              Initialize(std::vector<avtStreamlineWrapper *> &);
    virtual void              Execute();
    virtual void              SendStatus(bool forceSend=false);
    virtual void              UpdateStatus();

  protected:
    int                       master, numTerminated, timeout;
    bool                      workToDo;
    std::vector<int>          status, prevStatus;
    std::list<avtStreamlineWrapper *> activeSLs, oobSLs;

    void                      ProcessMessages(bool &done, bool &newMsg);
};


// ****************************************************************************
// Class: SlaveInfo
//
// Purpose:
//    Class to keep track of slave information.
//
// Programmer: Dave Pugmire
// Creation:   Mon Jan 26 13:25:58 EST 2009
//
// ****************************************************************************
class SlaveInfo
{
  public:
    SlaveInfo( int r, int nDomains );
    ~SlaveInfo() {}

    void AddSL( int slDomain);
    void LoadDom( int slDomain );
    void RemoveSL( int dom );
    void Update( vector<int> &status, bool debug=false );
    void Reset() { justUpdated = false; }
    void Debug();

    bool justUpdated;
    int canGive, canAccept, slCount, slLoadedCount, slOOBCount, rank;
    int domLoadedCount;
    vector<int> domainCnt;
    vector<bool> domainLoaded;
    vector<int> domainHistory;
};

#endif

#endif
