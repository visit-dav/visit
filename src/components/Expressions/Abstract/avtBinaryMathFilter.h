// ************************************************************************* //
//                             avtBinaryMathFilter.h                         //
// ************************************************************************* //

#ifndef AVT_BINARY_MATH_FILTER_H
#define AVT_BINARY_MATH_FILTER_H

#include <expression_exports.h>
#include <avtMultipleInputExpressionFilter.h>

class     vtkDataArray;


// ****************************************************************************
//  Class: avtBinaryMathFilter
//
//  Purpose:
//      A filter that performs a calculation on two variables.
//
//  Programmer: Sean Ahern
//  Creation:   Tue Jun 25 19:40:54 PDT 2002
//
// ****************************************************************************

class EXPRESSION_API avtBinaryMathFilter 
    : public avtMultipleInputExpressionFilter
{
  public:
                              avtBinaryMathFilter() {;};
    virtual                  ~avtBinaryMathFilter() {;};

    virtual const char       *GetType(void)   { return "avtBinaryMathFilter";};
    virtual const char       *GetDescription(void) = 0;

  protected:
    virtual vtkDataArray     *DeriveVariable(vtkDataSet *);
    virtual void              DoOperation(vtkDataArray *in1, vtkDataArray *in2,
                                          vtkDataArray *out, int, int) = 0;

    avtCentering              centering;
};


#endif


