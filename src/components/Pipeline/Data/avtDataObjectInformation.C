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
//                         avtDataObjectInformation.C                        //
// ************************************************************************* //

#ifdef PARALLEL
#include <mpi.h>
#include <avtParallel.h>
#endif

#include <avtDataObjectInformation.h>
#include <avtDataObjectString.h>
#include <avtDataObjectWriter.h>
#include <avtWebpage.h>


// ****************************************************************************
//  Method: avtDataObjectInformation constructor
//
//  Purpose:
//      Defines the constructor.  Note: this should not be inlined in the
//      header because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   February 5, 2004
//
// ****************************************************************************

avtDataObjectInformation::avtDataObjectInformation()
{
    ;
}


// ****************************************************************************
//  Method: avtDataObjectInformation destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   February 5, 2004
//
// ****************************************************************************

avtDataObjectInformation::~avtDataObjectInformation()
{
    ;
}


// ****************************************************************************
//  Method: avtDataObjectInformation::Copy
//
//  Purpose:
//      Copies the data object information from the argument to this object.
//
//  Arguments:
//      di      Data object information from another object.
//
//  Programmer: Hank Childs
//  Creation:   October 25, 2000
//
//  Modifications:
//
//    Hank Childs, Sat Mar 24 15:21:04 PST 2001
//    Re-wrote function, blew away previous comments.
//
// ****************************************************************************

void
avtDataObjectInformation::Copy(const avtDataObjectInformation &di)
{
    atts.Copy(di.atts);
    validity.Copy(di.validity);
}


// ****************************************************************************
//  Method: avtDataObjectInformation::Merge
//
//  Purpose:
//      Merges the data object information from the argument and this object.
//
//  Arguments:
//      di      Data object information from another object.
//
//  Programmer: Hank Childs
//  Creation:   March 24, 2001
//
//  Modifications:
//
//    Mark C. Miller, Thu Jan 29 17:31:20 PST 2004
//    Added passage of bools from data validity regarding whether any domain
//    was owned to the merge of the attributes
//
// ****************************************************************************

void
avtDataObjectInformation::Merge(const avtDataObjectInformation &di)
{
    // see if either operand has ever been left unset due to processor 
    // not having read any domain 
    bool ignoreThis =    validity.GetHasEverOwnedAnyDomain() ? false : true;
    bool ignoreThat = di.validity.GetHasEverOwnedAnyDomain() ? false : true;

    atts.Merge(di.atts, ignoreThis, ignoreThat);
    validity.Merge(di.validity);
}

// ****************************************************************************
//  Method: avtDataObjectInformation::RecvResult
//
//  Purpose:
//      Recieves avtDataObjectInformation from another processor and
//      copies it into this object.
//
//  Programmer: Mark C. Miller
//  Creation:   02Sep03 
//
//  Modifications:
//
//    Mark C. Miller, Mon Jan 22 22:09:01 PST 2007
//    Changed MPI_COMM_WORLD to VISIT_MPI_COMM
// ****************************************************************************
void
avtDataObjectInformation::RecvResult(const avtDataObjectWriter_p dobw,
   int swapWithProc, int mpiResultLenTag, int mpiResultStrTag)
{
#ifdef PARALLEL

   MPI_Status mpiStatus;
   int dstLen;

   MPI_Recv(&dstLen, 1, MPI_INT, swapWithProc, mpiResultLenTag, VISIT_MPI_COMM, &mpiStatus);
   char *dstStr = new char [dstLen];
   MPI_Recv(dstStr, dstLen, MPI_CHAR, swapWithProc, mpiResultStrTag, VISIT_MPI_COMM, &mpiStatus);

   avtDataObjectInformation dstDobInfo;
   dstDobInfo.Read(dstStr);

   delete [] dstStr;

   // make this object's info the info we just recieved
   Copy(dstDobInfo);

#endif
}

