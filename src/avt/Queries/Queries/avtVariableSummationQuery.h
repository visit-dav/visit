// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                          avtVariableSummationQuery.h                      //
// ************************************************************************* //

#ifndef AVT_VARIABLE_SUMMATION_QUERY_H
#define AVT_VARIABLE_SUMMATION_QUERY_H

#include <query_exports.h>

#include <avtSummationQuery.h>

class avtCondenseDatasetFilter;


// ****************************************************************************
//  Class: avtVariableSummationQuery
//
//  Purpose:
//      A query that will sum all of one variables values.
//
//  Programmer: Hank Childs
//  Creation:   February 3, 2004
//
//  Modifications:
//    Kathleen Bonnell, Thu Mar  2 15:05:17 PST 2006
//    Add ApplyFilters() and condense filter.
//
//    Kathleen Biagas, Wed Sep 11, 2024
//    Added GetTimeCurveSpecs.
//
// ****************************************************************************

class QUERY_API avtVariableSummationQuery : public avtSummationQuery
{
  public:
                         avtVariableSummationQuery();
    virtual             ~avtVariableSummationQuery();

    virtual const char  *GetType(void)
                             { return "avtVariableSummationQuery"; }

    const MapNode       &GetTimeCurveSpecs(const QueryAttributes *) override;

  protected:
    avtCondenseDatasetFilter   *condense;

    virtual void               VerifyInput(void);
    virtual avtDataObject_p    ApplyFilters(avtDataObject_p);
};


#endif


