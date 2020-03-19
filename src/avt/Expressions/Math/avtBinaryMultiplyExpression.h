// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                          avtBinaryMultiplyExpression.h                        //
// ************************************************************************* //

#ifndef AVT_BINARY_MULTIPLY_FILTER_H
#define AVT_BINARY_MULTIPLY_FILTER_H

#include <avtBinaryMathExpression.h>

class     vtkDataArray;


// ****************************************************************************
//  Class: avtBinaryMultiplyExpression
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
//    Hank Childs, Mon Jan 14 17:58:58 PST 2008
//    Allow constants to be created as singletons.
//
// ****************************************************************************

class EXPRESSION_API avtBinaryMultiplyExpression : public avtBinaryMathExpression
{
  public:
                              avtBinaryMultiplyExpression();
    virtual                  ~avtBinaryMultiplyExpression();

    virtual const char       *GetType(void)
                               { return "avtBinaryMultiplyExpression"; };
    virtual const char       *GetDescription(void)
                               { return "Calculating binary multiplication"; };

  protected:
    virtual void     DoOperation(vtkDataArray *in1, vtkDataArray *in2,
                                 vtkDataArray *out, int ncomps, int ntuples);
    virtual int      GetNumberOfComponentsInOutput(int, int);
    virtual bool     CanHandleSingletonConstants(void) {return true;};
};


#endif


