// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                          avtCurve3DOptions.h                              //
// ************************************************************************* //

#ifndef AVT_Curve3D_OPTIONS_H
#define AVT_Curve3D_OPTIONS_H

class DBOptionsAttributes;

#include <string>


// ****************************************************************************
//  Functions: avtCurve3DOptions
//
//  Purpose:
//      Creates the options for  Curve3D readers and/or writers.
//
//  Programmer: Kathleen Biagas 
//  Creation:   August 31, 2018
//
// ****************************************************************************

DBOptionsAttributes *GetCurve3DReadOptions(void);
DBOptionsAttributes *GetCurve3DWriteOptions(void);


#endif
