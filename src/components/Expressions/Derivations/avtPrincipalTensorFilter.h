// ************************************************************************* //
//                           avtPrincipalTensorFilter.h                      //
// ************************************************************************* //

#ifndef AVT_PRINCIPAL_TENSOR_FILTER_H
#define AVT_PRINCIPAL_TENSOR_FILTER_H


#include <avtUnaryMathFilter.h>


// ****************************************************************************
//  Class: avtPrincipalTensorFilter
//
//  Purpose:
//      Calculates the principals of a tensor.
//
//  Programmer: Hank Childs
//  Creation:   September 23, 2003
//
// ****************************************************************************

class EXPRESSION_API avtPrincipalTensorFilter : public avtUnaryMathFilter
{
  public:
                               avtPrincipalTensorFilter() {;};
    virtual                   ~avtPrincipalTensorFilter() {;};

    virtual const char       *GetType(void)  
                               { return "avtPrincipalTensorFilter"; };
    virtual const char       *GetDescription(void)
                               {return "Calculating the principals";};

  protected:
    virtual void              DoOperation(vtkDataArray *in, vtkDataArray *out,
                                          int ncomponents, int ntuples);
    virtual int               GetNumberOfComponentsInOutput(int) { return 3; };
};

#endif


