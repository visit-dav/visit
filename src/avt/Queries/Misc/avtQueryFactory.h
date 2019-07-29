// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef AVT_QUERY_FACTORY_H
#define AVT_QUERY_FACTORY_H

#include <string>
#include <query_exports.h>

class avtDataObjectQuery;
class QueryAttributes;


// ****************************************************************************
//  Class: avtQueryFactory
//
//  Purpose:
//    avtQueryFactory is a factory for creating queries.  It is a singleton
//    class.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   March 30, 2004 
//
//  Modifications:
//    Kathleen Biagas, Fri Jul 15 11:15:00 PDT 2011
//    Added GetDefaultInputParams.
//
// ****************************************************************************

class QUERY_API avtQueryFactory 
{
  public:
    virtual                        ~avtQueryFactory();

    static avtQueryFactory         *Instance();

    avtDataObjectQuery             *CreateQuery(const QueryAttributes *);
    std::string                     GetDefaultInputParams(
                                        const std::string &queryName);

  private:
                                    avtQueryFactory();
    static void                     DeleteInstance();

    static avtQueryFactory         *instance;
};

#endif

 
