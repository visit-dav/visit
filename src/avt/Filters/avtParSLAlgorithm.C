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
//                              avtParSLAlgorithm.C                          //
// ************************************************************************* //

#include "avtParSLAlgorithm.h"
#include <TimingsManager.h>

using namespace std;

#ifdef PARALLEL

int avtParSLAlgorithm::STATUS_TAG =  420000;
int avtParSLAlgorithm::STREAMLINE_TAG = 420001;

// ****************************************************************************
//  Method: avtParSLAlgorithm::avtParSLAlgorithm
//
//  Purpose:
//      avtParSLAlgorithm constructor.
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

avtParSLAlgorithm::avtParSLAlgorithm(avtStreamlineFilter *slFilter)
    : avtSLAlgorithm(slFilter),
      CommTime("comT"), MsgCnt("msgC"), SLCommCnt("slcC"), BytesCnt("byteC")
{
    nProcs = PAR_Size();
    rank = PAR_Rank();
    msgID = 0;
    statusMsgSz = -1;
    numAsyncRecvs = -1;
    slMsgSz = 10*1024*1024;
}

// ****************************************************************************
//  Method: avtParSLAlgorithm::~avtParSLAlgorithm
//
//  Purpose:
//      avtParSLAlgorithm destructor.
//
//  Programmer: Dave Pugmire
//  Creation:   January 27, 2009
//
// ****************************************************************************

avtParSLAlgorithm::~avtParSLAlgorithm()
{
}

// ****************************************************************************
//  Method: avtParSLAlgorithm::Initialize
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
// ****************************************************************************

void
avtParSLAlgorithm::Initialize(vector<avtStreamlineWrapper *> &seedPts,
                              int msgSz,
                              int numRecvs)
{
    //Standardmsg + 1(sender rank) +1(msg ID).
    statusMsgSz = msgSz+1+1;
    numAsyncRecvs = numRecvs;
    
    if (statusMsgSz <= 0 || numAsyncRecvs <= 0)
        EXCEPTION0(ImproperUseException);
    
    avtSLAlgorithm::Initialize(seedPts);
    InitRequests();
}


// ****************************************************************************
//  Method: avtParSLAlgorithm::PostRunAlgorithm
//
//  Purpose:
//      Cleanup.
//
//  Programmer: Dave Pugmire
//  Creation:   September 24, 2009
//
// ****************************************************************************

void
avtParSLAlgorithm::PostRunAlgorithm()
{
    ExchangeSLSteps();
}

// ****************************************************************************
//  Method: avtParSLAlgorithm::PostExecute
//
//  Purpose:
//      Cleanup.
//
//  Programmer: Dave Pugmire
//  Creation:   January 27, 2009
//
// ****************************************************************************

void
avtParSLAlgorithm::PostExecute()
{
    CleanupAsynchronous();
    avtSLAlgorithm::PostExecute();
}

// ****************************************************************************
//  Method: avtParSLAlgorithm::InitRequests
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
// ****************************************************************************

void
avtParSLAlgorithm::InitRequests()
{
    debug5<<"avtParSLAlgorithm::InitRequests() sz= "<<numAsyncRecvs<<endl;
    statusRecvRequests.resize(numAsyncRecvs, MPI_REQUEST_NULL);
    slRecvRequests.resize(numAsyncRecvs, MPI_REQUEST_NULL);
    
    for (int i = 0; i < statusRecvRequests.size(); i++)
    {
        PostRecvStatusReq(i);
        PostRecvSLReq(i);
    }
}

static int
CountIDs(list<avtStreamlineWrapper *> &l, int id)
{
    int cnt = 0;
    list<avtStreamlineWrapper*>::const_iterator si = l.begin();
    for (si = l.begin(); si != l.end(); si++)
    {
        if ((*si)->id == id)
            cnt++;
    }
    return cnt;
}

// ****************************************************************************
//  Method: avtParSLAlgorithm::ExchangeSLSteps
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
// ****************************************************************************

