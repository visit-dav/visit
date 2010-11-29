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
//                              avtParICAlgorithm.h                          //
// ************************************************************************* //

#ifndef AVT_PAR_IC_ALGORITHM_H
#define AVT_PAR_IC_ALGORITHM_H
#ifdef PARALLEL

#include "avtICAlgorithm.h"

class vtkDataSet;
class MemStream;

// ****************************************************************************
// Class: avtParICAlgorithm
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
//   Removed ComputeStatistics. (Moved to avtICAlgorithm)
//
//   Dave Pugmire, Tue Mar 17 12:02:10 EDT 2009
//   Create a new RecvSLs method that doesn't check for domain inclusion.
//   
//   Dave Pugmire, Mon Mar 23 12:48:12 EDT 2009
//   Change how timings are reported/calculated.
//
//   Dave Pugmire, Wed Apr  1 11:21:05 EDT 2009
//   Message size and number of receives as member data. Add msgID to track msgs.
//
//   Dave Pugmire, Thu Sep 24 13:52:59 EDT 2009
//   Replace Execute() with RunAlgorithm(). Add a Pre/Post RunAlgorithm.
//   Add code to exchange communicated SLs after all processing is complete.
//
//   Hank Childs, Fri Jun  4 19:58:30 CDT 2010
//   Use avtStreamlines, not avtStreamlineWrappers.
//
//   Hank Childs, Sun Jun  6 12:25:31 CDT 2010
//   Change reference from avtStreamlineFilter to avtPICSFilter.
//
//   Hank Childs, Sun Jun  6 14:54:08 CDT 2010
//   Rename class "IC" from "SL", to reflect the emphasis on integral curves,
//   as opposed to streamlines.
//
//   Hank Childs, Mon Jun  7 14:57:13 CDT 2010
//   Rename Initialize method to be InitializeBuffers to prevent name collision.
//
//   Hank Childs, Tue Jun  8 09:30:45 CDT 2010
//   Rename method to make its communication pattern more clear.
//
//   Dave Pugmire, Fri Sep 10 13:36:58 EDT 2010
//   Rewrite of most of this class.
//
//   Dave Pugmire, Fri Nov  5 15:39:58 EDT 2010
//   Fix for unstructured meshes. Need to account for particles that are sent to domains
//   that based on bounding box, and the particle does not lay in any cells.
//
//   Dave Pugmire, Mon Nov 29 09:23:01 EST 2010
//   Add optional tag argument to CleanupRequests.
//
// ****************************************************************************

class avtParICAlgorithm : public avtICAlgorithm
{
  public:
    avtParICAlgorithm(avtPICSFilter *icFilter);
    virtual ~avtParICAlgorithm();

    virtual void              InitializeBuffers(std::vector<avtIntegralCurve *> &seeds,
                                                int msgSize,
                                                int numMsgRecvs,
                                                int numICRecvs,
                                                int numDSRecvs=0);
    virtual void              PostExecute();

  protected:
    int                       rank, nProcs;
    std::list<avtIntegralCurve *> communicatedICs;

    virtual void              PostRunAlgorithm();

    //Manage communication.
    void                      CleanupRequests(int tag=-1);
    void                      CheckPendingSendRequests();

    // Send/Recv Integral curves.
    void                      SendICs(int dst, std::vector<avtIntegralCurve*> &v);
    bool                      RecvICs(std::list<avtIntegralCurve*> &v, std::list<int> *ranks=NULL);

    // Send/Recv messages.
    void                      SendMsg(int dst, std::vector<int> &msg);
    void                      SendAllMsg(std::vector<int> &msg);
    bool                      RecvMsg(std::vector<std::vector<int> > &msgs);

    // Send/Recv datasets.
    void                      SendDS(int dst, std::vector<vtkDataSet *> &ds);
    bool                      RecvDS(std::vector<vtkDataSet *> &ds);


    void                      RestoreIntegralCurveSequence();
    void                      MergeTerminatedICSequences();
    

    virtual void              CompileTimingStatistics();
    virtual void              CompileCounterStatistics();
    virtual void              CalculateExtraTime();
    virtual void              ReportTimings(ostream &os, bool totals);
    virtual void              ReportCounters(ostream &os, bool totals);

    //Timers/Counters.
    ICStatistics              CommTime;
    ICStatistics              MsgCnt, ICCommCnt, BytesCnt;
    
  private:
    void                      PostRecv(int tag);
    void                      PostRecv(int tag, int sz);
    void                      SendData(int dst, int tag, MemStream *buff);
    bool                      RecvData(int tag, std::vector<MemStream *> &buffers);
    void                      AddHeader(MemStream *buff);
    void                      RemoveHeader(MemStream *input, MemStream *header, MemStream *buff);

    bool                      DoSendICs(int dst,
                                        std::vector<avtIntegralCurve*> &);
    void                      PrepareForSend(int tag, MemStream *buff, std::vector<unsigned char *> &buffList);
    static bool               PacketCompare(const unsigned char *a, const unsigned char *b);
    void                      ProcessReceivedBuffers(int tag, vector<unsigned char*> &incomingBuffers,
                                                     vector<MemStream *> &buffers);

    // Send/Recv buffer management structures.
    typedef std::pair<MPI_Request, int> RequestTagPair;
    typedef std::pair<int, int> RankIdPair;
    typedef std::map<RequestTagPair, unsigned char *>::iterator bufferIterator;
    typedef std::map<RankIdPair, std::list<unsigned char *> >::iterator packetIterator;
    
    std::map<RequestTagPair, unsigned char *> sendBuffers, recvBuffers;
    std::map<RankIdPair, std::list<unsigned char *> > recvPackets;

    
    // Maps MPI_TAG to pair(num buffers, data size).
    std::map<int, std::pair<int, int> > messageTagInfo;
    int numMsgRecvs, numSLRecvs, numDSRecvs;
    int slSize, slsPerRecv, msgSize;
    
    
    static int                MESSAGE_TAG, STREAMLINE_TAG, DATASET_PREP_TAG, DATASET_TAG;
    int                       msgID;

    //Message headers.
    typedef struct
    {
        int rank, id, numPackets, packet, packetSz, dataSz;
    } Header;
};

#endif
#endif