// ****************************************************************************
//  Method: avtDataObjectInformation::SendResults
//
//  Purpose:
//      Sends avtDataObjectInformation from this processor to another processor 
//
//  Programmer: Mark C. Miller
//  Creation:   02Sep03 
//
//  Modifications:
//
//    Mark C. Miller, Thu Jun 10 10:05:09 PDT 2004
//    Modified to use unique message tags
//
//    Mark C. Miller, Mon Jan 22 22:09:01 PST 2007
//    Changed MPI_COMM_WORLD to VISIT_MPI_COMM
// ****************************************************************************
void
avtDataObjectInformation::SendResult(const avtDataObjectWriter_p dobw,
   int swapWithProc, int mpiResultLenTag, int mpiResultStrTag)
{
#ifdef PARALLEL

   int srcLen;
   char *srcStr;
   avtDataObjectString srcDobStr;

   Write(srcDobStr, *dobw);
   srcDobStr.GetWholeString(srcStr, srcLen);

   MPI_Send(&srcLen, 1, MPI_INT, swapWithProc, mpiResultLenTag, VISIT_MPI_COMM);
   MPI_Send(srcStr, srcLen, MPI_CHAR, swapWithProc, mpiResultStrTag, VISIT_MPI_COMM);

#endif
}

// ****************************************************************************
//  Method: avtDataObjectInformation::SwapAndMerge
//
//  Purpose:
//      Swaps avtDataObjectInformation between this processor and another and
//      Merges it into this object.
//
//  Programmer: Mark C. Miller
//  Creation:   02Sep03 
//
//  Modifications:
//
//    Mark C. Miller, Thu Jun 10 10:05:09 PDT 2004
//    Modified to use unique message tags
//
//    Mark C. Miller, Mon Jan 22 22:09:01 PST 2007
//    Changed MPI_COMM_WORLD to VISIT_MPI_COMM
// ****************************************************************************
void
avtDataObjectInformation::SwapAndMerge(const avtDataObjectWriter_p dobw,
   int swapWithProc, int mpiSwapLenTag, int mpiSwapStrTag)
{
#ifdef PARALLEL

   MPI_Status mpiStatus;
   char *srcStr, *dstStr;
   int   srcLen,  dstLen;

   // serialize the data object information into a string
   avtDataObjectString srcDobStr;
   Write(srcDobStr, *dobw);
   srcDobStr.GetWholeString(srcStr, srcLen);

   // swap string lengths
   MPI_Sendrecv(&srcLen, 1, MPI_INT, swapWithProc, mpiSwapLenTag,
                &dstLen, 1, MPI_INT, swapWithProc, mpiSwapLenTag,
                VISIT_MPI_COMM, &mpiStatus);

   dstStr = new char [dstLen];

   // swap strings
   MPI_Sendrecv(srcStr, srcLen, MPI_CHAR, swapWithProc, mpiSwapStrTag,
                dstStr, dstLen, MPI_CHAR, swapWithProc, mpiSwapStrTag,
                VISIT_MPI_COMM, &mpiStatus);

   // unserialize the dst string only
   avtDataObjectInformation dstDobInfo;
   dstDobInfo.Read(dstStr);

   delete [] dstStr;

   // normal merge
   Merge(dstDobInfo);

#endif
}

// ****************************************************************************
//  Method: avtDataObjectInformation::ParallelMerge
//
//  Purpose:
//      Performs a tree-like sequence of swap and merge operations. At each
//      iteration through the loop, the entire communicator of processors is
//      divided into groups which are known to have merged results. One processor
//      in each even numbered group is paired with one processor in each odd
//      numbered group and vice versa. As long as the processor identified to
//      swap with is in the range of the communicator, the avtDataObjectInformation
//      is swapped and merged between these pairs of processors. The group
//      size is doubled and the process of pairing and swapping is repeated.
//      This continues until the group size equals or exceeds the communicator
//      size. At this point, one or two processors has merged results that include
//      the influence of every other processor even if they did not explicitly
//      communicate with each other. Those results are then re-distributed to
//      all the other processors.
//
//  Arguments:
//      dobw data object writer used in the write calls 
//
//  Programmer: Mark C. Miller 
//  Creation:   April 1, 2003
//
//  Modifications:
//     Mark C. Miller, 02Sep03
//     Completely re-wrote to avoid MPI_Allreduce and permit growth in
//     message size as merges are performed. For example, the list of material
//     labels may get longer and longer.
//
//     Mark C. Miller, Wed Jan 21 13:36:01 PST 2004
//     Moved call to CanUseCummulativeAsTrueOrCurrent to outside the 
//     #ifdef PARALLEL block
//
//     Hank Childs, Wed Feb 25 11:00:09 PST 2004
//     Correct mispelling of cumulative.
//
//     Mark C. Miller, Thu Jun 10 10:05:09 PDT 2004
//     Modified to use unique message tags
//
//     Mark C. Miller, Mon Jan 22 22:09:01 PST 2007
//     Changed MPI_COMM_WORLD to VISIT_MPI_COMM
// ****************************************************************************

