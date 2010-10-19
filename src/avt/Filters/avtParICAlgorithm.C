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
//                              avtParICAlgorithm.C                          //
// ************************************************************************* //

#include "avtParICAlgorithm.h"

#include <avtStateRecorderIntegralCurve.h>

#include <TimingsManager.h>
#include <VisItException.h>

using namespace std;

#ifdef PARALLEL

int avtParICAlgorithm::STATUS_TAG =  420000;
int avtParICAlgorithm::STREAMLINE_TAG = 420001;

// ****************************************************************************
//  Method: avtParICAlgorithm::avtParICAlgorithm
//
//  Purpose:
//      avtParICAlgorithm constructor.
//
//  Programmer: Dave Pugmire
//  Creation:   January 27, 2009
//
//   Dave Pugmire, Mon Mar 23 12:48:12 EDT 2009
//   Change how timings are reported/calculated.
//
//   Dave Pugmire, Wed Apr  1 11:21:05 EDT 2009
//   Limit the number of async recvs outstanding.
//   
// ****************************************************************************

avtParICAlgorithm::avtParICAlgorithm(avtPICSFilter *icFilter)
    : avtICAlgorithm(icFilter),
      CommTime("comT"), MsgCnt("msgC"), ICCommCnt("iccC"), BytesCnt("byteC")
{
    nProcs = PAR_Size();
    rank = PAR_Rank();
    msgID = 0;
    statusMsgSz = -1;
    numAsyncRecvs = -1;
    icMsgSz = 10*1024*1024;
}

// ****************************************************************************
//  Method: avtParICAlgorithm::~avtParICAlgorithm
//
//  Purpose:
//      avtParICAlgorithm destructor.
//
//  Programmer: Dave Pugmire
//  Creation:   January 27, 2009
//
// ****************************************************************************

avtParICAlgorithm::~avtParICAlgorithm()
{
}

// ****************************************************************************
//  Method: avtParICAlgorithm::Initialize
//
//  Purpose:
//      Initialize the request buffers.
//
//  Programmer: Dave Pugmire
//  Creation:   January 27, 2009
//
//  Modifications:
//
//   Dave Pugmire, Wed Apr  1 11:21:05 EDT 2009
//   Limit the number of async recvs outstanding.
//
//   Hank Childs, Fri Jun  4 19:58:30 CDT 2010
//   Use avtStreamlines, not avtStreamlineWrappers.
//
//   Hank Childs, Mon Jun  7 14:57:13 CDT 2010
//   Rename to InitializeBuffers to prevent name collision.
//
// ****************************************************************************

void
avtParICAlgorithm::InitializeBuffers(vector<avtIntegralCurve *> &seedPts,
                                     int msgSz,
                                     int numRecvs)
{
    //Standardmsg + 1(sender rank) +1(msg ID).
    statusMsgSz = msgSz+1+1;
    numAsyncRecvs = numRecvs;
    
    if (statusMsgSz <= 0 || numAsyncRecvs <= 0)
        EXCEPTION0(ImproperUseException);
    
    avtICAlgorithm::Initialize(seedPts);
    InitRequests();
}


// ****************************************************************************
//  Method: avtParICAlgorithm::PostRunAlgorithm
//
//  Purpose:
//      Carry out whatever communication pattern is necessary to get the 
//      integral curves in their intended location.
//
//  Programmer: Dave Pugmire
//  Creation:   September 24, 2009
//
//  Modifications:
//
//    Hank Childs, Tue Jun  8 09:30:45 CDT 2010
//    Add infrastructure to support new communication patterns.
//
// ****************************************************************************

void
avtParICAlgorithm::PostRunAlgorithm()
{
    // We are enumerating the possible communication styles and then just
    // calling the correct one.  This is an okay solution if there are a small
    // number of styles (which there are right now).  That said, it would be
    // perfectly fine if someone wanted to make this more extensible to handle
    // a wide array of communication patterns ... it just didn't seem worth
    // the effort when this was implemented.

    avtPICSFilter::CommunicationPattern pattern = 
                                         picsFilter->GetCommunicationPattern();
 
    if (pattern == avtPICSFilter::RestoreSequence)
        RestoreIntegralCurveSequence();
    else if (pattern == avtPICSFilter::LeaveOnCurrentProcessor)
        ;
    else if (pattern == avtPICSFilter::ReturnToOriginatingProcessor)
    { 
        EXCEPTION1(VisItException, 
                   "This communication pattern has not been implemented."); 
    }
    else
    { 
        EXCEPTION1(VisItException, "Undefined communication pattern");
    }
}

// ****************************************************************************
//  Method: avtParICAlgorithm::PostExecute
//
//  Purpose:
//      Cleanup.
//
//  Programmer: Dave Pugmire
//  Creation:   January 27, 2009
//
// ****************************************************************************

void
avtParICAlgorithm::PostExecute()
{
    CleanupAsynchronous();
    avtICAlgorithm::PostExecute();
}

