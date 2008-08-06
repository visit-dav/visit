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
//                                  avtParallel.C                            //
// ************************************************************************* //

#include <float.h>

#include <avtParallel.h>

#ifdef PARALLEL
  #include <mpi.h>
  #include <BufferConnection.h>
#endif

#include <DebugStream.h>
#include <ImproperUseException.h>
#include <AttributeGroup.h>

#include <cstring>

using std::string;
using std::vector;

// handle for user-defined reduction operator for min/max in single reduce
#ifdef PARALLEL

// VisIt's own MPI communicator
static MPI_Comm VISIT_MPI_COMM_OBJ;
void *VISIT_MPI_COMM_PTR;

static MPI_Op AVT_MPI_MINMAX = MPI_OP_NULL;
static int mpiTagUpperBound = 32767;

// If MPI was already initalized for us, we don't need to finalize either
static bool  we_initialized_MPI = true;
#endif

// Minimum value for use in GetUniqueMessageTag
// So that certain other tags can be hard-coded with values < MIN_TAG_VALUE
// if they should be needed prior to MPI_Init
#define MIN_TAG_VALUE 100

// Variables to hold process size information
static int   par_rank = 0, par_size = 1;

// A buffer to temporarily receive broadcast data before permanent storage
static vector<char> broadcastBuffer(1000);

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
// ****************************************************************************

