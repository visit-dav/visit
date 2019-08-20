// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                            avtUintahOptions.h                             //
// ************************************************************************* //

#ifndef AVT_UINTAH_OPTIONS_H
#define AVT_UINTAH_OPTIONS_H

class DBOptionsAttributes;

namespace UintahDBOptions
{
  // Read options
  const char *const UINTAH_LOAD_EXTRA = "Load extra geometry";
  const char *const UINTAH_LOAD_EXTRA_NONE    = "None";
  const char *const UINTAH_LOAD_EXTRA_CELLS   = "Cells";
  const char *const UINTAH_LOAD_EXTRA_PATCHES = "Patches";

  const char *const UINTAH_DATA_VARIES_OVER_TIME = "Data varies over time";
}

// ****************************************************************************
//  Functions: avtUintahOptions
//
//  Purpose:
//      Creates the options for Uintah readers and/or writers.
//
//  Programmer: Allen Sanderson -- generated by xml2avt
//  Creation:   Feb 2017
//
// ****************************************************************************

DBOptionsAttributes *GetUintahReadOptions(void);
DBOptionsAttributes *GetUintahWriteOptions(void);

#endif
