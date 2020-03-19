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


// ****************************************************************************
//  Class: avtQueryOverTimeFilter
//
//  Purpose:
//      Acts as a base class for query over time filters. 
//
//  Note: This class is replacing a class that previously held this name. 
//        That class is now named avtTimeLoopQOTFilter.  
//
//  Programmer: Alister Maguire
//  Creation:   Tue Sep 24 13:46:56 MST 2019 
//
//  Modifications:
//
// ****************************************************************************

class QUERY_API avtQueryOverTimeFilter : public avtDatasetToDatasetFilter
{
  public:
                                  avtQueryOverTimeFilter(const AttributeGroup*);
    virtual                      ~avtQueryOverTimeFilter();

    virtual const char           *GetType(void)  
                                    { return "avtQueryOverTimeFilter"; };

    virtual const char           *GetDescription(void) 
                                    { return "Querying over Time"; };

    virtual void                  SetSILAtts(const SILRestrictionAttributes *);

    virtual bool                  FilterSupportsTimeParallelization(void)
                                      { return false; };

  protected:
    QueryOverTimeAttributes       atts;
    SILRestrictionAttributes      querySILAtts;
};

#endif
