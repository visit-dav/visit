// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                               avtTimeQuery.h                              //
// ************************************************************************* //

#ifndef AVT_TIME_QUERY_H
#define AVT_TIME_QUERY_H

#include <query_exports.h>

#include <avtGeneralQuery.h>
#include <QueryAttributes.h>

#include <string>

// ****************************************************************************
//  Class: avtTimeQuery
//
//  Purpose:
//      Gets the time for the current dataset.
//
//  Programmer: Hank Childs
//  Creation:   October 4, 2003
//
//  Modifications:
//
//    Hank Childs, Thu Feb  5 17:11:06 PST 2004
//    Moved inlined constructor and destructor definitions to .C files
//    because certain compilers have problems with them.
//
//    Cyrus Harrison, Tue Sep 18 13:57:12 PDT 2007
//    Added queryAtts
//
// ****************************************************************************

class QUERY_API avtTimeQuery : public avtGeneralQuery
{
  public:
                               avtTimeQuery();
    virtual                   ~avtTimeQuery();

    virtual const char        *GetType(void) { return "avtTimeQuery"; };
    virtual const char        *GetDescription(void) 
                                             { return "Getting the time"; };

    virtual void               PerformQuery(QueryAttributes *);
    virtual std::string        GetResultMessage(void);
  
  private:
    QueryAttributes            queryAtts;
  
};


#endif


