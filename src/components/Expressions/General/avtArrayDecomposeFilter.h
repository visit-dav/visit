// ************************************************************************* //
//                         avtArrayDecomposeFilter.h                         //
// ************************************************************************* //

#ifndef AVT_ARRAY_DECOMPOSE_FILTER_H
#define AVT_ARRAY_DECOMPOSE_FILTER_H

#include <avtSingleInputExpressionFilter.h>

class     vtkDataArray;
class     ArgsExpr;
class     ExprPipelineState;

// ****************************************************************************
//  Class: avtArrayDecomposeFilter
//
//  Purpose:
//      Composes scalar variables into an array.
//          
//  Programmer: Hank Childs
//  Creation:   July 21, 2005
//
// ****************************************************************************

class EXPRESSION_API avtArrayDecomposeFilter 
    : public avtSingleInputExpressionFilter
{
  public:
                              avtArrayDecomposeFilter();
    virtual                  ~avtArrayDecomposeFilter();

    virtual const char       *GetType(void) 
                                     { return "avtArrayDecomposeFilter"; };
    virtual const char       *GetDescription(void)
                                     { return "Decomposing an array"; };
    virtual void              ProcessArguments(ArgsExpr*, ExprPipelineState *);

  protected:
    bool issuedWarning;
    int  index;

    virtual vtkDataArray     *DeriveVariable(vtkDataSet *);
    virtual void              PreExecute(void);
};


#endif


