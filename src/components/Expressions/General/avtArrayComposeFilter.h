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
//  Modifications:
//
//    Hank Childs, Fri Aug  5 16:48:24 PDT 2005
//    Add support for types and subnames.
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

    virtual void              RefashionDataObjectInfo(void);
    virtual vtkDataArray     *DeriveVariable(vtkDataSet *);
    virtual avtVarType        GetVariableType(void) { return AVT_ARRAY_VAR; };
};


#endif


