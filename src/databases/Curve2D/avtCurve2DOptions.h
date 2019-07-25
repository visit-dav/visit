// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                          avtCurve2DOptions.h                              //
// ************************************************************************* //

#ifndef AVT_Curve2D_OPTIONS_H
#define AVT_Curve2D_OPTIONS_H

class DBOptionsAttributes;

#include <string>


// ****************************************************************************
//  Functions: avtCurve2DOptions
//
//  Purpose:
//      Creates the options for  Curve2D readers and/or writers.
//
//  Programmer: Kathleen Biagas 
//  Creation:   August 31, 2018
//
// ****************************************************************************

DBOptionsAttributes *GetCurve2DReadOptions(void);
DBOptionsAttributes *GetCurve2DWriteOptions(void);


#endif
