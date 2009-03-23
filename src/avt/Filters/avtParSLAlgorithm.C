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
// ****************************************************************************

avtParSLAlgorithm::avtParSLAlgorithm(avtStreamlineFilter *slFilter)
    : avtSLAlgorithm(slFilter),
      CommTime("comT"), MsgCnt("msgC"), SLCommCnt("slcC"), BytesCnt("byteC")
{
    nProcs = PAR_Size();
    rank = PAR_Rank();
    statusMsgSz = 1;
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
// ****************************************************************************

void
avtParSLAlgorithm::Initialize(vector<avtStreamlineWrapper *> &seedPts)
{
    avtSLAlgorithm::Initialize(seedPts);
    InitRequests();
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
// ****************************************************************************

void
avtParSLAlgorithm::InitRequests()
{
    debug5<<"avtParSLAlgorithm::InitRequests()\n";
    statusRecvRequests.resize(nProcs, MPI_REQUEST_NULL);
    slRecvRequests.resize(nProcs, MPI_REQUEST_NULL);

    for (int i = 0; i < nProcs; i++)
    {
        if (i != rank)
        {
            PostRecvStatusReq(i);
            PostRecvSLReq(i);
        }
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
// ****************************************************************************

void
avtParSLAlgorithm::PostRecvStatusReq(int proc)
{
    MPI_Request req;
    int *buff = new int[statusMsgSz];
    MPI_Irecv(buff, statusMsgSz, MPI_INT, proc,
              avtParSLAlgorithm::STATUS_TAG,
              VISIT_MPI_COMM, &req);
    debug5 << "Post Statusrecv from " << proc<<" req= "<<req<<endl;
    statusRecvRequests[proc] = req;
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
// ****************************************************************************

void
avtParSLAlgorithm::PostRecvSLReq(int proc)
{
    MPI_Request req;
    unsigned char *buff = new unsigned char[slMsgSz];
    MPI_Irecv(buff, slMsgSz,
              MPI_UNSIGNED_CHAR, proc,
              avtParSLAlgorithm::STREAMLINE_TAG, 
              VISIT_MPI_COMM, &req);

    debug5 << "Post SLrecv from " << proc<<" req= "<<req<<endl;
    slRecvRequests[proc] = req;
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
    
    MPI_Request req;
    for (int i = 0; i < msg.size(); i++)
        buff[i] = msg[i];
        
    int err = MPI_Isend(buff, statusMsgSz, MPI_INT, dst,
                        avtParSLAlgorithm::STATUS_TAG,
                        VISIT_MPI_COMM, &req);
    debug5<<"Send: "<<dst<<" [";
    for(int i = 0; i < msg.size(); i++) debug5<<buff[i]<<" ";
    debug5<<"] err= "<<err<<endl;

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

                //Copy the sender rank and msg.
                vector<int> msg;
                msg.push_back(idx);
                for (int i = 0; i < statusMsgSz; i++)
                    msg.push_back(buff[i]);
                msgs.push_back(msg);

                debug5<<"msg= [";
                for(int i = 0; i < msg.size(); i++) debug5<<msg[i]<<" ";
                debug5<<"]\n";
                
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
// ****************************************************************************

void
avtParSLAlgorithm::SendSLs(int dst, 
                           vector<avtStreamlineWrapper*> &sls)
{
    if (dst == rank)
        return;
  
    size_t szz = sls.size();
    if (szz == 0)
        return;

    int communicationTimer = visitTimer->StartTimer();
    MemStream buff;
    buff.write(&szz, 1);
    for (int i = 0; i < sls.size(); i++)
    {
        avtStreamlineWrapper *slSeg = sls[i];
        slSeg->numTimesCommunicated++;
        slSeg->Serialize(MemStream::WRITE, buff, GetSolver());
        delete slSeg;
        
        SLCommCnt.value ++;
    }
    
    //Empty the array.
    sls.resize(0);

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
