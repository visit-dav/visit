/*****************************************************************************
*
* Copyright (c) 2000 - 2017, Lawrence Livermore National Security, LLC
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
//                                  avtParallel.C                            //
// ************************************************************************* //

#include <avtParallel.h>
#include <avtParallelContext.h>

#ifdef PARALLEL
  #include <mpi.h>
  #include <BufferConnection.h>
#endif

#include <DebugStream.h>
#include <ImproperUseException.h>
#include <AttributeGroup.h>

#include <cstring>

#ifdef PARALLEL
// VisIt's own MPI communicator
static MPI_Comm VISIT_MPI_COMM_OBJ;
void *VISIT_MPI_COMM_PTR = NULL;

// If MPI was already initalized for us, we don't need to finalize either
static bool  we_initialized_MPI = true;
#endif

// The global parallel context.
static avtParallelContext globalContext;

// Variables to hold process size information
static int par_rank = 0, par_size = 1;

// ****************************************************************************
//  Function: PAR_Exit
//
//  Purpose:
//      Exits the program gracefully.
//
//  Programmer: Brad Whitlock
//  Creation:   Wed Jul 12 16:02:58 PST 2000
//
//  Modifications:
//    Eric Brugger, Tue Aug 31 10:27:20 PDT 2004
//    Made the mpi coding conditional on PARALLEL.
//
//    Jeremy Meredith, Mon Nov  1 16:42:58 PST 2004
//    Do not close down MPI if we were not the ones to start it.
//
//    Mark C. Miller, Wed Jan 12 14:56:33 PST 2005
//    Removed unary negation from test for whether to call MPI_Finalize
//
// ****************************************************************************

void
PAR_Exit(void)
{
#ifdef PARALLEL
    if (we_initialized_MPI)
        MPI_Finalize();
#endif
}


// ****************************************************************************
//  Function: PAR_Init
//
//  Purpose:
//      Initializes parallel state information.
//
//  Programmer: Brad Whitlock
//  Creation:   Wed Jul 12 16:03:46 PST 2000
//
//  Modifications:
//    Eric Brugger, Tue Aug 31 10:27:20 PDT 2004
//    Made the mpi coding conditional on PARALLEL.  Removed the call to
//    PAR_CreateTypes.
//
//    Kathleen Bonnell, Wed Sep  8 15:08:00 PDT 2004
//    Retrieve MPI_TAG_UB value and use it to set mpiTagUpperBound.  Use
//    32767 if unsuccessful.
//
//    Jeremy Meredith, Mon Nov  1 16:42:58 PST 2004
//    If MPI was already started, then (1) we don't want to call MPI_Init
//    again, and (2) we don't want to call MPI_Finalize when we quit.  This
//    check will come into play with the VisIt Library for simulations.
//
//    Mark C. Miller, Mon Jan 22 22:09:01 PST 2007
//    Added call to dup MPI_COMM_WORLD to create VISIT_MPI_COMM
//
//    Brad Whitlock, Fri Jul 20 11:23:05 PDT 2012
//    Duplicate custom communicator if it has been set up.
//
// ****************************************************************************

void
PAR_Init (int &argc, char **&argv)
{
#ifndef PARALLEL
    (void)argc;
    (void)argv;
#else
    int flag;
    MPI_Initialized(&flag);
    we_initialized_MPI = !flag;

    if (we_initialized_MPI)
        MPI_Init (&argc, &argv);

    MPI_Comm vcomm;
    if(VISIT_MPI_COMM_PTR == NULL)
    {
        debug5 << "Par_Init: Duplicating MPI_COMM_WORLD" << endl;
        vcomm = MPI_COMM_WORLD;
    }
    else
    {
        debug5 << "Par_Init: Duplicating a custom MPI communicator." << endl;
        vcomm = *((MPI_Comm *)VISIT_MPI_COMM_PTR);
    }
    if (MPI_Comm_dup(vcomm, &VISIT_MPI_COMM_OBJ) != MPI_SUCCESS)
        VISIT_MPI_COMM_OBJ = vcomm;
    VISIT_MPI_COMM_PTR = (void*) &VISIT_MPI_COMM_OBJ;

    //
    // Find the current process rank and the size of the process pool.
    //
    MPI_Comm_rank (VISIT_MPI_COMM, &par_rank);
    MPI_Comm_size (VISIT_MPI_COMM, &par_size);

    // Do some initialization.
    avtParallelContext::Init();

    // Replace the communicator that we had in the global context.
    globalContext.SetCommunicator(VISIT_MPI_COMM);
#endif
}

// ****************************************************************************
// Function: PAR_SetComm
//
// Purpose:
//   Set the communicator for VisIt to use.
//
// Arguments:
//   newcomm : The new communicator.
//
// Returns:    True on success; false on failure.
//
// Note:
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 26 10:03:19 PDT 2011
//
// Modifications:
//
// ****************************************************************************

bool
PAR_SetComm(void *newcomm)
{
#ifndef PARALLEL
    (void)newcomm;
    return false;
#else
    if(newcomm == NULL)
    {
        // switch back to the dup'd world communicator.
        VISIT_MPI_COMM_PTR = (void*) &VISIT_MPI_COMM_OBJ;
    }
    else
    {
// Test that it is actually a comm?

        // Use the communicator that was passed in.
        debug5 << "PAR_SetComm: Setting VISIT_MPI_COMM_PTR to " << newcomm << endl;
        VISIT_MPI_COMM_PTR = newcomm;
    }

    //
    // Find the current process rank and the size of the process pool.
    //
    MPI_Comm_rank (VISIT_MPI_COMM, &par_rank);
    MPI_Comm_size (VISIT_MPI_COMM, &par_size);

    // Replace the communicator that we had in the global context.
    globalContext.SetCommunicator(VISIT_MPI_COMM);

    return true;
#endif
}

// ****************************************************************************
//  Function: PAR_Rank
//
//  Purpose:
//      Returns the rank of this processor.
//
//  Programmer: Hank Childs
//  Creation:   November 19, 2002
//
//  Modifications:
//    Eric Brugger, Tue Aug 31 10:27:20 PDT 2004
//    Modified the routine to return the cached rank.
//
// ****************************************************************************

int
PAR_Rank(void)
{
    return par_rank;
}


// ****************************************************************************
//  Function: PAR_Size
//
//  Purpose:
//      Returns the number of processors.
//
//  Programmer: Hank Childs
//  Creation:   November 19, 2002
//
//  Modifications:
//    Eric Brugger, Tue Aug 31 10:27:20 PDT 2004
//    Modified the routine to return the cached size.
//
// ****************************************************************************

int
PAR_Size(void)
{
    return par_size;
}


// ****************************************************************************
//  Function: PAR_UIProcess
//
//  Purpose:
//      Returns true if the process's rank is 0.
//
//  Programmer: Brad Whitlock
//  Creation:   Thu Jul 13 11:47:10 PDT 2000
//
// ****************************************************************************

bool
PAR_UIProcess(void)
{
    return (par_rank == 0);
}

// ****************************************************************************
//  Function: PAR_WaitForDebugger
//
//  Purpose:
//      Spins indefinitely until a developer loads up a debugger on process 0
//      and fixes it.
//
//  Programmer: Tom Fogal
//  Creation:   October 17, 2008
//
//  Modifications:
//
//    Tom Fogal, Wed Jun 10 15:55:35 MDT 2009
//    Wrap in ifdef parallel.
//
// ****************************************************************************

void
PAR_WaitForDebugger(void)
{
#ifdef PARALLEL
    volatile int i = 0;
    if(PAR_Rank() == 0)
    {
        do {
            // nothing
        } while(i == 0);
    }
    Barrier();
#endif
}

// ****************************************************************************
//  Function: UnifyMinMax
//
//  Purpose:
//      Makes a collective call across all processors to unify an array that
//      has alternating minimum and maximum values.
//
//  Arguments:
//      buff     An array that contains alternating min's and max's.
//      size     The size of exts (this clearly must be even).
//
//  Programmer:  Hank Childs
//  Creation:    June 18, 2001
//
//  Modifications
//
//    Mark C. Miller, Thu Jan 29 21:26:10 PST 2004
//    Modified to use a single all reduce with the user-defined reduction
//    operator. Also modified to accept an alternate size to use if there is
//    a chance that all processors don't agree on size upon entering. ALL
//    PROCESSORS MUST AGREE ON altsize THOUGH. An altsize of zero means to
//    use size (e.g. operate as normally). An altsize > 0 means to use
//    the alternate size for any communication, but truncate the result to
//    size upon exit. An altsize of -1 means to do an initial communication
//    to get all processors to agree on a size before proceeding.
//
//    Mark C. Miller, Mon Jan 22 22:09:01 PST 2007
//    Changed MPI_COMM_WORLD to VISIT_MPI_COMM
// ****************************************************************************

/* ARGSUSED */
void
UnifyMinMax(double *buff, int size, int altsize)
{
    globalContext.UnifyMinMax(buff, size, altsize);
}

