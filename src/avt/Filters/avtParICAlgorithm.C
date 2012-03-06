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
//                              avtParICAlgorithm.C                          //
// ************************************************************************* //

#include "avtParICAlgorithm.h"
#include <vtkDataSetWriter.h>
#include <vtkDataSetReader.h>
#include <vtkCharArray.h>

#include <TimingsManager.h>
#include <VisItException.h>

using namespace std;

#ifdef PARALLEL

int avtParICAlgorithm::MESSAGE_TAG =  420000;
int avtParICAlgorithm::STREAMLINE_TAG = 420001;
int avtParICAlgorithm::DATASET_PREP_TAG = 420002;
int avtParICAlgorithm::DATASET_TAG = 420003;

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
//   Dave Pugmire, Mon Dec  6 14:42:45 EST 2010
//   Fixes for SendDS, and RecvDS
//   
// ****************************************************************************

avtParICAlgorithm::avtParICAlgorithm(avtPICSFilter *icFilter)
    : avtICAlgorithm(icFilter),
      CommTime("comT"), MsgCnt("msgC"), ICCommCnt("iccC"), BytesCnt("byteC"), DSCnt("dsC")
{
    nProcs = PAR_Size();
    rank = PAR_Rank();
    msgID = 0;
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
//  Function: IntegralCurveSort
//
//  Purpose:
//      Sorts integral curves.
//
//  Programmer: Hank Childs
//  Creation:   December 6, 2011
//
// ****************************************************************************

bool
IntegralCurveSort(const avtIntegralCurve *ic1, const avtIntegralCurve *ic2)
{
    if (ic1 == NULL)
        return true;
    if (ic2 == NULL)
        return false;
    return ic1->LessThan(ic2);
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
    CleanupRequests();
    avtICAlgorithm::PostExecute();
}

// ****************************************************************************
//  Method: avtParICAlgorithm::InitializeBufers
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
//   Dave Pugmire, Tue Feb  1 09:40:18 EST 2011
//   Compute proper message size.
//
// ****************************************************************************

void
avtParICAlgorithm::InitializeBuffers(vector<avtIntegralCurve *> &seeds,
                                     int mSz,
                                     int nMsgRecvs,
                                     int nICRecvs,
                                     int nDSRecvs)
{
    avtICAlgorithm::Initialize(seeds);

    numMsgRecvs = nMsgRecvs;
    numSLRecvs = nICRecvs;
    numDSRecvs = nDSRecvs;

    // Msgs are handled as vector<int>.
    // Serialization of msg consists: size_t (num elements) +
    // sender rank + message size.
    int msgSize = sizeof(size_t);
    msgSize += sizeof(int); // sender rank.
    msgSize += (mSz * sizeof(int));

    //During particle advection, the IC state is only serialized.
    slSize = 256;
    slsPerRecv = 64;
    
    int dsSize = 2*sizeof(int);
    
    messageTagInfo[avtParICAlgorithm::MESSAGE_TAG] = pair<int,int>(numMsgRecvs, msgSize);
    messageTagInfo[avtParICAlgorithm::STREAMLINE_TAG] = pair<int,int>(numSLRecvs, slSize*slsPerRecv);
    messageTagInfo[avtParICAlgorithm::DATASET_PREP_TAG] = pair<int,int>(numDSRecvs, dsSize);

    //Setup receive buffers.
    map<int, pair<int, int> >::const_iterator it;
    for (it = messageTagInfo.begin(); it != messageTagInfo.end(); it++)
    {
        int tag = it->first, num = it->second.first;
        for (int i = 0; i < num; i++)
            PostRecv(tag);
    }
}

// ****************************************************************************
//  Method: avtParICAlgorithm::CleanupRequests
//
//  Purpose:
//      Claenup the buffers used when doing asynchronous processing.
//
//  Programmer: Dave Pugmire
//  Creation:   June 16, 2008
//
//  Modifications:
//   Dave Pugmire, Mon Nov 29 09:23:01 EST 2010
//   Add optional tag argument to CleanupRequests.
//
//   Dave Pugmire, Tue Feb  1 09:40:18 EST 2011
//   If tag is set, the map was not being cleaned out properly.
//
// ****************************************************************************

void
avtParICAlgorithm::CleanupRequests(int tag)
{
    vector<RequestTagPair> delKeys;
    for (bufferIterator i = recvBuffers.begin(); i != recvBuffers.end(); i++)
    {
        if (tag == -1 || tag == i->first.second)
            delKeys.push_back(i->first);
    }
    
    if (! delKeys.empty())
    {
        vector<RequestTagPair>::const_iterator it;
        for (it = delKeys.begin(); it != delKeys.end(); it++)
        {
            RequestTagPair v = *it;
            
            unsigned char *buff = recvBuffers[v];
            MPI_Cancel(&(v.first));
            delete [] buff;
            recvBuffers.erase(v);
        }
    }
}

// ****************************************************************************
// Method:  avtParICAlgorithm::PostRecv
//
// Purpose:
//   
//
// Programmer:  Dave Pugmire
// Creation:    August 31, 2010
//
// ****************************************************************************

void
avtParICAlgorithm::PostRecv(int tag)
{
    map<int, pair<int, int> >::const_iterator it = messageTagInfo.find(tag);
    if (it != messageTagInfo.end())
        PostRecv(tag, it->second.second);
}

void
avtParICAlgorithm::PostRecv(int tag, int sz, int src)
{
    sz += sizeof(avtParICAlgorithm::Header);
    unsigned char *buff = new unsigned char[sz];
    memset(buff, 0, sz);
    
    MPI_Request req;
    if (src == -1)
        MPI_Irecv(buff, sz, MPI_BYTE, MPI_ANY_SOURCE, tag, VISIT_MPI_COMM, &req);
    else
        MPI_Irecv(buff, sz, MPI_BYTE, src, tag, VISIT_MPI_COMM, &req);
    
    RequestTagPair entry(req, tag);
    recvBuffers[entry] = buff;

    //debug5<<"PostRecv: ("<<req<<", "<<tag<<") buff= "<<(void*)buff<<" sz= "<<sz<<endl;
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
    bufferIterator it;
    vector<MPI_Request> req, copy;
    vector<int> tags;
    
    for (it = sendBuffers.begin(); it != sendBuffers.end(); it++)
    {
        req.push_back(it->first.first);
        copy.push_back(it->first.first);
        tags.push_back(it->first.second);
    }

    if (req.empty())
        return;
    
    //See if any sends are done.
    int num = 0, *indices = new int[req.size()];
    MPI_Status *status = new MPI_Status[req.size()];
    int err = MPI_Testsome(req.size(), &req[0], &num, indices, status);

    for (int i = 0; i < num; i++)
    {
        MPI_Request r = copy[indices[i]];
        int tag = tags[indices[i]];
        
        RequestTagPair k(r,tag);
        bufferIterator entry = sendBuffers.find(k);
        if (entry != sendBuffers.end())
        {
            delete [] entry->second;
            sendBuffers.erase(entry);
        }
    }

    delete [] indices;
    delete [] status;
}

// ****************************************************************************
// Method:  avtParICAlgorithm::PacketCompare
//
// Purpose: Compare packets for sorting.
//   
// Programmer:  Dave Pugmire
// Creation:    September 10, 2010
//
// ****************************************************************************

bool
avtParICAlgorithm::PacketCompare(const unsigned char *a, const unsigned char *b)
{
    avtParICAlgorithm::Header ha, hb;
    memcpy(&ha, a, sizeof(ha));
    memcpy(&hb, b, sizeof(hb));

    return ha.packet < hb.packet;
}

// ****************************************************************************
// Method:  avtParICAlgorithm::PrepareForSend
//
// Purpose: Prepare a MemStream for sending. Adds header, breaks up into pieces
//          if needed.
//   
//
// Programmer:  Dave Pugmire
// Creation:    September 10, 2010
//
// ****************************************************************************

void
avtParICAlgorithm::PrepareForSend(int tag, MemStream *buff, vector<unsigned char *> &buffList)
{
    map<int, pair<int, int> >::const_iterator it = messageTagInfo.find(tag);
    if (it == messageTagInfo.end())
        EXCEPTION0(ImproperUseException);
    
    int bytesLeft = buff->len();
    int maxDataLen = it->second.second;

    avtParICAlgorithm::Header header;
    header.tag = tag;
    header.rank = rank;
    header.id = msgID;
    header.numPackets = 1;
    if (buff->len() > maxDataLen)
        header.numPackets += buff->len() / maxDataLen;
    
    header.packet = 0;
    header.packetSz = 0;
    header.dataSz = 0;
    msgID++;

    buffList.resize(header.numPackets);

    size_t pos = 0;
    for (int i = 0; i < header.numPackets; i++)
    {
        header.packet = i;
        if (i == (header.numPackets-1))
            header.dataSz = bytesLeft;
        else
            header.dataSz = maxDataLen;

        header.packetSz = header.dataSz + sizeof(header);
        unsigned char *b = new unsigned char[header.packetSz];

        //Write the header.
        unsigned char *bPtr = b;
        memcpy(bPtr, &header, sizeof(header));
        bPtr += sizeof(header);

        //Write the data.
        memcpy(bPtr, &buff->data()[pos], header.dataSz);
        pos += header.dataSz;

        buffList[i] = b;
        bytesLeft -= maxDataLen;
    }
}

// ****************************************************************************
// Method: avtParICAlgorithm::SendData 
//
// Purpose: Send data to destination.
//   
//
// Programmer:  Dave Pugmire
// Creation:    September 10, 2010
//
// ****************************************************************************

void
avtParICAlgorithm::SendData(int dst, int tag, MemStream *buff)
{
    vector<unsigned char *> bufferList;

    //Add headers, break into multiple buffers if needed.
    PrepareForSend(tag, buff, bufferList);
    
    avtParICAlgorithm::Header header;
    for (int i = 0; i < bufferList.size(); i++)
    {
        memcpy(&header, bufferList[i], sizeof(header));

        MPI_Request req;
        int err = MPI_Isend(bufferList[i], header.packetSz, MPI_BYTE, dst,
                            tag, VISIT_MPI_COMM, &req);
        BytesCnt.value += header.packetSz;
    
        //Add it to sendBuffers
        RequestTagPair entry(req, tag);
        sendBuffers[entry] = bufferList[i];
    }
    
    delete buff;
}

// ****************************************************************************
// Method: avtParICAlgorithm::RecvData 
//
// Purpose: Receive data from a destination.
//   
//
// Programmer:  Dave Pugmire
// Creation:    September 10, 2010
//
// ****************************************************************************

bool
avtParICAlgorithm::RecvData(set<int> &tags,
                            vector<pair<int, MemStream *> > &buffers,
                            bool blockAndWait)
{
    buffers.resize(0);
    
    //Find all recv of type tag.
    vector<MPI_Request> req, copy;
    vector<int> reqTags;
    for (bufferIterator i = recvBuffers.begin(); i != recvBuffers.end(); i++)
    {
        if (tags.find(i->first.second) != tags.end())
        {
            req.push_back(i->first.first);
            copy.push_back(i->first.first);
            reqTags.push_back(i->first.second);
        }
    }

    if (req.empty())
        return false;

    MPI_Status *status = new MPI_Status[req.size()];
    int *indices = new int[req.size()], num = 0;
    if (blockAndWait)
        MPI_Waitsome(req.size(), &req[0], &num, indices, status);
    else
        MPI_Testsome(req.size(), &req[0], &num, indices, status);

    if (num == 0)
    {
        delete [] status;
        delete [] indices;
        return false;
    }

    vector<unsigned char *> incomingBuffers(num);
    for (int i = 0; i < num; i++)
    {
        RequestTagPair entry(copy[indices[i]], reqTags[indices[i]]);
        bufferIterator it = recvBuffers.find(entry);
        if ( it == recvBuffers.end())
        {
            delete [] status;
            delete [] indices;
            EXCEPTION0(ImproperUseException);
        }

        incomingBuffers[i] = it->second;

        recvBuffers.erase(it);
    }
    
    ProcessReceivedBuffers(incomingBuffers, buffers);
    
    for (int i = 0; i < num; i++)
        PostRecv(reqTags[indices[i]]);
    
    delete [] status;
    delete [] indices;

    return ! buffers.empty();
}

// ****************************************************************************
// Method:  avtParICAlgorithm::ProcessReceivedBuffers
//
// Purpose: 
//   
//
// Programmer:  Dave Pugmire
// Creation:    September 10, 2010
//
// ****************************************************************************

void
avtParICAlgorithm::ProcessReceivedBuffers(vector<unsigned char*> &incomingBuffers,
                                          vector<pair<int, MemStream *> > &buffers)
{
    for (int i = 0; i < incomingBuffers.size(); i++)
    {
        unsigned char *buff = incomingBuffers[i];
        
        //Grab the header.
        avtParICAlgorithm::Header header;
        memcpy(&header, buff, sizeof(header));
        
        //Only 1 packet, strip off header and add to list.
        if (header.numPackets == 1)
        {
            MemStream *b = new MemStream(header.dataSz, (buff + sizeof(header)));
            b->rewind();
            pair<int, MemStream*> entry(header.tag, b);
            buffers.push_back(entry);
            delete [] buff;
        }

        //Multi packet....
        else
        {
            RankIdPair k(header.rank, header.id);
            packetIterator i2 = recvPackets.find(k);
            
            //First packet. Create a new list and add it.
            if (i2 == recvPackets.end())
            {
                list<unsigned char *> l;
                l.push_back(buff);
                recvPackets[k] = l;
            }
            else
            {
                i2->second.push_back(buff);

                // The last packet came in, merge into one MemStream.
                if (i2->second.size() == header.numPackets)
                {
                    //Sort the packets into proper order.
                    i2->second.sort(avtParICAlgorithm::PacketCompare);

                    MemStream *mergedBuff = new MemStream;
                    list<unsigned char *>::iterator listIt;

                    for (listIt = i2->second.begin(); listIt != i2->second.end(); listIt++)
                    {
                        unsigned char *bi = *listIt;

                        avtParICAlgorithm::Header header;
                        memcpy(&header, bi, sizeof(header));
                        mergedBuff->write(&bi[sizeof(header)], header.dataSz);
                        delete [] bi;
                    }

                    mergedBuff->rewind();
                    
                    pair<int, MemStream*> entry(header.tag, mergedBuff);
                    buffers.push_back(entry);
                    recvPackets.erase(i2);
                }
            }
        }
    }
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
avtParICAlgorithm::SendMsg(int dst, vector<int> &msg)
{
    int timerHandle = visitTimer->StartTimer();
    MemStream *buff = new MemStream;
    
    //Write data.
    buff->write(rank);
    buff->write(msg);

    SendData(dst, avtParICAlgorithm::MESSAGE_TAG, buff);
    MsgCnt.value++;
    CommTime.value += visitTimer->StopTimer(timerHandle, "SendMsg");
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
// Method:  avtParICAlgorithm::RecvAny
//
// Purpose: Receive anything.
//   
// Programmer:  Dave Pugmire
// Creation:    January  5, 2011
//
// Modifications:
//
//   Dave Pugmire, Fri Jan  7 14:19:46 EST 2011
//   Use MemStream instead of vtk serialization.
//
// ****************************************************************************

bool
avtParICAlgorithm::RecvAny(vector<MsgCommData> *msgs,
                           list<ICCommData> *recvICs,
                           vector<DSCommData> *ds,
                           bool blockAndWait)
{
    set<int> tags;
    if (msgs)
    {
        tags.insert(avtParICAlgorithm::MESSAGE_TAG);
        msgs->resize(0);
    }
    if (recvICs)
    {
        tags.insert(avtParICAlgorithm::STREAMLINE_TAG);
        recvICs->resize(0);
    }
    if (ds)
    {
        tags.insert(avtParICAlgorithm::DATASET_TAG);
        tags.insert(avtParICAlgorithm::DATASET_PREP_TAG);
        ds->resize(0);
    }
    
    if (tags.empty())
        return false;
    
    vector<pair<int, MemStream *> > buffers;
    if (! RecvData(tags, buffers, blockAndWait))
        return false;

    int timerHandle = visitTimer->StartTimer();

    for (int i = 0; i < buffers.size(); i++)
    {
        if (buffers[i].first == avtParICAlgorithm::MESSAGE_TAG)
        {
            int sendRank;
            vector<int> m;
            buffers[i].second->read(sendRank);
            buffers[i].second->read(m);
            MsgCommData msg(sendRank, m);

            msgs->push_back(msg);
        }
        else if (buffers[i].first == avtParICAlgorithm::STREAMLINE_TAG)
        {
            int num, sendRank;
            buffers[i].second->read(sendRank);
            buffers[i].second->read(num);
            for (int j = 0; j < num; j++)
            {
                avtIntegralCurve *ic = picsFilter->CreateIntegralCurve();
                ic->Serialize(MemStream::READ, *(buffers[i].second), GetSolver());
                ICCommData d(sendRank, ic);
                recvICs->push_back(d);
            }
        }
        else if (buffers[i].first == avtParICAlgorithm::DATASET_TAG)
        {
            DomainType dom;
            int dsLen;
            buffers[i].second->read(dom);

            vtkDataSet *d;
            buffers[i].second->read(&d);
            DSCommData dsData(dom, d);
            ds->push_back(dsData);
        }
        else if (buffers[i].first == avtParICAlgorithm::DATASET_PREP_TAG)
        {
            int sendRank, dsLen;
            buffers[i].second->read(sendRank);
            buffers[i].second->read(dsLen);

            PostRecv(avtParICAlgorithm::DATASET_TAG, dsLen);
        }

        delete buffers[i].second;
    }
    
    CommTime.value += visitTimer->StopTimer(timerHandle, "RecvAny");
    return true;
}

// ****************************************************************************
// Method:  avtParICAlgorithm::RecvMsg
//
// Purpose: Check for incoming messages.
//   
// Programmer:  Dave Pugmire
// Creation:    September 10, 2010
//
// Modifications:
//
//   Dave Pugmire, Wed Jan  5 07:57:21 EST 2011
//   New datastructures for msg/ic/ds.
//
// ****************************************************************************

bool
avtParICAlgorithm::RecvMsg(vector<MsgCommData> &msgs)
{
    return RecvAny(&msgs, NULL, NULL, false);
}

// ****************************************************************************
// Method:  avtParICAlgorithm::SendICs
//
// Purpose: Send ICs to a destination.
//   
// Programmer:  Dave Pugmire
// Creation:    September 10, 2010
//
// ****************************************************************************

void
avtParICAlgorithm::SendICs(int dst, vector<avtIntegralCurve*> &ics)
{
    int timerHandle = visitTimer->StartTimer();
    for (int i = 0; i < ics.size(); i++)
        ics[i]->PrepareForSend();

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
    }
    
    ICCommCnt.value += ics.size();
    ics.resize(0);
    CommTime.value += visitTimer->StopTimer(timerHandle, "SendICs");
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
//   Dave Pugmire, Fri Nov  5 15:39:58 EDT 2010
//   Fix for unstructured meshes. Need to account for particles that are sent to domains
//   that based on bounding box, and the particle does not lay in any cells.
//
//   Dave Pugmire, Wed Jan  5 07:57:21 EST 2011
//   New datastructures for msg/ic/ds.
//
// ****************************************************************************

bool
avtParICAlgorithm::RecvICs(list<ICCommData> &recvICs)
{
    return RecvAny(NULL, &recvICs, NULL, false);
}

// ****************************************************************************
// Method:  avtParICAlgorithm::RecvICs
//
// Purpose: Recv Ics.
//   
// Programmer:  Dave Pugmire
// Creation:    January  5, 2011
//
// ****************************************************************************

bool
avtParICAlgorithm::RecvICs(list<avtIntegralCurve *> &recvICs)
{
    list<ICCommData> incoming;
    bool val = RecvICs(incoming);
    if (val)
    {
        list<ICCommData>::iterator it;
        for (it = incoming.begin(); it != incoming.end(); it++)
            recvICs.push_back((*it).ic);
    }
    
    return val;
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
//   Dave Pugmire, Fri Nov  5 15:39:58 EDT 2010
//   Fix for unstructured meshes. Need to account for particles that are sent to domains
//   that based on bounding box, and the particle does not lay in any cells.
//
// ****************************************************************************

bool
avtParICAlgorithm::DoSendICs(int dst, 
                             vector<avtIntegralCurve*> &ics)
{
    if (dst == rank || ics.empty())
        return false;

    MemStream *buff = new MemStream;
    
    buff->write(rank);
    int num = ics.size();
    buff->write(num);
    
    for ( int i = 0; i < ics.size(); i++)
        ics[i]->Serialize(MemStream::WRITE, *buff, GetSolver());
    
    SendData(dst, avtParICAlgorithm::STREAMLINE_TAG, buff);
    
    return true;
}

// ****************************************************************************
// Method:  avtParICAlgorithm::SendDS
//
// Purpose: Send a vtk dataset.
//   
// Programmer:  Dave Pugmire
// Creation:    September 10, 2010
//
// Modifications:
//
//   Dave Pugmire, Mon Dec  6 14:42:45 EST 2010
//   Fixes for SendDS, and RecvDS
//
//   Dave Pugmire, Fri Jan  7 14:19:46 EST 2011
//   Use MemStream instead of vtk serialization.
//
// ****************************************************************************

void
avtParICAlgorithm::SendDS(int dst, vector<vtkDataSet *> &ds, vector<DomainType> &doms)
{
    int timerHandle = visitTimer->StartTimer();

    //Serialize the data sets.
    for (int i = 0; i < ds.size(); i++)
    {
        MemStream *dsBuff = new MemStream;

        dsBuff->write(doms[i]);
        dsBuff->write(ds[i]);
        int dsLen = dsBuff->len();
        int totalLen = dsBuff->len();
        
        MemStream *msgBuff = new MemStream(2*sizeof(int));
        msgBuff->write(rank);
        msgBuff->write(totalLen);
        SendData(dst, avtParICAlgorithm::DATASET_PREP_TAG, msgBuff);
        MsgCnt.value++;

        //Send dataset.
        messageTagInfo[avtParICAlgorithm::DATASET_TAG] = pair<int,int>(1, totalLen+sizeof(avtParICAlgorithm::Header));
        SendData(dst, avtParICAlgorithm::DATASET_TAG, dsBuff);
        messageTagInfo.erase(messageTagInfo.find(avtParICAlgorithm::DATASET_TAG));

        DSCnt.value++;
    }
    CommTime.value += visitTimer->StopTimer(timerHandle, "SendDS");
}

// ****************************************************************************
// Method:  avtParICAlgorithm::RecvDS
//
// Purpose: Check for incoming vtk datasets.
//
// Programmer:  Dave Pugmire
// Creation:    September 10, 2010
//
// Modifications:
//
//   Dave Pugmire, Mon Dec  6 14:42:45 EST 2010
//   Fixes for SendDS, and RecvDS
//
//   Dave Pugmire, Fri Dec 17 12:15:04 EST 2010
//   Use vtkCharArray to be more memory efficient.
//
//   Dave Pugmire, Wed Jan  5 07:57:21 EST 2011
//   New datastructures for msg/ic/ds.
//
// ****************************************************************************

bool
avtParICAlgorithm::RecvDS(vector<DSCommData> &ds)
{
    return RecvAny(NULL, NULL, &ds, false);
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
//   Dave Pugmire, Fri Jan 14 11:07:41 EST 2011
//   Added a new communication pattern, RestoreSequenceAssembleUniformly and
//   renamed RestoreIntegralCurveSequence to RestoreIntegralCurveSequenceAssembleOnCurrentProcessor
//
//   Dave Pugmire, Thu Sep  1 07:44:48 EDT 2011
//   Implement RestoreIntegralCurveToOriginatingProcessor.
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
 
    if (pattern == avtPICSFilter::RestoreSequenceAssembleOnCurrentProcessor)
        RestoreIntegralCurveSequenceAssembleOnCurrentProcessor();
    else if (pattern == avtPICSFilter::RestoreSequenceAssembleUniformly)
        RestoreIntegralCurveSequenceAssembleUniformly();
    else if (pattern == avtPICSFilter::LeaveOnCurrentProcessor)
        ;
    else if (pattern == avtPICSFilter::ReturnToOriginatingProcessor)
        RestoreIntegralCurveToOriginatingProcessor();
    else
    { 
        EXCEPTION1(VisItException, "Undefined communication pattern");
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
//  Method: avtParICAlgorithm::RestoreIntegralCurveSequenceAssembleOnCurrentProcessor
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
//   Dave Pugmire, Mon Nov 29 09:23:01 EST 2010
//   Cleanup only the STREAMLINE_TAG requests.
//
//   Dave Pugmire, Fri Jan 14 11:07:41 EST 2011
//   Renamed RestoreIntegralCurveSequence to RestoreIntegralCurveSequenceAssembleOnCurrentProcessor
//
//   Hank Childs, Tue Dec  6 19:04:16 PST 2011
//   Remove dependence on avtStateRecorderIntegralCurve.
//
// ****************************************************************************

void
avtParICAlgorithm::RestoreIntegralCurveSequenceAssembleOnCurrentProcessor()
{
    if (DebugStream::Level5())
        debug5<<"RestoreIntegralCurveSequence: communicatedICs: "
              <<communicatedICs.size()
              <<" terminatedICs: "<<terminatedICs.size()<<endl;

    //Create larger streamline buffers.

    CleanupRequests(avtParICAlgorithm::STREAMLINE_TAG);
    messageTagInfo[avtParICAlgorithm::STREAMLINE_TAG] = pair<int,int>(numSLRecvs, 512*1024);

    for (int i = 0; i < numSLRecvs; i++)
        PostRecv(avtParICAlgorithm::STREAMLINE_TAG);

    //Communicate to everyone where the terminators are located.
    //Do this "N" streamlines at a time, so we don't have a super big buffer.
    int N;
    if (numSeedPoints > 500)
        N = 500;
    else
        N = numSeedPoints;
    
    long *idBuffer = new long[2*N], *myIDs = new long[2*N];

    //Sort the terminated/communicated ICs by id.
    terminatedICs.sort(IntegralCurveSort);
    communicatedICs.sort(IntegralCurveSort);

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
        list<avtIntegralCurve*>::iterator t = terminatedICs.begin(); //MOVE TO front of loop!
        while (t != terminatedICs.end() && (*t)->id <= maxId)
        {
            if ((*t)->id >= minId)
            {
                int idx = (*t)->id % N;
                myIDs[idx] = rank;
                myIDs[idx+N] += 1;
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
            }
            c++;
        }

        //Exchange ID owners and sequence counts.
        MPI_Allreduce(myIDs, idBuffer, 2*N, MPI_LONG, MPI_SUM, VISIT_MPI_COMM);
        if (0) //(DebugStream::Level5())
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
                    s->PrepareForFinalCommunication();
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
            terminatedICs.sort(IntegralCurveSort);
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
        CheckPendingSendRequests();
    }

    //All ICs are distributed, merge the sequences into single streamlines.
    MergeTerminatedICSequences();
    
    delete [] idBuffer;
    delete [] myIDs;
}

// ****************************************************************************
// Method:  avtParICAlgorithm::RestoreIntegralCurveSequenceAssembleUniformly
//
// Purpose: Communicate streamlines pieces to destinations.
//      When a streamline is communicated, only the state information is sent.
//      All the integration steps need to resassmbled. This method assigns curves
//      curves uniformly across all procs, and assembles the pieces.
//
// Programmer:  Dave Pugmire
// Creation:    January 14, 2011
//
// ****************************************************************************

void
avtParICAlgorithm::RestoreIntegralCurveSequenceAssembleUniformly()
{
    RestoreIntegralCurve(true);
}

// ****************************************************************************
// Method:  avtParICAlgorithm::RestoreIntegralCurveToOriginatingProcessor
//
// Purpose: Communicate streamlines pieces to destinations.
//      When a streamline is communicated, only the state information is sent.
//      All the integration steps need to resassmbled. This method assigns curves
//      curves uniformly across all procs, and assembles the pieces.
//
// Programmer:  Dave Pugmire
// Creation:    Mon Aug 29 15:59:30 EDT 2011
//
// ****************************************************************************

void
avtParICAlgorithm::RestoreIntegralCurveToOriginatingProcessor()
{
    RestoreIntegralCurve(false);
}

// ****************************************************************************
// Method:  avtParICAlgorithm::RestoreIntegralCurve
//
// Purpose:
//   
//
// Arguments:
//   
//
// Programmer:  Dave Pugmire
// Creation:    August 29, 2011
//
// Modifications:
//
//   Hank Childs, Tue Dec  6 19:04:16 PST 2011
//   Add fix from David Pugmire when MaxID is different on different
//   processors.
//
//   Hank Childs, Tue Dec  6 19:04:16 PST 2011
//   Remove dependence on avtStateRecorderIntegralCurve.
//
// ****************************************************************************

void
avtParICAlgorithm::RestoreIntegralCurve(bool uniformlyDistrubute)
{
    if (DebugStream::Level5())
        debug5<<"RestoreIntegralCurveSequenceAssembleUniformly: communicatedICs: "
          <<communicatedICs.size()
          <<" terminatedICs: "<<terminatedICs.size()<<endl;

    //Create larger streamline buffers.
    CleanupRequests(avtParICAlgorithm::STREAMLINE_TAG);
    messageTagInfo[avtParICAlgorithm::STREAMLINE_TAG] = pair<int,int>(numSLRecvs, 512*1024);

    for (int i = 0; i < numSLRecvs; i++)
        PostRecv(avtParICAlgorithm::STREAMLINE_TAG);

    //Stuff all ICs into one list, and sort.
    std::list<avtIntegralCurve *> allICs;
    allICs.insert(allICs.end(), terminatedICs.begin(), terminatedICs.end());
    allICs.insert(allICs.end(), communicatedICs.begin(), communicatedICs.end());
    allICs.sort(IntegralCurveSort);

    int myMaxID = -1;
    if (!allICs.empty())
        myMaxID = (allICs.back())->id+1;
    numSeedPoints = UnifyMaximumValue(myMaxID);

    if (numSeedPoints < 0)
        numSeedPoints = 0;
       
    terminatedICs.clear();
    communicatedICs.clear();

    //Communicate to everyone where the pieces are located.
    //Do this "N" streamlines at a time, so we don't have a super big buffer.
    int N;
    if (numSeedPoints > 500)
        N = 500;
    else
        N = numSeedPoints;
    
    long *idBuffer = new long[N], *myIDs = new long[N];
    int *sendList, *mySendList;
    if (!uniformlyDistrubute)
    {
        sendList = new int[nProcs];
        mySendList = new int[nProcs];
    }

    int minId = 0;
    int maxId = N-1;
    int nLoops = 0;

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
            myIDs[i] = 0;
        }
        
        if (!uniformlyDistrubute)
        {
            for (int i = 0; i < nProcs; i++)
            {
                sendList[i] = 0;
                mySendList[i] = 0;
            }
        }

        //Count ICs by id (could have multiple IDs).
        list<avtIntegralCurve*>::iterator it = allICs.begin();
        while (it != allICs.end() && (*it)->id <= maxId)
        {
            avtIntegralCurve *ic = *it;
            if (ic->id >= minId)
            {
                int idx = ic->id % N;
                myIDs[idx] ++;
                if (!uniformlyDistrubute && ic->originatingRank != rank)
                    mySendList[ic->originatingRank] ++;
            }
            it++;
        }
        //if (DebugStream::Level1())
        //{
            //debug1<<"mySendList= [";
            //for(int i=0;i<nProcs;i++) debug1<<mySendList[i]<<" ";
            //debug1<<"]"<<endl;
        //}

        //Exchange ID owners and sequence counts.
        MPI_Allreduce(myIDs, idBuffer, N, MPI_LONG, MPI_SUM, VISIT_MPI_COMM);
        if (!uniformlyDistrubute)
        {
            MPI_Allreduce(mySendList, sendList, nProcs, MPI_INT, MPI_SUM, VISIT_MPI_COMM);

            //if (DebugStream::Level1())
            //{
                //debug1<<"sendList= [";
                //for(int i=0;i<nProcs;i++) debug1<<sendList[i]<<" ";
                //debug1<<"]"<<endl;
            //}
        }
        
        //Now we know where all ICs belong and how many sequences for each.
        //Send communicatedICs to the owners.
        map<int, vector<avtIntegralCurve *> > sendICs;
        int numSeqAlreadyHere = 0;

        while (!allICs.empty())
        {
            avtIntegralCurve *s = allICs.front();
            if (s->id > maxId)
                break;
            allICs.pop_front();
            
            int owner = -1;
            
            if (uniformlyDistrubute)
                owner = s->id % nProcs;
            else
                owner = s->originatingRank;
            
            //IC is mine.
            if (owner == rank)
            {
                terminatedICs.push_back(s);
                numSeqAlreadyHere++;
            }
            else
            {
                s->PrepareForFinalCommunication();

                map<int, vector<avtIntegralCurve *> >::iterator it;
                it = sendICs.find(owner);
                if (it == sendICs.end())
                {
                    vector<avtIntegralCurve*> v(1);
                    v[0] = s;
                    sendICs[owner] = v;
                }
                else
                {
                    it->second.push_back(s);
                }
            }
        }

        //Send all the ICs.
        map<int, vector<avtIntegralCurve *> >::iterator s_it;
        for (s_it = sendICs.begin(); s_it != sendICs.end(); s_it++)
        {
            if (s_it->second.size() > 0)
            {
                //debug1<<"SendIC : "<<s_it->second[0]->id<<" to "<<s_it->first<<" num= "<<s_it->second.size()<<endl;
                DoSendICs(s_it->first, s_it->second);

                for (int i = 0; i < s_it->second.size(); i++)
                    delete s_it->second[i];
            }
        }
        sendICs.clear();

        //Wait for all the sequences to arrive. The total number is known for
        //each IC, so wait until they all come.
        int numICsToBeRecvd = 0;
        if (uniformlyDistrubute)
        {
            for (int i = minId; i <= maxId; i++)
            {
                if (i % nProcs == rank)
                    numICsToBeRecvd += idBuffer[i%N];
            }
            numICsToBeRecvd -= numSeqAlreadyHere;
        }
        else
        {
            numICsToBeRecvd += sendList[rank];
        }
        //debug1<<"numICsToBeRecvd= "<<numICsToBeRecvd<<endl;

        while (numICsToBeRecvd > 0)
        {
            list<ICCommData> ICs;
            RecvAny(NULL, &ICs, NULL, true);
            list<ICCommData>::iterator it;
            for (it = ICs.begin(); it != ICs.end(); it++)            
            {
                terminatedICs.push_back((*it).ic);
                numICsToBeRecvd--;
            }
            
            CheckPendingSendRequests();
        }
        
        //Advance to next N curves.
        maxId += N;
        minId += N;
        CheckPendingSendRequests();
    }

    //All ICs are distributed, merge the sequences into single curves.
    MergeTerminatedICSequences();

    delete [] idBuffer;
    delete [] myIDs;
    if (!uniformlyDistrubute)
    {
        delete [] sendList;
        delete [] mySendList;
    }
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
//   David Camp, Mon Aug 15 13:43:24 PDT 2011
//   Changed the code to allow the avtIntegralCurve to merge the IC sequence.
//
//   Hank Childs, Tue Dec  6 19:04:16 PST 2011
//   Remove dependence on avtStateRecorderIntegralCurve.
//
// ****************************************************************************

void
avtParICAlgorithm::MergeTerminatedICSequences()
{
    // Sort them by id and sequence so we can process them one at a time.
    terminatedICs.sort(IntegralCurveSort);

    // Split them up into sequences.
    vector<vector<avtIntegralCurve *> > seqs;
    while (!terminatedICs.empty())
    {
        avtIntegralCurve *s = terminatedICs.front();
        terminatedICs.pop_front();
        
        // Empty or new ID, add a new entry.
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
    
    // Merge the sequences together, put them into terminated list.
    for (int i = 0; i < seqs.size(); i++)
    {
        avtIntegralCurve *s = seqs[i][0]->MergeIntegralCurveSequence(seqs[i]);
        terminatedICs.push_back(s);
    }
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
    ComputeStatistic(DSCnt);
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
    PrintCounter(os, "DSCommCnt", DSCnt, totals);
    PrintCounter(os, "ComBytes", BytesCnt, totals);
}

#endif
