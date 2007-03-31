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
//  Modifications:
//      Sean Ahern, Fri Jun 13 11:16:09 PDT 2003
//      Added the NumVariableArguments function, specifying that all
//      subclasses of BinaryMathFilter process two variable arguments.
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
    virtual int               NumVariableArguments() { return 2; }

  protected:
    virtual vtkDataArray     *DeriveVariable(vtkDataSet *);
    virtual void              DoOperation(vtkDataArray *in1, vtkDataArray *in2,
                                          vtkDataArray *out, int, int) = 0;

    avtCentering              centering;
};


#endif