// ****************************************************************************
//  Function: UnifyMinimumValue
//
//  Purpose:
//      Makes a collective call across all processors to unify the minimum
//      value over all processors.
//
//  Arguments:
//      mymin    The minimum on this processor.
//
//  Returns:     The minimum over all processors.
//
//  Programmer:  Dave Pugmire
//  Creation:    May 19, 2008
//
// ****************************************************************************

int
UnifyMinimumValue(int mymin)
{
    return globalContext.UnifyMinimumValue(mymin);
}

// ****************************************************************************
//  Function: UnifyMinimumValue
//
//  Purpose:
//      Makes a collective call across all processors to unify the minimum
//      value over all processors.
//
//  Arguments:
//      mymin    The minimum on this processor.
//
//  Returns:     The minimum over all processors.
//
//  Programmer:  Dave Pugmire
//  Creation:    May 19, 2008
//
// ****************************************************************************

float
UnifyMinimumValue(float mymin)
{
    return globalContext.UnifyMinimumValue(mymin);
}

// ****************************************************************************
//  Function: UnifyMinimumValue
//
//  Purpose:
//      Makes a collective call across all processors to unify the minimum
//      value over all processors.
//
//  Arguments:
//      mymin    The minimum on this processor.
//
//  Returns:     The minimum over all processors.
//
//  Programmer:  Dave Pugmire
//  Creation:    May 19, 2008
//
// ****************************************************************************

double
UnifyMinimumValue(double mymin)
{
    return globalContext.UnifyMinimumValue(mymin);
}

// ****************************************************************************
//  Function: UnifyMaximumValue
//
//  Purpose:
//      Makes a collective call across all processors to unify the maximum
//      value over all processors.
//
//  Arguments:
//      mymax    The maximum on this processor.
//
//  Returns:     The maximum over all processors.
//
//  Programmer:  Hank Childs
//  Creation:    February 6, 2002
//
//  Modifications:
//
//    Mark C. Miller, Mon Jan 22 22:09:01 PST 2007
//    Changed MPI_COMM_WORLD to VISIT_MPI_COMM
// ****************************************************************************

int
UnifyMaximumValue(int mymax)
{
    return globalContext.UnifyMaximumValue(mymax);
}

// ****************************************************************************
//  Function: UnifyMaximumValue
//
//  Purpose:
//      Makes a collective call across all processors to unify the maximum
//      value over all processors.
//
//  Arguments:
//      mymax    The maximum on this processor.
//
//  Returns:     The maximum over all processors.
//
//  Programmer:  Dave Pugmire
//  Creation:    May 19, 2008
//
// ****************************************************************************

float
UnifyMaximumValue(float mymax)
{
    return globalContext.UnifyMaximumValue(mymax);
}

