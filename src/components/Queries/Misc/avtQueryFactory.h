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
//
// ****************************************************************************

class QUERY_API avtQueryFactory 
{
  public:
    virtual                        ~avtQueryFactory();

    static avtQueryFactory         *Instance();

    avtDataObjectQuery             *CreateQuery(const QueryAttributes *);
    avtDataObjectQuery             *CreateTimeQuery(const QueryAttributes *);

  private:
                                    avtQueryFactory();

    static avtQueryFactory         *instance;
};

#endif

 
