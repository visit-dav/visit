// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include <avtMemory.h>
#include <stdio.h>

#ifndef _WIN32
#include <unistd.h>
#if defined(__APPLE__)
#include <malloc/malloc.h> // for mstat
#endif
#if defined(VISIT_BLUE_GENE_Q) && defined(PARALLEL)
#include <spi/include/kernel/memory.h>
#endif
#endif

// ****************************************************************************
//  Function: avtMemory::GetMemorySize
//
//  Purpose:
//      Gets the amount of memory in use and the RSS (resident set size).
//
//  Programmer: Hank Childs (from code from Peter Lindstrom)
//  Creation:   February 28, 2008
//
//  Modifications:
//    Cyrus Harrison, Wed Apr  1 12:16:19 PDT 2009
//    Modified to use unsigned ints so we can go above 2 gigabytes (up to
//    4 gigabytes).
//
//    Brad Whitlock, Tue Jun 23 17:07:57 PDT 2009
//    I added a Mac implementation.
//
//    David Camp, Mon Mar  5 14:04:42 PST 2012
//    Modified to use unsigned long so we can go above 4 gigabytes
//
//    Brad Whitlock, Wed Oct 29 14:27:17 PDT 2014
//    Added BGQ implementation.
//
// ****************************************************************************

void
avtMemory::GetMemorySize(unsigned long &size, unsigned long &rss)
{
    size = 0;
    rss  = 0;
#if defined(__APPLE__)
    struct mstats m = mstats();
    size = (unsigned long)m.bytes_used; // The bytes used out of the bytes_total.
    rss = (unsigned long)m.bytes_total; // not quite accurate but this should be the total
                                        // amount allocated by malloc.
#elif !defined(_WIN32)

#if defined(VISIT_BLUE_GENE_Q) && defined(PARALLEL)
    uint64_t mm;
    Kernel_GetMemorySize(KERNEL_MEMSIZE_MMAP, &mm);
    size = (unsigned long)mm;
#else
    FILE *file = fopen("/proc/self/statm", "r");
    if (file == NULL)
    {
        return;
    }

    int count = fscanf(file, "%lu%lu", &size, &rss);
    if (count != 2)
    {
        fclose(file);
        return;
    }
    size *= (unsigned long)getpagesize();
    rss  *= (unsigned long)getpagesize();
    fclose(file);
#endif

#endif
}

// ****************************************************************************
// Method: avtMemory::GetAvailableMemorySize
//
// Purpose:
//   Returns the amount of memory available to this process.
//
// Arguments:
//   size : The return value.
//
// Returns:    
//
// Note:       Need to add implementations for other platforms.
//
// Programmer: Brad Whitlock
// Creation:   Wed Oct 29 14:30:40 PDT 2014
//
// Modifications:
//
// ****************************************************************************

void
avtMemory::GetAvailableMemorySize(unsigned long &size)
{
    size = 0;
#if defined(VISIT_BLUE_GENE_Q) && defined(PARALLEL)
    uint64_t mm;
    Kernel_GetMemorySize(KERNEL_MEMSIZE_HEAPAVAIL, &mm);
    size = (unsigned long)mm;
#endif
}

// ****************************************************************************
// Method: avtMemory::GetTotalMemorySize
//
// Purpose:
//   Returns the total amount of memory available to this process before any
//   allocations were made.
//
// Arguments:
//   size : The return value.
//
// Returns:    
//
// Note:       Need to add implementations for other platforms.
//
// Programmer: Brad Whitlock
// Creation:   Wed Oct 29 14:30:40 PDT 2014
//
// Modifications:
//
// ****************************************************************************

void
avtMemory::GetTotalMemorySize(unsigned long &size)
{
    size = 0;
#if defined(VISIT_BLUE_GENE_Q) && defined(PARALLEL)
    uint64_t mm;
    Kernel_GetMemorySize(KERNEL_MEMSIZE_HEAPMAX, &mm);
    size = (unsigned long)mm;
#endif
}