// ****************************************************************************
//  Function: UnifyMaximumValue
//
//  Purpose:
//      Makes a collective call across all processors to unify the maximum
//      value over all processors.
//
//  Arguments:
//      mymax    The maximum on this processor.
//
//  Returns:     The maximum over all processors.
//
//  Programmer:  Dave Pugmire
//  Creation:    May 19, 2008
//
// ****************************************************************************

double
UnifyMaximumValue(double mymax)
{
    return globalContext.UnifyMaximumValue(mymax);
}

// ****************************************************************************
//  Function: Collect
//
//  Purpose:
//      Takes the buffer from a specific processor and take the maximum entry
//      in each buffer.
//
//  Arguments:
//      buff    The buffer.
//      size    The length of the buffer.
//
//  Returns:    Whether or not the buffer is up-to-date for this processor.  It
//              only reduces the data onto processor 0 (but this makes it
//              convenient that derived types don't have to worry about MPI).
//
//  Programmer: Hank Childs
//  Creation:   June 18, 2001
//
//  Modifications:
//
//    Hank Childs, Tue Dec 18 07:55:21 PST 2001
//    Fix memory leak.
//
//    Mark C. Miller, Mon Jan 22 22:09:01 PST 2007
//    Changed MPI_COMM_WORLD to VISIT_MPI_COMM
// ****************************************************************************

bool
Collect(float *buff, int size)
{
    return globalContext.Collect(buff, size);
}

bool
Collect(double *buff, int size)
{
    return globalContext.Collect(buff, size);
}

bool
Collect(int *buff, int size)
{
    return globalContext.Collect(buff, size);
}

// ****************************************************************************
//  Function: Barrier
//
//  Purpose:
//      Acts as a barrier.  This is sometimes useful in determining
//      bottlenecks.
//
//  Programmer: Hank Childs
//  Creation:   November 28, 2001
//
//  Modifications:
//
//    Mark C. Miller, Mon Jan 22 22:09:01 PST 2007
//    Changed MPI_COMM_WORLD to VISIT_MPI_COMM
// ****************************************************************************

void
Barrier(void)
{
    globalContext.Barrier();
}

// ****************************************************************************
//  Function: SumIntArrayAcrossAllProcessors
//
//  Purpose:
//      Sums an array of integers across all processors.
//
//  Arguments:
//      inArray    The input.
//      outArray   The sums of the inArrays across all processors.
//      nArray     The number of entries in inArray/outArray.
//
//  Programmer:    Hank Childs
//  Creation:      January 1, 2002
//
//  Modifications:
//
//    Mark C. Miller, Mon Jan 22 22:09:01 PST 2007
//    Changed MPI_COMM_WORLD to VISIT_MPI_COMM
// ****************************************************************************

void
SumIntArrayAcrossAllProcessors(int *inArray, int *outArray, int nArray)
{
    globalContext.SumIntArrayAcrossAllProcessors(inArray, outArray, nArray);
}

// ****************************************************************************
//  Function: SumLongLongArrayAcrossAllProcessors
//
//  Purpose:
//      Sums an array of long longs across all processors.
//
//  Arguments:
//      inArray    The input.
//      outArray   The sums of the inArrays across all processors.
//      nArray     The number of entries in inArray/outArray.
//
//  Programmer:    Jeremy Meredith
//  Creation:      February 22, 2008
//
//  Modifications:
//    Mark C. Miller, Tue Jan 27 18:34:41 PST 2009
//    MPI_Type_get_extent is only in MPI-2. Likewise for
//    MPI_UNSIGNED_LONG_LONG. So, I made the first check conditional
//    on MPI_UNSIGNED_LONG_LONG being defined.
//
//    Gunther H. Weber, Mon Apr  6 20:50:50 PDT 2009
//    Check whether MPI_INTEGER8 defined even if MPI_UNSIGNED_LONG_LONG
//    is defined.
//
//    Brad Whitlock, Mon Apr 20 12:06:25 PDT 2009
//    Check MPI_VERSION and MPI_SUBVERSION before using MPI_Type_get_extent.
//
// ****************************************************************************

void
SumLongLongArrayAcrossAllProcessors(VISIT_LONG_LONG *inArray,
                                    VISIT_LONG_LONG *outArray, int nArray)
{
    globalContext.SumLongLongArrayAcrossAllProcessors(inArray, outArray, nArray);
}

// ****************************************************************************
//  Function: SumDoubleArrayAcrossAllProcessors
//
//  Purpose:
//      Sums an array of doubles across all processors.
//
//  Arguments:
//      inArray    The input.
//      outArray   The sums of the inArrays across all processors.
//      nArray     The number of entries in inArray/outArray.
//
//  Programmer:    Hank Childs
//  Creation:      August 30, 2002
//
//  Modifications:
//
//    Mark C. Miller, Mon Jan 22 22:09:01 PST 2007
//    Changed MPI_COMM_WORLD to VISIT_MPI_COMM
// ****************************************************************************

void
SumDoubleArrayAcrossAllProcessors(double *inArray, double *outArray,int nArray)
{
    globalContext.SumDoubleArrayAcrossAllProcessors(inArray, outArray, nArray);
}

// ****************************************************************************
// Function:  SumDoubleArray
//
// Purpose:
//   Sum an array across all procs, and leave result on process 0.
//
// Programmer:  Dave Pugmire
// Creation:    November 23, 2011
//
// ****************************************************************************


void
SumDoubleArray(double *inArray, double *outArray, int nArray)
{
    globalContext.SumDoubleArray(inArray, outArray, nArray);
}

// ****************************************************************************
// Function:  SumDoubleArrayInPlace
//
// Purpose:
//   Allow the root proc (0) to reuse its input buffer to store the sum
//   accross all procs. The input data is taken from inOutArray and then replaced
//   by the output data. This method should ONLY be called by the root proc (0), all
//   other procs should call SumDoubleArray.
//
// Arguments:
//   inOutArray The root proc input data and also where the overall sum (output data)
//               will be placed.
//   nArray     The number of entries in inOutArray
//
// Programmer:  Kevin Griffin
// Creation:    July 7, 2015
//
// ****************************************************************************


