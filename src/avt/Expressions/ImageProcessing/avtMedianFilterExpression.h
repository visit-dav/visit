// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                        avtMedianFilterExpression.h                        //
// ************************************************************************* //

#ifndef AVT_MEDIAN_FILTER_EXPRESSION_H
#define AVT_MEDIAN_FILTER_EXPRESSION_H

#include <avtUnaryMathExpression.h>

class     vtkDataArray;


// ****************************************************************************
//  Class: avtMedianFilterExpression
//
//  Purpose:
//      A filter that calculates the median of a zone and its surrounding 
//      neighbors.
//
//  Programmer: Hank Childs
//  Creation:   August 14, 2005
// 
//  Modifications:
//    Justin Privitera, Mon Oct 28 10:15:57 PDT 2024
//    Pass in_ds to DoOperation().
//
// ****************************************************************************

class EXPRESSION_API avtMedianFilterExpression : public avtUnaryMathExpression
{
  public:
                              avtMedianFilterExpression();
    virtual                  ~avtMedianFilterExpression();

    virtual const char       *GetType(void) 
                                 { return "avtMedianFilterExpression"; };
    virtual const char       *GetDescription(void)
                                 { return "Taking median filter"; };

  protected:
    bool             haveIssuedWarning;

    virtual void     PreExecute(void);
    virtual void     DoOperation(vtkDataArray *in1, vtkDataArray *out, 
                                 int ncomps, int ntuples, vtkDataSet *in_ds);
};


#endif


