// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                 avtAggregateChordLengthDistributionQuery.h                //
// ************************************************************************* //

#ifndef AVT_AGGREGATE_CHORD_LENGTH_DISTRIBUTION_QUERY_H
#define AVT_AGGREGATE_CHORD_LENGTH_DISTRIBUTION_QUERY_H

#include <query_exports.h>

#include <avtLineScanQuery.h>


// ****************************************************************************
//  Class: avtAggregateChordLengthDistributionQuery
//
//  Purpose:
//    A query that calculates the "total" chord length distribution, meaning
//    the CLD summed over all segments created by intersecting a shape with
//    a line.
//
//  Programmer: Hank Childs
//  Creation:   August 31, 2006
//
// ****************************************************************************

class QUERY_API avtAggregateChordLengthDistributionQuery : public avtLineScanQuery
{
  public:
                              avtAggregateChordLengthDistributionQuery();
    virtual                  ~avtAggregateChordLengthDistributionQuery();

    virtual const char       *GetType(void) 
                                 { return "avtAggregateChordLengthDistributionQuery"; };
    virtual const char       *GetDescription(void)
                          { return "Calculating chord length distribution."; };

  protected:
    int                      *numChords;

    virtual void              PreExecute(void);
    virtual void              PostExecute(void);
    virtual void              ExecuteLineScan(vtkPolyData *);
};


#endif


