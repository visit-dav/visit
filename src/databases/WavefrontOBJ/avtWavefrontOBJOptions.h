// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                             avtWavefrontOBJOptions.h                              //
// ************************************************************************* //

#ifndef AVT_WAVEFRONTOBJ_OPTIONS_H
#define AVT_WAVEFRONTOBJ_OPTIONS_H

class DBOptionsAttributes;

#include <string>


// ****************************************************************************
//  Functions: avtWavefrontOBJOptions
//
//  Purpose:
//      Creates the options for WavefrontOBJ readers and/or writers.
//
//  Programmer: Justin Privitera
//  Creation:   11/03/23
//
// ****************************************************************************

DBOptionsAttributes *GetWavefrontOBJReadOptions(void);
DBOptionsAttributes *GetWavefrontOBJWriteOptions(void);


#endif
