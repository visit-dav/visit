// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers. See the top-level LICENSE file for dates and other
// details. No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                             avtXolotlOptions.h                              //
// ************************************************************************* //

#ifndef AVT_Xolotl_OPTIONS_H
#define AVT_Xolotl_OPTIONS_H

class DBOptionsAttributes;

#include <string>


// ****************************************************************************
//  Functions: avtXolotlOptions
//
//  Purpose:
//      Creates the options for  Xolotl readers and/or writers.
//
//  Programmer: js9 -- generated by xml2avt
//  Creation:   Tue Mar 22 15:09:52 PST 2016
//
// ****************************************************************************

DBOptionsAttributes *GetXolotlReadOptions(void);
DBOptionsAttributes *GetXolotlWriteOptions(void);


#endif