// ****************************************************************************
//  Method: avtParICAlgorithm::InitRequests
//
//  Purpose:
//      Initialize the request buffers.
//
//  Programmer: Dave Pugmire
//  Creation:   June 16, 2008
//
//   Dave Pugmire, Wed Apr  1 11:21:05 EDT 2009
//   Limit the number of async recvs outstanding.
//
//   Hank Childs, Fri Jun  4 19:58:30 CDT 2010
//   Use avtStreamlines, not avtStreamlineWrappers.
//
// ****************************************************************************

void
avtParICAlgorithm::InitRequests()
{
    debug5<<"avtParICAlgorithm::InitRequests() sz= "<<numAsyncRecvs<<endl;
    statusRecvRequests.resize(numAsyncRecvs, MPI_REQUEST_NULL);
    icRecvRequests.resize(numAsyncRecvs, MPI_REQUEST_NULL);
    
    for (int i = 0; i < statusRecvRequests.size(); i++)
    {
        PostRecvStatusReq(i);
        PostRecvICReq(i);
    }
}

static int
CountIDs(list<avtIntegralCurve *> &l, int id)
{
    int cnt = 0;
    list<avtIntegralCurve*>::const_iterator si = l.begin();
    for (si = l.begin(); si != l.end(); si++)
    {
        if ((*si)->id == id)
            cnt++;
    }
    return cnt;
}

// ****************************************************************************
//  Method: avtParICAlgorithm::RestoreIntegralCurveSequence
//
//  Purpose:
//      Communicate streamlines pieces to destinations.
//      When a streamline is communicated, only the state information is sent.
//      All the integration steps need to sent to the proc that owns the terminated
//      streamline.  This method figures out where each streamline has terminated and
//      sends all the pieces there.
//
//  Programmer: Dave Pugmire
//  Creation:   September 21, 2009
//
//  Modifications:
//
//   Hank Childs, Fri Jun  4 19:58:30 CDT 2010
//   Use avtStreamlines, not avtStreamlineWrappers.
//
//   Hank Childs, Tue Jun  8 09:30:45 CDT 2010
//   Rename method, as we plan to add more communication methods.
//
// ****************************************************************************

void
avtParICAlgorithm::RestoreIntegralCurveSequence()
{
    debug5<<"RestoreIntegralCurveSequence: communicatedICs: "
          <<communicatedICs.size()
          <<" terminatedICs: "<<terminatedICs.size()<<endl;

    //Communicate to everyone where the terminators are located.
    //Do this "N" streamlines at a time, so we don't have a super big buffer.
    int N;
    if (numSeedPoints > 500)
        N = 500;
    else
        N = numSeedPoints;
    
    long *idBuffer = new long[2*N], *myIDs = new long[2*N];

    //Sort the terminated/communicated ICs by id.
    terminatedICs.sort(avtStateRecorderIntegralCurve::IdSeqCompare);
    communicatedICs.sort(avtStateRecorderIntegralCurve::IdSeqCompare);

    vector<vector<avtIntegralCurve *> >sendICs(N);
    vector<int> owners(N);
    
    int minId = 0;
    int maxId = N-1;

    int nLoops = 0 ; 

    if( N > 0 )
    {
        nLoops = numSeedPoints/N;

        if (numSeedPoints % N != 0)
            nLoops++;
    }

    for (int l = 0; l < nLoops; l++)
    {
        //Initialize arrays for this round.
        for (int i = 0; i < N; i++)
        {
            idBuffer[i] = 0;
            idBuffer[i+N] = 0;
            myIDs[i] = 0;
            myIDs[i+N] = 0;
            sendICs[i].resize(0);
            owners[i] = 0;
        }

        //Set array for ICs that terminated here. Update sequence counts for communicated
        //ICs.
        list<avtIntegralCurve*>::iterator t = terminatedICs.begin();
        while (t != terminatedICs.end() && (*t)->id <= maxId)
        {
            if ((*t)->id >= minId)
            {
                int idx = (*t)->id % N;
                myIDs[idx] = rank;
                myIDs[idx+N] += 1;
                debug5<<"I own id= "<<(*t)->id<<" "<<(((avtStateRecorderIntegralCurve *)*t))->sequenceCnt<<" idx= "<<idx<<endl;
            }

            t++;
        }
        
        list<avtIntegralCurve*>::const_iterator c = communicatedICs.begin();
        while (c != communicatedICs.end() && (*c)->id <= maxId)
        {
            if ((*c)->id >= minId)
            {
                int idx = (*c)->id % N;
                myIDs[idx+N] += 1;
                debug5<<"I have "<<(*c)->id<<" "<<(((avtStateRecorderIntegralCurve *)*c))->sequenceCnt<<" idx= "<<idx<<endl;
            }
            c++;
        }
        
        //Exchange ID owners and sequence counts.
        MPI_Allreduce(myIDs, idBuffer, 2*N, MPI_LONG, MPI_SUM, VISIT_MPI_COMM);
        if (0)
        {
            debug5<<"idBuffer:  [";
            for(int i=0; i<2*N;i++)
                debug5<<idBuffer[i]<<" ";
            debug5<<"]"<<endl;
        }
        
        //Now we know where all ICs belong and how many sequences for each.
        //Send communicatedICs to the owners.
        while (!communicatedICs.empty())
        {
            avtIntegralCurve *s = communicatedICs.front();
            if (s->id <= maxId)
            {
                int idx = s->id%N;
                int owner = idBuffer[idx];
                if (owner == rank)
                    terminatedICs.push_back(s);
                else
                {
                    ((avtStateRecorderIntegralCurve *)s)->serializeFlags = avtIntegralCurve::SERIALIZE_STEPS; //Write IC steps.
                    sendICs[idx].push_back(s);
                    owners[idx] = owner;
                }
                communicatedICs.pop_front();
            }
            else
                break;
        }
        
        for (int i = 0; i < N; i++)
        {
            if (sendICs[i].size() > 0)
            {
                DoSendICs(owners[i], sendICs[i]);

                for (int j = 0; j < sendICs[i].size(); j++)
                    delete sendICs[i][j];
            }
        }
        
        //Wait for all the sequences to arrive. The total number is known for
        //each IC, so wait until they all come.
        bool seqGathered = false;
        while (!seqGathered)
        {
            RecvICs(terminatedICs);
            
            //See if we have all the sequences we need.
            terminatedICs.sort(avtStateRecorderIntegralCurve::IdSeqCompare);
            bool needMore = false;
            for (int i = 0; i < N && !needMore; i++)
                if (idBuffer[i] == rank)
                    needMore = (CountIDs(terminatedICs, i+minId) < idBuffer[i+N]);
            
            //Everyone done.
            seqGathered = !needMore;
        }
        
        //Advance to next N streamlines.
        maxId += N;
        minId += N;
    }

    //All ICs are distributed, merge the sequences into single streamlines.
    MergeTerminatedICSequences();
    
    delete [] idBuffer;
    delete [] myIDs;
}

