// ************************************************************************* //
//                            avtRangeMaxTable.C                             //
// ************************************************************************* //

#include <avtRangeMaxTable.h>

// For NULL
#include <stdio.h>

#include <BadIndexException.h>


// ****************************************************************************
//  Method: avtRangeMaxTable constructor
//
//  Programmer: Hank Childs
//  Creation:   December 7, 2001
//
// ****************************************************************************

avtRangeMaxTable::avtRangeMaxTable()
{
    entries    = NULL;
    numEntries = 0;
    powRange   = NULL;
}


// ****************************************************************************
//  Method: avtRangeMaxTable destructor
//
//  Programmer: Hank Childs
//  Creation:   December 7, 2001
//
// ****************************************************************************

avtRangeMaxTable::~avtRangeMaxTable()
{
    if (entries != NULL)
    {
        delete [] entries;
        entries = NULL;
    }
    if (powRange != NULL)
    {
        delete [] powRange;
        powRange = NULL;
    }
}


// ****************************************************************************
//  Method: avtRangeMaxTable::SetTable
//
//  Purpose:
//      Sets up the table.  This accepts the input and creates its internal
//      data structures for quick lookups.
//
//  Arguments:
//      tab       The table.
//      tabSize   The size of the table.
//
//  Notes:        This object owns the "tab" argument after the method call.
//
//  Programmer:   Hank Childs
//  Creation:     December 7, 2001
//
// ****************************************************************************

void
avtRangeMaxTable::SetTable(int tabSize, float *tab)
{
    int  i, j;

    numEntries = tabSize;
    entries    = tab;

    //
    // Off-by-one errors make it necessary to start logRange at 2.
    //
    logRange = 2;  
    int temp = 1;
    while (temp < numEntries)
    {
        logRange++;
        temp *= 2;
    }

    //
    // Pow range will be an array that is indexed as follows:
    // - the first "logRange" entries correspond to entry 1 of the table.
    // - the second "logRange" entries correspond to entry 2 of the table.
    // ...
    // Of a section of "logRange" entries, the Oth element is the value
    // of the table entry (say entry k).
    // The 1st element is the maximum of k and k+1.
    // The 2nd element is the maximum of k - k+3.
    // ...
    // The ith element is the maximum of k - k+2^i.
    // ...
    //

    //
    // We must initialize the 0th elements with the table.
    //
    powRange = new float[numEntries*logRange];
    for (j = 0 ; j < numEntries ; j++)
    {
        powRange[j*logRange + 0] = entries[j];
    }

    //
    // Now recursively determine the "ith" elements in a way that avoids
    // computation.
    //
    int offset = 1;
    for (i = 1 ; i < logRange ; i++)
    {
        for (j = 0 ; j < numEntries ; j++)
        {
            if (j + offset < numEntries)
            {
                float val1 = powRange[j*logRange + i-1];
                float val2 = powRange[(j+offset)*logRange + i-1];
                float max = (val1 > val2 ? val1 : val2);
                powRange[j*logRange + i] = max;
            }
            else if ((j + offset/2) >= numEntries)
            {
                powRange[j*logRange + i] = powRange[j*logRange + (i-1)];
            }
            else
            {
                float val1 = powRange[j*logRange + i-1];
                float val2 = powRange[(numEntries-1-offset/2)*logRange + i-1];
                float max = (val1 > val2 ? val1 : val2);
                powRange[j*logRange + i] = max;
            }
        }
        offset *= 2;
        if (i == 1)
        {
            offset = 1; // The numbers work out funny for the small cases.
        }
    }
}


// ****************************************************************************
//  Method: avtRangeMaxTable::GetMaximumOverRange
//
//  Purpose:
//      Determines the maximum value of the table over the specified range.
//
//  Arguments:
//      minIndex   The minimum index.
//      maxIndex   The maximum index.
//
//  Returns:       The maximum value of the table between the miminum and
//                 maximum indices specified, inclusive on both ends.
//
//  Notes:         The cost to call this routine should be constant, regardless
//                 of table size.  It should be *very* cheap.
//
//  Programmer:    Hank Childs
//  Creation:      December 7, 2001
//
// ****************************************************************************

float
avtRangeMaxTable::GetMaximumOverRange(int minIndex, int maxIndex)
{
    if (minIndex < 0 || minIndex >= numEntries)
    {
        EXCEPTION2(BadIndexException, minIndex, numEntries);
    }
    if (maxIndex < 0 || maxIndex >= numEntries)
    {
        EXCEPTION2(BadIndexException, maxIndex, numEntries);
    }

    int diff = maxIndex-minIndex;
    int powIndex = 0;
    int distanceFromEnd = 0;
    while (diff >= 1)
    {
        diff /= 2;
        distanceFromEnd *= 2;
        if (distanceFromEnd == 0)
        {
            distanceFromEnd = 1;
        }
        powIndex++;
    }

    //
    // Find the maximum over some range from the front half, the maximum over
    // some range from the back half, and then the maximum of those two.
    // 
    float val1 = powRange[minIndex*logRange + powIndex];
    int index2 = maxIndex - distanceFromEnd;
    float val2 = powRange[index2*logRange + powIndex];

    return (val1 > val2 ? val1 : val2);
}


