// ************************************************************************* //
//                          avtArrayComposeFilter.h                          //
// ************************************************************************* //

#ifndef AVT_ARRAY_COMPOSE_FILTER_H
#define AVT_ARRAY_COMPOSE_FILTER_H

#include <avtMultipleInputExpressionFilter.h>

class     vtkDataArray;
class     ArgsExpr;
class     ExprPipelineState;

// ****************************************************************************
//  Class: avtArrayComposeFilter
//
//  Purpose:
//      Composes scalar variables into an array.
//          
//  Programmer: Hank Childs
//  Creation:   July 21, 2005
//
// ****************************************************************************

class EXPRESSION_API avtArrayComposeFilter 
    : public avtMultipleInputExpressionFilter
{
  public:
                              avtArrayComposeFilter();
    virtual                  ~avtArrayComposeFilter();

    virtual const char       *GetType(void) { return "avtArrayComposeFilter"; };
    virtual const char       *GetDescription(void)
                                     { return "Composing an array"; };
    virtual void              ProcessArguments(ArgsExpr*, ExprPipelineState *);
    virtual int               NumVariableArguments(void) { return nargs; };

  protected:
    int                       nargs;

    virtual vtkDataArray     *DeriveVariable(vtkDataSet *);
};


#endif


