// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                       avtQueryOverTimeFilter.h                            //
// ************************************************************************* //

#ifndef AVT_QUERYOVERTIME_FILTER_H
#define AVT_QUERYOVERTIME_FILTER_H

#include <query_exports.h>

#include <avtDatasetToDatasetFilter.h>
#include <QueryOverTimeAttributes.h>
#include <SILRestrictionAttributes.h>

#include <string>

class vtkRectilinearGrid;

// ****************************************************************************
//  Class: avtQueryOverTimeFilter
//
//  Purpose:
//    Performs a query over time. 
//
//  Programmer: 
//  Creation:   
//
//  Modifications:
//
// ****************************************************************************

class QUERY_API avtQueryOverTimeFilter : public avtDatasetToDatasetFilter
{
  public:
                          avtQueryOverTimeFilter(const AttributeGroup*);
    virtual              ~avtQueryOverTimeFilter();

    virtual const char   *GetType(void)  { return "avtQueryOverTimeFilter"; };
    virtual const char   *GetDescription(void) { return "Querying over Time"; };

    void                  SetSILAtts(const SILRestrictionAttributes *silAtts);

    virtual bool          FilterSupportsTimeParallelization(void)
                              { return false; };

  protected:
    QueryOverTimeAttributes   atts;
    SILRestrictionAttributes  querySILAtts;
};


#endif


