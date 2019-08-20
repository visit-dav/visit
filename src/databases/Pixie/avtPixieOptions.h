// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                             avtPixieOptions.h                             //
// ************************************************************************* //

#ifndef AVT_PIXIE_OPTIONS_H
#define AVT_PIXIE_OPTIONS_H

class DBOptionsAttributes;

#include <string>

namespace PixieDBOptions
{
    enum PartitioningDirection {XSLAB=0, YSLAB, ZSLAB, KDTREE};

  // Read options
    const char *const RDOPT_PARTITIONING = "Partitioning";
    const char *const RDOPT_DUPLICATE    = "Duplicate data for particle advection (slower for all other techniques)";
}

// ****************************************************************************
//  Functions: avtSiloOptions
//
//  Purpose:
//      Creates the options for  Silo readers and/or writers.
//
//  Programmer: Jean Favre
//  Creation:   Thu Jun 21 16:20:41 PDT 2012
//
// ****************************************************************************

DBOptionsAttributes *GetPixieReadOptions(void);
DBOptionsAttributes *GetPixieWriteOptions(void);


#endif
