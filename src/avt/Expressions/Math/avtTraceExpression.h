// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                               avtTraceExpression.h                        //
// ************************************************************************* //

#ifndef AVT_TRACE_FILTER_H
#define AVT_TRACE_FILTER_H

#include <avtUnaryMathExpression.h>

class     vtkDataArray;


// ****************************************************************************
//  Class: avtTraceExpression
//
//  Purpose:
//      A filter that calculates trace (sum of diagonals of a tensor).
//
//  Programmer: Hank Childs
//  Creation:   September 19, 2003
//
//  Modifications:
//
//    Hank Childs, Thu Feb  5 17:11:06 PST 2004
//    Moved inlined constructor and destructor definitions to .C files
//    because certain compilers have problems with them.
//
// ****************************************************************************

class EXPRESSION_API avtTraceExpression : public avtUnaryMathExpression
{
  public:
                              avtTraceExpression();
    virtual                  ~avtTraceExpression();

    virtual const char       *GetType(void)   { return "avtTraceExpression"; };
    virtual const char       *GetDescription(void) 
                                              { return "Calculating trace"; };

  protected:
    virtual void              DoOperation(vtkDataArray *in, vtkDataArray *out,
                                          int ncomponents, int ntuples, vtkDataSet *in_ds);
    virtual int               GetNumberOfComponentsInOutput(int)
                                          { return 1; };
};


#endif


