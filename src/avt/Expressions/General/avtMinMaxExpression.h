// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                             avtMinMaxExpression.h                         //
// ************************************************************************* //

#ifndef AVT_MINMAX_EXPRESSION_H
#define AVT_MINMAX_EXPRESSION_H

#include <avtMultipleInputMathExpression.h>

class     vtkDataArray;


// ****************************************************************************
//  Class: avtMinMaxExpression
//
//  Purpose:
//      A filter that calculates resrad -- which adjusts the resolution
//      using a monte carlo resampling with a given radius.
//
//  Programmer: Hank Childs
//  Creation:   March 13, 2006
//
//  Modifications:
//
//    Hank Childs, Fri Jun  9 14:26:59 PDT 2006
//    Change return value of SetDoMinimum from bool (mistake) to void (correct)
//
//    Hank Childs, Mon Jan 14 17:58:58 PST 2008
//    Allow constants to be created as singletons.
//
//    Eddie Rusu, Mon Sep 30 15:00:24 PDT 2019
//    Inherit from avtMultiInputMathExpressions to allow taking the min/max
//    over more than 2 variables.
//
// ****************************************************************************

class EXPRESSION_API avtMinMaxExpression
    : public avtMultipleInputMathExpression
{
  public:
             avtMinMaxExpression();
             avtMinMaxExpression(bool);
    virtual ~avtMinMaxExpression();

    virtual const char *GetType(void) { return "avtMinMaxExpression"; };
    virtual const char *GetDescription(void)
                          { return "Calculating min or max"; };

  protected:
    virtual vtkDataArray *DoOperation();
    virtual bool          CanHandleSingletonConstants(void) {return true;};

    bool doMin;
  
  private:
    void DoOperationHelper(vtkDataArray*, vtkDataArray*, vtkDataArray*);
};


#endif


