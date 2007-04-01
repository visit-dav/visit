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
//  Modifications:
//
//    Hank Childs, Thu Aug 14 13:40:20 PDT 2003
//    Tell the output that we have one component if we are doing a dot product.
//
//    Hank Childs, Thu Feb  5 17:11:06 PST 2004
//    Moved inlined constructor and destructor definitions to .C files
//    because certain compilers have problems with them.
//
// ****************************************************************************

class EXPRESSION_API avtBinaryMultiplyFilter : public avtBinaryMathFilter
{
  public:
                              avtBinaryMultiplyFilter();
    virtual                  ~avtBinaryMultiplyFilter();

    virtual const char       *GetType(void)
                               { return "avtBinaryMultiplyFilter"; };
    virtual const char       *GetDescription(void)
                               { return "Calculating binary multiplication"; };

  protected:
    virtual void     DoOperation(vtkDataArray *in1, vtkDataArray *in2,
                                 vtkDataArray *out, int ncomps, int ntuples);
    virtual int      GetNumberOfComponentsInOutput(int, int);
};


#endif


