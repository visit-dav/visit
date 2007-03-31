// ************************************************************************* //
//                              avtUnaryMathFilter.h                         //
// ************************************************************************* //

#ifndef AVT_UNARY_MATH_FILTER_H
#define AVT_UNARY_MATH_FILTER_H

#include <expression_exports.h>
#include <avtSingleInputExpressionFilter.h>

class     vtkDataArray;


// ****************************************************************************
//  Class: avtUnaryMathFilter
//
//  Purpose:
//      A filter that performs a calculation on a single variable.
//
//  Programmer: Sean Ahern
//  Creation:   Wed Jun 12 16:43:09 PDT 2002
//
// ****************************************************************************

class EXPRESSION_API avtUnaryMathFilter : public avtSingleInputExpressionFilter
{
  public:
                              avtUnaryMathFilter() {;};
    virtual                  ~avtUnaryMathFilter() {;};

    virtual const char       *GetType(void)   { return "avtUnaryMathFilter";};
    virtual const char       *GetDescription(void) = 0;

  protected:
    virtual vtkDataArray     *DeriveVariable(vtkDataSet *);
    virtual void              DoOperation(vtkDataArray *in, vtkDataArray *out,
                                          int ncomponents, int ntuples) = 0;
    virtual vtkDataArray     *CreateArray(vtkDataArray *);

    avtCentering              centering;
};


#endif


