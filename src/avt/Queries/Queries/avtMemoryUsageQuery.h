// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                        avtMemoryUsageQuery.h                              //
// ************************************************************************* //

#ifndef AVT_MEMORY_USAGE_QUERY_H
#define AVT_MEMORY_USAGE_QUERY_H

#include <query_exports.h>

#include <avtGeneralQuery.h>
#include <QueryAttributes.h>

#include <string>
#include <vector>

// ****************************************************************************
//  Class: avtMemoryUsageQuery
//
//  Purpose:
//      Gets the memory usage for all engine processors. 
//
//  Programmer: Cyrus Harrison
//  Creation:   March 5, 2008
//
//  Modifications:
//
// ****************************************************************************

class QUERY_API avtMemoryUsageQuery : public avtGeneralQuery
{
  public:
                               avtMemoryUsageQuery();
    virtual                   ~avtMemoryUsageQuery();

    virtual const char        *GetType(void) { return "avtMemoryUsageQuery"; };
    virtual const char        *GetDescription(void) 
                                     { return "Getting Engine Memory Usage"; };

    virtual void               PerformQuery(QueryAttributes *);
    virtual std::string        GetResultMessage(void);

  private:
    QueryAttributes            queryAtts;
    std::vector<double>        memSizeVals;
};


#endif