void
SumDoubleArrayInPlace(double *inOutArray, int nArray)
{
    globalContext.SumDoubleArrayInPlace(inOutArray, nArray);
}

// ****************************************************************************
//  Function: SumFloatArrayAcrossAllProcessors
//
//  Purpose:
//      Sums an array of floats across all processors.
//
//  Arguments:
//      inArray    The input.
//      outArray   The sums of the inArrays across all processors.
//      nArray     The number of entries in inArray/outArray.
//
//  Programmer:    Hank Childs
//  Creation:      June 26, 2002
//
//  Modifications:
//
//    Mark C. Miller, Mon Jan 22 22:09:01 PST 2007
//    Changed MPI_COMM_WORLD to VISIT_MPI_COMM
// ****************************************************************************

void
SumFloatArrayAcrossAllProcessors(float *inArray, float *outArray, int nArray)
{
    globalContext.SumFloatArrayAcrossAllProcessors(inArray, outArray, nArray);
}

// ****************************************************************************
// Function:  SumFloatArray
//
// Purpose:
//   Sum an array across all procs, and leave result on process 0.
//
// Programmer:  Dave Pugmire
// Creation:    November 23, 2011
//
// ****************************************************************************


void
SumFloatArray(float *inArray, float *outArray, int nArray)
{
    globalContext.SumFloatArray(inArray, outArray, nArray);
}

// ****************************************************************************
//  Function: SumFloatAcrossAllProcessors
//
//  Purpose:
//      Sums a single float across all processors.
//
//  Arguments:
//      value      The input and output.
//
//  Programmer:    Jeremy Meredith
//  Creation:      April 12, 2003
//
//  Modifications:
//
//    Mark C. Miller, Mon Jan 22 22:09:01 PST 2007
//    Changed MPI_COMM_WORLD to VISIT_MPI_COMM
// ****************************************************************************

void
SumFloatAcrossAllProcessors(float &value)
{
    globalContext.SumFloatAcrossAllProcessors(value);
}

// ****************************************************************************
//  Function: UnifyMinimumDoubleArrayAcrossAllProcessors
//
//  Purpose:
//      Finds the minimum for each element of an array of doubles
//      across all processors.
//
//  Arguments:
//      inArray    The input.
//      outArray   The minimums of the inArrays across all processors.
//      nArray     The number of entries in inArray/outArray.
//
//  Programmer:    Cyrus Harrison
//  Creation:      March 27, 2009
//
//  Modifications:
//
// ****************************************************************************

void
UnifyMinimumDoubleArrayAcrossAllProcessors(double *inArray, double *outArray,
                                           int nArray)
{
    globalContext.UnifyMinimumDoubleArrayAcrossAllProcessors(inArray, outArray, nArray);
}

// ****************************************************************************
//  Function: UnifyMinimumFloatArrayAcrossAllProcessors
//
//  Purpose:
//      Finds the minimum for each element of an array of floats
//      across all processors.
//
//  Arguments:
//      inArray    The input.
//      outArray   The minimums of the inArrays across all processors.
//      nArray     The number of entries in inArray/outArray.
//
//  Programmer:    Hank Childs
//  Creation:      February 20, 2006
//
//  Modifications:
//
//    Mark C. Miller, Mon Jan 22 22:09:01 PST 2007
//    Changed MPI_COMM_WORLD to VISIT_MPI_COMM
// ****************************************************************************

void
UnifyMinimumFloatArrayAcrossAllProcessors(float *inArray, float *outArray,
                                          int nArray)
{
    globalContext.UnifyMinimumFloatArrayAcrossAllProcessors(inArray, outArray, nArray);
}

// ****************************************************************************
//  Function: UnifyMaximumDoubleArrayAcrossAllProcessors
//
//  Purpose:
//      Finds the maximum for each element of an array of doubles
//      across all processors.
//
//  Arguments:
//      inArray    The input.
//      outArray   The maximums of the inArrays across all processors.
//      nArray     The number of entries in inArray/outArray.
//
//  Programmer:    Cyrus Harrison
//  Creation:      March 27, 2009
//
//  Modifications:
//
// ****************************************************************************

void
UnifyMaximumDoubleArrayAcrossAllProcessors(double *inArray, double *outArray,
                                            int nArray)
{
    globalContext.UnifyMaximumDoubleArrayAcrossAllProcessors(inArray, outArray, nArray);
}

// ****************************************************************************
//  Function: UnifyMaximumFloatArrayAcrossAllProcessors
//
//  Purpose:
//      Finds the maximum for each element of an array of floats
//      across all processors.
//
//  Arguments:
//      inArray    The input.
//      outArray   The maximums of the inArrays across all processors.
//      nArray     The number of entries in inArray/outArray.
//
//  Programmer:    Hank Childs
//  Creation:      February 20, 2006
//
//  Modifications:
//
//    Mark C. Miller, Mon Jan 22 22:09:01 PST 2007
//    Changed MPI_COMM_WORLD to VISIT_MPI_COMM
// ****************************************************************************

void
UnifyMaximumFloatArrayAcrossAllProcessors(float *inArray, float *outArray,
                                          int nArray)
{
    globalContext.UnifyMaximumFloatArrayAcrossAllProcessors(inArray, outArray, nArray);
}

// ****************************************************************************
//  Function: SumIntAcrossAllProcessors
//
//  Purpose:
//      Sums a single int across all processors.
//
//  Arguments:
//      value      The input and output.
//
//  Programmer:    Jeremy Meredith
//  Creation:      April 17, 2003
//
//  Modifications:
//
//    Mark C. Miller, Mon Jan 22 22:09:01 PST 2007
//    Changed MPI_COMM_WORLD to VISIT_MPI_COMM
// ****************************************************************************

