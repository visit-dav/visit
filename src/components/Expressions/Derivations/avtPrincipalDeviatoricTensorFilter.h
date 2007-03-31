// ************************************************************************* //
//                   avtPrincipalDeviatoricTensorFilter.h                    //
// ************************************************************************* //

#ifndef AVT_PRINCIPAL_DEVIATORIC_TENSOR_FILTER_H
#define AVT_PRINCIPAL_DEVIATORIC_TENSOR_FILTER_H


#include <avtUnaryMathFilter.h>


// ****************************************************************************
//  Class: avtPrincipalDeviatoricTensorFilter
//
//  Purpose:
//      Calculates the principal deviatoric for a tensor.
//
//  Programmer: Hank Childs
//  Creation:   September 23, 2003
//
// ****************************************************************************

class EXPRESSION_API avtPrincipalDeviatoricTensorFilter 
    : public avtUnaryMathFilter
{
  public:
                               avtPrincipalDeviatoricTensorFilter() {;};
    virtual                   ~avtPrincipalDeviatoricTensorFilter() {;};

    virtual const char       *GetType(void)  
                              { return "avtPrincipalDeviatoricTensorFilter"; };
    virtual const char       *GetDescription(void)
                        {return "Calculating the principal deviatoricTensor";};

  protected:
    virtual void              DoOperation(vtkDataArray *in, vtkDataArray *out,
                                          int ncomponents, int ntuples);
    virtual int               GetNumberOfComponentsInOutput(int) { return 3; };
};

#endif


