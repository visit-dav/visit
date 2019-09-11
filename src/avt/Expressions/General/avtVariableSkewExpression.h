// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                       avtVariableSkewExpression.h                         //
// ************************************************************************* //

#ifndef AVT_VARIALE_SKEW_FILTER_H
#define AVT_VARIALE_SKEW_FILTER_H


#include <avtBinaryMathExpression.h>

class     vtkDataArray;


// ****************************************************************************
//  Class: avtVariableSkewExpression
//
//  Purpose:
//      A filter that calculates the skewed value of its input.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   March 2, 2005 
//
//  Modifications:
//    Brad Whitlock, Fri Dec 19 15:55:26 PST 2008
//    I removed SkewTheValue.
//
// ****************************************************************************

class EXPRESSION_API avtVariableSkewExpression : public avtBinaryMathExpression
{
  public:
                              avtVariableSkewExpression();
    virtual                  ~avtVariableSkewExpression();

    virtual const char       *GetType(void)   { return "avtVariableSkewExpression"; };
    virtual const char       *GetDescription(void) 
                                   { return "Calculating skew"; };

  protected:
    virtual void     DoOperation(vtkDataArray *in1, vtkDataArray *in2,
                                 vtkDataArray *out, int ncomponents, int ntuples);
};


#endif