void
SumIntAcrossAllProcessors(int &value)
{
    globalContext.SumIntAcrossAllProcessors(value);
}

// ****************************************************************************
//  Function: SumLongAcrossAllProcessors
//
//  Purpose:
//      Sums a single int across all processors.
//
//  Arguments:
//      value      The input and output.
//
//  Programmer:    Jeremy Meredith
//  Creation:      April 17, 2003
//
//  Modifications:
//
// ****************************************************************************

void
SumLongAcrossAllProcessors(long &value)
{
    globalContext.SumLongAcrossAllProcessors(value);
}

// ****************************************************************************
//  Function: SumDoubleAcrossAllProcessors
//
//  Purpose:
//      Sums a single double across all processors.
//
//  Arguments:
//      value      The input and output.
//
//  Programmer:    Jeremy Meredith
//  Creation:      April 12, 2003
//
//  Modifications:
//
//    Mark C. Miller, Mon Jan 22 22:09:01 PST 2007
//    Changed MPI_COMM_WORLD to VISIT_MPI_COMM
// ****************************************************************************

void
SumDoubleAcrossAllProcessors(double &value)
{
    globalContext.SumDoubleAcrossAllProcessors(value);
}

// ****************************************************************************
//  Function: ThisProcessorHasMinimumValue
//
//  Purpose:
//      Determines if this processor has the minimum value over all the
//      processors.
//
//  Arguments:
//      min     The minimum for this processor.
//
//  Returns:    true if it does have the minimum, false otherwise.
//
//  Programmer: Hank Childs
//  Creation:   March 15, 2002
//
//  Modifications:
//    Kathleen Bonnell, Mon Jan  6 11:01:09 PST 2003
//    The rcount argument of MPI_Allgather should be the number of elements
//    received from any processor, not the size of the receiving buffer.
//
//    Kathleen Bonnell, Mon Apr  7 16:20:20 PDT 2003
//    Changed theBestMin from bool to double so this method will
//    work properly.
//
//    Mark C. Miller, Thu Aug 19 15:58:19 PDT 2004
//    Fixed memory leak for allValues by re-implementing using more-efficient
//    MPI_Allreduce and the MINLOC operator. Where before the processor of
//    maximum rank with minimum value would return true, now the processor of
//    minimum rank with minimum value will return true. Otherwise, behavior
//    is identical. And, now there is no need to allocate an array of size
//    numProcs or do an expensive Allgather.
//
//    Mark C. Miller, Mon Jan 22 22:09:01 PST 2007
//    Changed MPI_COMM_WORLD to VISIT_MPI_COMM
// ****************************************************************************

bool
ThisProcessorHasMinimumValue(double min)
{
    return globalContext.ThisProcessorHasMinimumValue(min);
}

// ****************************************************************************
//  Function: ThisProcessorHasMaximumValue
//
//  Purpose:
//      Determines if this processor has the maximum value over all the
//      processors.
//
//  Arguments:
//      max     The maximum for this processor.
//
//  Returns:    true if it does have the maximum, false otherwise.
//
//  Programmer: Kathleen Bonnell
//  Creation:   October 27, 2003
//
//  Modifications:
//
//    Mark C. Miller, Thu Aug 19 15:58:19 PDT 2004
//    Fixed memory leak for allValues by re-implementing using more-efficient
//    MPI_Allreduce and the MAXLOC operator. Where before the processor of
//    maximum rank with minimum value would return true, now the processor of
//    minimum rank with minimum value will return true. Otherwise, behavior
//    is identical. And, now there is no need to allocate an array of size
//    numProcs or do an expensive Allgather.
//
//    Mark C. Miller, Mon Jan 22 22:09:01 PST 2007
//    Changed MPI_COMM_WORLD to VISIT_MPI_COMM
// ****************************************************************************

bool
ThisProcessorHasMaximumValue(double max)
{
    return globalContext.ThisProcessorHasMaximumValue(max);
}

// ****************************************************************************
//  Function:  BroadcastInt
//
//  Purpose:
//    Broadcast an integer from processor 0 to all other processors
//
//  Arguments:
//    i          the int
//
//  Programmer:  Jeremy Meredith
//  Creation:    July 15, 2003
//
//  Modifications:
//
//    Mark C. Miller, Mon Jan 22 22:09:01 PST 2007
//    Changed MPI_COMM_WORLD to VISIT_MPI_COMM
// ****************************************************************************
void BroadcastInt(int &value)
{
    globalContext.BroadcastInt(value);
}

// ****************************************************************************
//  Function:  BroadcastLongLong
//
//  Purpose:
//    Broadcast a long long from processor 0 to all other processors
//
//  Arguments:
//    l          the long long
//
//  Programmer:  Hank Childs
//  Creation:    December 10, 2012
//
// ****************************************************************************
void BroadcastLongLong(VISIT_LONG_LONG &value)
{
    globalContext.BroadcastLongLong(value);
}

// ****************************************************************************
// Function: BroadcastIntArray
//
// Purpose:
//   Broadcast an array of int from processor 0 to all other processors.
//
// Arguments:
//   array  : The array to send (or receive on non-rank-0).
//   nArray : The number of values to send/receive.
//
// Returns:
//
// Note:
//
// Programmer: Brad Whitlock
// Creation:   Fri May 20 15:00:02 PDT 2011
//
// Modifications:
//
// ****************************************************************************

void BroadcastIntArray(int *array, int nArray)
{
    globalContext.BroadcastIntArray(array, nArray);
}

