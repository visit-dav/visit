// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                           avtRangeMaxTable.h                              //
// ************************************************************************* //

#ifndef AVT_RANGE_MAX_TABLE_H
#define AVT_RANGE_MAX_TABLE_H

#include <pipeline_exports.h>


// ****************************************************************************
//  Class: avtRangeMaxTable
//
//  Purpose:
//      A table that can efficiently find the maximum value over a range.
//
//  Programmer: Hank Childs
//  Creation:   December 7, 2001
//
//  Modifications:
//
//    Hank Childs, Tue Dec 18 10:04:43 PST 2007
//    Define private copy constructor and assignment operator to prevent
//    accidental use of default, bitwise copy implementations.
//
// ****************************************************************************

class PIPELINE_API avtRangeMaxTable
{
  public:
                                 avtRangeMaxTable();
    virtual                     ~avtRangeMaxTable();

    void                         SetTable(int, double *);
    double                       GetMaximumOverRange(int, int);

  protected:
    int                          numEntries;
    double                      *entries;
    double                      *powRange;
    int                          logRange;

    // These methods are defined to prevent accidental use of bitwise copy
    // implementations.  If you want to re-define them to do something
    // meaningful, that's fine.
                        avtRangeMaxTable(const avtRangeMaxTable &) {;};
    avtRangeMaxTable   &operator=(const avtRangeMaxTable &) { return *this; };
};


#endif


