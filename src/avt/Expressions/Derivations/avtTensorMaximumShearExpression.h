// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                     avtTensorMaximumShearExpression.h                     //
// ************************************************************************* //

#ifndef AVT_TENSOR_MAXIMUM_SHEAR_FILTER_H
#define AVT_TENSOR_MAXIMUM_SHEAR_FILTER_H


#include <avtUnaryMathExpression.h>


// ****************************************************************************
//  Class: avtTensorMaximumShearExpression
//
//  Purpose:
//      Calculates the maximum shear for a tensor.
//
//  Programmer: Hank Childs
//  Creation:   September 23, 2003
//
//  Modifications:
//
//    Hank Childs, Thu Feb  5 17:11:06 PST 2004
//    Moved inlined constructor and destructor definitions to .C files
//    because certain compilers have problems with them.
//
// ****************************************************************************

class EXPRESSION_API avtTensorMaximumShearExpression : public avtUnaryMathExpression
{
  public:
                               avtTensorMaximumShearExpression();
    virtual                   ~avtTensorMaximumShearExpression();

    virtual const char       *GetType(void)  
                                     { return "avtTensorMaximumShearExpression"; };
    virtual const char       *GetDescription(void)
                                     { return "Calculating maximum shear"; };

  protected:
    virtual void              DoOperation(vtkDataArray *in, vtkDataArray *out,
                                          int ncomponents, int ntuples, vtkDataSet *in_ds);
    virtual int               GetNumberOfComponentsInOutput(int) { return 1; };
};

#endif