// ****************************************************************************
//  Function:  BroadcastIntVector
//
//  Purpose:
//    Broadcast a vector<int> from processor 0 to all other processors
//
//  Arguments:
//    vi         the vector<int>
//    myrank     the rank of this process
//
//  Programmer:  Jeremy Meredith
//  Creation:    July 15, 2003
//
//  Modifications:
//    Mark C. Miller, Mon Jan 22 22:09:01 PST 2007
//    Changed MPI_COMM_WORLD to VISIT_MPI_COMM
//
//    Tom Fogal, Mon May 25 16:05:23 MDT 2009
//    Added a check for empty vectors.
//
// ****************************************************************************
void BroadcastIntVector(std::vector<int> &vi, int myrank)
{
    globalContext.BroadcastIntVector(vi, myrank);
}

// ****************************************************************************
//  Function:  BroadcastIntVectorFromAny
//
//  Purpose:
//    Broadcast a vector<int> from processor root to all other processors
//
//  Arguments:
//    vi         the vector<int>
//    myrank     the rank of this process
//    root       the rank of the broadcast root
//
//  Programmer:  Kevin Griffin
//  Creation:    Mon Aug 3 15:03:38 PDT 2015
//
//  Modifications:
//
// ****************************************************************************
void BroadcastIntVectorFromAny(std::vector<int> &vi, int myrank, int root)
{
    globalContext.BroadcastIntVectorFromAny(vi, myrank, root);
}

// ****************************************************************************
//  Function:  BroadcastBool
//
//  Purpose: Broadcast a bool from processor 0 to all other processors
//
//  Arguments:
//    b          reference to the bool
//
//  Programmer:  Mark C. Miller
//  Creation:    June 7, 2005
//
// ****************************************************************************
void BroadcastBool(bool &b)
{
    globalContext.BroadcastBool(b);
}

// ****************************************************************************
//  Function:  BroadcastBoolVector
//
//  Purpose:
//    Broadcast a vector<bool> from processor 0 to all other processors
//
//  Arguments:
//    vi         the vector<bool>
//    myrank     the rank of this process
//
//  Programmer:  Brad Whitlock
//  Creation:    Thu Jun 18 12:02:26 PDT 2009
//
//  Modifications:
//
// ****************************************************************************
void BroadcastBoolVector(std::vector<bool> &vb, int myrank)
{
    globalContext.BroadcastBoolVector(vb, myrank);
}

// ****************************************************************************
//  Function:  BroadcastDouble
//
//  Purpose:
//    Broadcast an double from processor 0 to all other processors
//
//  Arguments:
//    i          the double
//
//  Programmer:  Hank Childs
//  Creation:    June 6, 2005
//
//  Modifications:
//
//    Mark C. Miller, Mon Jan 22 22:09:01 PST 2007
//    Changed MPI_COMM_WORLD to VISIT_MPI_COMM
// ****************************************************************************
void BroadcastDouble(double &value)
{
    globalContext.BroadcastDouble(value);
}

// ****************************************************************************
// Function: BroadcastDoubleArray
//
// Purpose:
//   Broadcast an array of double from processor 0 to all other processors.
//
// Arguments:
//   array  : The array to send (or receive on non-rank-0).
//   nArray : The number of values to send/receive.
//
// Returns:
//
// Note:
//
// Programmer: Brad Whitlock
// Creation:   Fri May 20 15:00:02 PDT 2011
//
// Modifications:
//
// ****************************************************************************

void BroadcastDoubleArray(double *array, int nArray)
{
    globalContext.BroadcastDoubleArray(array, nArray);
}

// ****************************************************************************
//  Function:  BroadcastDoubleArrayFromAny
//
//  Purpose:
//    Broadcast a vector<double> from processor root to all other processors
//
//  Arguments:
//    array      The array to send (or receive on non-root).
//    nArray     The number of values to send/receive.
//    myrank     the rank of this process
//    root       the rank of the broadcast root
//
//
//  Programmer:  Kevin Griffin
//  Creation:    Mon Aug 3 15:03:38 PDT 2015
//
//  Modifications:
//
// ****************************************************************************
void BroadcastDoubleArrayFromAny(double *array, int nArray, int root)
{
    globalContext.BroadcastDoubleArrayFromAny(array, nArray, root);
}

// ****************************************************************************
//  Function:  BroadcastDoubleVector
//
//  Purpose:
//    Broadcast a vector<double> from processor 0 to all other processors
//
//  Arguments:
//    vi         the vector<double>
//    myrank     the rank of this process
//
//  Programmer:  Hank Childs
//  Creation:    June 6, 2005
//
//  Modifications:
//
//    Hank Childs, Thu Jun 23 14:02:03 PDT 2005
//    Change type of "myrank" to be int.  Too much cut-n-paste previously.
//
//    Tom Fogal, Mon May 25 16:06:09 MDT 2009
//    Added check to make sure we don't try to broadcast an empty vector.
//
// ****************************************************************************
void BroadcastDoubleVector(std::vector<double> &vi, int myrank)
{
    globalContext.BroadcastDoubleVector(vi, myrank);
}

// ****************************************************************************
//  Function:  BroadcastDoubleVectorFromAny
//
//  Purpose:
//    Broadcast a vector<double> from processor root to all other processors
//
//  Arguments:
//    vi         the vector<int>
//    myrank     the rank of this process
//    root       the rank of the broadcast root
//
//  Programmer:  Kevin Griffin
//  Creation:
//
//  Modifications:
//
// ****************************************************************************
void BroadcastDoubleVectorFromAny(std::vector<double> &vi, int myrank, int root)
{
    globalContext.BroadcastDoubleVectorFromAny(vi, myrank, root);
}

// ****************************************************************************
//  Function:  BroadcastString
//
//  Purpose:
//    Broadcast an STL string from processor 0 to all other processors
//
//  Arguments:
//    s          the string
//    myrank     the rank of this process
//
//  Programmer:  Jeremy Meredith
//  Creation:    July 15, 2003
//
//  Modifications:
//
//    Mark C. Miller, Mon Jan 22 22:09:01 PST 2007
//    Changed MPI_COMM_WORLD to VISIT_MPI_COMM
// ****************************************************************************
void BroadcastString(std::string &s, int myrank)
{
    globalContext.BroadcastString(s, myrank);
}

