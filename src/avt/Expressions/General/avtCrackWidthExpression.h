// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ****************************************************************************
//  avtCrackWidthExpression.h
// ****************************************************************************

#ifndef AVT_CRACKWIDTH_EXPRESSION_H
#define AVT_CRACKWIDTH_EXPRESSION_H

#include <avtMultipleInputExpressionFilter.h>

// ****************************************************************************
//  Class: avtCrackWidthExpression
//
//  Purpose:  Calculates crack width, given a crack_dir (vector),
//            strain (tensor), crack number, and cell volume (scalar).
//
//  Programmer: Kathleen Biagas 
//  Creation:   June 13, 2022
//
// ****************************************************************************

class EXPRESSION_API avtCrackWidthExpression : public avtMultipleInputExpressionFilter
{
public:
                   avtCrackWidthExpression();
    virtual       ~avtCrackWidthExpression();

    void           ProcessArguments(ArgsExpr *, ExprPipelineState *) override;
    int            NumVariableArguments() override
                       { return 5; }
    const char    *GetType(void) override
                       { return "avtCrackWidthExpression"; }
    const char    *GetDescription(void) override
                       { return "Calculate crack width"; }

protected:

    int            GetVariableDimension(void) override
                       { return 1; }

    vtkDataArray  *DeriveVariable(vtkDataSet *, int ) override;

private:
   int crackNum;

};

#endif /* AVT_CRACKWIDTH_EXPRESSION_H */
