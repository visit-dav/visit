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
//  Programmer: Olivier Cessenat
//  Creation:   Oct 5, 2023
//
//  Copied from Curve2D plugin as of Aug 31, 2018 and adjusted for 3D
//  and make a global mesh with materials out of the figures.
//
// ****************************************************************************

DBOptionsAttributes *GetCurve3DReadOptions(void);
DBOptionsAttributes *GetCurve3DWriteOptions(void);


#endif
