// ************************************************************************* //
//                                  avtParallel.C                            //
// ************************************************************************* //

#include <avtParallel.h>

#ifdef PARALLEL
  #include <mpi.h>
#endif

#include <DebugStream.h>
#include <ImproperUseException.h>


// ****************************************************************************
//  Function: PAR_Rank
//
//  Purpose:
//      Returns the rank of this processor.
//
//  Programmer: Hank Childs
//  Creation:   November 19, 2002
//
// ****************************************************************************

int
PAR_Rank(void)
{
    int rank = 0;

#ifdef PARALLEL
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
#endif

    return rank;
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
// ****************************************************************************

int
PAR_Size(void)
{
    int nProcs = 1;

#ifdef PARALLEL
    MPI_Comm_size(MPI_COMM_WORLD, &nProcs);
#endif

    return nProcs;
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
// ****************************************************************************

/* ARGSUSED */
void
UnifyMinMax(double *buff, int size)
{
#ifdef PARALLEL
    int  i;

    if (size % 2 != 0)
    {
        debug1 << "Min/max layout must be divisible by 2." << endl;
        EXCEPTION0(ImproperUseException);
    }

    //
    // Create some buffers that can be used for the maximums and minimums.
    //
    double *outbuff = new double[size/2];
    double *inbuff  = new double[size/2];

    //
    // Copy just the minimums into the buffer.
    //
    for (i = 0 ; i < size/2 ; i++)
    {
        outbuff[i] = buff[2*i];
    }

    //
    // Take the minimum of all of the minimums in each dimension.
    //
    MPI_Allreduce(outbuff, inbuff, size/2, MPI_DOUBLE, MPI_MIN,MPI_COMM_WORLD);

    //
    // Now copy those minimums back into the input buffer.
    //
    for (i = 0 ; i < size/2 ; i++)
    {
         buff[2*i] = inbuff[i];
    }
    
    //
    // Do the same thing for the maximums.
    //
    for (i = 0 ; i < size/2 ; i++)
    {
        outbuff[i] = buff[2*i+1];
    }
    MPI_Allreduce(outbuff, inbuff, size/2, MPI_DOUBLE, MPI_MAX,MPI_COMM_WORLD);
    for (i = 0 ; i < size/2 ; i++)
    {
        buff[2*i+1] = inbuff[i];
    }

    delete [] inbuff;
    delete [] outbuff;
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
// ****************************************************************************

/* ARGSUSED */
int
UnifyMaximumValue(int mymax)
{
#ifdef PARALLEL
    int allmax;
    MPI_Allreduce(&mymax, &allmax, 1, MPI_INT, MPI_MAX, MPI_COMM_WORLD);
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
// ****************************************************************************

/* ARGSUSED */
bool
Collect(float *buff, int size)
{
#ifdef PARALLEL

    float *newbuff = new float[size];
    MPI_Reduce(buff, newbuff, size, MPI_FLOAT, MPI_MAX, 0, MPI_COMM_WORLD);
    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
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
// ****************************************************************************

void
Barrier(void)
{
#ifdef PARALLEL
    MPI_Barrier(MPI_COMM_WORLD);
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
// ****************************************************************************

void
SumIntArrayAcrossAllProcessors(int *inArray, int *outArray, int nArray)
{
#ifdef PARALLEL
    MPI_Allreduce(inArray, outArray, nArray, MPI_INT, MPI_SUM, MPI_COMM_WORLD);
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
// ****************************************************************************

void
SumDoubleArrayAcrossAllProcessors(double *inArray, double *outArray,int nArray)
{
#ifdef PARALLEL
    MPI_Allreduce(inArray, outArray, nArray, MPI_DOUBLE, MPI_SUM,
                  MPI_COMM_WORLD);
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
// ****************************************************************************

void
SumFloatAcrossAllProcessors(float &value)
{
#ifdef PARALLEL
    float newvalue;
    MPI_Allreduce(&value, &newvalue, 1, MPI_FLOAT, MPI_SUM,
                  MPI_COMM_WORLD);
    value = newvalue;
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
// ****************************************************************************

void
SumIntAcrossAllProcessors(int &value)
{
#ifdef PARALLEL
    int newvalue;
    MPI_Allreduce(&value, &newvalue, 1, MPI_INT, MPI_SUM,
                  MPI_COMM_WORLD);
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
// ****************************************************************************

void
SumDoubleAcrossAllProcessors(double &value)
{
#ifdef PARALLEL
    double newvalue;
    MPI_Allreduce(&value, &newvalue, 1, MPI_DOUBLE, MPI_SUM,
                  MPI_COMM_WORLD);
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
// ****************************************************************************

bool
ThisProcessorHasMinimumValue(double min)
{
#ifdef PARALLEL
    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    int nProcs;
    MPI_Comm_size(MPI_COMM_WORLD, &nProcs);
    double *allValues = new double[nProcs];
    MPI_Allgather(&min, 1, MPI_DOUBLE, allValues, 1, MPI_DOUBLE,
                  MPI_COMM_WORLD);
    double theBestMin = min;
    int  theCurrentWinner = rank;
    for (int i = 0 ; i < nProcs ; i++)
    {
        //
        // A bit tricky here -- if many processors have the same minimum, then
        // doing < rather than '<=' could cause some of them to believe that
        // they are the winner arbitrarily.  Would be correct, but not in the
        // spirit of the routine (we want all the processors to believe that
        // one single processor has the best minimum).
        //
        if (allValues[i] <= theBestMin)
        {
            theBestMin = allValues[i];
            theCurrentWinner = i;
        }
    }
    return (theCurrentWinner == rank ? true : false);
#else
    return true;
#endif
}