// ****************************************************************************
//  Method: avtParICAlgorithm::MergeTerminatedICSequences
//
//  Purpose:
//      Merge streamline sequences.
//
//  Programmer: Dave Pugmire
//  Creation:   Sept 21, 2009
//
//  Modifications:
//
//   Hank Childs, Fri Jun  4 19:58:30 CDT 2010
//   Use avtStreamlines, not avtStreamlineWrappers.
//
//   Hank Childs, Tue Jun  8 09:30:45 CDT 2010
//   Reflect movement of some routines to state recorder IC class.
//
// ****************************************************************************

void
avtParICAlgorithm::MergeTerminatedICSequences()
{
    //Sort them by id and sequence so we can process them one at a time.
    terminatedICs.sort(avtStateRecorderIntegralCurve::IdSeqCompare);

    //Split them up into sequences.
    vector<vector<avtIntegralCurve *> > seqs;
    while (!terminatedICs.empty())
    {
        avtIntegralCurve *s = terminatedICs.front();
        terminatedICs.pop_front();
        
        //Empty or new ID, add a new entry.
        if (seqs.size() == 0 ||
            seqs[seqs.size()-1][0]->id != s->id)
        {
            vector<avtIntegralCurve *> v;
            v.push_back(s);
            seqs.push_back(v);
        }
        else
        {
            seqs[seqs.size()-1].push_back(s);
        }
    }
    terminatedICs.clear();
    
    //Merge the sequences together, put them into terminated list.
    for (int i = 0; i < seqs.size(); i++)
    {
        avtIntegralCurve *s = 
            avtStateRecorderIntegralCurve::MergeIntegralCurveSequence(seqs[i]);
        terminatedICs.push_back(s);
    }
}

// ****************************************************************************
//  Method: avtParICAlgorithm::CleanupAsynchronous
//
//  Purpose:
//      Claenup the buffers used when doing asynchronous processing.
//
//  Programmer: Dave Pugmire
//  Creation:   June 16, 2008
//
// ****************************************************************************

void
avtParICAlgorithm::CleanupAsynchronous()
{
    for (int i = 0; i < statusRecvRequests.size(); i++)
    {
        MPI_Request req = statusRecvRequests[i];
        if (req != MPI_REQUEST_NULL)
            MPI_Cancel(&req);
    } 

    for (int i = 0; i < icRecvRequests.size(); i++)
    {
        MPI_Request req = icRecvRequests[i];
        if (req != MPI_REQUEST_NULL)
            MPI_Cancel(&req);
    }

    // Cleanup recv buffers.
    map<MPI_Request, unsigned char*>::const_iterator it;
    for (it = recvICBufferMap.begin(); it != recvICBufferMap.end(); ++it)
    {
        char *buff = (char *)it->second;
        if (it->second != NULL)
            delete [] it->second;
    }
    recvICBufferMap.clear();

    map<MPI_Request, int*>::const_iterator itt;
    for (itt = recvIntBufferMap.begin(); itt != recvIntBufferMap.end(); ++itt)
    {
        char *buff = (char *)itt->second;
        if (itt->second != NULL)
            delete [] itt->second;
    }
    recvIntBufferMap.clear();
}


