// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                 avtIndividualChordLengthDistributionQuery.h               //
// ************************************************************************* //

#ifndef AVT_INDIVIDUAL_CHORD_LENGTH_DISTRIBUTION_QUERY_H
#define AVT_INDIVIDUAL_CHORD_LENGTH_DISTRIBUTION_QUERY_H

#include <query_exports.h>

#include <avtLineScanQuery.h>


// ****************************************************************************
//  Class: avtIndividualChordLengthDistributionQuery
//
//  Purpose:
//    A query that calculates the individual chord length distribution.
//    In this case, individual means that if a chord intersects a shape to form
//    several, disjoint segments, those segments are considered individually.
//
//  Programmer: Hank Childs
//  Creation:   July 7, 2006
//
//  Modifications:
//
//    Hank Childs, Fri Sep  1 15:13:33 PDT 2006
//    Renamed to individual chord length distribution query.
//
// ****************************************************************************

class QUERY_API avtIndividualChordLengthDistributionQuery : public avtLineScanQuery
{
  public:
                              avtIndividualChordLengthDistributionQuery();
    virtual                  ~avtIndividualChordLengthDistributionQuery();

    virtual const char       *GetType(void) 
                                 { return "avtIndividualChordLengthDistributionQuery"; };
    virtual const char       *GetDescription(void)
                          { return "Calculating chord length distribution."; };

  protected:
    int                      *numChords;

    virtual void              PreExecute(void);
    virtual void              PostExecute(void);
    virtual void              ExecuteLineScan(vtkPolyData *);
};


#endif


