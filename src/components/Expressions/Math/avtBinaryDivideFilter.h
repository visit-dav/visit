// ************************************************************************* //
//                           avtBinaryDivideFilter.h                         //
// ************************************************************************* //

#ifndef AVT_BINARY_DIVIDE_FILTER_H
#define AVT_BINARY_DIVIDE_FILTER_H

#include <avtBinaryMathFilter.h>

class     vtkDataArray;


// ****************************************************************************
//  Class: avtBinaryDivideFilter
//
//  Purpose:
//      A filter that calculates the quotient of its two inputs
//
//  Programmer: Sean Ahern
//  Creation:   Tue Jun 11 16:23:45 PDT 2002
//
// ****************************************************************************

class EXPRESSION_API avtBinaryDivideFilter : public avtBinaryMathFilter
{
  public:
                              avtBinaryDivideFilter() {;};
    virtual                  ~avtBinaryDivideFilter() {;};

    virtual const char       *GetType(void) 
                                     { return "avtBinaryDivideFilter"; };
    virtual const char       *GetDescription(void) 
                                     { return "Calculating binary division"; };

  protected:
    virtual void     DoOperation(vtkDataArray *in1, vtkDataArray *in2,
                                 vtkDataArray *out, int ncomps, int ntuples);
};


#endif


