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
//  Modifications:
//
//    Hank Childs, Thu Feb  5 17:11:06 PST 2004
//    Moved inlined constructor and destructor definitions to .C files
//    because certain compilers have problems with them.
//
//    Hank Childs, Tue Aug 16 09:03:39 PDT 2005
//    Add data member cur_mesh so derived types can access mesh when
//    deriving variables.
//
// ****************************************************************************

class EXPRESSION_API avtUnaryMathFilter : public avtSingleInputExpressionFilter
{
  public:
                              avtUnaryMathFilter();
    virtual                  ~avtUnaryMathFilter();

    virtual const char       *GetType(void)   { return "avtUnaryMathFilter";};
    virtual const char       *GetDescription(void) = 0;

  protected:
    virtual vtkDataArray     *DeriveVariable(vtkDataSet *);
    virtual void              DoOperation(vtkDataArray *in, vtkDataArray *out,
                                          int ncomponents, int ntuples) = 0;
    virtual vtkDataArray     *CreateArray(vtkDataArray *);
    virtual int               GetNumberOfComponentsInOutput(int numInInput)
                                          { return numInInput; };

    avtCentering              centering;
    vtkDataSet               *cur_mesh;
};


#endif


