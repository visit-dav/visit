// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                          avtSymmEvalExpression.h                          //
// ************************************************************************* //

#ifndef AVT_SYMM_EVAL_EXPRESSION_H
#define AVT_SYMM_EVAL_EXPRESSION_H

#include <avtExpressionFilter.h>

#include <string>
#include <vector>

// ****************************************************************************
//  Class: avtSymmEvalExpression
//
//  Purpose:
//      The base class for doing symmetry evaluations.
//
//  Programmer: Hank Childs
//  Creation:   January 21, 2006
//
// ****************************************************************************

class EXPRESSION_API avtSymmEvalExpression : public avtExpressionFilter
{
  public:
                              avtSymmEvalExpression();
    virtual                  ~avtSymmEvalExpression();

    virtual const char       *GetType(void) { return "avtSymmEvalExpression";};
    virtual const char       *GetDescription(void) 
                                         { return "Evaluating symmetry"; };
    virtual void              ProcessArguments(ArgsExpr*, ExprPipelineState *);
    virtual int               NumVariableArguments() { return 2; };
    virtual void              AddInputVariableName(const char *);

  protected:
    std::vector<double>       inputParameters;
    std::vector<std::string>  varnames;

    virtual void              Execute(void);
    virtual int               GetNumberOfInputParameters() = 0;
    virtual avtDataObject_p   TransformData(avtDataObject_p) = 0;
};


#endif


