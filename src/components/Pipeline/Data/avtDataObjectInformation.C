// ************************************************************************* //
//                         avtDataObjectInformation.C                        //
// ************************************************************************* //

#ifdef PARALLEL
#include <mpi.h>
#endif

#include <avtDataObjectInformation.h>
#include <avtDataObjectString.h>
#include <avtDataObjectWriter.h>

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
// ****************************************************************************

void
avtDataObjectInformation::Merge(const avtDataObjectInformation &di)
{
    atts.Merge(di.atts);
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
// ****************************************************************************
void
avtDataObjectInformation::RecvResult(const avtDataObjectWriter_p dobw,
   int swapWithProc)
{
#ifdef PARALLEL

   static const int mpiTag = 41; // arbitrary
   MPI_Status mpiStatus;
   int dstLen;

   MPI_Recv(&dstLen, 1, MPI_INT, swapWithProc, mpiTag, MPI_COMM_WORLD, &mpiStatus);
   char *dstStr = new char [dstLen];
   MPI_Recv(dstStr, dstLen, MPI_CHAR, swapWithProc, mpiTag+1, MPI_COMM_WORLD, &mpiStatus);

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
// ****************************************************************************
void
avtDataObjectInformation::SendResult(const avtDataObjectWriter_p dobw,
   int swapWithProc)
{
#ifdef PARALLEL

   static const int mpiTag = 41; // arbitrary
   int srcLen;
   char *srcStr;
   avtDataObjectString srcDobStr;

   Write(srcDobStr, *dobw);
   srcDobStr.GetWholeString(srcStr, srcLen);

   MPI_Send(&srcLen, 1, MPI_INT, swapWithProc, mpiTag, MPI_COMM_WORLD);
   MPI_Send(srcStr, srcLen, MPI_CHAR, swapWithProc, mpiTag+1, MPI_COMM_WORLD);

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
// ****************************************************************************
void
avtDataObjectInformation::SwapAndMerge(const avtDataObjectWriter_p dobw,
   int swapWithProc)
{
#ifdef PARALLEL

   static const int mpiTag = 37; // arbitrary
   MPI_Status mpiStatus;
   char *srcStr, *dstStr;
   int   srcLen,  dstLen;

   // serialize the data object information into a string
   avtDataObjectString srcDobStr;
   Write(srcDobStr, *dobw);
   srcDobStr.GetWholeString(srcStr, srcLen);

   // swap string lengths
   MPI_Sendrecv(&srcLen, 1, MPI_INT, swapWithProc, mpiTag,
                &dstLen, 1, MPI_INT, swapWithProc, mpiTag,
                MPI_COMM_WORLD, &mpiStatus);

   dstStr = new char [dstLen];

   // swap strings
   MPI_Sendrecv(srcStr, srcLen, MPI_CHAR, swapWithProc, mpiTag+1,
                dstStr, dstLen, MPI_CHAR, swapWithProc, mpiTag+1,
                MPI_COMM_WORLD, &mpiStatus);

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
// ****************************************************************************

void
avtDataObjectInformation::ParallelMerge(const avtDataObjectWriter_p dobw)
{
#ifdef PARALLEL

   int groupSize = 1;
   int myRank, commSize;

   MPI_Comm_size(MPI_COMM_WORLD, &commSize);
   MPI_Comm_rank(MPI_COMM_WORLD, &myRank);
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
         SwapAndMerge(dobw, swapWithProc);
      
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
            RecvResult(dobw, swapWithProc);
      }
      else                  // myGroupNum is even
      {
         swapWithProc = (myGroupNum + 1) * groupSize + myGroupIdx;
         if ((myGroupIdx == 0) &&
             (0 <= swapWithProc) && (swapWithProc < commSize))
            SendResult(dobw, swapWithProc);
      }

      groupSize >>= 1;
   }

   // indicate that it is ok to use cummulative true extents as true extents;
   GetAttributes().SetCanUseCummulativeAsTrueOrCurrent(true);

#endif
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


