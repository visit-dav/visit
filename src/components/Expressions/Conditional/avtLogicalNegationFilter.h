// ************************************************************************* //
//                          avtLogicalNegationFilter.h                       //
// ************************************************************************* //

#ifndef AVT_LOGICAL_NEGATION_FILTER_H
#define AVT_LOGICAL_NEGATION_FILTER_H

#include <avtUnaryMathFilter.h>

#include <vtkUnsignedCharArray.h>


// ****************************************************************************
//  Class: avtLogicalNegationFilter
//
//  Purpose:
//      A filter that finds the logical negation of two inputs.
//
//  Programmer: Hank Childs
//  Creation:   August 21, 2003
//
// ****************************************************************************

class EXPRESSION_API avtLogicalNegationFilter : public avtUnaryMathFilter
{
  public:
                              avtLogicalNegationFilter() {;};
    virtual                  ~avtLogicalNegationFilter() {;};

    virtual const char       *GetType(void)
                                  { return "avtLogicalNegationFilter"; };
    virtual const char       *GetDescription(void) 
                                  { return "Performing Logical \'Negation\'"; };

  protected:
    virtual void           DoOperation(vtkDataArray *in, vtkDataArray *out,
                                       int ncomps, int ntuples);
    virtual int            GetNumberOfComponentsInOutput() { return 1; };
    virtual vtkDataArray  *CreateArray(vtkDataArray *)
                                { return vtkUnsignedCharArray::New(); };
};


#endif


