// ************************************************************************* //
//                          avtBinarySubtractFilter.h                        //
// ************************************************************************* //

#ifndef AVT_BINARY_SUBTRACT_FILTER_H
#define AVT_BINARY_SUBTRACT_FILTER_H

#include <avtBinaryMathFilter.h>

class     vtkDataArray;


// ****************************************************************************
//  Class: avtBinarySubtractFilter
//
//  Purpose:
//      A filter that calculates the difference of its two inputs
//
//  Programmer: Sean Ahern
//  Creation:   Wed Jun 26 14:32:51 PDT 2002
//
// ****************************************************************************

class EXPRESSION_API avtBinarySubtractFilter : public avtBinaryMathFilter
{
  public:
                              avtBinarySubtractFilter() {;};
    virtual                  ~avtBinarySubtractFilter() {;};

    virtual const char       *GetType(void)
                                  { return "avtBinarySubtractFilter"; };
    virtual const char       *GetDescription(void) 
                                  { return "Calculating binary subtraction"; };

  protected:
    virtual void     DoOperation(vtkDataArray *in1, vtkDataArray *in2,
                                 vtkDataArray *out, int ncomps, int ntuples);
};


#endif