// ****************************************************************************
//  Method: avtParICAlgorithm::CheckPendingSendRequests
//
//  Purpose:
//      Check to see if there are any pending send requests.
//
//  Programmer: Dave Pugmire
//  Creation:   June 16, 2008
//
//  Modifications:
//
//    Dave Pugmire, Wed Mar 18 17:07:07 EDT 2009
//    Delete entry from map after send is complete.
//
//   Dave Pugmire, Mon Mar 23 12:48:12 EDT 2009
//   Change how timings are reported/calculated.
//
//   Dave Pugmire, Sat Mar 28 22:21:49 EDT 2009
//   Bug fix. "notCompleted" wasn't in an else clause for the INT messages.
//   
//   Hank Childs, Sat Feb 20 16:53:18 CST 2010
//   Don't output timing values to the timing logs.
//
// ****************************************************************************
void
avtParICAlgorithm::CheckPendingSendRequests()
{
    debug5<<"avtParICAlgorithm::CheckPendingSendRequests()\n";
    int communicationTimer = visitTimer->StartTimer();
    
    if (sendICBufferMap.size() > 0)
    {
        vector<MPI_Request> req, copy;

        int notCompleted = 0;
        map<MPI_Request, unsigned char*>::const_iterator it;
        for (it = sendICBufferMap.begin(); it != sendICBufferMap.end(); ++it)
        {
            if (it->first != MPI_REQUEST_NULL && it->second != NULL)
            {
                req.push_back(it->first);
                copy.push_back(it->first);
            }
            else
                notCompleted++;
        }

        debug5 << "\tCheckPendingSendRequests() IC completed = "<<req.size()
               <<" not completed: "<<notCompleted<<endl;

        if (req.size() > 0)
        {
            // See if any sends have completed. Delete buffers if they have.
            int num = 0, *indices = new int[req.size()];
            MPI_Status *status = new MPI_Status[req.size()];
            int err = MPI_Testsome(req.size(), &req[0], &num, indices, status);
            
            for (int i = 0; i < num; i++)
            {
                int idx = indices[i];
                MPI_Request r = copy[idx];
                unsigned char *buff = sendICBufferMap[r];
                debug5 << "\tidx = " << idx << " r = " << r << " buff = " 
                       << (void *)buff << endl;
                if (buff)
                    delete [] buff;

                sendICBufferMap[r] = NULL;
                sendICBufferMap.erase(r);
            }
            
            delete [] indices;
            delete [] status;
        }
    }

    if (sendIntBufferMap.size() > 0)
    {
        vector<MPI_Request> req, copy;
        map<MPI_Request, int*>::const_iterator it;
        int notCompleted = 0;

        for (it = sendIntBufferMap.begin(); it != sendIntBufferMap.end(); ++it)
        {
            if (it->first != MPI_REQUEST_NULL && it->second != NULL)
            {
                req.push_back(it->first);
                copy.push_back(it->first);
            }
            else
                notCompleted++;
        }

        debug5 << "\tCheckPendingSendRequests() INT completed = "<<req.size()
               <<" not completed: "<<notCompleted<<endl;
        
        if (req.size() > 0)
        {
            // See if any sends have completed. Delete buffers if they have.
            int num = 0, *indices = new int[req.size()];
            MPI_Status *status = new MPI_Status[req.size()];
            int err = MPI_Testsome(req.size(), &req[0], &num, indices, status);
            
            for (int i = 0; i < num; i++)
            {
                int idx = indices[i];
                MPI_Request r = copy[idx];
                int *buff = sendIntBufferMap[r];
                debug5 << "\tidx = " << idx << " r = " << r << " buff = " 
                       << (void *)buff << endl;
                if (buff)
                    delete [] buff;

                sendIntBufferMap[r] = NULL;
                sendIntBufferMap.erase(r);
            }
            
            delete [] indices;
            delete [] status;
        }
    }

    bool nov = visitTimer->GetNeverOutputValue();
    visitTimer->NeverOutput(true);
    CommTime.value += visitTimer->StopTimer(communicationTimer, 
                                            "CheckPending");
    visitTimer->NeverOutput(nov);
    debug5 << "DONE  CheckPendingSendRequests()\n";
}

// ****************************************************************************
//  Method: avtParICAlgorithm::PostRecvStatusReq
//
//  Purpose:
//      Receives status requests.
//
//  Programmer: Dave Pugmire
//  Creation:   June 16, 2008
//
//   Dave Pugmire, Wed Apr  1 11:21:05 EDT 2009
//   Limit the number of async recvs outstanding.
//
// ****************************************************************************