// ****************************************************************************
//  Function:  BroadcastStringVector
//
//  Purpose:
//    Broadcast an vector<string> from processor 0 to all other processors
//
//  Arguments:
//    vs         the vector<string>
//    myrank     the rank of this process
//
//  Programmer:  Jeremy Meredith
//  Creation:    July 15, 2003
//
//  Modifications:
//
//    Hank Childs, Mon Jun  6 17:13:08 PDT 2005
//    Re-implemented to improve efficiency for vectors with lots of strings.
//
//    Mark C. Miller, Mon Jan 22 22:09:01 PST 2007
//    Changed MPI_COMM_WORLD to VISIT_MPI_COMM
//
//    Tom Fogal, Mon May 25 15:53:31 MDT 2009
//    Added a check for empty string vectors.
//
// ****************************************************************************
void BroadcastStringVector(std::vector<std::string> &vs, int myrank)
{
    globalContext.BroadcastStringVector(vs, myrank);
}

// ****************************************************************************
//  Function:  BroadcastStringVectorVector
//
//  Purpose:
//    Broadcast a vector<vector<string>> from processor 0 to all
//    other processors
//
//  Arguments:
//    vvs        the vector<vector<string>>
//    myrank     the rank of this process
//
//  Programmer:  Jeremy Meredith
//  Creation:    July 15, 2003
//
//  Modifications:
//
//    Mark C. Miller, Mon Jan 22 22:09:01 PST 2007
//    Changed MPI_COMM_WORLD to VISIT_MPI_COMM
//
//    Tom Fogal, Mon May 25 16:07:08 MDT 2009
//    Added check to make sure we don't try to broadcast an empty vector.
//
// ****************************************************************************
void BroadcastStringVectorVector(std::vector< std::vector<std::string> > &vvs, int myrank)
{
    globalContext.BroadcastStringVectorVector(vvs, myrank);
}

// ****************************************************************************
//  Function: GetListToRootProc
//
//  Purpose:
//      Gets a variable list to processor 0.  At the beginning of this call,
//      it is not clear which processor has a good list.
//
//  Programmer: Hank Childs
//  Creation:   August 8, 2003
//
//  Modifications:
//
//    Mark C. Miller, Wed Jun  9 21:50:12 PDT 2004
//    Eliminated use of MPI_ANY_TAG and modified to use GetUniqueMessageTags
//
//    Mark C. Miller, Mon Jan 22 22:09:01 PST 2007
//    Changed MPI_COMM_WORLD to VISIT_MPI_COMM
// ****************************************************************************

bool GetListToRootProc(std::vector<std::string> &vars, int total)
{
    return globalContext.GetListToRootProc(vars, total);
}

// ****************************************************************************
//  Function: CollectIntArraysOnRootProc
//
//  Purpose:
//      Collects a collection of arrays from all the processors on the root
//      process.  The arrays can be of different sizes.  The receiveBuf and
//      receiveCounts are allocated in this routine and must be deleted by
//      the caller.
//
//  Programmer: Eric Brugger
//  Creation:   June 22, 2009
//
//  Modifications:
//    Brad Whitlock, Fri May 20 13:54:46 PDT 2011
//    I moved the body into a static helper that I templated so I could add
//    another version that uses doubles.
//
// ****************************************************************************

void
CollectIntArraysOnRootProc(int *&receiveBuf, int *&receiveCounts,
    int *sendBuf, int sendCount)
{
    globalContext.CollectIntArraysOnRootProc(receiveBuf, receiveCounts, sendBuf, sendCount);
}

void
CollectDoubleArraysOnRootProc(double *&receiveBuf, int *&receiveCounts,
    double *sendBuf, int sendCount)
{
    globalContext.CollectDoubleArraysOnRootProc(receiveBuf, receiveCounts, sendBuf, sendCount);
}

// ****************************************************************************
//  Function: GetUniqueMessageTag
//
//  Purpose: Returns a suitable, unique message tag to be used in MPI_Send/Recv
//           Calls. NOTE, IF ANY PROCESSOR CALLS THIS FUNCTION FROM SOME POINT
//           IN VISIT'S EXECUTION, THEN ALL MUST CALL IT FROM THAT POINT.
//           Otherwise, processors won't agree on message tags
//
//  Programmer: Mark C. Miller
//  Creation:   June 9, 2004
//
//  Modifications:
//    Kathleen Bonnell, Wed Sep  8 15:08:00 PDT 2004
//    Use mpiTagUpperBound instead of MPI_TAG_UB (which was being used
//    incorrectly).  Also use MIN_TAG_VALUE.
//
// ****************************************************************************

int GetUniqueMessageTag()
{
    return globalContext.GetUniqueMessageTag();
}

void GetUniqueMessageTags(int *tags, int ntags)
{
    return globalContext.GetUniqueMessageTags(tags, ntags);
}

// ****************************************************************************
//  Function: GetUniqueStaticMessageTag
//
//  Purpose: Returns a suitable, unique message tag to be used in MPI_Send/Recv
//           Calls. This is to be used for tags that must be initialized
//           before PAR_Init is called -- e.g. for static message tags.
//
//  Programmer: Kathleen Bonnell
//  Creation:   September 9, 2004
//
//  Modifications:
//
// ****************************************************************************

int GetUniqueStaticMessageTag()
{
    return globalContext.GetUniqueStaticMessageTag();
}

// ****************************************************************************
//  Function: GetAttToRootProc
//
//  Purpose:
//    Gets an attribute to processor 0.
//
//  Arguments:
//    att       The att to get to proc 0.
//    hasAtt    Inidicates whether his processor has the attribute or not.
//
//  Programmer: Kathleen Bonnell
//  Creation:   July 6, 2004
//
//  Modifications:
//
//    Mark C. Miller, Mon Jan 22 22:09:01 PST 2007
//    Changed MPI_COMM_WORLD to VISIT_MPI_COMM
//
//    Burlen Loring, Mon Aug  3 09:48:30 PDT 2015
//    Use MPI collectives
//
// ****************************************************************************