void
PAR_Init (int &argc, char **&argv)
{
#ifdef PARALLEL
    int flag;
    MPI_Initialized(&flag);
    we_initialized_MPI = !flag;

    if (we_initialized_MPI)
        MPI_Init (&argc, &argv);

    // duplicate the communicator
    if (MPI_Comm_dup(MPI_COMM_WORLD, &VISIT_MPI_COMM_OBJ) != MPI_SUCCESS)
        VISIT_MPI_COMM_OBJ = MPI_COMM_WORLD;
    VISIT_MPI_COMM_PTR = (void*) &VISIT_MPI_COMM_OBJ;

    //
    // Find the current process rank and the size of the process pool.
    //
    MPI_Comm_rank (VISIT_MPI_COMM, &par_rank);
    MPI_Comm_size (VISIT_MPI_COMM, &par_size);

    int success = 0;
    // MPI_Attr_get requires void *
    // Also, MPI_TAG_UB is perm attr of the lib and accessible only
    // from MPI_COMM_WORLD
    void *value; 
    MPI_Attr_get(MPI_COMM_WORLD, MPI_TAG_UB, &value, &success);
    if (success)
    {
        mpiTagUpperBound = *(int*)value;
    }
    else
    {
        // Cannot use debug logs here, because they haven't been initialized.
        cerr << "Unable to get value for MPI_TAG_UB, assuming 32767." << endl;
        cerr << "success = " << success << endl;
        mpiTagUpperBound = 32767; 
    }
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
//  Function: MinMaxOp 
//
//  Purpose:
//      User defined MPI reduction operator. We can assume double values
//
//  Programmer: Mark C. Miller 
//  Creation:   January 29, 2004 
//
// ****************************************************************************

#ifdef PARALLEL
static void
MinMaxOp(void *ibuf, void *iobuf, int *len, MPI_Datatype *dtype)
{
    int i;
    double *iovals = (double *) iobuf;
    double  *ivals = (double *) ibuf;


    // there is a chance, albeit slim for small values of *len, that if MPI
    // decides to chop up the buffers, it could decide to chop them on an
    // odd boundary. That would be catastrophic!
    if (*len % 2 != 0)
    {
        EXCEPTION0(ImproperUseException);
    }

    // handle the minimums by copying any values in ibuff that are less than
    // respective value in iobuff into iobuff
    for (i = 0; i < *len; i += 2)
    {
        if (ivals[i] < iovals[i])
            iovals[i] = ivals[i];
    }

    // handle the maximums by copying any values in ibuff that are greater than
    // respective value in iobuff into iobuff
    for (i = 1; i < *len; i += 2)
    {
        if (ivals[i] > iovals[i])
            iovals[i] = ivals[i];
    }
}
#endif

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
#ifdef PARALLEL
    int  i;
    double *rbuff;

    // if it hasn't been created yet, create the min/max MPI reduction operator
    if (AVT_MPI_MINMAX == MPI_OP_NULL)
        MPI_Op_create(MinMaxOp, true, &AVT_MPI_MINMAX);

    // we do this 'extra' communication if we can't be sure all processors
    // have an agreed upon size to work with. This will have effect of 
    // overwriting altsize with a maximum agreed upon size
    if (altsize == -1)
        MPI_Allreduce(&size, &altsize, 1, MPI_INT, MPI_MAX, VISIT_MPI_COMM);

    if (altsize == 0)
    {
        if (size % 2 != 0)
        {
            debug1 << "Min/max layout must be divisible by 2." << endl;
            EXCEPTION0(ImproperUseException);
        }

        rbuff = new double[size];

        MPI_Allreduce(buff, rbuff, size, MPI_DOUBLE, AVT_MPI_MINMAX, VISIT_MPI_COMM);
    }
    else if (altsize > 0)
    {
        if ((altsize % 2 != 0) || (altsize < size))
        {
            EXCEPTION0(ImproperUseException);
        }

        rbuff = new double[altsize];

        // we're going to be reducing a buffer that is larger than size
        // so populate it with appropriate default values
        double *tbuff = new double[altsize];
        for (i = 0; i < size; i++)
            tbuff[i] = buff[i];
        for (i = size; i < altsize; i += 2)
        {
            tbuff[i  ] = +DBL_MAX;
            tbuff[i+1] = -DBL_MAX;
        }

        MPI_Allreduce(tbuff, rbuff, altsize, MPI_DOUBLE, AVT_MPI_MINMAX, VISIT_MPI_COMM);

    }
    else
    {
        EXCEPTION0(ImproperUseException);
    }

    // put the reduced results back into buff
    for (i = 0; i < size ; i++)
        buff[i] = rbuff[i];

    delete [] rbuff;
#endif
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
#ifdef PARALLEL
    int allmin;
    MPI_Allreduce(&mymin, &allmin, 1, MPI_INT, MPI_MIN, VISIT_MPI_COMM);
    return allmin;
#else
    return mymin;
#endif
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
#ifdef PARALLEL
    float allmin;
    MPI_Allreduce(&mymin, &allmin, 1, MPI_FLOAT, MPI_MIN, VISIT_MPI_COMM);
    return allmin;
#else
    return mymin;
#endif
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

/* ARGSUSED */
int
UnifyMaximumValue(int mymax)
{
#ifdef PARALLEL
    int allmax;
    MPI_Allreduce(&mymax, &allmax, 1, MPI_INT, MPI_MAX, VISIT_MPI_COMM);
    return allmax;
#else
    return mymax;
#endif
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
#ifdef PARALLEL
    float allmax;
    MPI_Allreduce(&mymax, &allmax, 1, MPI_FLOAT, MPI_MAX, VISIT_MPI_COMM);
    return allmax;
#else
    return mymax;
#endif
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

/* ARGSUSED */
bool
Collect(float *buff, int size)
{
#ifdef PARALLEL

    float *newbuff = new float[size];
    MPI_Reduce(buff, newbuff, size, MPI_FLOAT, MPI_MAX, 0, VISIT_MPI_COMM);
    int rank;
    MPI_Comm_rank(VISIT_MPI_COMM, &rank);
    if (rank == 0)
    {
        for (int i = 0 ; i < size ; i++)
        {
            buff[i] = newbuff[i];
        }
    }

    delete [] newbuff;

    return (rank == 0 ? true : false);

#else
    return true;
#endif
}


// ****************************************************************************
//  Function: Collect
//
//  Purpose:
//      Takes the buffer from a specific processor and take the maximum entry
//      in each buffer.  This variant is for ints.
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
//  Creation:   June 30, 2003
//
//  Modifications:
//
//    Mark C. Miller, Mon Jan 22 22:09:01 PST 2007
//    Changed MPI_COMM_WORLD to VISIT_MPI_COMM
// ****************************************************************************

/* ARGSUSED */
bool
Collect(int *buff, int size)
{
#ifdef PARALLEL

    int *newbuff = new int[size];
    MPI_Reduce(buff, newbuff, size, MPI_INT, MPI_MAX, 0, VISIT_MPI_COMM);
    int rank;
    MPI_Comm_rank(VISIT_MPI_COMM, &rank);
    if (rank == 0)
    {
        for (int i = 0 ; i < size ; i++)
        {
            buff[i] = newbuff[i];
        }
    }

    delete [] newbuff;

    return (rank == 0 ? true : false);

#else
    return true;
#endif
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
#ifdef PARALLEL
    MPI_Barrier(VISIT_MPI_COMM);
#endif
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
#ifdef PARALLEL
    MPI_Allreduce(inArray, outArray, nArray, MPI_INT, MPI_SUM, VISIT_MPI_COMM);
#else
    for (int i = 0 ; i < nArray ; i++)
    {
        outArray[i] = inArray[i];
    }
#endif
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
#ifdef PARALLEL
    MPI_Allreduce(inArray, outArray, nArray, MPI_DOUBLE, MPI_SUM,
                  VISIT_MPI_COMM);
#else
    for (int i = 0 ; i < nArray ; i++)
    {
        outArray[i] = inArray[i];
    }
#endif
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
#ifdef PARALLEL
    MPI_Allreduce(inArray, outArray, nArray, MPI_FLOAT, MPI_SUM,
                  VISIT_MPI_COMM);
#else
    for (int i = 0 ; i < nArray ; i++)
    {
        outArray[i] = inArray[i];
    }
#endif
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
#ifdef PARALLEL
    float newvalue;
    MPI_Allreduce(&value, &newvalue, 1, MPI_FLOAT, MPI_SUM,
                  VISIT_MPI_COMM);
    value = newvalue;
#endif
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
#ifdef PARALLEL
    MPI_Allreduce(inArray, outArray, nArray, MPI_FLOAT, MPI_MIN,
                  VISIT_MPI_COMM);
#else
    memcpy(outArray, inArray, nArray*sizeof(float));
#endif
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
#ifdef PARALLEL
    MPI_Allreduce(inArray, outArray, nArray, MPI_FLOAT, MPI_MAX,
                  VISIT_MPI_COMM);
#else
    memcpy(outArray, inArray, nArray*sizeof(float));
#endif
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
#ifdef PARALLEL
    int newvalue;
    MPI_Allreduce(&value, &newvalue, 1, MPI_INT, MPI_SUM,
                  VISIT_MPI_COMM);
    value = newvalue;
#endif
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
#ifdef PARALLEL
    double newvalue;
    MPI_Allreduce(&value, &newvalue, 1, MPI_DOUBLE, MPI_SUM,
                  VISIT_MPI_COMM);
    value = newvalue;
#endif
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
#ifdef PARALLEL
    int rank;
    MPI_Comm_rank(VISIT_MPI_COMM, &rank);
    struct { double val; int rank; } tmp, rtmp;
    tmp.val = min;
    tmp.rank = rank;
    MPI_Allreduce(&tmp, &rtmp, 1, MPI_DOUBLE_INT, MPI_MINLOC, VISIT_MPI_COMM);
    return (rtmp.rank == rank ? true : false);
#else
    return true;
#endif
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
#ifdef PARALLEL
    int rank;
    MPI_Comm_rank(VISIT_MPI_COMM, &rank);
    struct { double val; int rank; } tmp, rtmp;
    tmp.val = max;
    tmp.rank = rank;
    MPI_Allreduce(&tmp, &rtmp, 1, MPI_DOUBLE_INT, MPI_MAXLOC, VISIT_MPI_COMM);
    return (rtmp.rank == rank ? true : false);
#else
    return true;
#endif
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
void BroadcastInt(int &i)
{
#ifdef PARALLEL
    MPI_Bcast(&i, 1, MPI_INT, 0, VISIT_MPI_COMM);
#endif
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
    int tmp = b ? 1 : 0;
    BroadcastInt(tmp);
    b = tmp == 1 ? true : false;
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
//
//    Mark C. Miller, Mon Jan 22 22:09:01 PST 2007
//    Changed MPI_COMM_WORLD to VISIT_MPI_COMM
// ****************************************************************************
void BroadcastIntVector(vector<int> &vi, int myrank)
{
#ifdef PARALLEL
    int len;
    if (myrank==0)
        len = vi.size();
    MPI_Bcast(&len, 1, MPI_INT, 0, VISIT_MPI_COMM);
    if (myrank!=0)
        vi.resize(len);

    MPI_Bcast(&vi[0], len, MPI_INT, 0, VISIT_MPI_COMM);
#endif
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
void BroadcastDouble(double &i)
{
#ifdef PARALLEL
    MPI_Bcast(&i, 1, MPI_DOUBLE, 0, VISIT_MPI_COMM);
#endif
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
// ****************************************************************************
void BroadcastDoubleVector(vector<double> &vi, int myrank)
{
#ifdef PARALLEL
    int len;
    if (myrank==0)
        len = vi.size();
    MPI_Bcast(&len, 1, MPI_INT, 0, VISIT_MPI_COMM);
    if (myrank!=0)
        vi.resize(len);

    MPI_Bcast(&vi[0], len, MPI_DOUBLE, 0, VISIT_MPI_COMM);
#endif
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
void BroadcastString(string &s, int myrank)
{
#ifdef PARALLEL
    int len;
    if (myrank==0)
        len = s.length();
    MPI_Bcast(&len, 1, MPI_INT, 0, VISIT_MPI_COMM);
    if (broadcastBuffer.size() < len+1)
        broadcastBuffer.resize(len+1);

    if (myrank==0)
    {
        MPI_Bcast((void*)(s.c_str()), len, MPI_CHAR, 0, VISIT_MPI_COMM);
    }
    else
    {
        MPI_Bcast(&broadcastBuffer[0], len, MPI_CHAR, 0, VISIT_MPI_COMM);
        broadcastBuffer[len] = '\0';
        s = &broadcastBuffer[0];
    }
#endif
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
// ****************************************************************************
void BroadcastStringVector(vector<string> &vs, int myrank)
{
#ifdef PARALLEL
    int i;

    int len;
    if (myrank==0)
        len = vs.size();
    MPI_Bcast(&len, 1, MPI_INT, 0, VISIT_MPI_COMM);

    vector<int> lens(len);
    if (myrank == 0)
        for (i = 0 ; i < len ; i++)
            lens[i] = vs[i].length();
    MPI_Bcast(&(lens[0]), len, MPI_INT, 0, VISIT_MPI_COMM);

    int total_len = 0;
    for (i = 0 ; i < len ; i++)
        total_len += lens[i];

    char *buff = new char[total_len];
    if (myrank == 0)
    {
        char *buff_ptr = buff;
        for (i = 0 ; i < len ; i++)
        {
            strncpy(buff_ptr, vs[i].c_str(), lens[i]);
            buff_ptr += lens[i];
        }
    }    

    MPI_Bcast((void*)buff, total_len, MPI_CHAR, 0, VISIT_MPI_COMM);

    if (myrank != 0)
    {
        vs.resize(len);

        int biggest = 0;
        for (i=0; i<len; i++)
            biggest = (biggest < lens[i] ? lens[i] : biggest);
        char *buff2 = new char[biggest+1];
        char *buff_ptr = buff;
        for (i=0; i<len; i++)
        {
            strncpy(buff2, buff_ptr, lens[i]);
            buff2[lens[i]] = '\0';
            vs[i] = buff2;
            buff_ptr += lens[i];
        }
        delete [] buff2;
    }
    delete [] buff;
#endif
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
// ****************************************************************************
void BroadcastStringVectorVector(vector< vector<string> > &vvs, int myrank)
{
#ifdef PARALLEL
    int len;
    if (myrank==0)
        len = vvs.size();
    MPI_Bcast(&len, 1, MPI_INT, 0, VISIT_MPI_COMM);
    if (myrank!=0)
        vvs.resize(len);

    for (int i=0; i<len; i++)
    {
        BroadcastStringVector(vvs[i], myrank);
    }
#endif
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
#ifdef PARALLEL
    int rank;
    MPI_Comm_rank(VISIT_MPI_COMM, &rank);
    int red_val = 10000000;
    if (vars.size() == total)
        red_val = rank;

    int mpiSizeTag = GetUniqueMessageTag();
    int mpiDataTag = GetUniqueMessageTag();
       
    int lowest_with_list = 0;
    MPI_Allreduce(&red_val, &lowest_with_list, 1, MPI_INT, MPI_MIN,
                  VISIT_MPI_COMM);

    if (lowest_with_list == 0)
        return (rank == 0);
    
    if (lowest_with_list == rank)
    {
        for (int i = 0 ; i < total ; i++)
        {
            int size = strlen(vars[i].c_str());
            MPI_Send(&size, 1, MPI_INT, mpiSizeTag, rank, VISIT_MPI_COMM);
            void *ptr = (void *) vars[i].c_str();
            MPI_Send(ptr, size, MPI_CHAR, mpiDataTag, rank, VISIT_MPI_COMM);
        }
    }
    else if (rank == 0)
    {
        vars.clear();
        for (int i = 0 ; i < total ; i++)
        {
            int len;
            MPI_Status stat;
            MPI_Recv(&len, 1, MPI_INT, MPI_ANY_SOURCE, mpiSizeTag,
                     VISIT_MPI_COMM, &stat);
            char *varname = new char[len+1];
            void *buff = (void *) varname;
            MPI_Recv(buff, len, MPI_CHAR, stat.MPI_SOURCE, mpiDataTag,
                     VISIT_MPI_COMM, &stat);
            varname[len] = '\0';
            vars.push_back(varname);
            delete [] varname;
        }
    }

    return (rank == 0);
#endif
    return true;
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
    static int retval = MIN_TAG_VALUE;
#ifdef PARALLEL
    if (retval == mpiTagUpperBound)
    {
        retval = MIN_TAG_VALUE;
        debug5 << "Unique message tags have wrapped back to "
               << MIN_TAG_VALUE << " from " << mpiTagUpperBound << endl;
    }
    else
        retval++;
#endif
    return retval;
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
    static int rv = 0; 
#ifdef PARALLEL
    //
    //  Cannot go beyond the starting value for normal UniqueMessageTags
    //
    if (rv == MIN_TAG_VALUE -1) 
    {
        rv = 0;
        debug1 << "Static Unique message tags have wrapped back to zero "
               << "from " << MIN_TAG_VALUE -1 << endl; 
    }
    else
        rv++;
#endif
    return rv;
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
// ****************************************************************************

void 
GetAttToRootProc(AttributeGroup &att, int hasAtt)
{
#ifdef PARALLEL
    int rank, nprocs, size, i;
    BufferConnection b;
    unsigned char *buf;

    MPI_Comm_rank(VISIT_MPI_COMM, &rank);
    MPI_Comm_size(VISIT_MPI_COMM, &nprocs);

    int mpiHasAttTag = GetUniqueMessageTag();
    int mpiSizeTag = GetUniqueMessageTag();
    int mpiDataTag = GetUniqueMessageTag();
       
    if (rank == 0)
    {
        for (i = 1; i < nprocs; i++)
        {
            MPI_Status stat, stat2;
            MPI_Recv(&hasAtt, 1, MPI_INT, MPI_ANY_SOURCE,
                     mpiHasAttTag, VISIT_MPI_COMM, &stat);
            if (hasAtt)
            {
                MPI_Recv(&size, 1, MPI_INT, stat.MPI_SOURCE, mpiSizeTag,
                         VISIT_MPI_COMM, &stat2);
                buf = new unsigned char[size];
                MPI_Recv(buf, size, MPI_UNSIGNED_CHAR, stat.MPI_SOURCE, mpiDataTag,
                         VISIT_MPI_COMM, &stat2);
                b.Append(buf, size);
                att.Read(b);
                delete [] buf;
            }
        }
    }
    else 
    {
        MPI_Send(&hasAtt, 1, MPI_INT, 0, mpiHasAttTag, VISIT_MPI_COMM);
        if (hasAtt)
        {
            att.SelectAll();
            att.Write(b);
            size = att.CalculateMessageSize(b);
            buf = new unsigned char[size];
            for (int i = 0; i < size; ++i)
                b.Read(buf+i);
 
            MPI_Send(&size, 1, MPI_INT, 0, mpiSizeTag, VISIT_MPI_COMM);
            MPI_Send(buf, size, MPI_UNSIGNED_CHAR, 0, mpiDataTag, VISIT_MPI_COMM);
            delete [] buf;
        }
    }
#endif
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
#ifdef PARALLEL
    int myRank, numProcs;
    MPI_Comm_rank(VISIT_MPI_COMM, &myRank);
    MPI_Comm_size(VISIT_MPI_COMM, &numProcs);
    int mpiGoodTag = GetUniqueMessageTag();
    int mpiFloatArrayTag = GetUniqueMessageTag();

    if (myRank == 0)
    {
        MPI_Status stat, stat2;
        int good; 
        for (int i = 1; i < numProcs; i++)
        {
            MPI_Recv(&good, 1, MPI_INT, MPI_ANY_SOURCE,
                     mpiGoodTag, VISIT_MPI_COMM, &stat);
            if (good)
            {
                MPI_Recv(fa, nf, MPI_FLOAT, stat.MPI_SOURCE, mpiFloatArrayTag,
                         VISIT_MPI_COMM, &stat2);
                success = good;
            }
        }
    }
    else
    {
        MPI_Send(&success, 1, MPI_INT, 0, mpiGoodTag, VISIT_MPI_COMM);
        if (success)
        {
            MPI_Send(fa, nf, MPI_FLOAT, 0, mpiFloatArrayTag, VISIT_MPI_COMM);
        }    
    }
#endif
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
UnifyMaximumValue(vector<int> &mymax, vector<int> &results)
{
#ifdef PARALLEL
    results.resize(mymax.size());
    MPI_Allreduce(&mymax[0], &results[0], mymax.size(), MPI_INT, MPI_MAX, 
                  VISIT_MPI_COMM);
#else
    results = mymax;
#endif
}


// ****************************************************************************
//  Function: GetDoubleArrayToRootProc
//
//  Purpose:
//    Gets a double array to processor 0.  
//
//  Arguments:
//    da        The doubee array.
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
#ifdef PARALLEL
    int myRank, numProcs;
    MPI_Comm_rank(VISIT_MPI_COMM, &myRank);
    MPI_Comm_size(VISIT_MPI_COMM, &numProcs);
    int mpiGoodTag = GetUniqueMessageTag();
    int mpiDoubleArrayTag = GetUniqueMessageTag();

    if (myRank == 0)
    {
        MPI_Status stat, stat2;
        int good; 
        for (int i = 1; i < numProcs; i++)
        {
            MPI_Recv(&good, 1, MPI_INT, MPI_ANY_SOURCE,
                     mpiGoodTag, VISIT_MPI_COMM, &stat);
            if (good)
            {
                MPI_Recv(da, nd, MPI_DOUBLE, stat.MPI_SOURCE, mpiDoubleArrayTag,
                         VISIT_MPI_COMM, &stat2);
                success = good;
            }
        }
    }
    else
    {
        int val = (int) success;
        MPI_Send(&val, 1, MPI_INT, 0, mpiGoodTag, VISIT_MPI_COMM);
        if (success)
        {
            MPI_Send(da, nd, MPI_DOUBLE, 0, mpiDoubleArrayTag, VISIT_MPI_COMM);
        }    
    }
#endif
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
#ifdef PARALLEL
    status.resize( reqs.size() );
    MPI_Waitall( reqs.size(), (MPI_Request *)&reqs[0], (MPI_Status *)&status[0] );
#endif
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
#ifdef PARALLEL
    status.resize( reqs.size() );
    done.resize( reqs.size() );
    int nDone;
    MPI_Waitsome( reqs.size(), (MPI_Request *)&reqs[0], &nDone, (int *)&done[0], (MPI_Status *)&status[0] );
    done.resize( nDone );
#endif
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
#ifdef PARALLEL
    status.resize( reqs.size() );
    done.resize( reqs.size() );
    int nDone;
    MPI_Testsome( reqs.size(), (MPI_Request *)&reqs[0], &nDone, (int *)&done[0], (MPI_Status *)&status[0] );
#endif
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
//
// ****************************************************************************

void
CancelRequest( int &req )
{
#ifdef PARALLEL
    MPI_Cancel(&req);
#endif
}
