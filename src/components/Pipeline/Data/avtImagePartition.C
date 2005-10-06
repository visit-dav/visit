// ************************************************************************* //
//                            avtImagePartition.C                            //
// ************************************************************************* //

#include <avtImagePartition.h>

#include <math.h>

#ifdef PARALLEL
#include <mpi.h>
#endif

#include <avtParallel.h>

#include <ImproperUseException.h>


typedef struct
{
    int     bytes;
    int     processor;
    int     partition;
}  Tuple3;


void          QuicksortTuple3(Tuple3 *, int);


// ****************************************************************************
//  Method: avtImagePartition constructor
//
//  Arguments:
//      w       The width of the image.
//      h       The height of the image.
//      np      The number of partitions.
//      tp      The rank of this processor.
//
//  Programmer: Hank Childs
//  Creation:   January 25, 2001
//
//  Modifications:
//
//    Hank Childs, Sun Mar  4 21:07:43 PST 2001
//    Do not assume that we know what thisPartition is.
//
//    Hank Childs, Tue Jan  1 11:24:10 PST 2002
//    Allow for scanlines to be adaptively assigned to the partitions.
//
//    Hank Childs, Tue Jul  1 21:13:38 PDT 2003
//    Assume we are running in serial if it is not specified.
//
//    Hank Childs, Fri Dec 10 10:49:43 PST 2004
//    Initialize shouldDoTiling.
//
//    Hank Childs, Fri Sep 30 17:27:02 PDT 2005
//    Initialize shouldProduceOverlaps.
//
// ****************************************************************************

avtImagePartition::avtImagePartition(int w, int h, int np, int tp)
{
    int   i;

    width  = w;
    height = h;

    numProcessors = np;
    if (numProcessors < 0)
    {
#ifdef PARALLEL
        MPI_Comm_size(MPI_COMM_WORLD, &numProcessors);
#else
        numProcessors = 1;
#endif
    }

    thisProcessor = tp;
    if (thisProcessor < 0)
    {
#ifdef PARALLEL
        MPI_Comm_rank(MPI_COMM_WORLD, &thisProcessor);
#else
        thisProcessor = 0;
#endif
    }
    thisPartition = thisProcessor;

    ptpAssignments   = new int[numProcessors];
    for (i = 0 ; i < numProcessors ; i++)
    {
        ptpAssignments[i] = i;
    }

    stpAssignments   = new int[height];
    for (i = 0 ; i < height ; i++)
    {
        stpAssignments[i] = 0;
    }

    partitionStartsOnScanline = new int[numProcessors];
    partitionStopsOnScanline  = new int[numProcessors];
    for (i = 0 ; i < numProcessors ; i++)
    {
        partitionStartsOnScanline[i] = -1;
        partitionStopsOnScanline[i]  = -1;
    }

    establishedPartitionBoundaries = false;
    shouldDoTiling = false;
    shouldProduceOverlaps = false;
}


// ****************************************************************************
//  Method: avtImagePartition destructor
//
//  Programmer: Hank Childs
//  Creation:   March 6, 2001
//
//  Modifications:
//
//    Hank Childs, Tue Jan  1 11:24:10 PST 2002
//    Clean up new data members.
//
// ****************************************************************************

avtImagePartition::~avtImagePartition()
{
    if (ptpAssignments != NULL)
    {
        delete [] ptpAssignments;
        ptpAssignments = NULL;
    }

    if (stpAssignments != NULL)
    {
        delete [] stpAssignments;
        stpAssignments = NULL;
    }

    if (partitionStartsOnScanline != NULL)
    {
        delete [] partitionStartsOnScanline;
        partitionStartsOnScanline = NULL;
    }

    if (partitionStopsOnScanline != NULL)
    {
        delete [] partitionStopsOnScanline;
        partitionStopsOnScanline = NULL;
    }
}


// ****************************************************************************
//  Method: avtImagePartition::RestrictToTile
//
//  Purpose:
//      Restricts the set of pixels that are being partitioned.
//
//  Programmer: Hank Childs
//  Creation:   December 10, 2004
//
// ****************************************************************************

void
avtImagePartition::RestrictToTile(int wmin, int wmax, int hmin, int hmax)
{
    shouldDoTiling = true;
    tile_width_min  = wmin;
    tile_width_max  = wmax;
    tile_height_min = hmin;
    tile_height_max = hmax;
}


// ****************************************************************************
//  Method: avtImagePartition::GetThisPartition
//
//  Purpose:
//      Gets the partition appropriate for this processor.
//
//  Arguments:
//      minW    The minimum pixel width for this partition.
//      maxW    The maximum pixel width for this partition.
//      minH    The minimum pixel height for this partition.
//      maxH    The maximum pixel height for this partition.
//
//  Programmer: Hank Childs
//  Creation:   January 25, 2001
//
// ****************************************************************************

