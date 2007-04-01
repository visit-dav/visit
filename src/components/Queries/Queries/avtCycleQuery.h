// ************************************************************************* //
//                              avtCycleQuery.h                              //
// ************************************************************************* //

#ifndef AVT_CYCLE_QUERY_H
#define AVT_CYCLE_QUERY_H

#include <query_exports.h>

#include <avtGeneralQuery.h>


// ****************************************************************************
//  Class: avtCycleQuery
//
//  Purpose:
//      Gets the cycle for the current dataset.
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
//
// ****************************************************************************

class QUERY_API avtCycleQuery : public avtGeneralQuery
{
  public:
                               avtCycleQuery();
    virtual                   ~avtCycleQuery();

    virtual const char        *GetType(void) { return "avtCycleQuery"; };
    virtual const char        *GetDescription(void) 
                                             { return "Getting the cycle"; };

    virtual void               PerformQuery(QueryAttributes *);
    virtual std::string        GetResultMessage(void);
};


#endif


