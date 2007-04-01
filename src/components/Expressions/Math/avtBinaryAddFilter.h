// ************************************************************************* //
//                              avtBinaryAddFilter.h                         //
// ************************************************************************* //

#ifndef AVT_BINARY_ADD_FILTER_H
#define AVT_BINARY_ADD_FILTER_H

#include <avtBinaryMathFilter.h>

class     vtkDataArray;


// ****************************************************************************
//  Class: avtBinaryAddFilter
//
//  Purpose:
//      A filter that calculates the sum of its two inputs.
//
//  Programmer: Sean Ahern
//  Creation:   Tue Jun 11 16:23:45 PDT 2002
//
//  Modifications:
//
//    Hank Childs, Thu Feb  5 17:11:06 PST 2004
//    Moved inlined constructor and destructor definitions to .C files
//    because certain compilers have problems with them.
//
// ****************************************************************************

class EXPRESSION_API avtBinaryAddFilter : public avtBinaryMathFilter
{
  public:
                              avtBinaryAddFilter();
    virtual                  ~avtBinaryAddFilter();

    virtual const char       *GetType(void)   { return "avtBinaryAddFilter"; };
    virtual const char       *GetDescription(void)
                                     { return "Calculating binary addition"; };

  protected:
    virtual void     DoOperation(vtkDataArray *in1, vtkDataArray *in2,
                                 vtkDataArray *out, int ncomps, int ntuples);
};


#endif


