// ************************************************************************* //
//                          avtBinaryMultiplyFilter.h                        //
// ************************************************************************* //

#ifndef AVT_BINARY_MULTIPLY_FILTER_H
#define AVT_BINARY_MULTIPLY_FILTER_H

#include <avtBinaryMathFilter.h>

class     vtkDataArray;


// ****************************************************************************
//  Class: avtBinaryMultiplyFilter
//
//  Purpose:
//      A filter that calculates the product of its two inputs
//
//  Programmer: Sean Ahern
//  Creation:   Thu Jun 27 11:15:41 PDT 2002
//
// ****************************************************************************

class EXPRESSION_API avtBinaryMultiplyFilter : public avtBinaryMathFilter
{
  public:
                              avtBinaryMultiplyFilter() {;};
    virtual                  ~avtBinaryMultiplyFilter() {;};

    virtual const char       *GetType(void)
                               { return "avtBinaryMultiplyFilter"; };
    virtual const char       *GetDescription(void)
                               { return "Calculating binary multiplication"; };

  protected:
    virtual void     DoOperation(vtkDataArray *in1, vtkDataArray *in2,
                                 vtkDataArray *out, int ncomps, int ntuples);
};


#endif