void
avtImagePartition::GetThisPartition(int &minW, int &maxW, int &minH, int &maxH)
{
    GetPartition(thisPartition, minW, maxW, minH, maxH);
}


// ****************************************************************************
//  Method: avtImagePartition::GetPartition
//
//  Purpose:
//      Gets the pixels bounds for the partition specified.
//
//  Arguments:
//      part    The index of the partition of interest.
//      minW    The minimum pixel width for this partition.
//      maxW    The maximum pixel width for this partition.
//      minH    The minimum pixel height for this partition.
//      maxH    The maximum pixel height for this partition.
//
//  Programmer: Hank Childs
//  Creation:   January 25, 2001
//
//  Modifications:
//
//    Hank Childs, Tue Sep 18 10:20:35 PDT 2001
//    Cast to get rid of compiler warning.
//
//    Hank Childs, Tue Jan  1 11:39:20 PST 2002
//    Reflect that partitions are no longer necessarily even distributed.
//
//    Hank Childs, Fri Dec 10 10:49:43 PST 2004
//    Account for tiling.
//
// ****************************************************************************

void
avtImagePartition::GetPartition(int part, int &minW, int &maxW, int &minH,
                                int &maxH)
{
    minW = 0;
    maxW = width-1;
    if (shouldDoTiling)
    {
        minW = tile_width_min;
        maxW = tile_width_max-1;
    }

    minH = partitionStartsOnScanline[part];
    if (shouldProduceOverlaps)
        if (minH > 0)
            minH--;
    maxH = partitionStopsOnScanline[part];
}


// ****************************************************************************
//  Method: avtImagePartition::EstablishPartitionBoundaries
//
//  Purpose:
//      Determines which scanline each partition should start and stop at based
//      on how many sample points are in each scanline.
//  
//  Arguments:
//      samples   The number of samples in each scanline.  This is an array of
//                size height.
//
//  Programmer:   Hank Childs
//  Creation:     January 1, 2002
//
//  Modifications:
//
//    Hank Childs, Thu Jan  3 09:45:35 PST 2002
//    Make sure that we never go past the last partition and also try to
//    prevent the last partition from getting overloaded.
//
//    Hank Childs, Fri Jan  4 09:53:44 PST 2002 
//    Account for problems with overflows when counting total number of
//    samples.
//
//    Hank Childs, Fri Dec 10 11:07:07 PST 2004
//    Account for tiles.
//
// ****************************************************************************

void
avtImagePartition::EstablishPartitionBoundaries(int *samples)
{
    int   i;

    int first_scanline = (shouldDoTiling ? tile_height_min : 0);
    int last_scanline = (shouldDoTiling ? tile_height_max : height);

    //
    // Find out how many samples there are in each scanline across all procs.
    //
    int *allSamples = new int[height];
    SumIntArrayAcrossAllProcessors(samples, allSamples, height);

    //
    // Find out how many total samples there are and what the target is.
    //
    int totalSamples = 0;
    for (i = first_scanline ; i < last_scanline ; i++)
    {
        //
        // There has been some problems with overflows when we have lots of
        // sample points and we are in send cells mode (since send cells
        // overestimates pretty dramatically how many samples it has).
        //
        // Normalize the number of samples.
        //
        if (allSamples[i] > 0 && allSamples[i] < 1000)
        {
            allSamples[i] = 1;
        }
        else
        {
            allSamples[i] /= 1000;
        }
        totalSamples += allSamples[i];
    }
    int target  = totalSamples / numProcessors;
    target      = (target <= 0 ? 1 : target); // Correction for when have
                                              // nothing to render.
    int tooHigh = (int) (target*1.5);
    int tooLow  = (int) (target*1.);

    int currentPartition = 0;
    int amountForCurrentPartition = 0;
    partitionStartsOnScanline[currentPartition] = first_scanline;
    for (i = first_scanline ; i < last_scanline ; i++)
    {
        if (amountForCurrentPartition + allSamples[i] > tooHigh)
        {
            if (amountForCurrentPartition > tooLow &&
                currentPartition+1 < numProcessors)
            {
                //
                // If we added the next scanline, we would be too high.  Also,
                // the number of scanlines added to the current partition is
                // sufficient to not be ridiculously low.  Declare this
                // partition closed off and start the next one.
                //
                partitionStopsOnScanline[currentPartition] = i-1;
                currentPartition++;
                amountForCurrentPartition = 0;
                partitionStartsOnScanline[currentPartition] = i;
            }
        }

        stpAssignments[i] = currentPartition;
        amountForCurrentPartition += allSamples[i];
    }
    partitionStopsOnScanline[currentPartition] = last_scanline-1;
    currentPartition++;

    //
    // We may have not assigned the last few processors some partitions, so
    // give them the equivalent of nothing.
    //
    while (currentPartition < numProcessors)
    {
        partitionStartsOnScanline[currentPartition] = last_scanline+1;
        partitionStopsOnScanline[currentPartition]  = last_scanline;
        currentPartition++;
    }

    establishedPartitionBoundaries = true;
    delete [] allSamples;
}