void
avtParICAlgorithm::PostRecvStatusReq(int idx)
{
    MPI_Request req;
    int *buff = new int[statusMsgSz];

    MPI_Irecv(buff, statusMsgSz, MPI_INT, MPI_ANY_SOURCE,
              avtParICAlgorithm::STATUS_TAG,
              VISIT_MPI_COMM, &req);
    debug5 << "Post Statusrecv " <<idx<<" req= "<<req<<endl;
    statusRecvRequests[idx] = req;
    recvIntBufferMap[req] = buff;
}

// ****************************************************************************
//  Method: avtParICAlgorithm::PostRecvICReq
//
//  Purpose:
//      Receives status requests.
//
//  Programmer: Dave Pugmire
//  Creation:   June 16, 2008
//
//  Modifications:
//
//   Dave Pugmire, Wed Apr  1 11:21:05 EDT 2009
//   Limit the number of async recvs outstanding.
//
// ****************************************************************************

void
avtParICAlgorithm::PostRecvICReq(int idx)
{
    MPI_Request req;
    unsigned char *buff = new unsigned char[icMsgSz];
    MPI_Irecv(buff, icMsgSz,
              MPI_UNSIGNED_CHAR, MPI_ANY_SOURCE,
              avtParICAlgorithm::STREAMLINE_TAG, 
              VISIT_MPI_COMM, &req);

    debug5 << "Post ICrecv " <<idx<<" req= "<<req<<endl;
    icRecvRequests[idx] = req;
    recvICBufferMap[req] = buff;
}


// ****************************************************************************
//  Method: avtParICAlgorithm::SendMsg
//
//  Purpose:
//      Send an asynchronous message.
//
//  Programmer: Dave Pugmire
//  Creation:   Dec 18, 2008
//
// Modifications:
//
//   Dave Pugmire, Wed Apr  1 11:21:05 EDT 2009
//   Add the senders rank and msgID to the front of the message.
//
//   Hank Childs, Sat Feb 20 16:53:18 CST 2010
//   Don't output timing values to the timing logs.
//
// ****************************************************************************

void
avtParICAlgorithm::SendMsg(int dst,
                           vector<int> &msg)
{
    int communicationTimer = visitTimer->StartTimer();
    if (msg.size() > statusMsgSz)
        EXCEPTION0(ImproperUseException);
    
    int *buff = new int[statusMsgSz];
    buff[0] = msgID;
    msgID++;
    buff[1] = rank;
    
    MPI_Request req;
    for (int i = 0; i < msg.size(); i++)
        buff[2+i] = msg[i];

    debug5<<"SendMsg to :"<<dst<<" [";
    for(int i = 0; i < statusMsgSz; i++) debug5<<buff[i]<<" ";
    debug5<<"]"<<endl;
        
    int err = MPI_Isend(buff, statusMsgSz, MPI_INT, dst,
                        avtParICAlgorithm::STATUS_TAG,
                        VISIT_MPI_COMM, &req);

    sendIntBufferMap[req] = buff;
    
    BytesCnt.value += (sizeof(int) *statusMsgSz);
    MsgCnt.value++;
    bool nov = visitTimer->GetNeverOutputValue();
    visitTimer->NeverOutput(true);
    CommTime.value += visitTimer->StopTimer(communicationTimer, 
                                            "SendMsg");
    visitTimer->NeverOutput(nov);
    debug5 << "DONE  CheckPendingSendRequests()\n";
}

// ****************************************************************************
//  Method: avtParICAlgorithm::SendAllMsg
//
//  Purpose:
//      Broadcast a message.
//
//  Programmer: Dave Pugmire
//  Creation:   Dec 18, 2008
//
// Modifications:
//
//
// ****************************************************************************

void
avtParICAlgorithm::SendAllMsg(vector<int> &msg)
{
    for (int i = 0; i < nProcs; i++)
        if (i != rank)
            SendMsg(i, msg);
}

// ****************************************************************************
//  Method: avtParICAlgorithm::RecvMsgs
//
//  Purpose:
//      Recieve any messages.
//
//  Programmer: Dave Pugmire
//  Creation:   Dec 18, 2008
//
// Modifications:
//
//   Dave Pugmire, Mon Mar 23 12:48:12 EDT 2009
//   Change how timings are reported/calculated.
//
//   Dave Pugmire, Wed Apr  1 11:21:05 EDT 2009
//   Senders rank and msgID is in the message now.
//   
//   Hank Childs, Sat Feb 20 16:53:18 CST 2010
//   Don't output timing values to the timing logs.
//
// ****************************************************************************

