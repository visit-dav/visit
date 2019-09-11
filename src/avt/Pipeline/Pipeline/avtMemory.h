// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef AVT_MEMORY_H
#define AVT_MEMORY_H
#include <pipeline_exports.h>

// ****************************************************************************
// Class: avtMemory
//
// Purpose:
//   Exposes useful functions for memory usage.
//
// Notes:    
//
// Programmer: Brad Whitlock
// Creation:   Wed Oct 29 14:32:22 PDT 2014
//
// Modifications:
//
// ****************************************************************************

class PIPELINE_API avtMemory
{
public:
    static void GetMemorySize(unsigned long &, unsigned long &);
    static void GetAvailableMemorySize(unsigned long &);
    static void GetTotalMemorySize(unsigned long &);
};

#endif