// ****************************************************************************
//  Method: avtImagePartition::DetermineAssigments
//
//  Purpose:
//      Determines the assignments for the image partition.  I am employing a 
//      greedy algorithm.  I suspect it will be within a factor of two of
//      the optimal, but don't see the point in proving that.
//
//  Arguments:
//      amount    An array of integers, where the ith entry is the amount of
//                bytes this processor would like to send to partition i.
//
//  Notes:    This algorithm is run identically on every processor, so there
//            is no need for communication after getting the lists of how
//            much each processor would like to send.
//
//  Programmer:   Hank Childs
//  Creation:     March 6, 2001
//
//  Modifications:
//
//    Hank Childs, Tue Dec 11 11:31:06 PST 2001
//    Fixed parallel bug where bookend assignments were not being done well.
//
//    Hank Childs, Mon Dec 17 17:54:25 PST 2001 
//    Fixed array bounds write.
//
// ****************************************************************************

void
avtImagePartition::DetermineAssignments(int *amount)
{
    if (!establishedPartitionBoundaries)
    {
        //
        // How could the argument (amount) possibly have the number of samples
        // for each partition when we haven't even decided where the partitions
        // lay yet.
        //
        EXCEPTION0(ImproperUseException);
    }

#ifdef PARALLEL
    //
    // We are going to greedily assign the partitions to the processors --
    // that is find maximum (processor, partition) pair in terms of bytes and
    // assign that partition to that processor.  That processor and partition
    // are then off-limits for subsequent matches, since we have a one-to-one
    // correspondence between processors and partitions.
    //

    int   i, j;
    int   numPartitions = numProcessors; // for clarity when iterating

    //
    // Create a buffer big enough to receive the amounts that each processor
    // has for each partition and then make the MPI call that will do this.
    // allAmounts will contain the amount that processor 0 has for partition 0
    // in [0], from proc 0 to partition 1 in [1], ..., from proc 1 to 
    // partition 0 in [i], from proc 1 to partition 1 in [i+1], etc.
    //
    int *allAmounts = new int[numProcessors*numProcessors];
    MPI_Allgather(amount, numProcessors, MPI_INT, allAmounts, numProcessors,
                  MPI_INT, MPI_COMM_WORLD);

    //
    // Set up a data structure that has the partition and processor associated
    // with the number of bytes, so we can know which processor, partition
    // pair this entry came from _after_ our sort.  Also throw out the entries
    // where there is zero bytes exchanged, since they will be numerous and
    // they contribute nothing.
    //
    int possibleEntries = numProcessors*numPartitions;
    int totalEntries = 0;
    Tuple3  *list = new Tuple3[possibleEntries];
    for (i = 0 ; i < numProcessors ; i++)
    {
        for (j = 0 ; j < numPartitions ; j++)
        {
            int index = i*numPartitions + j;
            if (allAmounts[index] > 0)
            {
                list[totalEntries].bytes = allAmounts[index];
                list[totalEntries].processor = i;
                list[totalEntries].partition = j;
                totalEntries++;
            }
        }
    }

    //
    // This will take care of sorting the list.  It sorts by the size of the
    // message (bytes field in Tuple3).  It puts the maximum element at the end
    // of the list (as one would expect).
    //
    QuicksortTuple3(list, totalEntries);

    bool *usedProcessor = new bool[numProcessors];
    for (i = 0 ; i < numProcessors ; i++)
    {
        usedProcessor[i] = false;
    }

    bool *usedPartition = new bool[numPartitions];
    for (i = 0 ; i < numPartitions ; i++)
    {
        usedPartition[i] = false;
    }

    //
    // Start at the end of the list, since that has the maximum entry.
    //
    int  totalMatches = 0;
    for (i = totalEntries-1 ; i >= 0 ; i--)
    {
        int proc = list[i].processor;
        int part = list[i].partition;

        //
        // This element of the list is the biggest remaining, but we can only
        // use it when making assignments if we haven't use both the processor
        // and the partition before.
        //
        if (!usedProcessor[proc] && !usedPartition[part])
        {
            ptpAssignments[part]   = proc;

            //
            // Since we are building a one-to-one correspondence, we have
            // mark this processor and this partition as unusable.
            //
            usedProcessor[proc] = true;
            usedPartition[part] = true;

            //
            // Store the partition in a data member if it is for our partition.
            //
            if (proc == thisProcessor)
            {
                thisPartition = part;
            }

            totalMatches++;
            if (totalMatches >= numProcessors)
            {
                //
                // We have matched everything up, so there is no need to
                // further iterate through the list.
                //
                break;
            }
        }
    }

    //
    // Since we threw out all of the zero entries, we need to make sure that
    // everything has a match.
    //
    int  lastProcessor = 0;
    int  lastPartition = 0;
    while ((lastProcessor<numProcessors) && (lastPartition<numPartitions))
    {
        while (usedProcessor[lastProcessor] && (lastProcessor < numProcessors))
        {
            lastProcessor++;
        }
        while (usedPartition[lastPartition] && (lastPartition < numPartitions))
        {
            lastPartition++;
        }

        if ((lastProcessor<numProcessors) && (lastPartition<numPartitions))
        {
            ptpAssignments[lastPartition]   = lastProcessor;
            if (lastProcessor == thisProcessor)
            {
                thisPartition = lastPartition;
            }
            usedProcessor[lastProcessor] = true;
            usedPartition[lastPartition] = true;
        }
    }

    //
    // Clean up memory
    //
    delete [] allAmounts;
    delete [] usedProcessor;
    delete [] usedPartition;
    delete [] list;
#endif
}