void
avtParICAlgorithm::RecvMsgs(std::vector<std::vector<int> > &msgs)
{
    debug5<<"avtParICAlgorithm::RecvMsgs()\n";
    int communicationTimer = visitTimer->StartTimer();
    
    msgs.resize(0);
    while (true)
    {
        int nReq = statusRecvRequests.size();
        MPI_Status *status = new MPI_Status[nReq];
        int *indices = new int[nReq];
        int num = 0, err;

        vector<MPI_Request> copy;
        for (int i = 0; i < statusRecvRequests.size(); i++)
            copy.push_back(statusRecvRequests[i]);

        err = MPI_Testsome(nReq, &copy[0], &num, indices, status);
        debug5<<"::RecvMsgs() err= "<<err<<" Testsome("<<nReq<<"); num= "<<num<<endl;

        if (num > 0)
        {
            for (int i = 0; i < num; i++)
            {
                int idx = indices[i];
                debug5<<"RecvMsg from "<<idx<<endl;

                MPI_Request req = statusRecvRequests[idx];
                if (req == MPI_REQUEST_NULL)
                    continue;
                
                int *buff = recvIntBufferMap[req];
                recvIntBufferMap.erase(req);
                if (buff == NULL)
                    continue;

                debug5<<"RecvMsg: [";
                for(int i = 0; i < statusMsgSz; i++) debug5<<buff[i]<<" ";
                debug5<<"]"<<endl;
                
                //Skip msg ID, copy buffer int msg.
                vector<int> msg;
                for (int i = 1; i < statusMsgSz; i++)
                    msg.push_back(buff[i]);
                msgs.push_back(msg);

                //Clean up.
                delete [] buff;
            }
        
            //Repost recv requests.
            for (int i = 0; i < num; i++)
                PostRecvStatusReq(indices[i]);
        }
            
        delete [] status;
        delete [] indices;
        if (num == 0)
            break;
    }
    bool nov = visitTimer->GetNeverOutputValue();
    visitTimer->NeverOutput(true);
    debug5 << "DONE  CheckPendingSendRequests()\n";
    CommTime.value += visitTimer->StopTimer(communicationTimer,
                                            "RecvMsgs");
    visitTimer->NeverOutput(nov);
}

// ****************************************************************************
//  Method: avtParICAlgorithm::SendICs
//
//  Purpose:
//      Send streamlines to a dst.
//
//  Programmer: Dave Pugmire
//  Creation:   June 16, 2008
//
//  Modifications:
//
//   Dave Pugmire, Fri Aug 22 14:47:11 EST 2008
//   Memory leak fix.
//
//   Dave Pugmire, Thu Sep 24 14:03:46 EDT 2009
//   Call new method, DoSendICs.
//
//   Hank Childs, Fri Jun  4 19:58:30 CDT 2010
//   Use avtStreamlines, not avtStreamlineWrappers.
//
//   Hank Childs, Tue Jun  8 09:30:45 CDT 2010
//   Use virtual methods to reduce dependence on a specific communication
//   pattern.
//
// ****************************************************************************

void
avtParICAlgorithm::SendICs(int dst, vector<avtIntegralCurve*> &ics)
{

    for (int i = 0; i < ics.size(); i++)
    {
        avtIntegralCurve *ic = ics[i];
        ic->PrepareForSend();
    }

    if (DoSendICs(dst, ics))
    {
        for (int i = 0; i < ics.size(); i++)
        {
            avtIntegralCurve *ic = ics[i];
            
            //Add if id/seq is unique. (single streamlines can be sent to multiple dst).
            list<avtIntegralCurve*>::const_iterator si = communicatedICs.begin();
            bool found = false;
            for (si = communicatedICs.begin(); !found && si != communicatedICs.end(); si++)
                found = (*si)->SameCurve(ic);
        
            if (!found)
                communicatedICs.push_back(ic);
        }
        
        //Empty the array.
        ics.resize(0);
    }
}


// ****************************************************************************
//  Method: avtParICAlgorithm::DoSendICs
//
//  Purpose:
//      Send streamlines to a dst.
//
//  Programmer: Dave Pugmire
//  Creation:   September 24, 2009
//
//  Modifications:
//
//   Hank Childs, Sat Feb 20 16:53:18 CST 2010
//   Don't output timing values to the timing logs.
//
//   Hank Childs, Fri Jun  4 19:58:30 CDT 2010
//   Use avtStreamlines, not avtStreamlineWrappers.
//
// ****************************************************************************

