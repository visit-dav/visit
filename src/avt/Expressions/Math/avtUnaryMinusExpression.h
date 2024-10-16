// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                            avtUnaryMinusExpression.h                      //
// ************************************************************************* //

#ifndef AVT_UNARYMINUS_FILTER_H
#define AVT_UNARYMINUS_FILTER_H


#include <avtUnaryMathExpression.h>

class     vtkDataArray;


// ****************************************************************************
//  Class: avtUnaryMinusExpression
//
//  Purpose:
//      A filter that calculates the unary minus of its input.
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

class EXPRESSION_API avtUnaryMinusExpression : public avtUnaryMathExpression
{
  public:
                              avtUnaryMinusExpression();
    virtual                  ~avtUnaryMinusExpression();

    virtual const char       *GetType(void)  { return "avtUnaryMinusExpression"; };
    virtual const char       *GetDescription(void) 
                                         { return "Calculating unary minus"; };

  protected:
    virtual void              DoOperation(vtkDataArray *in, vtkDataArray *out,
                                          int ncomponents, int ntuples, vtkDataSet *in_ds);
};


#endif


