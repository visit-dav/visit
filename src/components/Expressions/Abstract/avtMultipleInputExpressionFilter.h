// ************************************************************************* //
//                        avtMultipleInputExpressionFilter.h                 //
// ************************************************************************* //

#ifndef AVT_MULTIPLE_INPUT_EXPRESSION_FILTER_H
#define AVT_MULTIPLE_INPUT_EXPRESSION_FILTER_H

#include <expression_exports.h>
#include <avtExpressionFilter.h>


// ****************************************************************************
//  Class: avtMultipleInputExpressionFilter
//
//  Purpose:
//      A filter that performs a calculation on multiple variables.
//
//  Programmer: Sean Ahern
//  Creation:   Fri Jun 14 17:57:37 PDT 2002
//
//  Modifications:
//
//    Hank Childs, Fri Sep 19 15:20:35 PDT 2003
//    Add destructor.
//
// ****************************************************************************

class EXPRESSION_API avtMultipleInputExpressionFilter 
    : public avtExpressionFilter
{
  public:
                              avtMultipleInputExpressionFilter() {}
    virtual                  ~avtMultipleInputExpressionFilter();

    virtual void              AddInputVariableName(const char *var);
    void                      ClearInputVariableNames(void);

    virtual const char       *GetType(void)
                                 { return "avtMultipleInputExpressionFilter"; }
    virtual const char       *GetDescription(void) = 0;

  protected:
    std::vector<char *>       varnames;

    virtual bool              IsPointVariable(void);
};

#endif