bool
avtParICAlgorithm::DoSendICs(int dst, 
                             vector<avtIntegralCurve*> &ics)
{
    if (dst == rank)
        return false;
  
    size_t szz = ics.size();
    if (szz == 0)
        return false;

    int communicationTimer = visitTimer->StartTimer();
    MemStream buff;
    buff.write(&szz, 1);

    for (int i = 0; i < ics.size(); i++)
    {
        avtIntegralCurve *ic = ics[i];
        ic->Serialize(MemStream::WRITE, buff, GetSolver());
        ICCommCnt.value ++;
    }
    
    // Break it up into multiple messages if needed.
    if (buff.buffLen() > icMsgSz)
        EXCEPTION0(ImproperUseException);
    
    // Copy it into a byte buffer.
    size_t sz = buff.buffLen();
    unsigned char *msg = new unsigned char[sz];
    memcpy(msg, buff.buff(), sz);

    //Send it along.
    MPI_Request req;
    int err = MPI_Isend(msg, sz, MPI_UNSIGNED_CHAR, dst,
                        avtParICAlgorithm::STREAMLINE_TAG,
                        VISIT_MPI_COMM, &req);
    debug5<<err<<" = MPI_Isend(msg, "<<sz<<", MPI_UNSIGNED_CHAR, to "<<dst<<", req= "<<req<<endl;
    sendICBufferMap[req] = msg;

    BytesCnt.value += sz;
    bool nov = visitTimer->GetNeverOutputValue();
    visitTimer->NeverOutput(true);
    CommTime.value += visitTimer->StopTimer(communicationTimer,
                                            "SendICs");
    visitTimer->NeverOutput(nov);
    debug5 << "DONE  CheckPendingSendRequests()\n";
    return true;
}

// ****************************************************************************
//  Method: avtParICAlgorithm::RecvICs
//
//  Purpose:
//      Recv streamlines.
//
//  Programmer: Dave Pugmire
//  Creation:   Mon Mar 16 15:45:11 EDT 2009
//
//  Modifications:
//
//  Dave Pugmire, Wed Mar 18 17:17:40 EDT 2009
//  RecvSLs broken into two methods.
//  
//  Dave Pugmire, Mon Mar 23 12:48:12 EDT 2009
//  Change how timings are reported/calculated.
//
//  Hank Childs, Sat Feb 20 16:53:18 CST 2010
//  Don't output timing values to the timing logs.
//
//  Hank Childs, Fri Jun  4 19:58:30 CDT 2010
//  Use avtStreamlines, not avtStreamlineWrappers.
//
//  Hank Childs, Sat Jun  5 16:21:27 CDT 2010
//  Use the PICS filter to instantiate integral curves, as this is now
//  an abstract type.
//
// ****************************************************************************

int
avtParICAlgorithm::RecvICs(list<avtIntegralCurve *> &recvICs,
                           list<int> *ranks)
{
    int communicationTimer = visitTimer->StartTimer();
    int icCount = 0;

    while (true)
    {
        int nReq = icRecvRequests.size();
        MPI_Status *status = new MPI_Status[nReq];
        int *indices = new int[nReq];
        int num = 0, err;

        vector<MPI_Request> copy;
        for (int i = 0; i < icRecvRequests.size(); i++)
            copy.push_back(icRecvRequests[i]);
        err = MPI_Testsome(nReq, &copy[0], &num, indices, status);

        if (num > 0)
        {
            for (int i = 0; i < num; i++)
            {
                int idx = indices[i];
                MPI_Request req = icRecvRequests[idx];
                if (req == MPI_REQUEST_NULL)
                    continue;
                
                //Grab the bytes, unserialize them, add to list.
                unsigned char *msg = recvICBufferMap[req];
                recvICBufferMap.erase(req);
                if (msg == NULL)
                    continue;
        
                MemStream buff(icMsgSz, msg);
                delete [] msg;
                msg = NULL;

                size_t numICs;
                buff.read(numICs);

                for (int j = 0; j < numICs; j++)
                {
                    avtIntegralCurve *ic = picsFilter->CreateIntegralCurve();
                    ic->Serialize(MemStream::READ, buff, GetSolver());
                    recvICs.push_back(ic);
                    icCount++;
                    if (ranks)
                        ranks->push_back(status[i].MPI_SOURCE);
                }
            }

            for (int i = 0; i < num; i++)
                PostRecvICReq(indices[i]);
        }

        delete [] status;
        delete [] indices;
        
        if (num == 0)
            break;
    }
    
    bool nov = visitTimer->GetNeverOutputValue();
    visitTimer->NeverOutput(true);
    CommTime.value += visitTimer->StopTimer(communicationTimer,
                                            "RecvICs");
    visitTimer->NeverOutput(nov);
    debug5 << "DONE  CheckPendingSendRequests()\n";
    return icCount;
}

