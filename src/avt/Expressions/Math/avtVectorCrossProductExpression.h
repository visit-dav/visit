// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                       avtVectorCrossProductExpression.h                   //
// ************************************************************************* //

#ifndef AVT_VCROSSPRODUCT_FILTER_H
#define AVT_VCROSSPRODUCT_FILTER_H

#include <avtBinaryMathExpression.h>

class     vtkDataArray;


// ****************************************************************************
//  Class: avtVectorCrossProductExpression
//
//  Purpose:
//      A filter that calculates the vector cross product of its two inputs.
//
//  Programmer: Sean Ahern
//  Creation:   Wed Jun 11 13:29:38 PDT 2003
//
//  Modifications:
//
//    Hank Childs, Thu Feb  5 17:11:06 PST 2004
//    Moved inlined constructor and destructor definitions to .C files
//    because certain compilers have problems with them.
//
//    Hank Childs, Wed Dec 21 14:51:43 CST 2011
//    Make cross product of 2D vectors be scalars.
//
// ****************************************************************************

class EXPRESSION_API avtVectorCrossProductExpression : public avtBinaryMathExpression
{
  public:
                              avtVectorCrossProductExpression();
    virtual                  ~avtVectorCrossProductExpression();

    virtual const char       *GetType(void)   { return "avtVectorCrossProductExpression"; };
    virtual const char       *GetDescription(void)
                                     { return "Calculating vector cross product"; };
    virtual int               NumVariableArguments() { return 2; }
    virtual int               GetNumberOfComponentsInOutput(int ncompsIn1,
                                                            int ncompsIn2)
                                      { return GetVariableDimension(); };
    virtual int               GetVariableDimension(void);

  protected:
    virtual void     DoOperation(vtkDataArray *in1, vtkDataArray *in2,
                                 vtkDataArray *out, int ncomps, int ntuples);
};


#endif
