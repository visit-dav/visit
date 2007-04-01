// ************************************************************************* //
//                               avtTimeQuery.h                              //
// ************************************************************************* //

#ifndef AVT_TIME_QUERY_H
#define AVT_TIME_QUERY_H

#include <query_exports.h>

#include <avtGeneralQuery.h>


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
};


#endif