void
avtDataObjectInformation::ParallelMerge(const avtDataObjectWriter_p dobw)
{
#ifdef PARALLEL

   int groupSize = 1;
   int myRank, commSize;

   MPI_Comm_size(VISIT_MPI_COMM, &commSize);
   MPI_Comm_rank(VISIT_MPI_COMM, &myRank);
   int mpiResultLenTag = GetUniqueMessageTag();
   int mpiResultStrTag = GetUniqueMessageTag();
   int mpiSwapLenTag   = GetUniqueMessageTag();
   int mpiSwapStrTag   = GetUniqueMessageTag();
   groupSize = 1;

   // walk up the communication tree, swapping and merging infos
   while (groupSize < commSize)
   {
      int swapWithProc = -1;
      int myGroupNum = myRank / groupSize;
      int myGroupIdx = myRank % groupSize;

      // determine processor to swap with
      if (myGroupNum % 2)   // myGroupNum is odd
         swapWithProc = (myGroupNum - 1) * groupSize + myGroupIdx;
      else                  // myGroupNum is even
         swapWithProc = (myGroupNum + 1) * groupSize + myGroupIdx;

      // only do the swap between 0th processors in each group AND only
      // if the processor to swap with is in range of communicator
      if ((myGroupIdx == 0) && 
          (0 <= swapWithProc) && (swapWithProc < commSize))
         SwapAndMerge(dobw, swapWithProc, mpiSwapLenTag, mpiSwapStrTag);
      
      groupSize <<= 1;
   }

   // At this point the processor(s) at the top of the tree have the
   // merged result. So, now we need to re-distribute it to all
   groupSize >>= 2;

   // walk back down the communication tree, sending results
   while (groupSize >= 1)
   {
      int swapWithProc = -1;
      int myGroupNum = myRank / groupSize;
      int myGroupIdx = myRank % groupSize;

      // determine processor to send to 
      if (myGroupNum % 2)   // myGroupNum is odd
      {
         swapWithProc = (myGroupNum - 1) * groupSize + myGroupIdx;
         if ((myGroupIdx == 0) &&
             (0 <= swapWithProc) && (swapWithProc < commSize))
            RecvResult(dobw, swapWithProc, mpiResultLenTag, mpiResultStrTag);
      }
      else                  // myGroupNum is even
      {
         swapWithProc = (myGroupNum + 1) * groupSize + myGroupIdx;
         if ((myGroupIdx == 0) &&
             (0 <= swapWithProc) && (swapWithProc < commSize))
            SendResult(dobw, swapWithProc, mpiResultLenTag, mpiResultStrTag);
      }

      groupSize >>= 1;
   }

#endif

   // indicate that it is ok to use cummulative true extents as true extents;
   GetAttributes().SetCanUseCumulativeAsTrueOrCurrent(true);
}

// ****************************************************************************
//  Method: avtDataObjectInformation::Write
//
//  Purpose:
//      Writes the data object information to a stream.
//
//  Arguments:
//      str     The string (stream) to write to.
//      wrtr    The writer that handles conversion to the destination format.
//
//  Programmer: Hank Childs
//  Creation:   March 25, 2001
//
// ****************************************************************************

void
avtDataObjectInformation::Write(avtDataObjectString &str, 
                                const avtDataObjectWriter *wrtr)
{
    atts.Write(str, wrtr);
    validity.Write(str, wrtr);
}


// ****************************************************************************
//  Method: avtDataObjectInformation::Read
//
//  Purpose:
//      Reads the data object information from a stream.
//
//  Arguments:
//      str     The string (stream) to write to.
//
//  Programmer: Hank Childs
//  Creation:   March 25, 2001
//
// ****************************************************************************

int
avtDataObjectInformation::Read(char *str)
{
    int size1 = atts.Read(str);
    int size2 = validity.Read(str + size1);
    return (size1 + size2);
}


// ****************************************************************************
//  Method: avtDataObjectInformation::DebugDump
//
//  Purpose:
//      Outputs the data object information to a webpage.
//
//  Programmer: Hank Childs
//  Creation:   December 21, 2006
//
// ****************************************************************************

void
avtDataObjectInformation::DebugDump(avtWebpage *webpage)
{
    atts.DebugDump(webpage);
    validity.DebugDump(webpage);
}


