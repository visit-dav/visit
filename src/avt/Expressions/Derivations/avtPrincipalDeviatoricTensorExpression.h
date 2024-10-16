// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//               avtPrincipalDeviatoricTensorExpression.h                    //
// ************************************************************************* //

#ifndef AVT_PRINCIPAL_DEVIATORIC_TENSOR_FILTER_H
#define AVT_PRINCIPAL_DEVIATORIC_TENSOR_FILTER_H


#include <avtUnaryMathExpression.h>


// ****************************************************************************
//  Class: avtPrincipalDeviatoricTensorExpression
//
//  Purpose:
//      Calculates the principal deviatoric for a tensor.
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

class EXPRESSION_API avtPrincipalDeviatoricTensorExpression 
    : public avtUnaryMathExpression
{
  public:
                               avtPrincipalDeviatoricTensorExpression();
    virtual                   ~avtPrincipalDeviatoricTensorExpression();

    virtual const char       *GetType(void)  
                              { return "avtPrincipalDeviatoricTensorExpression"; };
    virtual const char       *GetDescription(void)
                        {return "Calculating the principal deviatoricTensor";};

  protected:
    virtual void              DoOperation(vtkDataArray *in, vtkDataArray *out,
                                          int ncomponents, int ntuples, vtkDataSet *in_ds);
    virtual int               GetNumberOfComponentsInOutput(int) { return 3; };
};

#endif


