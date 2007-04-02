/*****************************************************************************
*
* Copyright (c) 2000 - 2006, The Regents of the University of California
* Produced at the Lawrence Livermore National Laboratory
* All rights reserved.
*
* This file is part of VisIt. For details, see http://www.llnl.gov/visit/. The
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
*    documentation and/or materials provided with the distribution.
*  - Neither the name of the UC/LLNL nor  the names of its contributors may be
*    used to  endorse or  promote products derived from  this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED.  IN  NO  EVENT  SHALL  THE  REGENTS  OF  THE  UNIVERSITY OF
* CALIFORNIA, THE U.S.  DEPARTMENT  OF  ENERGY OR CONTRIBUTORS BE  LIABLE  FOR
* ANY  DIRECT,  INDIRECT,  INCIDENTAL,  SPECIAL,  EXEMPLARY,  OR CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

// ************************************************************************* //
//                        avtSamplePointCommunicator.C                       //
// ************************************************************************* //

#include <avtSamplePointCommunicator.h>

#include <avtCellList.h>
#include <avtImagePartition.h>
#include <avtParallel.h>
#include <avtSamplePoints.h>
#include <avtVolume.h>

#include <ImproperUseException.h>
#include <TimingsManager.h>
#include <Utility.h>

//
// Much of this code depends on MPI, so just ifdef out the parallel code
// if we are not in parallel.
//
#ifdef PARALLEL
#include <mpi.h>
#endif

// ****************************************************************************
//  Method: avtSamplePointCommunicator constructor
//
//  Programmer: Hank Childs
//  Creation:   January 24, 2001
//
// ****************************************************************************

avtSamplePointCommunicator::avtSamplePointCommunicator()
{
#ifdef PARALLEL
    MPI_Comm_size(MPI_COMM_WORLD, &numProcs);
    MPI_Comm_rank(MPI_COMM_WORLD, &myRank);
#else
    numProcs = 1; myRank = 0;
#endif
    imagePartition = NULL;
}


// ****************************************************************************
//  Method: avtSamplePointCommunicator destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   February 5, 2004
//
// ****************************************************************************

avtSamplePointCommunicator::~avtSamplePointCommunicator()
{
    ;
}


// ****************************************************************************
//  Method: avtSamplePointCommunicator::SetImagePartition
//
//  Purpose:
//      Sets the image partition to be used for the sample point communicator.
//
//  Arguments:
//      ip       The image partition to use.
//
//  Programmmer: Hank Childs
//  Creation:    March 6, 2001
//
// ****************************************************************************

void
avtSamplePointCommunicator::SetImagePartition(avtImagePartition *ip)
{
    imagePartition = ip;
}


// ****************************************************************************
//  Method: avtSamplePointCommunicator::Execute
//
//  Purpose:
//      Executes the sample point communicator.
//
//  Programmer: Hank Childs
//  Creation:   January 24, 2001
//
//  Modifications:
//
//    Hank Childs, Sun Feb  4 10:16:13 PST 2001
//    Offloaded work onto rays, cells.
//
//    Hank Childs, Sun Mar  4 19:51:03 PST 2001
//    Distributed partitions of image space dynamically to minimize
//    communication.
//
//    Eric Brugger, Mon Nov  5 13:44:45 PST 2001
//    Modified to always compile the timing code.
//
//    Hank Childs, Tue Dec 18 08:24:00 PST 2001
//    Changed location of calls to free memory to reduce size of peak memory.
//
//    Hank Childs, Sat Dec 11 11:27:16 PST 2004
//    Add support multiple executions that come about because of tiling.
//
//    Hank Childs, Fri Sep 30 14:04:39 PDT 2005
//    Added support for serial communication (ie pass-thru).
//
//    Hank Childs, Jan 27 14:49:36 PST 2006
//    Tell the cell list not to extract samples that are outside the volume
//    of interest.
//
// ****************************************************************************

void
avtSamplePointCommunicator::Execute(void)
{
#ifdef PARALLEL
    int timingsIndex = visitTimer->StartTimer();
    int nProgressStages = 14;
    int currentStage    = 1;

    //
    // We are typically still waiting for the sample point extractors, so put
    // in a barrier so this filter can be absolved of blame.
    //
    Barrier();
    UpdateProgress(currentStage++, nProgressStages);

    if (imagePartition == NULL)
    {
        EXCEPTION0(ImproperUseException);
    }

    EstablishImagePartitionBoundaries();
    UpdateProgress(currentStage++, nProgressStages);

    avtVolume *involume      = GetTypedInput()->GetVolume();
    int        volumeWidth   = involume->GetVolumeWidth();
    int        volumeHeight  = involume->GetVolumeHeight();
    int        volumeDepth   = involume->GetVolumeDepth();

    //
    // Have the rays serialize their sample points.
    //
    int   *out_points_count = new int[numProcs];
    char **out_points_msgs  = new char*[numProcs];
    char *tmpcat1 = involume->ConstructMessages(imagePartition,
                                            out_points_msgs, out_points_count);
    UpdateProgress(currentStage++, nProgressStages);

    //
    // Have the cells serialize themselves.
    //
    avtCellList *incl  = GetTypedInput()->GetCellList();
    int   *out_cells_count = new int[numProcs];
    char **out_cells_msgs  = new char*[numProcs];
    char *tmpcat2 = incl->ConstructMessages(imagePartition, out_cells_msgs,
                                            out_cells_count);
    UpdateProgress(currentStage++, nProgressStages);

    //
    // Determine which cells/points are going where and distribute them in a
    // way that minimizes communication.
    //
    DetermineImagePartitionAssignments(out_points_count, out_cells_count);
    UpdateProgress(currentStage++, nProgressStages);

    //
    // The messages are set up for image partition assignments of 0->0, 1->1,
    // etc.  Rework them so that they can be sent in to our CommunicateMessage
    // routines.
    //
    char *pointsOnThisProc;
    int   numPointsOnThisProc;
    char *concat1 = MutateMessagesByAssignment(out_points_msgs,
                      out_points_count, pointsOnThisProc, numPointsOnThisProc);
    delete [] tmpcat1; // No longer needed.  out_points_msgs contains the
                       // same info with the proper ordering.
    UpdateProgress(currentStage++, nProgressStages);

    char *cellsOnThisProc;
    int   numCellsOnThisProc;
    char *concat2 = MutateMessagesByAssignment(out_cells_msgs,
                         out_cells_count, cellsOnThisProc, numCellsOnThisProc);
    delete [] tmpcat2; // No longer needed.  out_cells_msgs contains the
                       // same info with the proper ordering.
    UpdateProgress(currentStage++, nProgressStages);

    //
    // Send the sample points.
    //
    int   *in_points_count  = new int[numProcs];
    char **in_points_msgs   = new char*[numProcs];
    char *concat3 = CommunicateMessages(out_points_msgs, out_points_count,
                                        in_points_msgs, in_points_count);
    delete [] concat1;
    delete [] out_points_count;
    delete [] out_points_msgs;
    UpdateProgress(currentStage++, nProgressStages);

    //
    // Send the cells.
    //
    int   *in_cells_count  = new int[numProcs];
    char **in_cells_msgs   = new char*[numProcs];
    char *concat4 = CommunicateMessages(out_cells_msgs, out_cells_count,
                                        in_cells_msgs, in_cells_count);
    delete [] concat2;
    delete [] out_cells_count;
    delete [] out_cells_msgs;
    UpdateProgress(currentStage++, nProgressStages);

    //
    // Create the output volume and let it know that it only is for a restricted
    // part of the volume.
    //
    int   outMinWidth, outMaxWidth, outMinHeight, outMaxHeight;
    imagePartition->GetThisPartition(outMinWidth, outMaxWidth, outMinHeight,
                                     outMaxHeight);
    int nv = GetTypedInput()->GetNumberOfVariables();
    nv = UnifyMaximumValue(nv);
    if (GetTypedInput()->GetUseWeightingScheme())
        GetTypedOutput()->SetUseWeightingScheme(true);
    if (GetTypedOutput()->GetVolume() == NULL)
    {
        GetTypedOutput()->SetNumberOfVariables(nv);
        GetTypedOutput()->SetVolume(volumeWidth, volumeHeight, volumeDepth);
    }
    else
        GetTypedOutput()->GetVolume()->ResetSamples();

    avtVolume *outvolume = GetTypedOutput()->GetVolume();
    outvolume->Restrict(outMinWidth, outMaxWidth, outMinHeight, outMaxHeight);

    //
    // Put the sample points into our output volume.
    //
    outvolume->ExtractSamples(in_points_msgs, in_points_count, numProcs);
    delete [] concat3;
    delete [] in_points_count;
    delete [] in_points_msgs;
    UpdateProgress(currentStage++, nProgressStages);

    outvolume->ExtractSamples(&pointsOnThisProc, &numPointsOnThisProc, 1);
    delete [] pointsOnThisProc;
    UpdateProgress(currentStage++, nProgressStages);
    
    //
    // Extract the sample points from the new cells.
    //
    avtCellList *outcl = GetTypedOutput()->GetCellList();
    outcl->Restrict(outMinWidth, outMaxWidth, outMinHeight, outMaxHeight);
    outcl->ExtractCells(in_cells_msgs, in_cells_count, numProcs, outvolume);
    UpdateProgress(currentStage++, nProgressStages);
    delete [] concat4;
    delete [] in_cells_count;
    delete [] in_cells_msgs;

    outcl->ExtractCells(&cellsOnThisProc, &numCellsOnThisProc, 1, outvolume);
    delete [] cellsOnThisProc;
    UpdateProgress(currentStage++, nProgressStages);

    visitTimer->StopTimer(timingsIndex, "Sample point communication");
#else
    GetTypedOutput()->Copy(*(GetTypedInput()));
#endif
}


// ****************************************************************************
//  Method: avtSamplePointCommunicator::DetermineImagePartitionAssignments
//
//  Purpose:
//      Determine which partitions of image space should be assigned to which
//      processor.  The actual work is done by the image partition.
//
//  Arguments:
//      out1           The first list of how much to send to each processor
//                     (from the sample points).
//      out2           The second list of how much to send to each processor 
//                     (from the cells).
//
//  Programmer: Hank Childs
//  Creation:   March 4, 2001
//
// ****************************************************************************

void
avtSamplePointCommunicator::DetermineImagePartitionAssignments(const int *out1,
                                                               const int *out2)
{
    int   i;

    //
    // We received two lists of output (one from sample points, one from
    // cells), so combine that into one list.
    //
    int *totalout = new int[numProcs];
    for (i = 0 ; i < numProcs ; i++)
    {
        totalout[i] = out1[i] + out2[i];
    }

    //
    // The image partition will determine the assignments and also store them.
    //
    imagePartition->DetermineAssignments(totalout);

    //
    // Clean up memory
    //
    delete [] totalout;
}


// ****************************************************************************
//  Method: avtSamplePointCommunicator::EstablishImagePartitionBoundaries
//
//  Purpose:
//      Determines how many samples there are in each scanline and gives that
//      information to the image partition, so that it can decide how to
//      place the boundaries such that the number of sample points are
//      relatively evenly distributed between samples.
//
//  Programmer: Hank Childs
//  Creation:   January 1, 2002
//
// ****************************************************************************

void
avtSamplePointCommunicator::EstablishImagePartitionBoundaries(void)
{
    avtVolume *involume      = GetTypedInput()->GetVolume();
    int        height        = involume->GetVolumeHeight();

    int       *samplesPerScanline = new int[height];
    for (int i = 0 ; i < height ; i++)
    {
        samplesPerScanline[i] = 0;
    }

    //
    // Determine how many samples we have in the sample points.
    //
    involume->EstimateNumberOfSamplesPerScanline(samplesPerScanline);

    //
    // Estimate how many samples we have in the cells.
    //
    avtCellList *incl  = GetTypedInput()->GetCellList();
    incl->EstimateNumberOfSamplesPerScanline(samplesPerScanline);

    //
    // Now let the image partition determines what the right boundaries is
    // so that each partition has approximately the same number of sample
    // points.
    //
    imagePartition->EstablishPartitionBoundaries(samplesPerScanline);
    
    delete [] samplesPerScanline;
}


// ****************************************************************************
//  Method: avtSamplePointCommunicator::CommunicateMessages
//
//  Purpose:
//      Communicates the messages across the network to the other processors.
//
//  Arguments:
//      sendmessages   The messages to send.
//      sendcount      The size of each message in sendmessages.
//      recvmessages   The messages from other processors.
//      recvcount      The size of the messages in recvmessages.
//
//  Returns:    A pointer to the array that recvmessages indexes into.
//
//  Programmer: Hank Childs
//  Creation:   January 24, 2001
//
// ****************************************************************************

char *
avtSamplePointCommunicator::CommunicateMessages(char **sendmessages,
                                                int   *sendcount,
                                                char **recvmessages,
                                                int   *recvcount)
{
#ifdef PARALLEL
    //
    // Figure out how much each processor needs to send/receive.
    //
    MPI_Alltoall(sendcount, 1, MPI_INT, recvcount, 1, MPI_INT, MPI_COMM_WORLD);

    //
    // Create a buffer we can receive into.
    //
    char *recvConcatList = CreateMessageStrings(recvmessages, recvcount,
                                                numProcs);
    
    //
    // Calculate the displacement lists.
    //
    int *senddisp = new int[numProcs];
    int *recvdisp = new int[numProcs];
    senddisp[0] = 0;
    recvdisp[0] = 0;
    for (int i = 1 ; i < numProcs ; i++)
    {
        senddisp[i] = senddisp[i-1] + sendcount[i-1];
        recvdisp[i] = recvdisp[i-1] + recvcount[i-1];
    }

    //
    // Do the actual transfer of sample points.   The messages arrays are
    // actually indexes into one big array.  Since MPI expects that big
    // array, give that (which is at location 0).
    //
    MPI_Alltoallv(sendmessages[0], sendcount, senddisp, MPI_CHAR,
                  recvmessages[0], recvcount, recvdisp, MPI_CHAR,
                  MPI_COMM_WORLD);

    delete [] senddisp;
    delete [] recvdisp;

    //
    // We need to return this buffer so the calling function can delete it.
    //
    return recvConcatList;
#else
    return 0;
#endif
}


// ****************************************************************************
//  Method: avtSamplePointCommunicator::MutateMessagesByAssignment
//
//  Purpose:
//      The objects (sample points and cells) made messages by assuming that
//      partition 0 was assigned to processor 0, etc.  As an optimization,
//      assignments were found to minimize communication.  This routine
//      will swap the messages around in a character string to reflect those
//      assignments.
//
//  Arguments:
//      msgs           The messages from this processor to all of the other
//                     processors.
//      sizes          The size of each message.
//      myMsg          A place to set aside the message from this processor
//                     to itself, we can treat it like the rest without
//                     having to use the MPI network to communicate it.
//      mySize         The size of myMsg.
//      
//  Returns:    The character string that the new messages sits on.  This
//              should be free'd by the calling function.
//
//  Programmer: Hank Childs
//  Creation:   March 4, 2001
//
//  Modifications:
//
//    Hank Childs, Tue Dec 18 08:27:54 PST 2001
//    No longer couple 'myMsg' with the buffer being returned, but allocate
//    its own memory so the bigger buffer can be freed earlier.
//
//    Hank Childs, Tue Jan  1 14:00:57 PST 2002
//    Reflect change in interface to avtImagePartition.
//
// ****************************************************************************

char *
avtSamplePointCommunicator::MutateMessagesByAssignment(char **msgs, int *sizes,
                                                     char *&myMsg, int &mySize)
{
#ifdef PARALLEL
    int   i;

    //
    // The assignments are stored with the image partition, so get them now.
    // If assignments is thought of as a function, then the domain is
    // partitions, and the range is processors.
    //
    const int *assignments =
                          imagePartition->GetPartitionToProcessorAssignments();

    //
    // A reverse list of assignments that takes processors to partitions would
    // be helpful when we are trying to unwind the naive assignment.
    //
    int *reverseAssignments = new int[numProcs];
    for (i = 0 ; i < numProcs ; i++)
    {
        reverseAssignments[assignments[i]] = i;
    }

    //
    // Determine the total size of the message.  'myMsg' will not be added to
    // this message since there is no need to do parallel communication with
    // it.
    //
    int totalSize = 0;
    for (i = 0 ; i < numProcs ; i++)
    {
        if (assignments[i] == myRank)
        {
            continue;
        }
        totalSize += sizes[i];
    }

    char *rv        = new char[totalSize];
    char **tmpMsgs  = new char*[numProcs];
    int   *tmpSizes = new int[numProcs];

    //
    // Put the messages in rv one at a time.
    //
    char *currentPtr = rv;
    for (i = 0 ; i < numProcs ; i++)
    {
        //
        // We are trying to build the message for processor i.  We can use our
        // reverse assignments map to determine which message index the correct
        // message (partition) will correspond to.
        //
        int a = reverseAssignments[i];
        if (i != myRank)
        {
            memcpy(currentPtr, msgs[a], sizes[a]);
            tmpMsgs[i]  = currentPtr;
            tmpSizes[i] = sizes[a];
            currentPtr += tmpSizes[i];
        }
        else
        {
            tmpMsgs[i]  = currentPtr;
            tmpSizes[i] = 0;
        }
    }

    //
    // We also wanted to couple the message to ourselves at the end of our
    // large byte array.  The msgs and size arrays have been set up so this
    // will not be sent.
    //
    int  myPartition = reverseAssignments[myRank];
    mySize = sizes[myPartition];
    myMsg  = new char[mySize];
    memcpy(myMsg, msgs[myPartition], mySize);

    //
    // We want msgs and sizes to be the same as tmpMsgs and tmpSizes now.
    //
    for (i = 0 ; i < numProcs ; i++)
    {
        msgs[i]  = tmpMsgs[i];
        sizes[i] = tmpSizes[i];
    }

    //
    // Clean up memory
    //
    delete [] reverseAssignments;
    delete [] tmpMsgs;
    delete [] tmpSizes;

    return rv;
#else
    return 0;
#endif
}


