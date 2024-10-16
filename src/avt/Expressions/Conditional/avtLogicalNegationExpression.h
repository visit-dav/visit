// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                      avtLogicalNegationExpression.h                       //
// ************************************************************************* //

#ifndef AVT_LOGICAL_NEGATION_FILTER_H
#define AVT_LOGICAL_NEGATION_FILTER_H

#include <avtUnaryMathExpression.h>

#include <vtkUnsignedCharArray.h>


// ****************************************************************************
//  Class: avtLogicalNegationExpression
//
//  Purpose:
//      A filter that finds the logical negation of two inputs.
//
//  Programmer: Hank Childs
//  Creation:   August 21, 2003
//
//  Modifications:
//
//    Hank Childs, Thu Feb  5 17:11:06 PST 2004
//    Moved inlined constructor and destructor definitions to .C files
//    because certain compilers have problems with them.
//
// ****************************************************************************

class EXPRESSION_API avtLogicalNegationExpression : public avtUnaryMathExpression
{
  public:
                              avtLogicalNegationExpression();
    virtual                  ~avtLogicalNegationExpression();

    virtual const char       *GetType(void)
                                  { return "avtLogicalNegationExpression"; };
    virtual const char       *GetDescription(void) 
                                  { return "Performing Logical \'Negation\'"; };

  protected:
    virtual void           DoOperation(vtkDataArray *in, vtkDataArray *out,
                                       int ncomps, int ntuples, vtkDataSet *in_ds);
    virtual int            GetNumberOfComponentsInOutput(int) { return 1; };
    virtual vtkDataArray  *CreateArray(vtkDataArray *)
                                { return vtkUnsignedCharArray::New(); };
};


#endif


