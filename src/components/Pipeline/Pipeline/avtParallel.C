// ************************************************************************* //
//                                  avtParallel.C                            //
// ************************************************************************* //

#include <avtParallel.h>

#ifdef PARALLEL
  #include <mpi.h>
#endif

#include <DebugStream.h>
#include <ImproperUseException.h>

using std::string;
using std::vector;

// A buffer to temporarily receive broadcast data before permanent storage
static vector<char> broadcastBuffer(1000);

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
// ****************************************************************************

/* ARGSUSED */
bool
Collect(int *buff, int size)
{
#ifdef PARALLEL

    int *newbuff = new int[size];
    MPI_Reduce(buff, newbuff, size, MPI_INT, MPI_MAX, 0, MPI_COMM_WORLD);
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
// ****************************************************************************

void
SumFloatArrayAcrossAllProcessors(float *inArray, float *outArray, int nArray)
{
#ifdef PARALLEL
    MPI_Allreduce(inArray, outArray, nArray, MPI_FLOAT, MPI_SUM,
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
// ****************************************************************************
void BroadcastInt(int &i)
{
#ifdef PARALLEL
    MPI_Bcast(&i, 1, MPI_INT, 0, MPI_COMM_WORLD);
#endif
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
// ****************************************************************************
void BroadcastIntVector(vector<int> &vi, int myrank)
{
#ifdef PARALLEL
    int len;
    if (myrank==0)
        len = vi.size();
    MPI_Bcast(&len, 1, MPI_INT, 0, MPI_COMM_WORLD);
    if (myrank!=0)
        vi.resize(len);

    MPI_Bcast(&vi[0], len, MPI_INT, 0, MPI_COMM_WORLD);
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
// ****************************************************************************
void BroadcastString(string &s, int myrank)
{
#ifdef PARALLEL
    int len;
    if (myrank==0)
        len = s.length();
    MPI_Bcast(&len, 1, MPI_INT, 0, MPI_COMM_WORLD);
    if (broadcastBuffer.size() < len+1)
        broadcastBuffer.resize(len+1);

    if (myrank==0)
    {
        MPI_Bcast((void*)(s.c_str()), len, MPI_CHAR, 0, MPI_COMM_WORLD);
    }
    else
    {
        MPI_Bcast(&broadcastBuffer[0], len, MPI_CHAR, 0, MPI_COMM_WORLD);
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
// ****************************************************************************
void BroadcastStringVector(vector<string> &vs, int myrank)
{
#ifdef PARALLEL
    int len;
    if (myrank==0)
        len = vs.size();
    MPI_Bcast(&len, 1, MPI_INT, 0, MPI_COMM_WORLD);
    if (myrank!=0)
        vs.resize(len);

    for (int i=0; i<len; i++)
    {
        BroadcastString(vs[i], myrank);
    }
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
// ****************************************************************************
void BroadcastStringVectorVector(vector< vector<string> > &vvs, int myrank)
{
#ifdef PARALLEL
    int len;
    if (myrank==0)
        len = vvs.size();
    MPI_Bcast(&len, 1, MPI_INT, 0, MPI_COMM_WORLD);
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
// ****************************************************************************

bool GetListToRootProc(std::vector<std::string> &vars, int total)
{
#ifdef PARALLEL
    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    int red_val = 10000000;
    if (vars.size() == total)
        red_val = rank;
       
    int lowest_with_list = 0;
    MPI_Allreduce(&red_val, &lowest_with_list, 1, MPI_INT, MPI_MIN,
                  MPI_COMM_WORLD);

    if (lowest_with_list == 0)
        return (rank == 0);
    
    if (lowest_with_list == rank)
    {
        for (int i = 0 ; i < total ; i++)
        {
            int size = strlen(vars[i].c_str());
            MPI_Send(&size, 1, MPI_INT, 0, rank, MPI_COMM_WORLD);
            void *ptr = (void *) vars[i].c_str();
            MPI_Send(ptr, size, MPI_CHAR, 0, rank, MPI_COMM_WORLD);
        }
    }
    else if (rank == 0)
    {
        vars.clear();
        for (int i = 0 ; i < total ; i++)
        {
            int len;
            MPI_Status stat;
            MPI_Recv(&len, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG,
                     MPI_COMM_WORLD, &stat);
            char *varname = new char[len+1];
            void *buff = (void *) varname;
            MPI_Recv(buff, len, MPI_CHAR, stat.MPI_SOURCE, MPI_ANY_TAG,
                     MPI_COMM_WORLD, &stat);
            varname[len] = '\0';
            vars.push_back(varname);
            delete [] varname;
        }
    }

    return (rank == 0);
#endif
    return true;
}