void
avtParSLAlgorithm::ExchangeSLSteps()
{
    debug5<<"ExchangeSLSteps: communicatedSLs: "<<communicatedSLs.size();
    debug5<<" terminatedSLs: "<<terminatedSLs.size()<<endl;

    //Communicate to everyone where the terminators are located.
    //Do this "N" streamlines at a time, so we don't have a super big buffer.
    int N;
    if (numSeedPoints > 500)
        N = 500;
    else
        N = numSeedPoints;
    
    long *idBuffer = new long[2*N], *myIDs = new long[2*N];

    //Sort the terminated/communicated SLs by id.
    terminatedSLs.sort(avtStreamlineWrapper::IdSeqCompare);
    communicatedSLs.sort(avtStreamlineWrapper::IdSeqCompare);

    vector<vector<avtStreamlineWrapper *> >sendSLs(N);
    vector<int> owners(N);
    
    int minId = 0;
    int maxId = N-1;
    int nLoops = numSeedPoints/N;
    if (numSeedPoints % N != 0)
        nLoops++;
    for (int l = 0; l < nLoops; l++)
    {
        //Initialize arrays for this round.
        for (int i = 0; i < N; i++)
        {
            idBuffer[i] = 0;
            idBuffer[i+N] = 0;
            myIDs[i] = 0;
            myIDs[i+N] = 0;
            sendSLs[i].resize(0);
            owners[i] = 0;
        }

        //Set array for SLs that terminated here. Update sequence counts for communicated
        //SLs.
        list<avtStreamlineWrapper*>::iterator t = terminatedSLs.begin();
        while (t != terminatedSLs.end() && (*t)->id <= maxId)
        {
            if ((*t)->id >= minId)
            {
                int idx = (*t)->id % N;
                myIDs[idx] = rank;
                myIDs[idx+N] += 1;
                debug5<<"I own id= "<<(*t)->id<<" "<<(*t)->sequenceCnt<<" idx= "<<idx<<endl;
            }

            t++;
        }
        
        list<avtStreamlineWrapper*>::const_iterator c = communicatedSLs.begin();
        while (c != communicatedSLs.end() && (*c)->id <= maxId)
        {
            if ((*c)->id >= minId)
            {
                int idx = (*c)->id % N;
                myIDs[idx+N] += 1;
                debug5<<"I have "<<(*c)->id<<" "<<(*c)->sequenceCnt<<" idx= "<<idx<<endl;
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
        
        //Now we know where all SLs belong and how many sequences for each.
        //Send communicatedSLs to the owners.
        while (!communicatedSLs.empty())
        {
            avtStreamlineWrapper *s = communicatedSLs.front();
            if (s->id <= maxId)
            {
                int idx = s->id%N;
                int owner = idBuffer[idx];
                if (owner == rank)
                    terminatedSLs.push_back(s);
                else
                {
                    s->serializeFlags = avtStreamlineWrapper::SERIALIZE_STEPS; //Write SL steps.
                    sendSLs[idx].push_back(s);
                    owners[idx] = owner;
                }
                communicatedSLs.pop_front();
            }
            else
                break;
        }
        
        for (int i = 0; i < N; i++)
        {
            if (sendSLs[i].size() > 0)
            {
                DoSendSLs(owners[i], sendSLs[i]);

                for (int j = 0; j < sendSLs[i].size(); j++)
                    delete sendSLs[i][j];
            }
        }
        
        //Wait for all the sequences to arrive. The total number is known for
        //each SL, so wait until they all come.
        bool seqGathered = false;
        while (!seqGathered)
        {
            RecvSLs(terminatedSLs);
            
            //See if we have all the sequences we need.
            terminatedSLs.sort(avtStreamlineWrapper::IdSeqCompare);
            bool needMore = false;
            for (int i = 0; i < N && !needMore; i++)
                if (idBuffer[i] == rank)
                    needMore = (CountIDs(terminatedSLs, i+minId) < idBuffer[i+N]);
            
            //Everyone done.
            seqGathered = !needMore;
        }
        
        //Advance to next N streamlines.
        maxId += N;
        minId += N;
    }

    //All SLs are distributed, merge the sequences into single streamlines.
    MergeTerminatedSLSequences();
    
    delete [] idBuffer;
    delete [] myIDs;
}

// ****************************************************************************
//  Method: avtParSLAlgorithm::MergeTerminatedSLSequences
//
//  Purpose:
//      Merge streamline sequences.
//
//  Programmer: Dave Pugmire
//  Creation:   Sept 21, 2009
//
// ****************************************************************************

void
avtParSLAlgorithm::MergeTerminatedSLSequences()
{
    //Sort them by id and sequence so we can process them one at a time.
    terminatedSLs.sort(avtStreamlineWrapper::IdSeqCompare);

    //Split them up into sequences.
    vector<vector<avtStreamlineWrapper *> > seqs;
    while (!terminatedSLs.empty())
    {
        avtStreamlineWrapper *s = terminatedSLs.front();
        terminatedSLs.pop_front();
        
        //Empty or new ID, add a new entry.
        if (seqs.size() == 0 ||
            seqs[seqs.size()-1][0]->id != s->id)
        {
            vector<avtStreamlineWrapper *> v;
            v.push_back(s);
            seqs.push_back(v);
        }
        else
        {
            seqs[seqs.size()-1].push_back(s);
        }
    }
    terminatedSLs.clear();
    
    //Merge the sequences together, put them into terminated list.
    for (int i = 0; i < seqs.size(); i++)
    {
        avtStreamlineWrapper *s = avtStreamlineWrapper::MergeStreamlineSequence(seqs[i]);
        terminatedSLs.push_back(s);
    }
}

// ****************************************************************************
//  Method: avtParSLAlgorithm::CleanupAsynchronous
//
//  Purpose:
//      Claenup the buffers used when doing asynchronous processing.
//
//  Programmer: Dave Pugmire
//  Creation:   June 16, 2008
//
// ****************************************************************************

void
avtParSLAlgorithm::CleanupAsynchronous()
{
    for (int i = 0; i < statusRecvRequests.size(); i++)
    {
        MPI_Request req = statusRecvRequests[i];
        if (req != MPI_REQUEST_NULL)
            MPI_Cancel(&req);
    } 

    for (int i = 0; i < slRecvRequests.size(); i++)
    {
        MPI_Request req = slRecvRequests[i];
        if (req != MPI_REQUEST_NULL)
            MPI_Cancel(&req);
    }

    // Cleanup recv buffers.
    map<MPI_Request, unsigned char*>::const_iterator it;
    for (it = recvSLBufferMap.begin(); it != recvSLBufferMap.end(); ++it)
    {
        char *buff = (char *)it->second;
        if (it->second != NULL)
            delete [] it->second;
    }
    recvSLBufferMap.clear();

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
//  Method: avtParSLAlgorithm::CheckPendingSendRequests
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
// ****************************************************************************
void
avtParSLAlgorithm::CheckPendingSendRequests()
{
    debug5<<"avtParSLAlgorithm::CheckPendingSendRequests()\n";
    int communicationTimer = visitTimer->StartTimer();
    
    if (sendSLBufferMap.size() > 0)
    {
        vector<MPI_Request> req, copy;

        int notCompleted = 0;
        map<MPI_Request, unsigned char*>::const_iterator it;
        for (it = sendSLBufferMap.begin(); it != sendSLBufferMap.end(); ++it)
        {
            if (it->first != MPI_REQUEST_NULL && it->second != NULL)
            {
                req.push_back(it->first);
                copy.push_back(it->first);
            }
            else
                notCompleted++;
        }

        debug5 << "\tCheckPendingSendRequests() SL completed = "<<req.size()
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
                unsigned char *buff = sendSLBufferMap[r];
                debug5 << "\tidx = " << idx << " r = " << r << " buff = " 
                       << (void *)buff << endl;
                if (buff)
                    delete [] buff;

                sendSLBufferMap[r] = NULL;
                sendSLBufferMap.erase(r);
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

    CommTime.value += visitTimer->StopTimer(communicationTimer, 
                                            "CheckPending");
    debug5 << "DONE  CheckPendingSendRequests()\n";
}

// ****************************************************************************
//  Method: avtParSLAlgorithm::PostRecvStatusReq
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
avtParSLAlgorithm::PostRecvStatusReq(int idx)
{
    MPI_Request req;
    int *buff = new int[statusMsgSz];

    MPI_Irecv(buff, statusMsgSz, MPI_INT, MPI_ANY_SOURCE,
              avtParSLAlgorithm::STATUS_TAG,
              VISIT_MPI_COMM, &req);
    debug5 << "Post Statusrecv " <<idx<<" req= "<<req<<endl;
    statusRecvRequests[idx] = req;
    recvIntBufferMap[req] = buff;
}

// ****************************************************************************
//  Method: avtParSLAlgorithm::PostRecvSLReq
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
avtParSLAlgorithm::PostRecvSLReq(int idx)
{
    MPI_Request req;
    unsigned char *buff = new unsigned char[slMsgSz];
    MPI_Irecv(buff, slMsgSz,
              MPI_UNSIGNED_CHAR, MPI_ANY_SOURCE,
              avtParSLAlgorithm::STREAMLINE_TAG, 
              VISIT_MPI_COMM, &req);

    debug5 << "Post SLrecv " <<idx<<" req= "<<req<<endl;
    slRecvRequests[idx] = req;
    recvSLBufferMap[req] = buff;
}


// ****************************************************************************
//  Method: avtParSLAlgorithm::SendMsg
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
// ****************************************************************************

void
avtParSLAlgorithm::SendMsg(int dst,
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
                        avtParSLAlgorithm::STATUS_TAG,
                        VISIT_MPI_COMM, &req);

    sendIntBufferMap[req] = buff;
    
    BytesCnt.value += (sizeof(int) *statusMsgSz);
    MsgCnt.value++;
    CommTime.value += visitTimer->StopTimer(communicationTimer, 
                                            "SendMsg");
}

// ****************************************************************************
//  Method: avtParSLAlgorithm::SendAllMsg
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
avtParSLAlgorithm::SendAllMsg(vector<int> &msg)
{
    for (int i = 0; i < nProcs; i++)
        if (i != rank)
            SendMsg(i, msg);
}

// ****************************************************************************
//  Method: avtParSLAlgorithm::RecvMsgs
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
// ****************************************************************************

void
avtParSLAlgorithm::RecvMsgs(std::vector<std::vector<int> > &msgs)
{
    debug5<<"avtParSLAlgorithm::RecvMsgs()\n";
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
    CommTime.value += visitTimer->StopTimer(communicationTimer,
                                            "RecvMsgs");
}

// ****************************************************************************
//  Method: avtParSLAlgorithm::SendSLs
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
//   Call new method, DoSendSLs.
//
// ****************************************************************************

void
avtParSLAlgorithm::SendSLs(int dst, 
                           vector<avtStreamlineWrapper*> &sls)
{

    for (int i = 0; i < sls.size(); i++)
    {
        avtStreamlineWrapper *slSeg = sls[i];
        slSeg->serializeFlags |= avtStreamlineWrapper::SERIALIZE_INC_SEQ;
    }

    if (DoSendSLs(dst, sls))
    {
        for (int i = 0; i < sls.size(); i++)
        {
            avtStreamlineWrapper *slSeg = sls[i];
            
            //Add if id/seq is unique. (single streamlines can be sent to multiple dst).
            list<avtStreamlineWrapper*>::const_iterator si = communicatedSLs.begin();
            bool found = false;
            for (si = communicatedSLs.begin(); !found && si != communicatedSLs.end(); si++)
                found = ((*si)->id == slSeg->id && (*si)->sequenceCnt == slSeg->sequenceCnt);
        
            if (!found)
                communicatedSLs.push_back(slSeg);
        }
        
        //Empty the array.
        sls.resize(0);
    }
}


// ****************************************************************************
//  Method: avtParSLAlgorithm::DoSendSLs
//
//  Purpose:
//      Send streamlines to a dst.
//
//  Programmer: Dave Pugmire
//  Creation:   September 24, 2009
//
//  Modifications:
//
// ****************************************************************************

bool
avtParSLAlgorithm::DoSendSLs(int dst, 
                             vector<avtStreamlineWrapper*> &sls)
{
    if (dst == rank)
        return false;
  
    size_t szz = sls.size();
    if (szz == 0)
        return false;

    int communicationTimer = visitTimer->StartTimer();
    MemStream buff;
    buff.write(&szz, 1);

    for (int i = 0; i < sls.size(); i++)
    {
        avtStreamlineWrapper *slSeg = sls[i];
        slSeg->Serialize(MemStream::WRITE, buff, GetSolver());
        SLCommCnt.value ++;
    }
    
    // Break it up into multiple messages if needed.
    if (buff.buffLen() > slMsgSz)
        EXCEPTION0(ImproperUseException);
    
    // Copy it into a byte buffer.
    size_t sz = buff.buffLen();
    unsigned char *msg = new unsigned char[sz];
    memcpy(msg, buff.buff(), sz);

    //Send it along.
    MPI_Request req;
    int err = MPI_Isend(msg, sz, MPI_UNSIGNED_CHAR, dst,
                        avtParSLAlgorithm::STREAMLINE_TAG,
                        VISIT_MPI_COMM, &req);
    debug5<<err<<" = MPI_Isend(msg, "<<sz<<", MPI_UNSIGNED_CHAR, to "<<dst<<", req= "<<req<<endl;
    sendSLBufferMap[req] = msg;

    BytesCnt.value += sz;
    CommTime.value += visitTimer->StopTimer(communicationTimer,
                                            "SendSLs");
    return true;
}

// ****************************************************************************
//  Method: avtParSLAlgorithm::RecvSLs
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
// ****************************************************************************

int
avtParSLAlgorithm::RecvSLs(list<avtStreamlineWrapper *> &recvSLs)
{
    int communicationTimer = visitTimer->StartTimer();
    int slCount = 0;

    while (true)
    {
        int nReq = slRecvRequests.size();
        MPI_Status *status = new MPI_Status[nReq];
        int *indices = new int[nReq];
        int num = 0, err;

        vector<MPI_Request> copy;
        for (int i = 0; i < slRecvRequests.size(); i++)
            copy.push_back(slRecvRequests[i]);
        err = MPI_Testsome(nReq, &copy[0], &num, indices, status);

        if (num > 0)
        {
            for (int i = 0; i < num; i++)
            {
                int idx = indices[i];
                MPI_Request req = slRecvRequests[idx];
                if (req == MPI_REQUEST_NULL)
                    continue;
                
                //Grab the bytes, unserialize them, add to list.
                unsigned char *msg = recvSLBufferMap[req];
                recvSLBufferMap.erase(req);
                if (msg == NULL)
                    continue;
        
                MemStream buff(slMsgSz, msg);
                delete [] msg;
                msg = NULL;

                size_t numSLs;
                buff.read(numSLs);

                for (int j = 0; j < numSLs; j++)
                {
                    avtStreamlineWrapper *slSeg = new avtStreamlineWrapper;
                    slSeg->Serialize(MemStream::READ, buff, GetSolver());
                    recvSLs.push_back(slSeg);
                    slCount++;
                }
            }

            for (int i = 0; i < num; i++)
                PostRecvSLReq(indices[i]);
        }

        delete [] status;
        delete [] indices;
        
        if (num == 0)
            break;
    }
    
    CommTime.value += visitTimer->StopTimer(communicationTimer,
                                            "RecvSLs");
    return slCount;
}

// ****************************************************************************
//  Method: avtParSLAlgorithm::RecvSLs
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
// ****************************************************************************
int
avtParSLAlgorithm::RecvSLs(list<avtStreamlineWrapper *> &streamlines,
                           int &earlyTerminations )
{
    list<avtStreamlineWrapper *> recvSLs;
    RecvSLs(recvSLs);

    earlyTerminations = 0;
    int slCount = 0;
    //Check to see if they in this domain.
    list<avtStreamlineWrapper *>::iterator s;
    for (s = recvSLs.begin(); s != recvSLs.end(); ++s)
    {
        avtVector pt;
        (*s)->GetEndPoint(pt);

        if (PointInDomain(pt, (*s)->domain))
        {
            streamlines.push_back(*s);
            slCount++;
        }
        else
        {
            // Point not in domain.
            delete *s;
            earlyTerminations++;
        }
    }

    return slCount;
}


// ****************************************************************************
//  Method: avtParSLAlgorithm::ExchangeSLs
//
//  Purpose:
//      Exchange streamlines.
//
//  Programmer: Dave Pugmire
//  Creation:   June 16, 2008
// Modifications:
//
//   Dave Pugmire, Thu Dec 18 13:24:23 EST 2008
//   Add early terminations flag.
//
// ****************************************************************************

bool
avtParSLAlgorithm::ExchangeSLs(list<avtStreamlineWrapper *> &streamlines,
                               vector<vector< avtStreamlineWrapper *> > &sendSLs,
                               int &earlyTerminations )
{
    bool newStreamlines = false;
    earlyTerminations = 0;

    // Do the SL sends.
    for (int i = 0; i < nProcs; i++)
    { 
        vector<avtStreamlineWrapper *> &sl = sendSLs[i];
        
        if (i != rank)
            SendSLs(i, sl);
        else // Pass them to myself....
        {
            for (int j = 0; j < sl.size(); j++)
                streamlines.push_back(sl[j]);
        }
    }

    // See if there are any recieves....
    int numNewSLs = RecvSLs(streamlines, earlyTerminations);
    newStreamlines = (numNewSLs > 0);
    return newStreamlines;
}

// ****************************************************************************
//  Method: avtParSLAlgorithm::CalculateTimingStatistics
//
//  Purpose:
//      Compute statistics over a value.
//
//  Programmer: Dave Pugmire
//  Creation:   March 23, 2009
//
// ****************************************************************************

void
avtParSLAlgorithm::CompileTimingStatistics()
{
    avtSLAlgorithm::CompileTimingStatistics();
    ComputeStatistic(CommTime);
}

// ****************************************************************************
//  Method: avtParSLAlgorithm::CalculateCounterStatistics
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
avtParSLAlgorithm::CompileCounterStatistics()
{
    avtSLAlgorithm::CompileCounterStatistics();
    ComputeStatistic(MsgCnt);
    ComputeStatistic(SLCommCnt);
    ComputeStatistic(BytesCnt);
}

// ****************************************************************************
//  Method: avtParallelStreamlineFilter::CalculateExtraTime
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
avtParSLAlgorithm::CalculateExtraTime()
{
    avtSLAlgorithm::CalculateExtraTime();
    if (CommTime.value > 0.0)
        ExtraTime.value -= CommTime.value;
}

// ****************************************************************************
//  Method: avtParSLAlgorithm::ReportTimings
//
//  Purpose:
//      Print timing data.
//
//  Programmer: Dave Pugmire
//  Creation:   January 28, 2009
//
// ****************************************************************************
void
avtParSLAlgorithm::ReportTimings(ostream &os, bool totals)
{
    avtSLAlgorithm::ReportTimings(os, totals);

    PrintTiming(os, "CommTime", CommTime, TotalTime, totals);
}


// ****************************************************************************
//  Method: avtParSLAlgorithm::ReportCounters
//
//  Purpose:
//      Print timing data.
//
//  Programmer: Dave Pugmire
//  Creation:   January 28, 2009
//
// ****************************************************************************
void
avtParSLAlgorithm::ReportCounters(ostream &os, bool totals)
{
    avtSLAlgorithm::ReportCounters(os, totals);

    PrintCounter(os, "MsgCount", MsgCnt, totals);
    PrintCounter(os, "SLComCnt", SLCommCnt, totals);
    PrintCounter(os, "ComBytes", BytesCnt, totals);
}

#endif
