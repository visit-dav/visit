// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                             avtTransposeExpression.h                      //
// ************************************************************************* //

#ifndef AVT_TRANSPOSE_FILTER_H
#define AVT_TRANSPOSE_FILTER_H


#include <avtUnaryMathExpression.h>


// ****************************************************************************
//  Class: avtTransposeExpression
//
//  Purpose:
//      Calculates the transpose of a tensor.
//
//  Programmer: Hank Childs
//  Creation:   February 21, 2008
//
// ****************************************************************************

class EXPRESSION_API avtTransposeExpression : public avtUnaryMathExpression
{
  public:
                               avtTransposeExpression();
    virtual                   ~avtTransposeExpression();

    virtual const char       *GetType(void)
                               { return "avtTransposeExpression"; };
    virtual const char       *GetDescription(void)
                               {return "Calculating transpose";};

  protected:
    virtual void              DoOperation(vtkDataArray *in, vtkDataArray *out,
                                          int ncomponents, int ntuples, vtkDataSet *in_ds);
};

#endif