// ****************************************************************************
//  Function: QuicksortTuple3
//
//  Purpose:
//      Sorts a Tuple3 list, sorting over the bytes field and putting the
//      largest element at the end of the list.
//
//  Arguments:
//      list    A list of tuples.
//      size    The number of elements in 'list'.
//
//  Programmer: Hank Childs
//  Creation:   March 6, 2001
//
// ****************************************************************************

inline void
SwapTuple3(Tuple3 &t1, Tuple3 &t2)
{
    Tuple3 tmp;

    tmp.bytes     = t1.bytes;
    tmp.processor = t1.processor;
    tmp.partition = t1.partition;

    t1.bytes      = t2.bytes;
    t1.processor  = t2.processor;
    t1.partition  = t2.partition;

    t2.bytes      = tmp.bytes;
    t2.processor  = tmp.processor;
    t2.partition  = tmp.partition;
}


void
QuicksortTuple3(Tuple3 *list, int size)
{
    //
    // Put in a base case.
    //
    if (size <= 1)
    {
        return;
    }
    if (size == 2)
    {
        if (list[0].bytes > list[1].bytes)
        {
            SwapTuple3(list[0], list[1]);
        }
        return;
    }

    //
    // Pick a pivot.  This is pretty random data (since the zeros were sorted
    // out), so take the middle of three elements as the pivot.
    //
    int pivot1 = list[0].bytes;
    int pivot2 = list[size/2].bytes;
    int pivot3 = list[size-1].bytes;
    if ((pivot1 < pivot2 && pivot2 < pivot3) ||
        (pivot1 > pivot2 && pivot2 > pivot3))
    {
        SwapTuple3(list[0], list[size/2]);
    }
    else if ((pivot1 < pivot3 && pivot3 < pivot2) ||
             (pivot1 > pivot3 && pivot3 > pivot2))
    {
         SwapTuple3(list[0], list[size-1]);
    }
    // no action on else, list[0] is the pivot and it is already in the right
    // position.

    int pivot     = list[0].bytes;
     
    int leftSide  = 1;
    int rightSide = size-1;

    while (leftSide < rightSide)
    {
        //
        // Find an element to swap on the left side.
        //
        while ((list[leftSide].bytes <= pivot) && (leftSide < rightSide))
        {
            leftSide++;
        }

        //
        // Find an element to swap on the right side.
        //
        while ((list[rightSide].bytes >= pivot) && (leftSide < rightSide))
        {
            rightSide--;
        }

        if (leftSide < rightSide)
        {
            SwapTuple3(list[leftSide], list[rightSide]);
        }
    }

    if (leftSide >= size-1)
    {
         //
         // Our pivot is tied for highest, so everything ended up on the left
         // side.
         //
         if (list[size-1].bytes < list[0].bytes)
         {
             SwapTuple3(list[0], list[size-1]);
         }
         QuicksortTuple3(list, size-1);
    }
    else
    {
        if (leftSide > 1)
        {
            QuicksortTuple3(list, leftSide);
        }
        if ((size-leftSide) > 1)
        {
            QuicksortTuple3(list+leftSide, size-leftSide);
        }
    }
}