// ****************************************************************************
//  Method: avtParICAlgorithm::RecvICs
//
//  Purpose:
//      Recv streamlines.
//
//  Programmer: Dave Pugmire
//  Creation:   June 16, 2008
//
//  Modifications:
//
//    Dave Pugmire, Mon Mar 16 15:45:11 EDT 2009
//    Call the other RecvSLs and then check for domain inclusion.
//
//    Dave Pugmire, Tue Mar 17 12:02:10 EDT 2009
//    Use new new RecvSLs method, then check for terminations.
//
//    Hank Childs, Fri Jun  4 03:52:48 PDT 2010
//    Rename GetEndPt to GetCurrentLocation.
//
//    Hank Childs, Fri Jun  4 19:58:30 CDT 2010
//    Use avtStreamlines, not avtStreamlineWrappers.
//
// ****************************************************************************
int
avtParICAlgorithm::RecvICs(list<avtIntegralCurve *> &streamlines,
                           int &earlyTerminations )
{
    list<avtIntegralCurve *> recvICs;
    RecvICs(recvICs);

    earlyTerminations = 0;
    int icCount = 0;
    //Check to see if they in this domain.
    list<avtIntegralCurve *>::iterator s;
    for (s = recvICs.begin(); s != recvICs.end(); ++s)
    {
        avtVector pt;
        (*s)->CurrentLocation(pt);

        if (PointInDomain(pt, (*s)->domain))
        {
            streamlines.push_back(*s);
            icCount++;
        }
        else
        {
            // Point not in domain.
            delete *s;
            earlyTerminations++;
        }
    }

    return icCount;
}


// ****************************************************************************
//  Method: avtParICAlgorithm::ExchangeICs
//
//  Purpose:
//      Exchange streamlines.
//
//  Programmer: Dave Pugmire
//  Creation:   June 16, 2008
//
//  Modifications:
//
//   Dave Pugmire, Thu Dec 18 13:24:23 EST 2008
//   Add early terminations flag.
//
//   Hank Childs, Fri Jun  4 19:58:30 CDT 2010
//   Use avtStreamlines, not avtStreamlineWrappers.
//
// ****************************************************************************

bool
avtParICAlgorithm::ExchangeICs(list<avtIntegralCurve *> &streamlines,
                               vector<vector< avtIntegralCurve *> > &sendICs,
                               int &earlyTerminations )
{
    bool newIntegralCurves = false;
    earlyTerminations = 0;

    // Do the IC sends.
    for (int i = 0; i < nProcs; i++)
    { 
        vector<avtIntegralCurve *> &ic = sendICs[i];
        
        if (i != rank)
            SendICs(i, ic);
        else // Pass them to myself....
        {
            for (int j = 0; j < ic.size(); j++)
                streamlines.push_back(ic[j]);
        }
    }

    // See if there are any recieves....
    int numNewICs = RecvICs(streamlines, earlyTerminations);
    newIntegralCurves = (numNewICs > 0);
    return newIntegralCurves;
}

// ****************************************************************************
//  Method: avtParICAlgorithm::CalculateTimingStatistics
//
//  Purpose:
//      Compute statistics over a value.
//
//  Programmer: Dave Pugmire
//  Creation:   March 23, 2009
//
// ****************************************************************************

void
avtParICAlgorithm::CompileTimingStatistics()
{
    avtICAlgorithm::CompileTimingStatistics();
    ComputeStatistic(CommTime);
}

// ****************************************************************************
//  Method: avtParICAlgorithm::CalculateCounterStatistics
//
//  Purpose:
//      Compute statistics over a value.
//
//  Programmer: Dave Pugmire
//  Creation:   March 23, 2009
//
//
// ****************************************************************************

void
avtParICAlgorithm::CompileCounterStatistics()
{
    avtICAlgorithm::CompileCounterStatistics();
    ComputeStatistic(MsgCnt);
    ComputeStatistic(ICCommCnt);
    ComputeStatistic(BytesCnt);
}

// ****************************************************************************
//  Method: avtParallelICAlgorithm::CalculateExtraTime
//
//  Purpose:
//      Calculate extra time.
//
//  Programmer: Dave Pugmire
//  Creation:   January 28, 2009
//
//  Dave Pugmire, Mon Mar 23 12:48:12 EDT 2009
//  Change how timings are reported/calculated.
//  
// ****************************************************************************

void
avtParICAlgorithm::CalculateExtraTime()
{
    avtICAlgorithm::CalculateExtraTime();
    if (CommTime.value > 0.0)
        ExtraTime.value -= CommTime.value;
}

// ****************************************************************************
//  Method: avtParICAlgorithm::ReportTimings
//
//  Purpose:
//      Print timing data.
//
//  Programmer: Dave Pugmire
//  Creation:   January 28, 2009
//
// ****************************************************************************
void
avtParICAlgorithm::ReportTimings(ostream &os, bool totals)
{
    avtICAlgorithm::ReportTimings(os, totals);

    PrintTiming(os, "CommTime", CommTime, TotalTime, totals);
}


// ****************************************************************************
//  Method: avtParICAlgorithm::ReportCounters
//
//  Purpose:
//      Print timing data.
//
//  Programmer: Dave Pugmire
//  Creation:   January 28, 2009
//
// ****************************************************************************
void
avtParICAlgorithm::ReportCounters(ostream &os, bool totals)
{
    avtICAlgorithm::ReportCounters(os, totals);

    PrintCounter(os, "MsgCount", MsgCnt, totals);
    PrintCounter(os, "ICComCnt", ICCommCnt, totals);
    PrintCounter(os, "ComBytes", BytesCnt, totals);
}

#endif
