// ************************************************************************* //
//                        avtMultipleInputExpressionFilter.h                 //
// ************************************************************************* //

#ifndef AVT_MULTIPLE_INPUT_EXPRESSION_FILTER_H
#define AVT_MULTIPLE_INPUT_EXPRESSION_FILTER_H

#include <expression_exports.h>

#include <avtExpressionStreamer.h>


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
//    Hank Childs, Mon Dec 27 10:37:15 PST 2004
//    Changed inheritance from avtExpressionFilter to avtExpressionStreamer.
//
//    Hank Childs, Thu Jan  6 11:00:35 PST 2005
//    Added MixedCenteringYieldsZonal method to give derived types a hook into
//    altering behavior of IsPointVariable.
//
// ****************************************************************************

class EXPRESSION_API avtMultipleInputExpressionFilter 
    : virtual public avtExpressionStreamer
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
    virtual bool              MixedCenteringYieldsZonal(void) { return true; };
};

#endif


