// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                              avtArcsinExpression.h                            //
// ************************************************************************* //

#ifndef AVT_ARCSIN_FILTER_H
#define AVT_ARCSIN_FILTER_H


#include <avtUnaryMathExpression.h>

class     vtkDataArray;


// ****************************************************************************
//  Class: avtArcsinExpression
//
//  Purpose:
//      A filter that calculates the arcsine of its input.
//
//  Programmer: Sean Ahern
//  Creation:   Thu Jun 27 11:21:41 PDT 2002
//
//  Modifications:
//
//    Hank Childs, Fri Nov 15 15:23:54 PST 2002
//    Added support for vectors and non-float arrays.
//
//    Hank Childs, Thu Feb  5 17:11:06 PST 2004
//    Moved inlined constructor and destructor definitions to .C files
//    because certain compilers have problems with them.
//
// ****************************************************************************

class EXPRESSION_API avtArcsinExpression : public avtUnaryMathExpression
{
  public:
                              avtArcsinExpression();
    virtual                  ~avtArcsinExpression();

    virtual const char       *GetType(void)   { return "avtArcsinExpression"; };
    virtual const char       *GetDescription(void) 
                                             { return "Calculating arcsine"; };

  protected:
    virtual void              DoOperation(vtkDataArray *in, vtkDataArray *out,
                                          int ncomponents, int ntuples, vtkDataSet *in_ds);
};


#endif


