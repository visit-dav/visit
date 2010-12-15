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
#include <vtkDataSetWriter.h>
#include <vtkDataSetReader.h>

#include <avtStateRecorderIntegralCurve.h>

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
// ****************************************************************************

void
avtParICAlgorithm::InitializeBuffers(vector<avtIntegralCurve *> &seeds,
                                     int msgSize,
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
    msgSize = sizeof(size_t);
    msgSize += sizeof(int); // sender rank.
    msgSize += msgSize * sizeof(int);

    //During particle advection, the IC state is only serialized.
    slSize = 256;
    slsPerRecv = 64;
    
    int dsSize = 2*sizeof(int);
    
    messageTagInfo[avtParICAlgorithm::MESSAGE_TAG] = std::pair<int,int>(numMsgRecvs, msgSize);
    messageTagInfo[avtParICAlgorithm::STREAMLINE_TAG] = std::pair<int,int>(numSLRecvs, slSize*slsPerRecv);
    messageTagInfo[avtParICAlgorithm::DATASET_PREP_TAG] = std::pair<int,int>(numDSRecvs, dsSize);

    //Setup receive buffers.
    std::map<int, std::pair<int, int> >::const_iterator it;
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
// ****************************************************************************

void
avtParICAlgorithm::CleanupRequests(int tag)
{
    for (bufferIterator i = recvBuffers.begin(); i != recvBuffers.end(); i++)
    {
        if (tag != -1 && tag != i->first.second)
            continue;
        
        MPI_Request r = i->first.first;
        if (r != MPI_REQUEST_NULL)
            MPI_Cancel(&r);
        if (i->second != NULL)
            delete [] i->second;
        
        recvBuffers.erase(i);
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
avtParICAlgorithm::PostRecv(int tag, int sz)
{
    sz += sizeof(avtParICAlgorithm::Header);
    unsigned char *buff = new unsigned char[sz];
    memset(buff, 0, sz);
    
    MPI_Request req;
    MPI_Irecv(buff, sz, MPI_BYTE, MPI_ANY_SOURCE, tag, VISIT_MPI_COMM, &req);
    
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
avtParICAlgorithm::PrepareForSend(int tag, MemStream *buff, std::vector<unsigned char *> &buffList)
{
    std::map<int, std::pair<int, int> >::const_iterator it = messageTagInfo.find(tag);
    if (it == messageTagInfo.end())
        EXCEPTION0(ImproperUseException);
    
    int bytesLeft = buff->len();
    int maxDataLen = it->second.second;

    avtParICAlgorithm::Header header;
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
avtParICAlgorithm::RecvData(int tag, vector<MemStream *> &buffers)
{
    buffers.resize(0);
    
    //Find all recv of type tag.
    vector<MPI_Request> req, copy;
    for (bufferIterator i = recvBuffers.begin(); i != recvBuffers.end(); i++)
    {
        if (i->first.second == tag)
        {
            req.push_back(i->first.first);
            copy.push_back(i->first.first);
        }
    }

    if (req.empty())
        return false;

    MPI_Status *status = new MPI_Status[req.size()];
    int *indices = new int[req.size()], num = 0;
    MPI_Testsome(req.size(), &req[0], &num, indices, status);

    if (num == 0)
        return false;

    vector<unsigned char *> incomingBuffers(num);
    for (int i = 0; i < num; i++)
    {
        RequestTagPair entry(copy[indices[i]], tag);
        bufferIterator it = recvBuffers.find(entry);
        if ( it == recvBuffers.end())
            EXCEPTION0(ImproperUseException);

        incomingBuffers[i] = it->second;

        recvBuffers.erase(it);
    }

    ProcessReceivedBuffers(tag, incomingBuffers, buffers);
    
    for (int i = 0; i < num; i++)
        PostRecv(tag);

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
avtParICAlgorithm::ProcessReceivedBuffers(int tag,
                                          vector<unsigned char*> &incomingBuffers,
                                          vector<MemStream *> &buffers)
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
            buffers.push_back(b);
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
                    buffers.push_back(mergedBuff);
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
    MemStream *buff = new MemStream;
    
    //Write data.
    buff->write(rank);
    buff->write(msg);

    SendData(dst, avtParICAlgorithm::MESSAGE_TAG, buff);
    MsgCnt.value++;
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
// Method:  avtParICAlgorithm::RecvMsg
//
// Purpose: Check for incoming messages.
//   
// Programmer:  Dave Pugmire
// Creation:    September 10, 2010
//
// ****************************************************************************

bool
avtParICAlgorithm::RecvMsg(std::vector<std::vector<int> > &msgs)
{
    msgs.resize(0);

    vector<MemStream *> buffers;
    while (RecvData(avtParICAlgorithm::MESSAGE_TAG, buffers))
    {
        for (int i = 0; i < buffers.size(); i++)
        {
            int sendRank;
            vector<int> m;
            buffers[i]->read(sendRank);
            buffers[i]->read(m);

            vector<int> msg(1);
            msg[0] = sendRank;
            msg.insert(msg.end(), m.begin(), m.end());

            msgs.push_back(msg);
            delete buffers[i];
        }
    }

    return ! msgs.empty();
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
avtParICAlgorithm::SendICs(int dst, std::vector<avtIntegralCurve*> &ics)
{
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
// ****************************************************************************

bool
avtParICAlgorithm::RecvICs(list<avtIntegralCurve*> &recvICs,
                           list<int> *ranks)
{
    vector<MemStream *> buffers;
    while (RecvData(avtParICAlgorithm::STREAMLINE_TAG, buffers))
    {
        for (int i = 0; i < buffers.size(); i++)
        {
            int num, sendRank;
            buffers[i]->read(sendRank);
            buffers[i]->read(num);
            for (int j = 0; j < num; j++)
            {
                avtIntegralCurve *ic = picsFilter->CreateIntegralCurve();
                ic->Serialize(MemStream::READ, *buffers[i], GetSolver());
                recvICs.push_back(ic);
                if (ranks)
                    ranks->push_back(sendRank);
            }
            delete buffers[i];
        }
    }

    return ! recvICs.empty();
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
// ****************************************************************************

void
avtParICAlgorithm::SendDS(int dst, std::vector<vtkDataSet *> &ds, std::vector<DomainType> &doms)
{
    //Serialize the data sets.
    for (int i = 0; i < ds.size(); i++)
    {
        vtkDataSetWriter *writer = vtkDataSetWriter::New();
        writer->WriteToOutputStringOn();
        writer->SetFileTypeToBinary();
        writer->SetInput(ds[i]);
        writer->Write();
        int dsLen = writer->GetOutputStringLength();
        int totalLen = dsLen + sizeof(DomainType) + sizeof(dsLen);
        
        MemStream *buff0 = new MemStream(2*sizeof(int));
        buff0->write(rank);
        buff0->write(totalLen);
        SendData(dst, avtParICAlgorithm::DATASET_PREP_TAG, buff0);

        MemStream *buff1 = new MemStream(totalLen);
        buff1->write(doms[i]);
        buff1->write(dsLen);
        buff1->write(writer->GetBinaryOutputString(), dsLen);
        messageTagInfo[avtParICAlgorithm::DATASET_TAG] = std::pair<int,int>(1, totalLen+sizeof(avtParICAlgorithm::Header));
        SendData(dst, avtParICAlgorithm::DATASET_TAG, buff1);
        messageTagInfo.erase(messageTagInfo.find(avtParICAlgorithm::DATASET_TAG));
        writer->Delete();

        DSCnt.value++;
    }
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
// ****************************************************************************

bool
avtParICAlgorithm::RecvDS(std::vector<vtkDataSet *> &ds, std::vector<DomainType> &doms)
{
    ds.resize(0);
    vector<MemStream *> buffers;

    while (RecvData(avtParICAlgorithm::DATASET_TAG, buffers))
    {
        for (int i = 0; i < buffers.size(); i++)
        {
            DomainType dom;
            int dsLen;
            buffers[i]->read(dom);
            buffers[i]->read(dsLen);
            doms.push_back(dom);
            
            vtkDataSetReader *reader = vtkDataSetReader::New();
            reader->ReadFromInputStringOn();

            const char *data = (const char *)&buffers[i]->data()[buffers[i]->pos()];
            reader->SetBinaryInputString(data, dsLen);
            reader->Update();
            vtkDataSet *d = reader->GetOutput();
            d->Register(NULL);
            reader->Delete();
            ds.push_back(d);
            delete buffers[i];
        }
    }
    
    while (RecvData(avtParICAlgorithm::DATASET_PREP_TAG, buffers))
    {
        for (int i = 0; i < buffers.size(); i++)
        {
            int sendRank, dsLen;
            buffers[i]->read(sendRank);
            buffers[i]->read(dsLen);

            PostRecv(avtParICAlgorithm::DATASET_TAG, dsLen);
            delete buffers[i];
        }
    }

    return ! ds.empty();
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
//   Dave Pugmire, Mon Nov 29 09:23:01 EST 2010
//   Cleanup only the STREAMLINE_TAG requests.
//
// ****************************************************************************

void
avtParICAlgorithm::RestoreIntegralCurveSequence()
{
    debug5<<"RestoreIntegralCurveSequence: communicatedICs: "
          <<communicatedICs.size()
          <<" terminatedICs: "<<terminatedICs.size()<<endl;

    //Create larger streamline buffers.
    CleanupRequests(avtParICAlgorithm::STREAMLINE_TAG);
    messageTagInfo[avtParICAlgorithm::STREAMLINE_TAG] = std::pair<int,int>(numSLRecvs, 512*1024);
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
        list<avtIntegralCurve*>::iterator t = terminatedICs.begin(); //MOVE TO front of loop!
        while (t != terminatedICs.end() && (*t)->id <= maxId)
        {
            if ((*t)->id >= minId)
            {
                int idx = (*t)->id % N;
                myIDs[idx] = rank;
                myIDs[idx+N] += 1;
                //debug5<<"I own id= "<<(*t)->id<<" "<<(((avtStateRecorderIntegralCurve *)*t))->sequenceCnt<<" idx= "<<idx<<endl;
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
                //debug5<<"I have "<<(*c)->id<<" "<<(((avtStateRecorderIntegralCurve *)*c))->sequenceCnt<<" idx= "<<idx<<endl;
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
