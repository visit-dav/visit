// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                     avtRelativeDifferenceExpression.h                     //
// ************************************************************************* //

#ifndef AVT_RELATIVE_DIFFERENCE_FILTER_H
#define AVT_RELATIVE_DIFFERENCE_FILTER_H

#include <avtBinaryMathExpression.h>

class     vtkDataArray;


// ****************************************************************************
//  Class: avtRelativeDifferenceExpression
//
//  Purpose:
//      A filter that calculates the relative difference between its two 
//      inputs.
//
//  Programmer: Hank Childs
//  Creation:   December 28, 2004
//
// ****************************************************************************

class EXPRESSION_API avtRelativeDifferenceExpression : public avtBinaryMathExpression
{
  public:
                              avtRelativeDifferenceExpression();
    virtual                  ~avtRelativeDifferenceExpression();

    virtual const char       *GetType(void) 
                                 { return "avtRelativeDifferenceExpression"; };
    virtual const char       *GetDescription(void)
                                 { return "Calculating relative difference"; };

  protected:
    virtual void     DoOperation(vtkDataArray *in1, vtkDataArray *in2,
                                 vtkDataArray *out, int ncomps, int ntuples);
};


#endif


