// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                            avtAverageValueQuery.h                         //
// ************************************************************************* //

#ifndef AVT_AVERAGE_VALUE_QUERY_H
#define AVT_AVERAGE_VALUE_QUERY_H

#include <query_exports.h>

#include <avtWeightedVariableSummationQuery.h>


// ****************************************************************************
//  Class: avtAverageValueQuery
//
//  Purpose:
//      A query that will calculate the average value of a scalar or vector.
//
//  Programmer: Hank Childs
//  Creation:   May 12. 2011
//
//  Modifications:
//    Kathleen Biagas, Wed Sep 11, 2024
//    Add GetTimeCurveSpecs method.
//
// ****************************************************************************

class QUERY_API avtAverageValueQuery : public avtWeightedVariableSummationQuery
{
  public:
                         avtAverageValueQuery();
    virtual             ~avtAverageValueQuery();

    virtual const char  *GetType(void)
                             { return "avtAverageValueQuery"; }
    const MapNode       &GetTimeCurveSpecs(const QueryAttributes *) override;

  protected:
    virtual avtDataObject_p    CreateVariable(avtDataObject_p d);
    virtual bool               CalculateAverage(void) { return true; }
};


#endif


