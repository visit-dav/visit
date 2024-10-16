// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                       avtTensorContractionExpression.h                    //
// ************************************************************************* //

#ifndef AVT_TENSOR_CONTRACTION_FILTER_H
#define AVT_TENSOR_CONTRACTION_FILTER_H


#include <avtUnaryMathExpression.h>


// ****************************************************************************
//  Class: avtTensorContractionExpression
//
//  Purpose:
//      Calculates the contraction of a tensor.
//
//  Programmer: Cyrus Harrison
//  Creation:   June 1, 2007
//
// ****************************************************************************

class EXPRESSION_API avtTensorContractionExpression : public avtUnaryMathExpression
{
  public:
                               avtTensorContractionExpression();
    virtual                   ~avtTensorContractionExpression();

    virtual const char       *GetType(void)
                               { return "avtTensorContractionExpression"; };
    virtual const char       *GetDescription(void)
                               {return "Calculating tensor contraction";};

  protected:
    virtual void              DoOperation(vtkDataArray *in, vtkDataArray *out,
                                          int ncomponents, int ntuples, vtkDataSet *in_ds);
    virtual int               GetNumberOfComponentsInOutput(int) { return 1; };
};

#endif


