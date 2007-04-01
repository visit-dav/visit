// ************************************************************************* //
//                          avtVariableSummationQuery.h                      //
// ************************************************************************* //

#ifndef AVT_VARIABLE_SUMMATION_QUERY_H
#define AVT_VARIABLE_SUMMATION_QUERY_H

#include <query_exports.h>

#include <avtSummationQuery.h>


// ****************************************************************************
//  Class: avtVariableSummationQuery
//
//  Purpose:
//      A query that will sum all of one variables values.
//
//  Programmer: Hank Childs
//  Creation:   February 3, 2004
//
// ****************************************************************************

class QUERY_API avtVariableSummationQuery : public avtSummationQuery
{
  public:
                         avtVariableSummationQuery();
    virtual             ~avtVariableSummationQuery();

    virtual const char  *GetType(void)  
                             { return "avtVariableSummationQuery"; };

  protected:
    virtual void               VerifyInput(void);
};


#endif


