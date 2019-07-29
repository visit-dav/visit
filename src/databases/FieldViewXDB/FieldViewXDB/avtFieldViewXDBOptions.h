// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                             avtFieldViewXDBOptions.h
// ************************************************************************* //

#ifndef AVT_FIELDVIEW_XDB_OPTIONS_H
#define AVT_FIELDVIEW_XDB_OPTIONS_H

class DBOptionsAttributes;

// ****************************************************************************
//  Functions: avtFieldViewXDBOptions
//
//  Purpose:
//      Creates the options for FieldViewXDB readers and/or writers.
//
//  Programmer: Brad Whitlock
//  Creation:   Mon Sep 28 16:06:41 PDT 2015
//
// ****************************************************************************

DBOptionsAttributes *GetFieldViewXDBReadOptions(void);
DBOptionsAttributes *GetFieldViewXDBWriteOptions(void);


#endif