void
GetAttToRootProc(AttributeGroup &att, int hasAtt)
{
    globalContext.GetAttToRootProc(att, hasAtt);
}

// ****************************************************************************
//  Function: GetFloatArrayToRootProc
//
//  Purpose:
//    Gets a float array to processor 0.
//
//  Arguments:
//    fa        The  float array.
//    nf        The number of items in the array.
//    success   Inidicates whether or not this processor has the float array.
//
//  Programmer: Kathleen Bonnell
//  Creation:   July 6, 2004
//
//  Modifications:
//
//    Mark C. Miller, Mon Jan 22 22:09:01 PST 2007
//    Changed MPI_COMM_WORLD to VISIT_MPI_COMM
// ****************************************************************************

void
GetFloatArrayToRootProc(float *fa, int nf, bool &success)
{
    globalContext.GetFloatArrayToRootProc(fa, nf, success);
}

// ****************************************************************************
//  Function: UnifyMaximumValue
//
//  Purpose:
//      Makes a collective call across all processors to unify the maximum
//      values in the intVector over all processors.
//
//  Arguments:
//      mymax    The maximum values on this processor.
//      results  The maximum values over all processors.
//
//  Returns:     The maximum over all processors.
//
//  Programmer:  Kathleen Bonnell
//  Creation:    November 9, 2004
//
//  Modifications:
//
//    Mark C. Miller, Mon Jan 22 22:09:01 PST 2007
//    Changed MPI_COMM_WORLD to VISIT_MPI_COMM
// ****************************************************************************

/* ARGSUSED */
void
UnifyMaximumValue(std::vector<int> &mymax, std::vector<int> &results)
{
    globalContext.UnifyMaximumValue(mymax, results);
}

// ****************************************************************************
//  Function: GetDoubleArrayToRootProc
//
//  Purpose:
//    Gets a double array to processor 0.
//
//  Arguments:
//    da        The double array.
//    nd        The number of items in the array.
//    success   Inidicates whether or not this processor has the double array.
//
//  Programmer: Kathleen Bonnell
//  Creation:   March 13, 2006
//
//  Modifications:
//
//    Mark C. Miller, Mon Jan 22 22:09:01 PST 2007
//    Changed MPI_COMM_WORLD to VISIT_MPI_COMM
//
//    Hank Childs, Tue Sep  4 17:00:21 PDT 2007
//    Cast a Boolean to an int, to make sure that a cast to a pointer was
//    correctly word aligned.
//
// ****************************************************************************

void
GetDoubleArrayToRootProc(double *da, int nd, bool &success)
{
    globalContext.GetDoubleArrayToRootProc(da, nd, success);
}

// ****************************************************************************
//  Function: WaitAll
//
//  Purpose:
//    Wait on all requests in a vector.
//
//  Arguments:
//    req       Vector of requests.
//    status    Vector of returned status.
//
//  Programmer: Dave Pugmire
//  Creation:   June 18, 2008
//
// ****************************************************************************

void
WaitAll(std::vector<int> &reqs, std::vector<int> &status )
{
    globalContext.WaitAll(reqs, status);
}

// ****************************************************************************
//  Function: WaitSome
//
//  Purpose:
//    Wait on a some requests in a vector.
//
//  Arguments:
//    req       Vector of requests.
//    done      Vector of completed requests.
//    status    Vector of returned status.
//
//  Programmer: Dave Pugmire
//  Creation:   June 18, 2008
//
// ****************************************************************************

void
WaitSome(std::vector<int> &reqs, std::vector<int> &done, std::vector<int> &status )
{
    globalContext.WaitSome(reqs, done, status);
}

// ****************************************************************************
//  Function: TestSome
//
//  Purpose:
//    Test a vector of requests.
//
//  Arguments:
//    req       Vector of requests.
//    done      Vector of completed requests.
//    status    Vector of returned status.
//
//  Programmer: Dave Pugmire
//  Creation:   June 18, 2008
//
//
// ****************************************************************************

void
TestSome(std::vector<int> &reqs, std::vector<int> &done, std::vector<int> &status )
{
    globalContext.TestSome(reqs, done, status);
}

// ****************************************************************************
//  Function: CancelRequest
//
//  Purpose:
//    Cancel a request.
//
//  Arguments:
//    req       The request.
//
//  Programmer: Dave Pugmire
//  Creation:   June 18, 2008
//
//  Modifications:
//
//    Tom Fogal, Wed Aug  6 11:36:21 EDT 2008
//    Changed the argument type and added a cast to avoid a type mismatch.
//
// ****************************************************************************

void
CancelRequest(void *req)
{
    globalContext.CancelRequest(req);
}

// ****************************************************************************
//  Function: PullInMPI_IOSymbols()
//
//  Purpose:
//    Don't call this, this is just to pull in MPI IO symbols.
//
//  Arguments:
//
//  Programmer: Dave Pugmire
//  Creation:   September 17, 2009
//
//  Modifications:
//
//
// ****************************************************************************

void
PullInMPI_IOSymbols()
{
#ifdef PARALLEL

    //Don't call this!
    EXCEPTION1(ImproperUseException, "Do not call PullInMPI_IOSymbols");

    MPI_Info info;
    MPI_File fh;
    MPI_Offset sz;
    char *nm;
    int whence;
    void *buf;
    int count;
    MPI_Datatype datatype;
    MPI_Status status;

    MPI_File_open(VISIT_MPI_COMM, nm, 0, info, &fh);
    MPI_File_get_size(fh, &sz);
    MPI_File_seek(fh, sz, whence);
    MPI_File_read(fh, buf, count, datatype, &status);
#endif
}
