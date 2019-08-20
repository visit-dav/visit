// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                             avtPLYOptions.h                              //
// ************************************************************************* //

#ifndef AVT_PLY_OPTIONS_H
#define AVT_PLY_OPTIONS_H

class DBOptionsAttributes;

#include <string>


// ****************************************************************************
//  Functions: avtPLYOptions
//
//  Purpose:
//      Creates the options for  PLY readers and/or writers.
//
//  Programmer: pugmire -- generated by xml2avt
//  Creation:   Tue Apr 16 09:43:05 PDT 2013
//
// ****************************************************************************

DBOptionsAttributes *GetPLYReadOptions(void);
DBOptionsAttributes *GetPLYWriteOptions(void);


#endif
